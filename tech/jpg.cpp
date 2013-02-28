///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "tech/filespec.h"
#include "tech/imageapi.h"
#include "tech/readwriteapi.h"
#include "tech/readwriteutils.h"

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

struct JpgErrorMgr
{
   struct jpeg_error_mgr pub; // "public" fields
   jmp_buf setjmp_buffer; // for return to caller
};

void JpgErrorExit(j_common_ptr cinfo)
{
   // cinfo->err really points to a my_error_mgr struct, so coerce pointer
   JpgErrorMgr * err = (JpgErrorMgr *)cinfo->err;

   // Always display the message.
   // We could postpone this until after returning, if we chose.
   (*cinfo->err->output_message)(cinfo);

   // Return control to the setjmp point
   longjmp(err->setjmp_buffer, 1);
}


////////////////////////////////////////////////////////////////////////////////

struct JpgReaderSourceMgr
{
   struct jpeg_source_mgr pub;   // public fields
   IReader * pReader;            // source stream
   JOCTET * buffer;              // start of buffer
   boolean start_of_file;        // have we gotten any data yet?
};

#define INPUT_BUF_SIZE 4096

static void JpgReaderInitSource(j_decompress_ptr cinfo)
{
   JpgReaderSourceMgr * src = (JpgReaderSourceMgr *)cinfo->src;

   // Reset the empty-input-file flag for each image, but don't clear the input buffer.
   // This is correct behavior for reading a series of images from one source.
   src->start_of_file = TRUE;
}

static boolean JpgReaderFillInputBuffer(j_decompress_ptr cinfo)
{
   JpgReaderSourceMgr * src = (JpgReaderSourceMgr *)cinfo->src;

   size_t nBytesRead = 0;
   tResult result = src->pReader->Read(src->buffer, INPUT_BUF_SIZE, &nBytesRead);
   if (FAILED(result))
   {
      ERREXIT(cinfo, JERR_FILE_READ);
   }
   else if (result == S_FALSE)
   {
      // Treat empty input file as fatal error
      if (src->start_of_file)
      {
         ERREXIT(cinfo, JERR_INPUT_EMPTY);
         WARNMS(cinfo, JWRN_JPEG_EOF);
      }

      // Insert a fake EOI marker
      src->buffer[0] = static_cast<JOCTET>(0xFF);
      src->buffer[1] = static_cast<JOCTET>(JPEG_EOI);
      nBytesRead = 2;
   }

   src->pub.next_input_byte = src->buffer;
   src->pub.bytes_in_buffer = nBytesRead;
   src->start_of_file = FALSE;

   return TRUE;
}

static void JpgReaderSkipInputData(j_decompress_ptr cinfo, long num_bytes)
{
   JpgReaderSourceMgr * src = (JpgReaderSourceMgr *)cinfo->src;

   if (num_bytes > 0)
   {
      while (num_bytes > (long)src->pub.bytes_in_buffer)
      {
         num_bytes -= src->pub.bytes_in_buffer;
         src->pReader->Seek(num_bytes, kSO_Cur);
      }
      src->pub.next_input_byte += num_bytes;
      src->pub.bytes_in_buffer -= num_bytes;
   }
}

static void JpgReaderTermSource(j_decompress_ptr cinfo)
{
}

void JpgReaderSrc(j_decompress_ptr cinfo, IReader * pReader)
{
   JpgReaderSourceMgr * src = NULL;

   if (cinfo->src == NULL)
   {
      cinfo->src = (jpeg_source_mgr *)(JpgReaderSourceMgr *)
         (*cinfo->mem->alloc_small)((j_common_ptr)cinfo, JPOOL_PERMANENT, sizeof(JpgReaderSourceMgr));

      src = (JpgReaderSourceMgr *)cinfo->src;

      src->buffer = (JOCTET *)(*cinfo->mem->alloc_small)((j_common_ptr)cinfo, JPOOL_PERMANENT, INPUT_BUF_SIZE * sizeof(JOCTET));
   }
   else
   {
      src = (JpgReaderSourceMgr *)cinfo->src;
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

   struct jpeg_decompress_struct cinfo;
   struct JpgErrorMgr jerr;

   // Set up the normal JPEG error routines, but override error_exit
   cinfo.err = jpeg_std_error(&jerr.pub);
   jerr.pub.error_exit = JpgErrorExit;

   // Establish a setjmp return point for the error handler to use
   if (setjmp(jerr.setjmp_buffer))
   {
      // If we get here, an error has occurred.
      jpeg_destroy_decompress(&cinfo);
      return NULL;
   }

   jpeg_create_decompress(&cinfo);

   // Set up the data source
   JpgReaderSrc(&cinfo, pReader);

   // Ignoring the return value because suspension is not supported, and
   // because the TRUE argument rejects tables-only JPEG files as an error
   jpeg_read_header(&cinfo, TRUE);

   // Could change decompression parameters here, but just leave the
   // defaults set by jpeg_read_header

   // Ignoring the return value because suspension is not supported
   jpeg_start_decompress(&cinfo);

   // Only 24-bit RGB supported
   if (cinfo.out_color_space != JCS_RGB || cinfo.num_components != 3)
   {
      return NULL;
   }

   int row_stride = cinfo.output_width * cinfo.output_components;
   JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);

   byte * pImageBits = NULL;
   cAutoBuffer imageBuffer;
   if (imageBuffer.Malloc(cinfo.output_height * row_stride, (void**)&pImageBits) != S_OK)
   {
      jpeg_destroy_decompress(&cinfo);
      return NULL;
   }
   byte * pImageScanLine = pImageBits;

   while (cinfo.output_scanline < cinfo.output_height)
   {
      jpeg_read_scanlines(&cinfo, buffer, 1);
      memcpy(pImageScanLine, buffer[0], row_stride);
      pImageScanLine += row_stride;
   }

   // Ignoring the return value because suspension is not supported
   jpeg_finish_decompress(&cinfo);

   cAutoIPtr<IImage> pOutImage;
   if (ImageCreate(cinfo.output_width, cinfo.output_height, kPF_RGB888, pImageBits, &pOutImage) != S_OK)
   {
      Assert(!pOutImage);
   }

   jpeg_destroy_decompress(&cinfo);

   // TODO: Check jerr.pub.num_warnings

   return CTAddRef(pOutImage);
}


////////////////////////////////////////////////////////////////////////////////
