///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_3DS_H
#define INCLUDED_3DS_H

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////

#pragma pack(push,2)

struct s3dsChunkHeader
{
   unsigned short id;
   unsigned long length;
};

#pragma pack(pop)

const int kSizeof3dsChunkHeader = 6; // 3DS chunk headers are 6 bytes

enum e3dsChunkType
{
   // 0xxxH Group

   NULL_CHUNK = 0x0000,
   Unknown_chunk = 0x0001,
   // float ???      
   M3D_VERSION = 0x0002,
   // short version;
   M3D_KFVERSION     = 0x0005,
   COLOR_F= 0x0010,
   // float red, grn, blu;    
   COLOR_24 = 0x0011,
   // char red, grn, blu;     
   LIN_COLOR_24   = 0x0012,
   // char red, grn, blu;
   LIN_COLOR_F    = 0x0013,
   // float red, grn, blu;
   INT_PERCENTAGE    = 0x0030,
   // short percentage;
   FLOAT_PERCENTAGE     = 0x0031,
   // float percentage;
   MASTER_SCALE   = 0x0100,
   // float scale;   
   ChunkType   = 0x0995,
   ChunkUnique    = 0x0996,
   NotChunk    = 0x0997,
   Container   = 0x0998,
   IsChunk     = 0x0999,
   C_SXP_SELFI_MASKDATA    = 0x0c3c,

   // 1xxxH Group

   BIT_MAP= 0x1100,
   // cstr filename;    
   USE_BIT_MAP    = 0x1101,
   SOLID_BGND= 0x1200,
   // followed by color_f
   USE_SOLID_BGND    = 0x1201,
   V_GRADIENT= 0x1300,
   // followed by three color_f: start, mid, end
   // float midpoint;   
   USE_V_GRADIENT    = 0x1301,
   LO_SHADOW_BIAS= 0x1400,
   // float bias;    
   HI_SHADOW_BIAS    = 0x1410,
   SHADOW_MAP_SIZE= 0x1420,
   // short size;    
   SHADOW_SAMPLES    = 0x1430,
   SHADOW_RANGE   = 0x1440,
   SHADOW_FILTER= 0x1450,
   // float filter;     
   RAY_BIAS = 0x1460,
   // float bias;    
   O_CONSTS= 0x1500,
   // float plane_x, plane_y, plane_z;    

   // 2xxxH Group

   AMBIENT_LIGHT     = 0x2100,
   FOG= 0x2200,
   // followed by color_f, fog_bgnd
   // float near_plane, near_density;
   // float far_plane, far_density;    
   USE_FOG     = 0x2201,
   FOG_BGND    = 0x2210,
   DISTANCE_CUE= 0x2300,
   // followed by dcue_bgnd
   // float near_plane, near_density;
   // float far_plane, far_density;    
   USE_DISTANCE_CUE     = 0x2301,
   LAYER_FOG= 0x2302,
   // float fog_z_from, fog_z_to;
   // float fog_density;
   // short fog_type;   
   USE_LAYER_FOG     = 0x2303,
   DCUE_BGND   = 0x2310,
   SMAGIC   = 0x2d2d,
   LMAGIC   = 0x2d3d,

   // 3xxxH Group

   DEFAULT_VIEW   = 0x3000,
   VIEW_TOP    = 0x3010,
   // float targe_x, target_y, target_z;
   // float view_width;    
   VIEW_BOTTOM    = 0x3020,
   // float targe_x, target_y, target_z;
   // float view_width;    
   VIEW_LEFT   = 0x3030,
   // float targe_x, target_y, target_z;
   // float view_width;    
   VIEW_RIGHT     = 0x3040,
   // float targe_x, target_y, target_z;
   // float view_width;    
   VIEW_FRONT= 0x3050,
   // float targe_x, target_y, target_z;
   // float view_width;    
   VIEW_BACK   = 0x3060,
   // float targe_x, target_y, target_z;
   // float view_width;    
   VIEW_USER   = 0x3070,
   // float targe_x, target_y, target_z;
   // float view_width;    
   VIEW_CAMERA= 0x3080,
   // cstr camera_name;    
   VIEW_WINDOW    = 0x3090,
   MDATA= 0x3d3d,
   // Mesh Data Magic Number (.3DS files sub of 4d4d)
   MESH_VERSION   = 0x3d3e,
   MLIBMAGIC= 0x3daa,
   // Material Library Magic Number (.MLI files)
   PRJMAGIC= 0x3dc2,
   // 3dS Project Magic Number (.PRJ files)
   MATMAGIC= 0x3dff,
   // Material File Magic Number (.MAT files)

   // 4xxxH Group

