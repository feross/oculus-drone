/*
 * AR Drone demo
 *
 */
#ifndef _CODEC_H
#define _CODEC_H

#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define TARGET_CPU_ARM 1

// this header contains:
//
//<VLIB/Platform/video_config.h>
//<VLIB/video_codec.h>
//<VLIB/video_controller.h>
//<VLIB/video_gob.h>
//<VLIB/video_huffman.h>
//<VLIB/video_macroblock.h>
//<VLIB/video_mem32.h>
//<VLIB/video_packetizer.h>
//<VLIB/video_picture.h>
//<VLIB/video_picture_defines.h>
//<VLIB/video_quantizer.h>
//<VP_Api/vp_api_picture.h>
//<VP_Os/vp_os_types.h>

#undef INLINE

#ifdef __GNUC__ // The Gnu Compiler Collection
#define _GNU_SOURCE
#define WINAPI
#define INLINE __inline__ __attribute__((always_inline))
#define WEAK __attribute__((weak))
#define NO_INSTRUMENT  __attribute__ ((no_instrument_function))
#endif // __GNUC__

#define C_RESULT        int
#define C_OK            0
#define C_FAIL          -1

#define SUCCESS         (0)
#define FAIL            (!SUCCESS)

#define SUCCEED(a)	(((a) & 0xffff) == SUCCESS)
#define FAILED(a)	(((a) & 0xffff) != SUCCESS)

//typedef float               float32_t;
typedef double              float64_t;

#define CBOOL int

typedef volatile uint8_t    vuint8;
typedef volatile uint16_t   vuint16;
typedef volatile uint32_t   vuint32;
typedef volatile uint64_t   vuint64;

typedef volatile int8_t     vint8;
typedef volatile int16_t    vint16;
typedef volatile int32_t    vint32;
typedef volatile int64_t    vint64;

#define bool_t  int32_t

#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE   0
#endif

#define BDADDR_SIZE   6

#if !defined(__BLUETOOTH_H)
typedef struct _bdaddr_t
{
  uint8_t b[BDADDR_SIZE];
} bdaddr_t;
#endif // !defined(__BLUETOOTH_H)

typedef C_RESULT (*Read)  (void* s, int8_t* buffer, int32_t* size);
typedef C_RESULT (*Write) (void* s, const int8_t* buffer, int32_t* size);

/**
 * Pixel format. Notes:
 *
 * PIX_FMT_RGBA32 is handled in an endian-specific manner. A RGBA
 * color is put together as:
 *  (A << 24) | (R << 16) | (G << 8) | B
 * This is stored as BGRA on little endian CPU architectures and ARGB on
 * big endian CPUs.
 *
 * When the pixel format is palettized RGB (PIX_FMT_PAL8), the palettized
 * image data is stored in AVFrame.data[0]. The palette is transported in
 * AVFrame.data[1] and, is 1024 bytes long (256 4-byte entries) and is
 * formatted the same as in PIX_FMT_RGBA32 described above (i.e., it is
 * also endian-specific). Note also that the individual RGB palette
 * components stored in AVFrame.data[1] should be in the range 0..255.
 * This is important as many custom PAL8 video codecs that were designed
 * to run on the IBM VGA graphics adapter use 6-bit palette components.
 */
