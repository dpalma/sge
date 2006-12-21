///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "tech/imageapi.h"
#include "tech/readwriteapi.h"

extern "C"
{
#undef FAR
#define NEED_FAR_POINTERS
#include "jpeglib.h"
#include "jerror.h"
}

#include <csetjmp>

#include "tech/dbgalloc.h" // must be last header

////////////////////////////////////////////////////////////////////////////////

struct my_error_mgr
{
   struct jpeg_error_mgr pub;	// "public" fields
   jmp_buf setjmp_buffer;	// for return to caller
};

typedef struct my_error_mgr * my_error_ptr;

void JpgErrorExit(j_common_ptr cinfo)
{
   // cinfo->err really points to a my_error_mgr struct, so coerce pointer
   my_error_ptr myerr = (my_error_ptr)cinfo->err;

   // Always display the message.
   // We could postpone this until after returning, if we chose.
   (*cinfo->err->output_message)(cinfo);

   // Return control to the setjmp point
   longjmp(myerr->setjmp_buffer, 1);
}


////////////////////////////////////////////////////////////////////////////////

struct JpgReaderSourceMgr
{
   struct jpeg_source_mgr pub;   // public fields
   IReader * pReader;            // source stream
   JOCTET * buffer;              // start of buffer
   boolean start_of_file;        // have we gotten any data yet?
};

typedef struct JpgReaderSourceMgr * my_src_ptr;

#define INPUT_BUF_SIZE 4096

static void JpgReaderInitSource(j_decompress_ptr cinfo)
{
   my_src_ptr src = (my_src_ptr)cinfo->src;

   // Reset the empty-input-file flag for each image, but don't clear the input buffer.
   // This is correct behavior for reading a series of images from one source.
   src->start_of_file = TRUE;
}

static boolean JpgReaderFillInputBuffer(j_decompress_ptr cinfo)
{
   my_src_ptr src = (my_src_ptr)cinfo->src;

   size_t nbytes = 0;
   if (src->pReader->Read(src->buffer, INPUT_BUF_SIZE, &nbytes) != S_OK)
   {
      // TODO: handle error
   }

   if (nbytes <= 0)
   {
      if (src->start_of_file)	// Treat empty input file as fatal error
         ERREXIT(cinfo, JERR_INPUT_EMPTY);
      WARNMS(cinfo, JWRN_JPEG_EOF);
      // Insert a fake EOI marker
      src->buffer[0] = (JOCTET) 0xFF;
      src->buffer[1] = (JOCTET) JPEG_EOI;
      nbytes = 2;
   }

   src->pub.next_input_byte = src->buffer;
   src->pub.bytes_in_buffer = nbytes;
   src->start_of_file = FALSE;

   return TRUE;
}

static void JpgReaderSkipInputData(j_decompress_ptr cinfo, long num_bytes)
{
   my_src_ptr src = (my_src_ptr) cinfo->src;

   /* Just a dumb implementation for now.  Could use fseek() except
   * it doesn't work on pipes.  Not clear that being smart is worth
   * any trouble anyway --- large skips are infrequent.
   */
   if (num_bytes > 0)
   {
      while (num_bytes > (long) src->pub.bytes_in_buffer)
      {
         num_bytes -= (long) src->pub.bytes_in_buffer;
         (void)JpgReaderFillInputBuffer(cinfo);
         // note we assume that fill_input_buffer will never return FALSE,
         // so suspension need not be handled.
      }
      src->pub.next_input_byte += (size_t) num_bytes;
      src->pub.bytes_in_buffer -= (size_t) num_bytes;
   }
}

static void JpgReaderTermSource(j_decompress_ptr cinfo)
{
   my_src_ptr src = (my_src_ptr)cinfo->src;
}

void JpgReaderSrc(j_decompress_ptr cinfo, IReader * pReader)
{
   my_src_ptr src = NULL;

   /* The source object and input buffer are made permanent so that a series
   * of JPEG images can be read from the same file by calling jpeg_stdio_src
   * only before the first one.  (If we discarded the buffer at the end of
   * one image, we'd likely lose the start of the next one.)
   * This makes it unsafe to use this manager and a different source
   * manager serially with the same JPEG object.  Caveat programmer.
   */
   if (cinfo->src == NULL)
   {
      cinfo->src = (struct jpeg_source_mgr *)(struct JpgReaderSourceMgr *)
         (*cinfo->mem->alloc_small)((j_common_ptr)cinfo, JPOOL_PERMANENT, sizeof(struct JpgReaderSourceMgr));

      src = (my_src_ptr)cinfo->src;

      src->buffer = (JOCTET *)(*cinfo->mem->alloc_small)((j_common_ptr)cinfo, JPOOL_PERMANENT, INPUT_BUF_SIZE * sizeof(JOCTET));
   }
   else
   {
      src = (my_src_ptr)cinfo->src;
   }

   src->pub.init_source = JpgReaderInitSource;
   src->pub.fill_input_buffer = JpgReaderFillInputBuffer;
   src->pub.skip_input_data = JpgReaderSkipInputData;
   src->pub.resync_to_restart = jpeg_resync_to_restart; // use default method
   src->pub.term_source = JpgReaderTermSource;
   src->pReader = pReader;
   src->pub.bytes_in_buffer = 0; // forces fill_input_buffer on first read
   src->pub.next_input_byte = NULL; // until buffer loaded
}