   NAMED_OBJECT   = 0x4000,
   // cstr name;
   OBJ_HIDDEN     = 0x4010,
   OBJ_VIS_LOFTER    = 0x4011,
   OBJ_DOESNT_CAST   = 0x4012,
   OBJ_MATTE   = 0x4013,
   OBJ_FAST    = 0x4014,
   OBJ_PROCEDURAL    = 0x4015,
   OBJ_FROZEN     = 0x4016,
   OBJ_DONT_RCVSHADOW   = 0x4017,
   N_TRI_OBJECT= 0x4100,
   // named triangle object
   //       followed by point_array, point_flag_array, mesh_matrix,
   //       face_array 
   POINT_ARRAY= 0x4110,
   // short npoints;
   // struct {
   //    float x, y, z;
   //    }  points[npoints];     
   POINT_FLAG_ARRAY= 0x4111,
   // short nflags;
   // short flags[nflags];    
   FACE_ARRAY= 0x4120,
   // may be followed by smooth_group
   // short nfaces;
   // struct {
   //    short vertex1, vertex2, vertex3;
   //    short flags;
   //    }  facearray[nfaces];
   MSH_MAT_GROUP= 0x4130,
   // mesh_material_group
   // cstr material_name;
   // short nfaces;
   // short facenum[nfaces];
   OLD_MAT_GROUP = 0x4131,
   TEX_VERTS= 0x4140,
   // short nverts;
   // struct {
   //    float x, y;
   //    }  vertices[nverts];    
   SMOOTH_GROUP = 0x4150,
   // short grouplist[n]   determined by length, seems to be 4 per face    
   MESH_MATRIX = 0x4160,
   // float matrix[4][3];     
   MESH_COLOR = 0x4165,
   // short color_index;   
   MESH_TEXTURE_INFO  = 0x4170,
   //  short map_type;
   // float x_tiling, y_tiling;
   // float icon_x, icon_y, icon_z;
   // float matrix[4][3];
   // float scaling, plan_icon_w, plan_icon_h, cyl_icon_h;
   PROC_NAME    = 0x4181,
   PROC_DATA    = 0x4182,
   MSH_BOXMAP      = 0x4190,
   N_D_L_OLD    = 0x4400,
   N_CAM_OLD    = 0x4500,
   N_DIRECT_LIGHT = 0x4600,
   // followed by color_f
   // float x, y, z;
   DL_SPOTLIGHT = 0x4610,
   // float target_x, target_y, target_z;
   // float hotspot_ang;
   // float falloff_ang;   
   DL_OFF    = 0x4620,
   DL_ATTENUATE    = 0x4625,
   DL_RAYSHAD      = 0x4627,
   DL_SHADOWED     = 0x4630,
   DL_LOCAL_SHADOW    = 0x4640,
   DL_LOCAL_SHADOW2      = 0x4641,
   DL_SEE_CONE     = 0x4650,
   DL_SPOT_RECTANGULAR      = 0x4651,
   DL_SPOT_OVERSHOOT     = 0x4652,
   DL_SPOT_PROJECTOR     = 0x4653,
   DL_EXCLUDE      = 0x4654,
   DL_RANGE     = 0x4655,
   DL_SPOT_ROLL = 0x4656,
   // float roll_ang;   
   DL_SPOT_ASPECT     = 0x4657,
   DL_RAY_BIAS = 0x4658,
   // float bias;    
   DL_INNER_RANGE = 0x4659,
   // float range;   
   DL_OUTER_RANGE  = 0x465a,
   // float range;   
   DL_MULTIPLIER = 0x465b,
   // float multiple;   
   N_AMBIENT_LIGHT    = 0x4680,
   N_CAMERA  = 0x4700,
   // float camera_x, camera_y, camera_z;
   // float target_x, target_y, target_z;
   // float bank_angle;
   // float focus;   
   CAM_SEE_CONE    = 0x4710,
   CAM_RANGES = 0x4720,
   // float near_range, far_range;     
   M3DMAGIC = 0x4d4d,
   // 3DS Magic Number (.3DS file)
   HIERARCHY    = 0x4f00,
   PARENT_OBJECT      = 0x4f10,
   PIVOT_OBJECT    = 0x4f20,
   PIVOT_LIMITS    = 0x4f30,
   PIVOT_ORDER     = 0x4f40,
   XLATE_RANGE     = 0x4f50,

   // 5xxxH Group

   POLY_2D      = 0x5000,
   SHAPE_OK     = 0x5010,
   SHAPE_NOT_OK    = 0x5011,
   SHAPE_HOOK      = 0x5020,

   // 6xxxH Group

   PATH_3D      = 0x6000,
   PATH_MATRIX     = 0x6005,
   SHAPE_2D     = 0x6010,
   M_SCALE      = 0x6020,
   M_TWIST      = 0x6030,
   M_TEETER     = 0x6040,
   M_FIT     = 0x6050,
   M_BEVEL      = 0x6060,
   XZ_CURVE     = 0x6070,
   YZ_CURVE     = 0x6080,
   INTERPCT     = 0x6090,
   DEFORM_LIMIT    = 0x60a0,
   USE_CONTOUR     = 0x6100,
   USE_TWEEN    = 0x6110,
   USE_SCALE    = 0x6120,
   USE_TWIST    = 0x6130,
   USE_TEETER      = 0x6140,
   USE_FIT      = 0x6150,
   USE_BEVEL    = 0x6160,