enum PixelFormat {
    PIX_FMT_NONE= -1,
    PIX_FMT_YUV420P,   ///< Planar YUV 4:2:0 (1 Cr & Cb sample per 2x2 Y samples)
    PIX_FMT_YUV422,    ///< Packed pixel, Y0 Cb Y1 Cr
    PIX_FMT_RGB24,     ///< Packed pixel, 3 bytes per pixel, RGBRGB...
    PIX_FMT_BGR24,     ///< Packed pixel, 3 bytes per pixel, BGRBGR...
    PIX_FMT_YUV422P,   ///< Planar YUV 4:2:2 (1 Cr & Cb sample per 2x1 Y samples)
    PIX_FMT_YUV444P,   ///< Planar YUV 4:4:4 (1 Cr & Cb sample per 1x1 Y samples)
    PIX_FMT_RGBA32,    ///< Packed pixel, 4 bytes per pixel, BGRABGRA..., stored in cpu endianness
    PIX_FMT_YUV410P,   ///< Planar YUV 4:1:0 (1 Cr & Cb sample per 4x4 Y samples)
    PIX_FMT_YUV411P,   ///< Planar YUV 4:1:1 (1 Cr & Cb sample per 4x1 Y samples)
    PIX_FMT_RGB565,    ///< always stored in cpu endianness
    PIX_FMT_RGB555,    ///< always stored in cpu endianness, most significant bit to 1
    PIX_FMT_GRAY8,
    PIX_FMT_MONOWHITE, ///< 0 is white
    PIX_FMT_MONOBLACK, ///< 0 is black
    PIX_FMT_PAL8,      ///< 8 bit with RGBA palette
    PIX_FMT_YUVJ420P,  ///< Planar YUV 4:2:0 full scale (jpeg)
    PIX_FMT_YUVJ422P,  ///< Planar YUV 4:2:2 full scale (jpeg)
    PIX_FMT_YUVJ444P,  ///< Planar YUV 4:4:4 full scale (jpeg)
    PIX_FMT_XVMC_MPEG2_MC,///< XVideo Motion Acceleration via common packet passing(xvmc_render.h)
    PIX_FMT_XVMC_MPEG2_IDCT,
    PIX_FMT_UYVY422,   ///< Packed pixel, Cb Y0 Cr Y1
    PIX_FMT_UYVY411,   ///< Packed pixel, Cb Y0 Y1 Cr Y2 Y3
    PIX_FMT_NB,
};

#define OVERPAD                 16
#define MB_WIDTH_Y              16
#define MB_HEIGHT_Y             MB_WIDTH_Y
#define MB_WIDTH_C              8
#define MB_HEIGHT_C             MB_WIDTH_C


// SQCIF
#define SQCIF_WIDTH             128
#define SQCIF_HEIGHT            96
#define SQCIF_SIZE              (SQCIF_WIDTH * SQCIF_HEIGHT)

// QCIF
#define QCIF_WIDTH              176
#define QCIF_HEIGHT             144
#define QCIF_SIZE               (QCIF_WIDTH * QCIF_HEIGHT)

#define QQVGA_WIDTH             160
#define QQVGA_HEIGHT            120
#define QQVGA_SIZE              (QQVGA_WIDTH * QQVGA_HEIGHT)

// QQCIF
#define QQCIF_WIDTH             88
#define QQCIF_HEIGHT            72
#define QQCIF_SIZE              (QQCIF_WIDTH * QQCIF_HEIGHT)

// QQVGA
#define QQVGA_WIDTH             160
#define QQVGA_HEIGHT            120
#define QQVGA_SIZE              (QQVGA_WIDTH * QQVGA_HEIGHT)

// QVGA
#define QVGA_WIDTH              320
#define QVGA_HEIGHT             240
#define QVGA_SIZE               (QVGA_WIDTH * QVGA_HEIGHT)

// TWEAKY QQVGA
#define TWEAKY_QQVGA_WIDTH      320
#define TWEAKY_QQVGA_HEIGHT     (240-16)
#define TWEAKY_QQVGA_SIZE       (TWEAKY_QQVGA_WIDTH * TWEAKY_QQVGA_HEIGHT)

// CIF
#define CIF_WIDTH               352
#define CIF_HEIGHT              288
#define CIF_SIZE                (CIF_WIDTH * CIF_HEIGHT)

// VGA
#define VGA_WIDTH               640
#define VGA_HEIGHT              480
#define VGA_SIZE                (VGA_WIDTH * VGA_HEIGHT)


typedef struct _vp_api_picture_
{
  enum PixelFormat format;    // camif -> encoder : PIX_FMT_YUV420P

  uint32_t         width;     // camif -> encoder
  uint32_t         height;    // camif -> encoder
  uint32_t         framerate; // camif -> encoder

  uint8_t         *y_buf;     // camif -> encoder
  uint8_t         *cb_buf;    // camif -> encoder
  uint8_t         *cr_buf;    // camif -> encoder

  uint32_t         y_pad;     // 2* camif_config.y_pad
  uint32_t         c_pad;     // 2* camif_config.c_pad

  uint32_t         y_line_size;
  uint32_t         cb_line_size;
  uint32_t         cr_line_size;

  uint32_t         vision_complete;
  uint32_t         complete;
  int32_t          blockline;
}
vp_api_picture_t;