////////////////////////////////////////////////////////////////////////////////

void * JpgLoad(IReader * pReader)
{
   Assert(pReader != NULL);
   if (pReader == NULL)
   {
      return NULL;
   }

   // Hold a reference to the IReader for the duration of this function
   cAutoIPtr<IReader> pStabilizeReader(CTAddRef(pReader));

   /* This struct contains the JPEG decompression parameters and pointers to
   * working space (which is allocated as needed by the JPEG library).
   */
   struct jpeg_decompress_struct cinfo;
   /* We use our private extension JPEG error handler.
   * Note that this struct must live as long as the main JPEG parameter
   * struct, to avoid dangling-pointer problems.
   */
   struct my_error_mgr jerr;

   /* More stuff */
   JSAMPARRAY buffer;		/* Output row buffer */
   int row_stride;		/* physical row width in output buffer */

   /* Step 1: allocate and initialize JPEG decompression object */

   /* We set up the normal JPEG error routines, then override error_exit. */
   cinfo.err = jpeg_std_error(&jerr.pub);
   jerr.pub.error_exit = JpgErrorExit;

   /* Establish the setjmp return context for my_error_exit to use. */
   if (setjmp(jerr.setjmp_buffer))
   {
      /* If we get here, the JPEG code has signaled an error.
      * We need to clean up the JPEG object, close the input file, and return.
      */
      jpeg_destroy_decompress(&cinfo);
      return 0;
   }

   /* Now we can initialize the JPEG decompression object. */
   jpeg_create_decompress(&cinfo);

   /* Step 2: specify data source (eg, a file) */

   JpgReaderSrc(&cinfo, pReader);

   /* Step 3: read file parameters with jpeg_read_header() */

   jpeg_read_header(&cinfo, TRUE);
   /* We can ignore the return value from jpeg_read_header since
   *   (a) suspension is not possible with the stdio data source, and
   *   (b) we passed TRUE to reject a tables-only JPEG file as an error.
   * See libjpeg.doc for more info.
   */

   /* Step 4: set parameters for decompression */

   /* In this example, we don't need to change any of the defaults set by
   * jpeg_read_header(), so we do nothing here.
   */

   /* Step 5: Start decompressor */

   (void)jpeg_start_decompress(&cinfo);
   /* We can ignore the return value since suspension is not possible
   * with the stdio data source.
   */

   if (cinfo.out_color_space != JCS_RGB || cinfo.num_components != 3)
   {
      return NULL;
   }

   /* We may need to do some setup of our own at this point before reading
   * the data.  After jpeg_start_decompress() we have the correct scaled
   * output image dimensions available, as well as the output colormap
   * if we asked for color quantization.
   * In this example, we need to make an output work buffer of the right size.
   */ 
   /* JSAMPLEs per row in output buffer */
   row_stride = cinfo.output_width * cinfo.output_components;

   /* Make a one-row-high sample array that will go away when done with image */
   buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);

   byte * pImageBits = new byte[cinfo.output_height * row_stride];
   byte * pImageScanLine = pImageBits;

   /* Step 6: while (scan lines remain to be read) */
   /*           jpeg_read_scanlines(...); */

   /* Here we use the library's state variable cinfo.output_scanline as the
   * loop counter, so that we don't have to keep track ourselves.
   */
   while (cinfo.output_scanline < cinfo.output_height)
   {
      /* jpeg_read_scanlines expects an array of pointers to scanlines.
      * Here the array is only one element long, but you could ask for
      * more than one scanline at a time if that's more convenient.
      */
      (void)jpeg_read_scanlines(&cinfo, buffer, 1);
      /* Assume put_scanline_someplace wants a pointer and sample count. */
      memcpy(pImageScanLine, buffer, row_stride);
      pImageScanLine += row_stride;
   }

   /* Step 7: Finish decompression */

   (void) jpeg_finish_decompress(&cinfo);
   /* We can ignore the return value since suspension is not possible
   * with the stdio data source.
   */

   jpeg_destroy_decompress(&cinfo);

   cAutoIPtr<IImage> pOutImage;
   if (ImageCreate(cinfo.output_width, cinfo.output_height, kPF_RGB888, pImageBits, &pOutImage) == S_OK)
   {
   }

   delete [] pImageBits;

   /* After finish_decompress, we can close the input file.
   * Here we postpone it until after no more JPEG errors are possible,
   * so as to simplify the setjmp error logic above.  (Actually, I don't
   * think that jpeg_destroy can do an error exit, but why assume anything...)
   */

   /* At this point you may want to check to see whether any corrupt-data
   * warnings occurred (test whether jerr.pub.num_warnings is nonzero).
   */

   return CTAddRef(pOutImage);
}


////////////////////////////////////////////////////////////////////////////////