   // 7xxxH Group

   VIEWPORT_LAYOUT_OLD      = 0x7000,
   VIEWPORT_LAYOUT = 0x7001,
   // followed by viewport_size, viewport_data
   // short form, top, ready, wstate, swapws, swapport, swapcur;
   VIEWPORT_DATA_OLD     = 0x7010,
   VIEWPORT_DATA   = 0x7011,
   // short flags, axis_lockout;
   // short win_x, win_y, win_w, winh_, win_view;
   // float zoom; 
   // float worldcenter_x, worldcenter_y, worldcenter_z;
   // float horiz_ang, vert_ang;
   // cstr camera_name;
   VIEWPORT_DATA_3  = 0x7012,
   // short flags, axis_lockout;
   // short win_x, win_y, win_w, winh_, win_view;
   // float zoom; 
   // float worldcenter_x, worldcenter_y, worldcenter_z;
   // float horiz_ang, vert_ang;
   // cstr camera_name;
   VIEWPORT_SIZE      = 0x7020,
   // short x, y, w, h;
   NETWORK_VIEW    = 0x7030,

   // 8xxxH Group

   XDATA_SECTION      = 0x8000,
   XDATA_ENTRY     = 0x8001,
   XDATA_APPNAME      = 0x8002,
   XDATA_STRING    = 0x8003,
   XDATA_FLOAT     = 0x8004,
   XDATA_DOUBLE    = 0x8005,
   XDATA_SHORT     = 0x8006,
   XDATA_LONG      = 0x8007,
   XDATA_VOID      = 0x8008,
   XDATA_GROUP     = 0x8009,
   XDATA_RFU6      = 0x800a,
   XDATA_RFU5      = 0x800b,
   XDATA_RFU4      = 0x800c,
   XDATA_RFU3      = 0x800d,
   XDATA_RFU2      = 0x800e,
   XDATA_RFU1      = 0x800f,
   PARENT_NAME     = 0x80f0,

   // AxxxH Group

   MAT_NAME = 0xa000,
   // cstr material_name;     
   MAT_AMBIENT = 0xa010,
   // followed by color chunk
   MAT_DIFFUSE = 0xa020,
   // followed by color chunk
   MAT_SPECULAR = 0xa030,
   // followed by color chunk
   MAT_SHININESS = 0xa040,
   // followed by percentage chunk
   MAT_SHIN2PCT = 0xa041,
   // followed by percentage chunk
   MAT_SHIN3PCT = 0xa042,
   // followed by percentage chunk
   MAT_TRANSPARENCY = 0xa050,
   // followed by percentage chunk
   MAT_XPFALL = 0xa052,
   // followed by percentage chunk
   MAT_REFBLUR = 0xa053,
   // followed by percentage chunk
   MAT_SELF_ILLUM     = 0xa080,
   MAT_TWO_SIDE    = 0xa081,
   MAT_DECAL    = 0xa082,
   MAT_ADDITIVE    = 0xa083,
   MAT_SELF_ILPCT = 0xa084,
   // followed by percentage chunk  
   MAT_WIRE     = 0xa085,
   MAT_SUPERSMP    = 0xa086,
   MAT_WIRESIZE = 0xa087,
   // float wire_size;     
   MAT_FACEMAP     = 0xa088,
   MAT_XPFALLIN    = 0xa08a,
   MAT_PHONGSOFT      = 0xa08c,
   MAT_WIREABS     = 0xa08e,
   MAT_SHADING = 0xa100,
   // short shading_value;    
   MAT_TEXMAP = 0xa200,
   // followed by percentage chunk, mat_mapname, 
   //       mat_map_tiling, mat_map_texblur...
   MAT_SPECMAP = 0xa204,
   // followed by percentage_chunk, mat_mapname
   MAT_OPACMAP = 0xa210,
   // followed by percentage_chunk, mat_mapname
   MAT_REFLMAP = 0xa220,
   // followed by percentage_chunk, mat_mapname
   MAT_BUMPMAP = 0xa230,
   // followed by percentage_chunk, mat_mapname
   MAT_USE_XPFALL     = 0xa240,
   MAT_USE_REFBLUR    = 0xa250,
   MAT_BUMP_PERCENT      = 0xa252,
   MAT_MAPNAME  = 0xa300,
   // cstr filename;    
   MAT_ACUBIC      = 0xa310,
   MAT_SXP_TEXT_DATA     = 0xa320,
   MAT_SXP_TEXT2_DATA    = 0xa321,
   MAT_SXP_OPAC_DATA     = 0xa322,
   MAT_SXP_BUMP_DATA     = 0xa324,
   MAT_SXP_SPEC_DATA     = 0xa325,
   MAT_SXP_SHIN_DATA     = 0xa326,
   MAT_SXP_SELFI_DATA    = 0xa328,
   MAT_SXP_TEXT_MASKDATA    = 0xa32a,
   MAT_SXP_TEXT2_MASKDATA      = 0xa32c,
   MAT_SXP_OPAC_MASKDATA    = 0xa32e,
   MAT_SXP_BUMP_MASKDATA    = 0xa330,
   MAT_SXP_SPEC_MASKDATA    = 0xa332,
   MAT_SXP_SHIN_MASKDATA    = 0xa334,
   MAT_SXP_SELFI_MASKDATA      = 0xa336,
   MAT_SXP_REFL_MASKDATA    = 0xa338,
   MAT_TEX2MAP     = 0xa33a,
   MAT_SHINMAP     = 0xa33c,
   MAT_SELFIMAP    = 0xa33d,
   MAT_TEXMASK     = 0xa33e,
   MAT_TEX2MASK    = 0xa340,
   MAT_OPACMASK    = 0xa342,
   MAT_BUMPMASK    = 0xa344,
   MAT_SHINMASK    = 0xa346,
   MAT_SPECMASK    = 0xa348,
   MAT_SELFIMASK      = 0xa34a,
   MAT_REFLMASK    = 0xa34c,
   MAT_MAP_TILINGOLD     = 0xa350,
   MAT_MAP_TILING = 0xa351,
   // short flags;   
   MAT_MAP_TEXBLUR_OLD      = 0xa352,
   MAT_MAP_TEXBLUR = 0xa353,
   // float blurring;   
   MAT_MAP_USCALE     = 0xa354,
   MAT_MAP_VSCALE     = 0xa356,
   MAT_MAP_UOFFSET    = 0xa358,
   MAT_MAP_VOFFSET    = 0xa35a,
   MAT_MAP_ANG     = 0xa35c,
   MAT_MAP_COL1    = 0xa360,
   MAT_MAP_COL2    = 0xa362,
   MAT_MAP_RCOL    = 0xa364,
   MAT_MAP_GCOL    = 0xa366,
   MAT_MAP_BCOL    = 0xa368,
   MAT_ENTRY    = 0xafff,