#define MCU_HEIGHT      8
#define MCU_WIDTH       8
#define MCU_BLOCK_SIZE  (MCU_HEIGHT * MCU_WIDTH)

// Offsets in video_picture_context structure
#define VIDEO_PICTURE_CONTEXT_Y_SRC      0
#define VIDEO_PICTURE_CONTEXT_CB_SRC     4
#define VIDEO_PICTURE_CONTEXT_CR_SRC     8
#define VIDEO_PICTURE_CONTEXT_Y_WOFFSET 12
#define VIDEO_PICTURE_CONTEXT_C_WOFFSET 16
#define VIDEO_PICTURE_CONTEXT_Y_HOFFSET 20

typedef struct _video_picture_context_t {
  uint8_t*  y_src;
  uint8_t*  cb_src;
  uint8_t*  cr_src;

  uint32_t  y_woffset; // = picture->y_line_size (in bytes)
  uint32_t  c_woffset; // = picture->cb_line_size (in bytes)
  uint32_t  y_hoffset; // = picture->y_line_size * MCU_HEIGHT (in bytes)

} video_picture_context_t;

// Transform picture in macro blocks
C_RESULT video_blockline_to_macro_blocks(video_picture_context_t* ctx, int16_t* macro_blocks, int32_t num_macro_blocks);

// Transform macro blocks in picture
C_RESULT video_blockline_from_macro_blocks(video_picture_context_t* ctx, int16_t* macro_blocks, int32_t num_macro_blocks, enum PixelFormat format);

// Default zigzag ordering matrix
extern int32_t video_zztable_t81[MCU_BLOCK_SIZE];

typedef struct _video_macroblock_t {
  int32_t   azq;  // All zero coefficients
  int32_t   dquant;
  int32_t   num_coeff_y0; // Number of non-zeros coefficients for block y0
  int32_t   num_coeff_y1; // Number of non-zeros coefficients for block y1
  int32_t   num_coeff_y2; // Number of non-zeros coefficients for block y2
  int32_t   num_coeff_y3; // Number of non-zeros coefficients for block y3
  int32_t   num_coeff_cb; // Number of non-zeros coefficients for block cb
  int32_t   num_coeff_cr; // Number of non-zeros coefficients for block cr
  int16_t*  data;         // macroblock's data
} video_macroblock_t;

//
// Description of a group of block compatible with the h263 standard
//  macroblocks is an array containing all the macroblocks of a blockline
//  quant is the default quantization value for the blockline
//
typedef struct _video_gob_t {
  video_macroblock_t* macroblocks;
  int32_t quant;
} video_gob_t;

#define VLIB_DEFAULT_BITRATE          (100) /* In kb/s */

enum {
  VIDEO_ENCODE  = 1,
  VIDEO_DECODE  = 2
};

enum {
  VIDEO_PICTURE_INTRA = 0,  // Picture is a reference frame
  VIDEO_PICTURE_INTER = 1,  // Picture is encoded using motion estimation / compensation
  VIDEO_PICTURE_PB    = 2,  // Picture is encoded using a PB frame
  VIDEO_PICTURE_B     = 3,
  VIDEO_PICTURE_EI    = 4,
  VIDEO_PICTURE_EP    = 5,
};

enum {
  VIDEO_STREAM_LITTLE_ENDIAN  = 1,
  VIDEO_STREAM_BIG_ENDIAN     = 2
};

typedef struct _video_controller_t  video_controller_t;
typedef struct _video_codec_t       video_codec_t;
typedef struct _video_stream_t      video_stream_t;

struct _video_stream_t {
  int32_t   length;     // Number of bits used in code (TODO why is it signed?)
  uint32_t  code;       // Currently read/write data
  uint32_t  used;       // Number of bytes used in stream
  uint32_t* bytes;      // Must be aligned on a 4-bytes boundary
  uint32_t  index;      // Position of next dword available for reading/writing
  uint32_t  size;       // Max size (in bytes, times of 4) of this stream
  uint32_t  endianess;  // Endianess of the stream
};

typedef C_RESULT (*encode_blockline_fc)( video_controller_t* controller, const vp_api_picture_t* blockline, bool_t picture_complete );
typedef C_RESULT (*decode_blockline_fc)( video_controller_t* controller, vp_api_picture_t* blockline, bool_t* got_image );
typedef C_RESULT (*update_fc)( video_controller_t* controller );
typedef C_RESULT (*cache_stream_fc)( video_controller_t* controller, video_stream_t* in );

struct _video_controller_t {
  // Configuration Data
  uint32_t        mode;           // encoding or decoding
  bool_t          use_me;         // use motion estimation / compensation
  bool_t          do_azq;
  int32_t         aq, bq;
  uint32_t        target_bitrate; // Target bitrate in bit/s

  // External & internal buffer used by packetizer layer
  // video_stream_t* ex_stream;      // External buffer
  video_stream_t  in_stream;      // Internal buffer

  // Internal statistics
  uint32_t  num_frames;           // Frame index
  int32_t   current_bits;         // Number of bits in the buffer
  int32_t   output_bits;          // Number of bits occupied by the previous encoded picture
  int32_t   original_framerate;   // Frame rate of the original video sequence in pictures per second
  int32_t   target_framerate;     // Target frame rate in pictures per second (original_framerate / target_framerate must be an int)

  // Video Data for currently processed picture
  uint32_t  picture_type;
  int32_t   width;                // Size of picture currently decoded
  int32_t   height;
  int32_t   num_blockline;        // Number of blocklines per picture
  int32_t   mb_blockline;         // Number of macroblocks per blockline for this picture
  int32_t   blockline;            // Current blockline in picture
  bool_t    picture_complete;     // tells if picture is complete

  int32_t   quant;
  int32_t   dquant;
  int32_t   Qp;
  int32_t   invQp;

  video_gob_t*  gobs;             // Description of the picture as an array of gob
  int16_t*      cache;            // Cache that holds data for the whole picture (used internally by gobs)

  video_macroblock_t* cache_mbs;  // Array of macroblocks describing blockline_cache (used for decoding)
  int16_t*      blockline_cache;  // Cache used to hold intermediate results (for hardware DCT for example)

  // Codec specific functions
  uint32_t        codec_type;
  video_codec_t*  video_codec;
};

C_RESULT video_controller_update( video_controller_t* controller, bool_t complete );

C_RESULT video_controller_set_mode( video_controller_t* controller, uint32_t mode );

C_RESULT video_controller_cleanup( video_controller_t* controller );

// Configuration api
// video_controller_set_bitrate allows you to set the target bitrate
C_RESULT video_controller_set_bitrate( video_controller_t* controller, uint32_t target );

// Set format for picture to be decoded
// This function resize internal buffers if needed
C_RESULT video_controller_set_format( video_controller_t* controller, int32_t width, int32_t height );

// Set picture type ( INTRA or INTER )
C_RESULT  video_controller_set_picture_type( video_controller_t* controller, uint32_t type );

// Set motion estimation usage
C_RESULT  video_controller_set_motion_estimation( video_controller_t* controller, bool_t use_me );

static INLINE uint8_t* video_controller_get_stream_ptr( video_controller_t* controller ) {
  return (uint8_t*)&controller->in_stream.bytes[0];
}

static INLINE uint32_t video_controller_get_stream_size( video_controller_t* controller ) {
  return controller->in_stream.used;
}
struct _video_codec_t {
  encode_blockline_fc encode_blockline;
  decode_blockline_fc decode_blockline;
  update_fc           update;
  cache_stream_fc     cache_stream;
};

/******** Available codecs ********/
typedef enum _codec_type_t {
  NULL_CODEC    = 0,
  UVLC_CODEC,
  MJPEG_CODEC,
  P263_CODEC
} codec_type_t;