   // BxxxH Group

   KFDATA = 0xb000,
   // followed by kfhdr
   AMBIENT_NODE_TAG      = 0xb001,
   OBJECT_NODE_TAG = 0xb002,
   // followed by node_hdr, pivot, pos_track_tag, 
   // rot_track_tag, scl_track_tag, morph_smooth...
   CAMERA_NODE_TAG = 0xb003,
   // followed by node_hdr, pos_track_tag, fov_track_tag,
   // roll_track_tag...
   TARGET_NODE_TAG = 0xb004,
   // followed by node_hdr, pos_track_tag...
   LIGHT_NODE_TAG = 0xb005,
   // followed by node_hdr, pos_track_tag, col_track_tag...
   L_TARGET_NODE_TAG = 0xb006,
   // followed by node_id, node_hdr, pos_track_tag
   SPOTLIGHT_NODE_TAG = 0xb007,
   // followed by node_id, node_hdr, pos_track_tag,               hot_track_tag, fall_track_tag, roll_track_tag,
   //       col_track_tag...
   KFSEG   = 0xb008,
   // short start, end; 
   KFCURTIME  = 0xb009,
   // short curframe;   
   KFHDR = 0xb00a,
   //followed by viewport_layout, kfseg, kfcurtime, 
   //       object_node_tag, light_node_tag, target_node_tag,
   //       camera_node_tag, l_target_node_tag, spotlight_node_tag
   //       ambient_node_tag...
   // short revision; 
   // cstr filename; 
   // short animlen;
   NODE_HDR = 0xb010,
   // cstr objname;
   // short flags1;
   // short flags2;     
   // short heirarchy;  ? 
   INSTANCE_NAME      = 0xb011,
   PRESCALE     = 0xb012,
   PIVOT = 0xb013,
   // float pivot_x, pivot_y, pivot_z;    
   BOUNDBOX     = 0xb014,
   MORPH_SMOOTH  = 0xb015,
   // float morph_smoothing_angle_rad;    
   POS_TRACK_TAG = 0xb020,
   // short flags;
   // short unknown[4];
   // short keys;
   // short unknown;
   // struct {
   //    short framenum;
   //    long unknown;
   //    float pos_x, pos_y, pos_z; 
   //    }  pos[keys];  
   ROT_TRACK_TAG      = 0xb021,
   // short flags;
   // short unknown[4];
   // short keys;
   // short unknown;
   // struct {
   //    short framenum;
   //    long unknown;
   //    float rotation_rad;
   //    float axis_x, axis_y, axis_z;  
   //    }  rot[keys];  
   SCL_TRACK_TAG      = 0xb022,
   // short flags;
   // short unknown[4];
   // short keys;
   // short unknown;
   // struct {
   //    short framenum;
   //    long unknown;
   //    float scale_x, scale_y, scale_z; 
   //    }  scale[keys];   
   FOV_TRACK_TAG = 0xb023,
   // short flags;
   // short unknown[4];
   // short keys;
   // short unknown;
   // struct {
   //    short framenum;
   //    long unknown;
   //    float camera_field_of_view;
   //    }  fov[keys]   
   ROLL_TRACK_TAG = 0xb024,
   // short flags;
   // short unknown[4];
   // short keys;
   // short unknown;
   // struct {
   //    short framenum;
   //    long unknown;
   //    float camera_roll;
   //    }  roll[keys];    
   COL_TRACK_TAG      = 0xb025,
   // short flags;
   // short unknown[4];
   // short keys;
   // short unknown;
   // struct {
   //    short framenum;
   //    long unknown;
   //    float red, rgn, blu;
   //    }  color[keys];
   MORPH_TRACK_TAG    = 0xb026,
   // short flags;
   // short unknown[4];
   // short keys;
   // short unknown;
   // struct {
   //    short framenum;
   //    long unknown;
   //    cstr obj_name;
   //    }  morph[keys];
   HOT_TRACK_TAG      = 0xb027,
   // short flags;
   // short unknown[4];
   // short keys;
   // short unknown;
   // struct {
   //    short framenum;
   //    long unknown;
   //    float hotspot_ang;
   //    }  hotspot[keys];
   FALL_TRACK_TAG     = 0xb028,
   // short flags;
   // short unknown[4];
   // short keys;
   // short unknown;
   // struct {
   //    short framenum;
   //    long unknown;
   //    float falloff_ang;
   //    }  falloff[keys];
   HIDE_TRACK_TAG     = 0xb029,
   NODE_ID   = 0xb030,
   // short id;   