/******** API ********/
C_RESULT video_codec_open( video_controller_t* controller, codec_type_t codec_type );
C_RESULT video_codec_close( video_controller_t* controller );

C_RESULT video_decode_picture( video_controller_t* controller, vp_api_picture_t* picture, video_stream_t* ex_stream, bool_t* got_image );

// Encode/Decode a blockline
static INLINE C_RESULT video_encode_blockline( video_controller_t* controller, const vp_api_picture_t* blockline, bool_t picture_complete )
{
  return controller->video_codec->encode_blockline( controller, blockline, picture_complete );
}

static INLINE C_RESULT video_decode_blockline( video_controller_t* controller, vp_api_picture_t* blockline, bool_t* got_image )
{
  return controller->video_codec->decode_blockline( controller, blockline, got_image );
}

/* Default configuration for ARM11 (like iphone) platform */

#define DEFAULT_QUANTIZATION          (6)
#define MAX_NUM_MACRO_BLOCKS_PER_CALL (1)
#define DEFAULT_INTERNAL_STREAM_SIZE  (1024 * 8 * 2)
#define VLIB_ALLOC_ALIGN              (16)

#define DCT_BUFFER_SIZE ( MAX_NUM_MACRO_BLOCKS_PER_CALL * 6 * MCU_BLOCK_SIZE )

#define NUM_MAX_DCT_BLOCKS  64U  /* Max number of blocks per dct calls */

// Default implementation for dct computation
void fdct(const unsigned short* in, short* out);
void idct(const short* in, unsigned short* out);

int16_t* video_fdct_compute(int16_t* in, int16_t* out, int32_t num_macro_blocks);
int16_t* video_idct_compute(int16_t* in, int16_t* out, int32_t num_macro_blocks);

# pragma pack (1)

typedef struct _huffman_code_t {
  int32_t index;
  union {
    struct {
      uint8_t   length;
      uint32_t  vlc:24;
    };
    int32_t code;
  };
} huffman_code_t;

# pragma pack () // resets packsize to default value

typedef struct _huffman_tree_data_t {
  huffman_code_t* code;
  int32_t         weight;
} huffman_tree_data_t;

typedef struct _huffman_tree_t {
  int32_t num_used_codes;
  int32_t num_max_codes;
  int32_t max_code_length;

  huffman_tree_data_t data[];
} huffman_tree_t;

huffman_tree_t* huffman_alloc( int32_t num_max_codes, int32_t max_code_length );
void huffman_free( huffman_tree_t* tree );

C_RESULT huffman_add_codes( huffman_tree_t* tree, huffman_code_t* codes, int32_t num_codes );
C_RESULT huffman_sort_codes( huffman_tree_t* tree );

C_RESULT huffman_check_code( huffman_tree_t* tree, uint32_t code, uint32_t length );
int32_t  huffman_stream_code( huffman_tree_t* tree, video_stream_t* stream );

C_RESULT video_zeromem32( uint32_t* dst, uint32_t length );

C_RESULT video_copy32(uint32_t* dst, uint32_t* src, uint32_t nb);
C_RESULT video_copy32_swap(uint32_t* dst, uint32_t* src, uint32_t nb);

C_RESULT video_packetizer_init( video_controller_t* controller );
C_RESULT video_packetizer_close( video_controller_t* controller );


/// Write to a stream

// Fills a stream with length bits from code
void     video_write_data( video_stream_t* const stream, uint32_t code, int32_t length );

// Updates stream as its length attributes is a multiple of 8
// Updates is done by adding bits
C_RESULT video_stuff8( video_stream_t* const stream );


/// Read from a stream

// Takes length bit from stream and updates it
C_RESULT video_read_data( video_stream_t* const stream, uint32_t* code, int32_t length );

// Takes length bit from stream without updating it
C_RESULT video_peek_data( const video_stream_t* const stream, uint32_t* code, int32_t length );

// Updates stream as its length attributes is a multiple of 8
// Updates is done by skipping bits
C_RESULT video_align8( video_stream_t* const stream );


/// Copy a stream

// Flush content of stream in into stream out
C_RESULT video_cache_stream( video_controller_t* controller, video_stream_t* in );
// (quant > 0)&&(quant < 31) => use constant quantization
// quant == 31 				 => use quantization table
#define TABLE_QUANTIZATION 31

// P6 DCT quantization table = 2^15/iquant_tab
/*static const int16_t quant_tab[64]  ={	8192, 4681, 3277, 2521, 2048, 1725, 1489, 1311,
										4681, 3277, 2521, 2048, 1725, 1489, 1311, 1170,
										3277, 2521, 2048, 1725, 1489, 1311, 1170, 1057,
										2521, 2048, 1725, 1489, 1311, 1170, 1057,  964,
										2048, 1725, 1489, 1311, 1170, 1057,  964,  886,
										1725, 1489, 1311, 1170, 1057,  964,  886,  819,
										1489, 1311, 1170, 1057,  964,  886,  819,  762,
										1311, 1170, 1057,  964,  886,  819,  762,  712
									  };*/

static const int16_t quant_tab[64]  ={  10923, 6554, 4681, 3641, 2979, 2521, 2185, 1928,
										 6554, 4681, 3641, 2979, 2521, 2185, 1928, 1725,
										 4681, 3641, 2979, 2521, 2185, 1928, 1725, 1560,
										 3641, 2979, 2521, 2185, 1928, 1725, 1560, 1425,
										 2979, 2521, 2185, 1928, 1725, 1560, 1425, 1311,
										 2521, 2185, 1928, 1725, 1560, 1425, 1311, 1214,
										 2185, 1928, 1725, 1560, 1425, 1311, 1214, 1130,
										 1928, 1725, 1560, 1425, 1311, 1214, 1130, 1057
									 };
// inverse quantization table
/*static const int16_t iquant_tab[64]  ={	 4,  7, 10, 13, 16, 19, 22, 25,
										 7, 10, 13, 16, 19, 22, 25, 28,
										10, 13, 16, 19, 22, 25, 28, 31,
										13, 16, 19, 22, 25, 28, 31, 34,
										16, 19, 22, 25, 28, 31, 34, 37,
										19, 22, 25, 28, 31, 34, 37, 40,
										22, 25, 28, 31, 34, 37, 40, 43,
										25, 28, 31, 34, 37, 40, 43, 46
									  };*/

static const int16_t iquant_tab[64]  ={  3,  5,  7,  9, 11, 13, 15, 17,
										 5,  7,  9, 11, 13, 15, 17, 19,
										 7,  9, 11, 13, 15, 17, 19, 21,
										 9, 11, 13, 15, 17, 19, 21, 23,
										11, 13, 15, 17, 19, 21, 23, 25,
										13, 15, 17, 19, 21, 23, 25, 27,
										15, 17, 19, 21, 23, 25, 27, 29,
										17, 19, 21, 23, 25, 27, 29, 31
									  };

// Utility functions
int16_t* do_quantize_intra_mb(int16_t* ptr, int32_t invQuant, int32_t* last_ptr);
int16_t* do_quantize_inter_mb(int16_t* ptr, int32_t quant, int32_t invQuant, int32_t* last_ptr);
C_RESULT do_unquantize(int16_t* ptr, int32_t picture_type, int32_t quant, int32_t num_coeff);

// Default quantization scheme
C_RESULT video_quantizer_init( video_controller_t* controller );
C_RESULT video_quantizer_update( video_controller_t* controller );
C_RESULT video_quantize( video_controller_t* controller, video_macroblock_t* macroblock, int32_t num_macro_blocks );
C_RESULT video_unquantize( video_controller_t* controller, video_macroblock_t* macroblock, int32_t num_macro_blocks );

void     uvlc_encode( video_stream_t* const stream, int32_t level, int32_t run, int32_t not_last );
C_RESULT uvlc_decode( video_stream_t* const stream, int32_t* run, int32_t* level, int32_t* last);

#define MAKE_START_CODE(gob)      ( 0x000020 | (gob) )
#define PICTURE_START_CODE        MAKE_START_CODE(0)
#define PICTURE_END_CODE          MAKE_START_CODE(0x1F)