   // CxxxH Group

   C_MDRAWER    = 0xc010,
   C_TDRAWER    = 0xc020,
   C_SHPDRAWER     = 0xc030,
   C_MODDRAWER     = 0xc040,
   C_RIPDRAWER     = 0xc050,
   C_TXDRAWER      = 0xc060,
   C_PDRAWER    = 0xc062,
   C_MTLDRAWER     = 0xc064,
   C_FLIDRAWER     = 0xc066,
   C_CUBDRAWER     = 0xc067,
   C_MFILE      = 0xc070,
   C_SHPFILE    = 0xc080,
   C_MODFILE    = 0xc090,
   C_RIPFILE    = 0xc0a0,
   C_TXFILE     = 0xc0b0,
   C_PFILE      = 0xc0b2,
   C_MTLFILE    = 0xc0b4,
   C_FLIFILE    = 0xc0b6,
   C_PALFILE    = 0xc0b8,
   C_TX_STRING     = 0xc0c0,
   C_CONSTS     = 0xc0d0,
   C_SNAPS      = 0xc0e0,
   C_GRIDS      = 0xc0f0,
   C_ASNAPS     = 0xc100,
   C_GRID_RANGE    = 0xc110,
   C_RENDTYPE      = 0xc120,
   C_PROGMODE      = 0xc130,
   C_PREVMODE      = 0xc140,
   C_MODWMODE      = 0xc150,
   C_MODMODEL      = 0xc160,
   C_ALL_LINES     = 0xc170,
   C_BACK_TYPE     = 0xc180,
   C_MD_CS      = 0xc190,
   C_MD_CE      = 0xc1a0,
   C_MD_SML     = 0xc1b0,
   C_MD_SMW     = 0xc1c0,
   C_LOFT_WITH_TEXTURE      = 0xc1c3,
   C_LOFT_L_REPEAT    = 0xc1c4,
   C_LOFT_W_REPEAT    = 0xc1c5,
   C_LOFT_UV_NORMALIZE      = 0xc1c6,
   C_WELD_LOFT     = 0xc1c7,
   C_MD_PDET    = 0xc1d0,
   C_MD_SDET    = 0xc1e0,
   C_RGB_RMODE     = 0xc1f0,
   C_RGB_HIDE      = 0xc200,
   C_RGB_MAPSW     = 0xc202,
   C_RGB_TWOSIDE      = 0xc204,
   C_RGB_SHADOW    = 0xc208,
   C_RGB_AA     = 0xc210,
   C_RGB_OVW    = 0xc220,
   C_RGB_OVH    = 0xc230,
   CMAGIC    = 0xc23d,
   C_RGB_PICTYPE      = 0xc240,
   C_RGB_OUTPUT    = 0xc250,
   C_RGB_TODISK    = 0xc253,
   C_RGB_COMPRESS     = 0xc254,
   C_JPEG_COMPRESSION    = 0xc255,
   C_RGB_DISPDEV      = 0xc256,
   C_RGB_HARDDEV      = 0xc259,
   C_RGB_PATH      = 0xc25a,
   C_BITMAP_DRAWER    = 0xc25b,
   C_RGB_FILE      = 0xc260,
   C_RGB_OVASPECT     = 0xc270,
   C_RGB_ANIMTYPE     = 0xc271,
   C_RENDER_ALL    = 0xc272,
   C_REND_FROM     = 0xc273,
   C_REND_TO    = 0xc274,
   C_REND_NTH      = 0xc275,
   C_PAL_TYPE      = 0xc276,
   C_RND_TURBO     = 0xc277,
   C_RND_MIP    = 0xc278,
   C_BGND_METHOD      = 0xc279,
   C_AUTO_REFLECT     = 0xc27a,
   C_VP_FROM    = 0xc27b,
   C_VP_TO      = 0xc27c,
   C_VP_NTH     = 0xc27d,
   C_REND_TSTEP    = 0xc27e,
   C_VP_TSTEP      = 0xc27f,
   C_SRDIAM     = 0xc280,
   C_SRDEG      = 0xc290,
   C_SRSEG      = 0xc2a0,
   C_SRDIR      = 0xc2b0,
   C_HETOP      = 0xc2c0,
   C_HEBOT      = 0xc2d0,
   C_HEHT    = 0xc2e0,
   C_HETURNS    = 0xc2f0,
   C_HEDEG      = 0xc300,
   C_HESEG      = 0xc310,
   C_HEDIR      = 0xc320,
   C_QUIKSTUFF     = 0xc330,
   C_SEE_LIGHTS    = 0xc340,
   C_SEE_CAMERAS      = 0xc350,
   C_SEE_3D     = 0xc360,
   C_MESHSEL    = 0xc370,
   C_MESHUNSEL     = 0xc380,
   C_POLYSEL    = 0xc390,
   C_POLYUNSEL     = 0xc3a0,
   C_SHPLOCAL      = 0xc3a2,
   C_MSHLOCAL      = 0xc3a4,
   C_NUM_FORMAT    = 0xc3b0,
   C_ARCH_DENOM    = 0xc3c0,
   C_IN_DEVICE     = 0xc3d0,
   C_MSCALE     = 0xc3e0,
   C_COMM_PORT     = 0xc3f0,
   C_TAB_BASES     = 0xc400,
   C_TAB_DIVS      = 0xc410,
   C_MASTER_SCALES    = 0xc420,
   C_SHOW_1STVERT     = 0xc430,
   C_SHAPER_OK     = 0xc440,
   C_LOFTER_OK     = 0xc450,
   C_EDITOR_OK     = 0xc460,
   C_KEYFRAMER_OK     = 0xc470,
   C_PICKSIZE      = 0xc480,
   C_MAPTYPE    = 0xc490,
   C_MAP_DISPLAY      = 0xc4a0,
   C_TILE_XY    = 0xc4b0,
   C_MAP_XYZ    = 0xc4c0,
   C_MAP_SCALE     = 0xc4d0,
   C_MAP_MATRIX_OLD      = 0xc4e0,
   C_MAP_MATRIX    = 0xc4e1,
   C_MAP_WID_HT    = 0xc4f0,
   C_OBNAME     = 0xc500,
   C_CAMNAME    = 0xc510,
   C_LTNAME     = 0xc520,
   C_CUR_MNAME     = 0xc525,
   C_CURMTL_FROM_MESH    = 0xc526,
   C_GET_SHAPE_MAKE_FACES      = 0xc527,
   C_DETAIL     = 0xc530,
   C_VERTMARK      = 0xc540,
   C_MSHAX      = 0xc550,
   C_MSHCP      = 0xc560,
   C_USERAX     = 0xc570,
   C_SHOOK      = 0xc580,
   C_RAX     = 0xc590,
   C_STAPE      = 0xc5a0,
   C_LTAPE      = 0xc5b0,
   C_ETAPE      = 0xc5c0,
   C_KTAPE      = 0xc5c8,
   C_SPHSEGS    = 0xc5d0,
   C_GEOSMOOTH     = 0xc5e0,
   C_HEMISEGS      = 0xc5f0,
   C_PRISMSEGS     = 0xc600,
   C_PRISMSIDES    = 0xc610,
   C_TUBESEGS      = 0xc620,
   C_TUBESIDES     = 0xc630,
   C_TORSEGS    = 0xc640,
   C_TORSIDES      = 0xc650,
   C_CONESIDES     = 0xc660,
   C_CONESEGS      = 0xc661,
   C_NGPARMS    = 0xc670,
   C_PTHLEVEL      = 0xc680,
   C_MSCSYM     = 0xc690,
   C_MFTSYM     = 0xc6a0,
   C_MTTSYM     = 0xc6b0,
   C_SMOOTHING     = 0xc6c0,
   C_MODICOUNT     = 0xc6d0,
   C_FONTSEL    = 0xc6e0,
   C_TESS_TYPE     = 0xc6f0,
   C_TESS_TENSION     = 0xc6f1,
   C_SEG_START     = 0xc700,
   C_SEG_END    = 0xc705,
   C_CURTIME    = 0xc710,
   C_ANIMLENGTH    = 0xc715,
   C_PV_FROM    = 0xc720,
   C_PV_TO      = 0xc725,
   C_PV_DOFNUM     = 0xc730,
   C_PV_RNG     = 0xc735,
   C_PV_NTH     = 0xc740,
   C_PV_TYPE    = 0xc745,
   C_PV_METHOD     = 0xc750,
   C_PV_FPS     = 0xc755,
   C_VTR_FRAMES    = 0xc765,
   C_VTR_HDTL      = 0xc770,
   C_VTR_HD     = 0xc771,
   C_VTR_TL     = 0xc772,
   C_VTR_IN     = 0xc775,
   C_VTR_PK     = 0xc780,
   C_VTR_SH     = 0xc785,
   C_WORK_MTLS     = 0xc790,
   C_WORK_MTLS_2      = 0xc792,
   C_WORK_MTLS_3      = 0xc793,
   C_WORK_MTLS_4      = 0xc794,
   C_BGTYPE     = 0xc7a1,
   C_MEDTILE    = 0xc7b0,
   C_LO_CONTRAST      = 0xc7d0,
   C_HI_CONTRAST      = 0xc7d1,
   C_FROZ_DISPLAY     = 0xc7e0,
   C_BOOLWELD      = 0xc7f0,
   C_BOOLTYPE      = 0xc7f1,
   C_ANG_THRESH    = 0xc900,
   C_SS_THRESH     = 0xc901,
   C_TEXTURE_BLUR_DEFAULT      = 0xc903,
   C_MAPDRAWER     = 0xca00,
   C_MAPDRAWER1    = 0xca01,
   C_MAPDRAWER2    = 0xca02,
   C_MAPDRAWER3    = 0xca03,
   C_MAPDRAWER4    = 0xca04,
   C_MAPDRAWER5    = 0xca05,
   C_MAPDRAWER6    = 0xca06,
   C_MAPDRAWER7    = 0xca07,
   C_MAPDRAWER8    = 0xca08,
   C_MAPDRAWER9    = 0xca09,
   C_MAPDRAWER_ENTRY     = 0xca10,
   C_BACKUP_FILE      = 0xca20,
   C_DITHER_256    = 0xca21,
   C_SAVE_LAST     = 0xca22,
   C_USE_ALPHA     = 0xca23,
   C_TGA_DEPTH     = 0xca24,
   C_REND_FIELDS      = 0xca25,
   C_REFLIP     = 0xca26,
   C_SEL_ITEMTOG      = 0xca27,
   C_SEL_RESET     = 0xca28,
   C_STICKY_KEYINF    = 0xca29,
   C_WELD_THRESHOLD      = 0xca2a,
   C_ZCLIP_POINT      = 0xca2b,
   C_ALPHA_SPLIT      = 0xca2c,
   C_KF_SHOW_BACKFACE    = 0xca30,
   C_OPTIMIZE_LOFT    = 0xca40,
   C_TENS_DEFAULT     = 0xca42,
   C_CONT_DEFAULT     = 0xca44,
   C_BIAS_DEFAULT     = 0xca46,
   C_DXFNAME_SRC      = 0xca50,
   C_AUTO_WELD     = 0xca60,
   C_AUTO_UNIFY       = 0xca70,
   C_AUTO_SMOOTH      = 0xca80,
   C_DXF_SMOOTH_ANG      = 0xca90,
   C_SMOOTH_ANG       = 0xcaa0,
   C_WORK_MTLS_5      = 0xcb00,
   C_WORK_MTLS_6      = 0xcb01,
   C_WORK_MTLS_7      = 0xcb02,
   C_WORK_MTLS_8      = 0xcb03,
   C_WORKMTL    = 0xcb04,
   C_SXP_TEXT_DATA    = 0xcb10,
   C_SXP_OPAC_DATA    = 0xcb11,
   C_SXP_BUMP_DATA    = 0xcb12,
   C_SXP_SHIN_DATA    = 0xcb13,
   C_SXP_TEXT2_DATA      = 0xcb20,
   C_SXP_SPEC_DATA    = 0xcb24,
   C_SXP_SELFI_DATA      = 0xcb28,
   C_SXP_TEXT_MASKDATA      = 0xcb30,
   C_SXP_TEXT2_MASKDATA     = 0xcb32,
   C_SXP_OPAC_MASKDATA      = 0xcb34,
   C_SXP_BUMP_MASKDATA      = 0xcb36,
   C_SXP_SPEC_MASKDATA      = 0xcb38,
   C_SXP_SHIN_MASKDATA      = 0xcb3a,
   C_SXP_REFL_MASKDATA      = 0xcb3e,
   C_NET_USE_VPOST    = 0xcc00,
   C_NET_USE_GAMMA    = 0xcc10,
   C_NET_FIELD_ORDER     = 0xcc20,
   C_BLUR_FRAMES      = 0xcd00,
   C_BLUR_SAMPLES     = 0xcd10,
   C_BLUR_DUR      = 0xcd20,
   C_HOT_METHOD    = 0xcd30,
   C_HOT_CHECK     = 0xcd40,
   C_PIXEL_SIZE    = 0xcd50,
   C_DISP_GAMMA    = 0xcd60,
   C_FBUF_GAMMA    = 0xcd70,
   C_FILE_OUT_GAMMA      = 0xcd80,
   C_FILE_IN_GAMMA    = 0xcd82,
   C_GAMMA_CORRECT    = 0xcd84,
   C_APPLY_DISP_GAMMA    = 0xcd90,
   C_APPLY_FBUF_GAMMA    = 0xcda0,
   C_APPLY_FILE_GAMMA    = 0xcdb0,
   C_FORCE_WIRE    = 0xcdc0,
   C_RAY_SHADOWS      = 0xcdd0,
   C_MASTER_AMBIENT      = 0xcde0,
   C_SUPER_SAMPLE     = 0xcdf0,
   C_OBJECT_MBLUR     = 0xce00,
   C_MBLUR_DITHER     = 0xce10,
   C_DITHER_24     = 0xce20,
   C_SUPER_BLACK      = 0xce30,
   C_SAFE_FRAME    = 0xce40,
   C_VIEW_PRES_RATIO     = 0xce50,
   C_BGND_PRES_RATIO     = 0xce60,
   C_NTH_SERIAL_NUM      = 0xce70,