#define UVLC_FORMAT_CIF   1
#define UVLC_FORMAT_VGA   2

#define UVLC_RESOLUTION_SUBQ  1 /* sub-QCIF */
#define UVLC_RESOLUTION_Q     2 /* QCIF     */
#define UVLC_RESOLUTION_1     3 /* CIF      */
#define UVLC_RESOLUTION_4     4 /* 4-CIF    */
#define UVLC_RESOLUTION_16    5 /* 16-CIF   */

typedef struct _uvlc_mb_layer_t {
  uint32_t desc;
  uint32_t dquant;
} uvlc_mb_layer_t;

typedef struct _uvlc_gob_layer_t {
  video_macroblock_t* macroblocks;
  uint32_t quant;
} uvlc_gob_layer_t;

typedef struct _uvlc_picture_layer_t {
  uint32_t format;
  uint32_t resolution;
  uint32_t picture_type;
  uint32_t quant;
  uvlc_gob_layer_t* gobs;
} uvlc_picture_layer_t;

C_RESULT uvlc_write_picture_layer( video_controller_t* controller, video_stream_t* stream );
C_RESULT uvlc_read_picture_layer( video_controller_t* controller, video_stream_t* stream );

C_RESULT uvlc_write_gob_layer( video_stream_t* stream, uvlc_gob_layer_t* gob );
C_RESULT uvlc_read_gob_layer( video_stream_t* stream, uvlc_gob_layer_t* gob );

C_RESULT uvlc_write_mb_layer( video_stream_t* stream, video_macroblock_t* mb, int32_t num_macro_blocks );
C_RESULT uvlc_read_mb_layer( video_stream_t* stream, video_macroblock_t* mb, int32_t num_macro_blocks );

typedef struct _uvlc_codec_t {
  // Compatibility with video_codec_t structure
  encode_blockline_fc encode_blockline;
  decode_blockline_fc decode_blockline;
  update_fc           update;
  cache_stream_fc     cache_stream;

  // Private data (see video source coding algorithm p.9)
  uvlc_picture_layer_t  picture_layer;
} uvlc_codec_t;

void uvlc_codec_alloc( video_controller_t* controller );
void uvlc_codec_free( video_controller_t* controller );

C_RESULT uvlc_pack_controller( video_controller_t* controller );
C_RESULT uvlc_unpack_controller( video_controller_t* controller );

C_RESULT uvlc_encode_blockline( video_controller_t* controller, const vp_api_picture_t* blockline, bool_t picture_complete );
C_RESULT uvlc_decode_blockline( video_controller_t* controller, vp_api_picture_t* picture, bool_t* got_image );
C_RESULT uvlc_update( video_controller_t* controller );
C_RESULT uvlc_cache( video_controller_t* controller, video_stream_t* ex_stream);

#define ASSERT assert
# define RTMON_START(cfg)         do{}while(0)
# define RTMON_STOP()             do{}while(0)
# define RTMON_FLUSH(...)         do{}while(0)
# define RTMON_USTART(id)         do{}while(0)
# define RTMON_USTOP(id)          do{}while(0)
# define RTMON_UVAL(id,value)     do{}while(0)

#ifdef __i386
static INLINE uint32_t _byteswap_ulong(uint32_t value)
{
  __asm("bswap %0":
      "=r" (value):
      "0" (value));

  return value;
}
#else
static INLINE uint32_t _byteswap_ulong(uint32_t value)
{
  int32_t tmp;

  __asm __volatile(
    "eor	%1, %2, %2, ror #16\n"
    "bic	%1, %1, #0x00ff0000\n"
    "mov	%0, %2, ror #8\n"
    "eor	%0, %0, %1, lsr #8"
    : "=r" (value), "=r" (tmp)
    : "r" (value)
  );

  return value;
}
#endif

#define clz   __builtin_clz
#define bswap _byteswap_ulong

void*
aligned_malloc(size_t size, size_t align_size);


/**
 * Must be used to free memory allocated by vp_os_aligned_malloc correctly
 */
void
aligned_free(void *ptr);


#endif //_CODEC_H