   // DxxxH Group

   VPDATA    = 0xd000,
   P_QUEUE_ENTRY      = 0xd100,
   P_QUEUE_IMAGE      = 0xd110,
   P_QUEUE_USEIGAMMA     = 0xd114,
   P_QUEUE_PROC    = 0xd120,
   P_QUEUE_SOLID      = 0xd130,
   P_QUEUE_GRADIENT      = 0xd140,
   P_QUEUE_KF      = 0xd150,
   P_QUEUE_MOTBLUR    = 0xd152,
   P_QUEUE_MB_REPEAT     = 0xd153,
   P_QUEUE_NONE    = 0xd160,
   P_QUEUE_RESIZE     = 0xd180,
   P_QUEUE_OFFSET     = 0xd185,
   P_QUEUE_ALIGN      = 0xd190,
   P_CUSTOM_SIZE      = 0xd1a0,
   P_ALPH_NONE     = 0xd210,
   P_ALPH_PSEUDO      = 0xd220,
   P_ALPH_OP_PSEUDO      = 0xd221,
   P_ALPH_BLUR     = 0xd222,
   P_ALPH_PCOL     = 0xd225,
   P_ALPH_C0    = 0xd230,
   P_ALPH_OP_KEY        = 0xd231,
   P_ALPH_KCOL          = 0xd235,
   P_ALPH_OP_NOCONV     = 0xd238,
   P_ALPH_IMAGE         = 0xd240,
   P_ALPH_ALPHA         = 0xd250,
   P_ALPH_QUES          = 0xd260,
   P_ALPH_QUEIMG        = 0xd265,
   P_ALPH_CUTOFF        = 0xd270,
   P_ALPHANEG           = 0xd280,
   P_TRAN_NONE          = 0xd300,
   P_TRAN_IMAGE         = 0xd310,
   P_TRAN_FRAMES        = 0xd312,
   P_TRAN_FADEIN        = 0xd320,
   P_TRAN_FADEOUT       = 0xd330,
   P_TRANNEG            = 0xd340,
   P_RANGES             = 0xd400,
   P_PROC_DATA          = 0xd500,

   // FxxxH Group

   POS_TRACK_TAG_KEY       = 0xf020,
   ROT_TRACK_TAG_KEY       = 0xf021,
   SCL_TRACK_TAG_KEY       = 0xf022,
   FOV_TRACK_TAG_KEY       = 0xf023,
   ROLL_TRACK_TAG_KEY      = 0xf024,
   COL_TRACK_TAG_KEY       = 0xf025,
   MORPH_TRACK_TAG_KEY     = 0xf026,
   HOT_TRACK_TAG_KEY       = 0xf027,
   FALL_TRACK_TAG_KEY      = 0xf028,
   POINT_ARRAY_ENTRY       = 0xf110,
   POINT_FLAG_ARRAY_ENTRY  = 0xf111,
   FACE_ARRAY_ENTRY        = 0xf120,
   MSH_MAT_GROUP_ENTRY     = 0xf130,
   TEX_VERTS_ENTRY         = 0xf140,
   SMOOTH_GROUP_ENTRY      = 0xf150,

   DUMMY                   = 0xffff
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_3DS_H
