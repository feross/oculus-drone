/*
 * AR Drone demo
 *
 */

#include "vlib.h"

#define HAS_UVLC_DECODE_BLOCKLINE

extern C_RESULT video_utils_init( video_controller_t* controller );
extern C_RESULT video_utils_close( video_controller_t* controller );

extern void uvlc_codec_alloc( video_controller_t* controller );
extern void uvlc_codec_free( video_controller_t* controller );

extern void p263_codec_alloc( video_controller_t* controller );
extern void p263_codec_free( video_controller_t* controller );


void*
aligned_realloc(void* ptr, size_t size, size_t align_size)
{
  void* ptr_ret;
  void* aligned_ptr;

  if( size == 0 )
  {
    ptr_ret = NULL;
    if( ptr != NULL )
		aligned_free(ptr);
  }
  else
  {
    if( ptr != NULL )
    {
      int* ptr2 = (int*)ptr - 1;
      size_t old_size;

      aligned_ptr = ptr;

      old_size = *ptr2--;

      ptr_ret = aligned_malloc(size, align_size);

      // Compute smallest size
      if( size > old_size )
      {
        size = old_size;
      }

      // Copy old data
      memcpy( ptr_ret, aligned_ptr, size );

      free( ((char*)ptr - *ptr2) );
    }
    else
    {
      ptr_ret = aligned_malloc(size, align_size);
    }
  }

  return ptr_ret;
}


void* aligned_malloc(size_t size, size_t align_size)
{
  char *ptr, *aligned_ptr;
  int* ptr2;
  int allocation_size;
  size_t align_mask = align_size - 1;

  // Check if align_size is a power of two
  // If the result of this test is non zero then align_size is not a power of two
  if( align_size & align_mask )
    return NULL;

  // Allocation size is :
  //    - Requested user size
  //    - a size (align_size) to make sure we can align on the requested boundary
  //    - 8 more bytes to register base adress & allocation size 
  allocation_size = size + align_size + 2*sizeof(int);

  ptr = (char*) malloc(allocation_size);
  if(ptr == NULL)
    return NULL;

  ptr2 = (int*)(ptr + 2*sizeof(int));
  aligned_ptr = ptr + 2*sizeof(int) + (align_size - ((size_t) ptr2 & align_mask));

  ptr2    = (int*)(aligned_ptr - 2*sizeof(int));
  *ptr2++ = (int) (aligned_ptr - ptr);
  *ptr2   = size;

  return aligned_ptr;
}

void aligned_free(void *ptr)
{
  int* ptr2 = (int*)ptr - 2;

  free( ((char*)ptr - *ptr2) );
}


C_RESULT video_codec_open( video_controller_t* controller, codec_type_t codec_type )
{
  C_RESULT res;
  // Data used to initilize macroblock's cache
  int32_t i;
  int16_t* cache;
  video_macroblock_t* mb;

  // Close any previously allocated codec for this controller
  video_codec_close( controller );

  video_utils_init( controller );

  controller->mode            = 0;
  controller->use_me          = FALSE;
  controller->do_azq          = FALSE;
  controller->aq              = 0;
  controller->bq              = 0;
  controller->target_bitrate  = VLIB_DEFAULT_BITRATE;
  controller->num_frames      = 0;
  controller->picture_type    = 0;
  controller->width           = 0;
  controller->height          = 0;
  controller->num_blockline   = 0;
  controller->mb_blockline    = 0;
  controller->blockline       = 0;
  controller->picture_complete= 0;
#ifdef USE_TABLE_QUANTIZATION
  controller->quant           = TABLE_QUANTIZATION;
#else
  controller->quant           = DEFAULT_QUANTIZATION;
#endif
  controller->dquant          = 0;
  controller->Qp              = 0;
  controller->invQp           = 1;
  controller->gobs            = NULL;
  controller->cache           = NULL;
  controller->codec_type      = 0;
  controller->video_codec     = NULL;

  if( controller->blockline_cache == NULL )
  {
    // We alloc two buffers to be compatible with an asynchronous DCT
    // When a DCT will be performed on one buffer, we will be able to use the other for caching or computing purpose
    // DCT_BUFFER_SIZE = MAX_NUM_MACRO_BLOCKS_PER_CALL * 6 * MCU_BLOCK_SIZE
    controller->blockline_cache = (int16_t*)aligned_malloc( 2*DCT_BUFFER_SIZE*sizeof(int16_t), VLIB_ALLOC_ALIGN );
  }

  controller->cache_mbs = malloc( 2 * MAX_NUM_MACRO_BLOCKS_PER_CALL * sizeof(video_macroblock_t) );
  mb = &controller->cache_mbs[0];
  cache = controller->blockline_cache;
  for(i = 2*MAX_NUM_MACRO_BLOCKS_PER_CALL; i > 0; i-- )
  {
    mb->data = cache;
    cache   += MCU_BLOCK_SIZE*6;
    mb ++;
  }

  video_packetizer_init( controller );
  video_quantizer_init( controller );

  switch( codec_type )
  {
    case UVLC_CODEC:
      uvlc_codec_alloc( controller );
      break;

    case P263_CODEC:
        //p263_codec_alloc( controller );
      break;

    default:
      controller->video_codec = NULL;
      break;
  }

  if( controller->video_codec != NULL )
  {
    controller->codec_type = codec_type;
    res = C_OK;
  }
  else
  {
    res = C_FAIL;
  }

  return res;
}

C_RESULT video_codec_close( video_controller_t* controller )
{
  video_utils_close( controller );

  if( controller->blockline_cache != NULL )
    aligned_free( controller->blockline_cache );

  if( controller->in_stream.bytes != NULL )
    video_packetizer_close( controller );

  switch( controller->codec_type )
  {
    case UVLC_CODEC:
      uvlc_codec_free( controller );
      break;

    case P263_CODEC:
      //p263_codec_free( controller );
      break;

    default:
      break;
  }

  // Cleanup caches
  video_controller_cleanup( controller );

  return C_OK;
}

C_RESULT video_encode_picture( video_controller_t* controller, const vp_api_picture_t* picture, bool_t* got_image )
{
  vp_api_picture_t blockline = { 0 };

  controller->mode  = VIDEO_ENCODE;

  video_controller_set_format( controller, picture->width, picture->height );

  blockline                   = *picture;
  blockline.height            = MB_HEIGHT_Y;
  blockline.complete          = 1;
  blockline.vision_complete   = 0;

  // Reset internal stream for new blockline/picture
  controller->in_stream.used  = 0;
  controller->in_stream.index = 0;

  while( !controller->picture_complete )
  {
    video_encode_blockline( controller, &blockline, blockline.blockline == (controller->num_blockline-1) );

    blockline.y_buf  += MB_HEIGHT_Y * picture->y_line_size;
    blockline.cb_buf += MB_HEIGHT_C * picture->cb_line_size;
    blockline.cr_buf += MB_HEIGHT_C * picture->cr_line_size;

    blockline.blockline++;
  }

  if( picture->complete )
  {
    video_write_data( &controller->in_stream, 0, controller->in_stream.length+1 );
    controller->in_stream.length = 32;
    controller->picture_complete = 0;
    *got_image = TRUE;
  }

  return C_OK;
}

C_RESULT video_decode_picture( video_controller_t* controller, vp_api_picture_t* picture, video_stream_t* ex_stream, bool_t* got_image )
{
  vp_api_picture_t blockline = { 0 };

  controller->mode  = VIDEO_DECODE; // mandatory because of video_cache_stream

  blockline                   = *picture;
  blockline.height            = MB_HEIGHT_Y;
  blockline.complete          = 1;
  blockline.vision_complete   = 0;

  while( SUCCEED(video_cache_stream( controller, ex_stream )) )
  {
    video_decode_blockline( controller, &blockline, got_image );
  }

  return C_OK;
}
C_RESULT video_packetizer_init( video_controller_t* controller )
{
  // Internal buffer configuration
  controller->in_stream.bytes     = malloc( DEFAULT_INTERNAL_STREAM_SIZE );
  controller->in_stream.used      = 0;
  controller->in_stream.size      = DEFAULT_INTERNAL_STREAM_SIZE;
  controller->in_stream.index     = 0;
  controller->in_stream.length    = 32;
  controller->in_stream.code      = 0;
  controller->in_stream.endianess = VIDEO_STREAM_LITTLE_ENDIAN;

  return C_OK;
}

C_RESULT video_packetizer_close( video_controller_t* controller )
{
  free( controller->in_stream.bytes );

  controller->in_stream.bytes   = NULL;
  controller->in_stream.used    = 0;
  controller->in_stream.size    = 0;
  controller->in_stream.index   = 0;
  controller->in_stream.length  = 0;
  controller->in_stream.code    = 0;

  return C_OK;
}

C_RESULT video_cache_stream( video_controller_t* controller, video_stream_t* in )
{
  video_codec_t* video_codec = controller->video_codec;

  return video_codec->cache_stream( controller, in );
}

#ifndef HAS_VIDEO_WRITE_DATA

// Fill stream->code from right to left with data in parameters (code & length)
// New bits are always always inserted at the rigth of stream->code (least significant bits)
// This way old bits are put in most significant bits
//            31  ....   0    (length-1)    ....    0
//  stream <= ------------ <= -----------------------
//            stream->bits            code
void video_write_data( video_stream_t* const stream, uint32_t code, int32_t length )
{
  while( length > stream->length )
  {
    // code's length is bigger than number of our free bits
    // we put as many bits in cache as possible
    stream->code <<= stream->length;
    stream->code  |= code >> (length - stream->length);

    length -= stream->length;  // Compute number of bits left
    code   &= (1 << length) - 1; // We keep only bits we didn't push in cache

    stream->bytes[stream->index] = stream->code;
    stream->index++;
    stream->used += 4;

    stream->code    = 0;
    stream->length  = 32;
  }

  if( length > 0 )
  {
    // In this case, previous loop ended with case length < stream->length
    stream->code   <<= length;
    stream->code    |= code;

    stream->length -= length;
  }
}

#endif

C_RESULT video_stuff8( video_stream_t* const stream )
{
  uint32_t length8;

  length8  = (stream->length & ~7); // TODO: Check if generated code use bic on arm

  stream->code    <<= ( stream->length - length8 );
  stream->length    = length8;

  return C_OK;
}

// Fill code from right to left with length bits from stream->code
// Next bits in stream->code to take are always at the left (most significant bits)
// This way new bits are put in least significant bits
//  (length-1)    ....    0    31  ....   0
//  ----------------------- <= ------------ <= stream
//         code                stream->bits 
C_RESULT video_read_data( video_stream_t* const stream, uint32_t* code, int32_t length )
{
  uint32_t out_code = *code;

  while( length > (32 - stream->length) )
  {
    /// We need more bits than available in current read bits

    out_code = (out_code << (32 - stream->length)) | (stream->code >> stream->length);
    length  -= (32 - stream->length);

    stream->code    = stream->bytes[stream->index];
    stream->length  = 0;
    stream->index++;
  }

  if( length > 0 )
  {
    out_code  = (out_code << length) | (stream->code >> ( 32 - length ));

    stream->code  <<= length;
    stream->length += length;
  }

  *code = out_code;

  return C_OK;
}

C_RESULT video_peek_data( const video_stream_t* const stream, uint32_t* code, int32_t length )
{
  uint32_t out_code = *code;
  uint32_t stream_code = stream->code;
  uint32_t stream_length = stream->length;

  while( length > (32 - stream_length) )
  {
    /// We need more bits than available in current read bits

    out_code = (out_code << (32 - stream_length)) | (stream_code >> stream_length);
    length  -= (32 - stream_length);

    stream_code    = stream->bytes[stream->index];
    stream_length  = 0;
  }

  if( length > 0 )
  {
    out_code  = (out_code << length) | (stream_code >> ( 32 - length ));
  }

  *code = out_code;

  return C_OK;
}

C_RESULT video_align8( video_stream_t* const stream )
{
  uint32_t length8, length = stream->length;

  if( length > 0 )
  {
    // Do alignment only when stream->length > 0
    length8  = ( length & ~7); // TODO: Check if generated code use bic on arm
    if( length8 != length )
    {
      length8 += 0x08;
      stream->code    <<= ( length8 - length );
      stream->length    = length8;
    }
  }

  return C_OK;
}

#ifndef HAS_UVLC_ENCODE

#define PACK_BITS( bits_out, length_out, bits_in, length_in ) \
  bits_out  <<= length_in;                                    \
  length_out += length_in;                                    \
  bits_out   |= bits_in;

void uvlc_encode( video_stream_t* const stream, int32_t level, int32_t run, int32_t not_last )
{
  int32_t sign, length, data, value_code, value_length;

  /// Encode number of zeros
  data = run;

  length      = 0;
  value_code  = 1;

  if( data > 0 )
  {
    length = 32 - clz(data);      // compute number of bits used in run ( = length of run )
    data  -= 1 << ( length - 1 ); // compute value of run
  }

  value_length  = length + 1;

  length -= 1;
  if( length > 0 )
  {
    PACK_BITS( value_code, value_length, data, length );
  }

  /// Encode level
  data = level;

  // sign handling
  sign = 0;
  if( data < 0 )
  {
    data = -data;
    sign = 1;
  }

  // TODO Check saturation & if level == -128
  length = 32 - clz(data);  // number of bits used in level ( = length of level )
  if( length > 1 )
  {
    data   -= 1 << (length - 1);
    length += 1;
  }

  PACK_BITS( value_code, value_length, 1, length );

  assert( length != 2 );

  length -= 2;
  if(length > 0)
  {
    PACK_BITS( value_code, value_length, data, length );
  }

  PACK_BITS( value_code, value_length, sign, 1 );

  /// Encode last
  // add sequence for end of block if required
  if( not_last == 0 )
  {
    PACK_BITS( value_code, value_length, 0x5, 3 );
  }

  /// Write output
  video_write_data( stream, value_code, value_length );
}

#endif // HAS_UVLC_ENCODE

C_RESULT uvlc_decode( video_stream_t* const stream, int32_t* run, int32_t* level, int32_t* last)
{
  uint32_t stream_code, stream_length;
  int32_t r = 0, z, sign;

  stream_code = stream_length = 0;

  // Peek 32 bits from stream because we know our datas fit in
  video_peek_data( stream, &stream_code, 32 );


  /// Decode number of zeros
  z = clz(stream_code);

  stream_code  <<= z + 1; // Skip all zeros & 1
  stream_length += z + 1;

  if( z > 1 )
  {
    r = stream_code >> (32 - (z-1));

    stream_code   <<= (z-1);
    stream_length  += (z-1);

    *run = r + (1 << (z-1));
  }
  else
  {
    *run = z;
  }


  /// Decode level / last
  z = clz(stream_code);

  stream_code  <<= z + 1; // Skip all zeros & 1
  stream_length += z + 1;

  if( z == 1 )
  {
    *run  = 0;
    *last = 1;
  }
  else
  {
    if( z == 0 )
    {
      z = 1;
      r = 1;
    }

    stream_length  += z;

    stream_code >>= (32 - z);
    sign = stream_code & 1;

    if( z != 0 )
    {
      r  = stream_code >> 1;
      r += 1 << (z-1);
    }

    *level = sign ? -r : r;
    *last  = 0;
  }

  // Do the real Read in stream to consume what we used
  video_read_data( stream, &stream_code, stream_length );

  return C_OK;
}

const uvlc_codec_t uvlc_codec = {
  uvlc_encode_blockline,
  uvlc_decode_blockline,
  uvlc_update,
  uvlc_cache,
  { 0 }
};

void uvlc_codec_alloc( video_controller_t* controller )
{
  video_codec_t* video_codec;

  video_codec = (video_codec_t*) malloc( sizeof(uvlc_codec) );

  memcpy(video_codec, &uvlc_codec, sizeof(uvlc_codec));

  controller->video_codec = video_codec;
}

void uvlc_codec_free( video_controller_t* controller )
{
  uvlc_codec_t* uvlc_codec = (uvlc_codec_t*) controller->video_codec;

  if( uvlc_codec != NULL )
  {
    free( uvlc_codec );
    controller->video_codec = NULL;
  }
}

static C_RESULT uvlc_flush_stream( video_stream_t* out, video_stream_t* in )
{
  // They are still data in cache
  // Always copy a number of bytes that is a times of 4.
  // Only for the last copy, we can have exactly the number of bytes left
  int32_t offset, size;
  uint32_t out_stream_size;

  if( in->length != 32 )
  {
    // flush & reset internal stream
    video_write_data( in, 0, in->length+1 );
    in->length = 32;
  }

  out_stream_size = out->size & ~3; // Round to the highest times of 4 available

  offset = in->index - (in->used >> 2);
  size = ( in->used < out_stream_size ) ? in->used : out_stream_size;

  memcpy(out->bytes, in->bytes + offset, size);

  out->index  = size >> 2;
  out->used   = size;

  in->used -= size;

  return C_OK;
}

static C_RESULT uvlc_load_stream( video_stream_t* out, video_stream_t* in )
{
  // We cache as many blockline as possible
  C_RESULT res;
  bool_t found, last_zero, last_zero_temp;
  uint8_t *dst, *src;

  int32_t value, nb_bytes;
  uint32_t in_index = (in->used >> 2) - 1;

  // -> start looking for last blockline's end
  found = FALSE;

  if( in->index == 0 ) // First call, we look for full blocklines
  {
    last_zero = FALSE;

    while( (in_index > in->index) && !found )
    {
      value = in->bytes[in_index];

      last_zero_temp = (value & 0xFF) == 0; // 0x??????00
      found = last_zero_temp & last_zero;

      if( !found )
      {
        last_zero = last_zero_temp;
        value >>= 8;

        last_zero_temp = (value & 0xFF) == 0; // 0x????00??
        found = last_zero_temp & last_zero;

        if( !found )
        {
          last_zero = last_zero_temp;
          value >>= 8;

          last_zero_temp = (value & 0xFF) == 0; // 0x??00????
          found = last_zero_temp & last_zero;

          if( !found )
          {
            in_index--; // Handle both the special case where blockline is dword aligned &
                        // blockline start is still not found

            last_zero = last_zero_temp;
            value >>= 8;

            last_zero_temp = (value & 0xFF) == 0; // 0x00??????
            found = last_zero_temp & last_zero;

            if( !found )
            {
              last_zero = last_zero_temp;
            }
          }
        }
      }
    }
  }

  in_index++;

  // configure parameters for memcpy
  if( !found )
  {
    // cache all data
    nb_bytes = in->used - in->index * 4;

    res = C_FAIL;
  }
  else
  {
    // cache only data containing full blocklines
    nb_bytes = (in_index - in->index) * 4;

    res = C_OK;
  }

  // Realloc internal stream to have enough space to hold all required data
  while( out->used + nb_bytes >= out->size )
  {
    out->bytes = realloc( out->bytes, out->size + 2048 ); // Add 2KB to internal stream
    out->size += 2048;
  }

  dst   = (uint8_t*)&out->bytes[0];
  dst  += out->used;

  src   = (uint8_t*)&in->bytes[0];
  src  += in->index*4;

  memcpy( dst, src, nb_bytes );

  out->used += nb_bytes;
  in->index  = in_index;

  ASSERT( out->used <= out->size );

  return res;
}

C_RESULT uvlc_pack_controller( video_controller_t* controller )
{
  video_stream_t* stream = &controller->in_stream;
  uvlc_codec_t* uvlc_codec = (uvlc_codec_t*) controller->video_codec;
  uvlc_picture_layer_t* picture_layer;
  uvlc_gob_layer_t* gob;

  gob = NULL;
  picture_layer = &uvlc_codec->picture_layer;

  video_stuff8( stream );

  picture_layer->gobs = (uvlc_gob_layer_t*) controller->gobs;
  gob = &picture_layer->gobs[controller->blockline];

  video_write_data( stream, MAKE_START_CODE(controller->blockline), 22 );

  if( controller->blockline == 0 )
  {
    picture_layer->quant = gob->quant;
    uvlc_write_picture_layer( controller, stream );
  }
  else
  {
    uvlc_write_gob_layer( stream, gob );
  }

  return C_OK;
}

C_RESULT uvlc_unpack_controller( video_controller_t* controller )
{
  uint32_t start_code = 0;
  video_stream_t* stream = &controller->in_stream;
  uvlc_codec_t* uvlc_codec = (uvlc_codec_t*) controller->video_codec;
  uvlc_picture_layer_t* picture_layer;
  uvlc_gob_layer_t* gob;

  gob = NULL;
  picture_layer = &uvlc_codec->picture_layer;

  video_align8( stream );
  video_read_data( stream, &start_code, 22 );

  controller->blockline = start_code & 0x1F;
  start_code &= ~0x1F; // TODO Check if compiler use arm instruction bic

  ASSERT( controller->blockline == 0x1F ||
                controller->num_blockline == 0 || // Check if cache is allocated for current picture
                (controller->num_blockline > 0 && controller->blockline < controller->num_blockline) );

  if( start_code == PICTURE_START_CODE )
  {
    if( controller->blockline == 0x1F )
    {
      controller->picture_complete = TRUE;
    }
    else
    {
      if( controller->blockline == 0 )
      {
        uvlc_read_picture_layer( controller, stream );

        picture_layer->gobs = (uvlc_gob_layer_t*) controller->gobs;
        gob = &picture_layer->gobs[controller->blockline];

        gob->quant = picture_layer->quant;
      }
      else
      {
        picture_layer->gobs = (uvlc_gob_layer_t*) controller->gobs;
        gob = &picture_layer->gobs[controller->blockline];

        uvlc_read_gob_layer( stream, gob );
      }
    }
  }

  return C_OK;
}

C_RESULT uvlc_encode_blockline( video_controller_t* controller, const vp_api_picture_t* blockline, bool_t picture_complete )
{
  video_codec_t* video_codec;
  int16_t *in = NULL, *out = NULL;
  int32_t num_macro_blocks = 0;
  video_macroblock_t* macroblock = NULL;
  video_picture_context_t blockline_ctx;
  video_gob_t*  gobs;

  video_stream_t* stream = &controller->in_stream;

  if( stream->used*2 >= stream->size )
  {
    stream->bytes = realloc( stream->bytes, stream->size + 2048 ); // Add 2KB to internal stream
    stream->size += 2048;
  }

  video_codec                   = controller->video_codec;
  controller->picture_complete  = picture_complete;
  controller->blockline         = blockline->blockline;

  uvlc_pack_controller( controller );

  blockline_ctx.y_src     = blockline->y_buf;
  blockline_ctx.cb_src    = blockline->cb_buf;
  blockline_ctx.cr_src    = blockline->cr_buf;
  blockline_ctx.y_woffset = blockline->y_line_size;
  blockline_ctx.c_woffset = blockline->cb_line_size;
  blockline_ctx.y_hoffset = blockline->y_line_size * MCU_HEIGHT;

  gobs        = &controller->gobs[controller->blockline];
  gobs->quant = controller->quant;
  macroblock  = &gobs->macroblocks[0];

  in  = controller->blockline_cache;
  out = macroblock->data;

  num_macro_blocks = controller->mb_blockline;

  ///> Cache blockline in dct format & perform dct
  while( num_macro_blocks > MAX_NUM_MACRO_BLOCKS_PER_CALL )
  {
    RTMON_USTART(VIDEO_VLIB_BLOCKLINE_TO_MB);
    video_blockline_to_macro_blocks(&blockline_ctx, in, MAX_NUM_MACRO_BLOCKS_PER_CALL);
    RTMON_USTOP(VIDEO_VLIB_BLOCKLINE_TO_MB);

    out = video_fdct_compute(in, out, MAX_NUM_MACRO_BLOCKS_PER_CALL);

    if( in == controller->blockline_cache )
      in += DCT_BUFFER_SIZE;
    else
      in -= DCT_BUFFER_SIZE;

    num_macro_blocks -= MAX_NUM_MACRO_BLOCKS_PER_CALL;
  }

  RTMON_USTART(VIDEO_VLIB_BLOCKLINE_TO_MB);
  video_blockline_to_macro_blocks(&blockline_ctx, in, num_macro_blocks);
  RTMON_USTOP(VIDEO_VLIB_BLOCKLINE_TO_MB);

  video_fdct_compute(in, out, num_macro_blocks);
  ///<

  ///> Do quantification on each macroblock
  RTMON_USTART(VIDEO_VLIB_QUANTIZE);
  video_quantize( controller, &controller->gobs[controller->blockline].macroblocks[0], controller->mb_blockline );
  RTMON_USTOP(VIDEO_VLIB_QUANTIZE);
  ///<

  ///> Packetize Data to output buffer
  RTMON_USTART(VIDEO_VLIB_PACKET);
  uvlc_write_mb_layer( stream, macroblock, controller->mb_blockline );
  RTMON_USTOP(VIDEO_VLIB_PACKET);
  ///<

  if( controller->picture_complete )
  {
    video_stuff8( stream );
    video_write_data( stream, PICTURE_END_CODE, 22 );
  }

  // Update controller according to user inputs & video statistics
  video_controller_update( controller, picture_complete );

  return C_OK;
}

#ifndef HAS_UVLC_DECODE_BLOCKLINE
C_RESULT uvlc_decode_blockline( video_controller_t* controller, vp_api_picture_t* picture, bool_t* got_image )
{
  video_codec_t* video_codec;
  vp_api_picture_t blockline = { 0 };
  int16_t *in = NULL, *out = NULL;
  int32_t num_macro_blocks = 0;
  video_macroblock_t* macroblock = NULL;
  video_picture_context_t blockline_ctx;
  video_gob_t*  gobs;

  controller->mode  = VIDEO_DECODE;
  video_codec       = controller->video_codec;

  blockline                   = *picture;
  blockline.height            = MB_HEIGHT_Y;
  blockline.complete          = 1;
  blockline.vision_complete   = 0;

  picture->complete  = controller->picture_complete;

  blockline_ctx.y_woffset = blockline.y_line_size;
  blockline_ctx.c_woffset = blockline.cb_line_size;
  blockline_ctx.y_hoffset = blockline.y_line_size * MCU_HEIGHT;

  // At least a complete blockline is found
  while( !controller->picture_complete && controller->in_stream.index < (controller->in_stream.used >> 2) )
  {
    uvlc_unpack_controller( controller );

    if( !controller->picture_complete )
    {
      blockline.blockline  = controller->blockline;

      blockline_ctx.y_src     = picture->y_buf + blockline.blockline * MB_HEIGHT_Y * picture->y_line_size;
      blockline_ctx.cb_src    = picture->cb_buf + blockline.blockline * MB_HEIGHT_C * picture->cb_line_size;
      blockline_ctx.cr_src    = picture->cr_buf + blockline.blockline * MB_HEIGHT_C * picture->cr_line_size;

      picture->blockline  = controller->blockline;
      num_macro_blocks    = controller->mb_blockline;

      macroblock  = &controller->cache_mbs[0];
      gobs        = &controller->gobs[controller->blockline];
      out         = gobs->macroblocks->data;

      if( gobs->quant != controller->quant )
      {
        controller->quant = gobs->quant;
        video_quantizer_update( controller );
      }

      while( num_macro_blocks > MAX_NUM_MACRO_BLOCKS_PER_CALL )
      {
        in = &macroblock->data[0];

        uvlc_read_mb_layer( &controller->in_stream, macroblock, MAX_NUM_MACRO_BLOCKS_PER_CALL );

        video_unquantize( controller, macroblock, MAX_NUM_MACRO_BLOCKS_PER_CALL );

        out = video_idct_compute( in, out, MAX_NUM_MACRO_BLOCKS_PER_CALL );

        if( macroblock == &controller->cache_mbs[0] )
          macroblock += MAX_NUM_MACRO_BLOCKS_PER_CALL;
        else
          macroblock -= MAX_NUM_MACRO_BLOCKS_PER_CALL;

        num_macro_blocks -= MAX_NUM_MACRO_BLOCKS_PER_CALL;
      }

      in = macroblock->data;

      uvlc_read_mb_layer( &controller->in_stream, macroblock, num_macro_blocks );

      video_unquantize( controller, macroblock, num_macro_blocks );

      video_idct_compute( in, out, num_macro_blocks );

      video_blockline_from_macro_blocks(&blockline_ctx, gobs->macroblocks->data, controller->mb_blockline, picture->format);

      // Update controller according to video statistics
      video_controller_update( controller, controller->picture_complete );
    }
  }

  if( controller->picture_complete )
  {
    picture->complete   = controller->picture_complete;
    picture->blockline  = 0;

    controller->picture_complete  = 0;
    controller->in_stream.length  = 32;
    controller->num_frames++;

    *got_image = TRUE;
  }
  else
  {
    controller->in_stream.used  = 0;
    controller->in_stream.index = 0;
  }

  return C_OK;
}
#endif

C_RESULT uvlc_update( video_controller_t* controller )
{
  return C_OK;
}

C_RESULT uvlc_cache( video_controller_t* controller, video_stream_t* ex_stream)
{
  C_RESULT res;

  video_stream_t* in_stream = &controller->in_stream;

  switch( controller->mode )
  {
  case VIDEO_ENCODE:
    res = uvlc_flush_stream( ex_stream, in_stream );
    break;

  case VIDEO_DECODE:
    res = uvlc_load_stream( in_stream, ex_stream );
    break;

  default:
    res = C_FAIL;
    break;
  }

  return res;
}

C_RESULT uvlc_write_gob_layer( video_stream_t* stream, uvlc_gob_layer_t* gob )
{
  video_write_data( stream, gob->quant, 5 );

  return C_OK;
}

C_RESULT uvlc_read_gob_layer( video_stream_t* stream, uvlc_gob_layer_t* gob )
{
  gob->quant = 0;

  video_read_data( stream, &gob->quant, 5 );

  return C_OK;
}

#ifndef HAS_UVLC_WRITE_BLOCK

void uvlc_write_block( video_stream_t* const stream, int16_t* data, int32_t num_coeff )
{
  int32_t* zztable;
  int32_t index, code, run;

  zztable = &video_zztable_t81[1];

  // DC coeff
  code = *data;
  video_write_data( stream, code, 10 );
  num_coeff--;

  // AC coeff
  run = 0;
  while( num_coeff > 0 )
  {
    index = *zztable++;
    code = data[index];
    if( code == 0 )
    {
      run ++;
    }
    else
    {
      num_coeff--;
      uvlc_encode( stream, code, run, num_coeff );
      run = 0;
    }
  }
}

#endif // HAS_UVLC_WRITE_BLOCK

#ifndef HAS_UVLC_READ_BLOCK

C_RESULT uvlc_read_block( video_stream_t* stream, int16_t* data, int32_t* num_coeff )
{
  int32_t* zztable;
  int32_t index, code, run, last, nc;

  zztable = &video_zztable_t81[0];

  nc = *num_coeff;

  // DC coeff
  code = run = last = 0;
  video_read_data( stream, (uint32_t*) &code, 10 );
  data[0] = code;

  if( nc > 0 )
  {
    // AC coeff
    while( last == 0 )
    {
      code = run = last = 0;
      uvlc_decode( stream, &run, &code, &last);

      ASSERT( run < 64 );

      if( last == 0 )
      {
        zztable    += (run+1);
        index       = *zztable;
        data[index] = code;

        nc++;
      }
    }

    *num_coeff = nc;
  }

  return C_OK;
}

#endif

C_RESULT uvlc_write_mb_layer( video_stream_t* stream, video_macroblock_t* mb, int32_t num_macro_blocks )
{
  int16_t* data;
  uint32_t code;

  while( num_macro_blocks > 0 )
  {
    video_write_data( stream, mb->azq, 1 );
    if( !mb->azq )
    {
      code  = 0x80;
      code |= (mb->num_coeff_y0 > 1) << 0;
      code |= (mb->num_coeff_y1 > 1) << 1;
      code |= (mb->num_coeff_y2 > 1) << 2;
      code |= (mb->num_coeff_y3 > 1) << 3;
      code |= (mb->num_coeff_cb > 1) << 4;
      code |= (mb->num_coeff_cr > 1) << 5;
      code |= (mb->dquant != 0) << 6;

      video_write_data( stream, code, 8 );

      if( mb->dquant != 0 )
      {
        code = ( mb->dquant < 0 ) ? ~mb->dquant : mb->dquant;
        video_write_data( stream, code, 2 );
      }

      /**************** Block Y0 ****************/
      data = mb->data;
      uvlc_write_block( stream, data, mb->num_coeff_y0 );

      /**************** Block Y1 ****************/
      data += MCU_BLOCK_SIZE;
      uvlc_write_block( stream, data, mb->num_coeff_y1 );

      /**************** Block Y2 ****************/
      data += MCU_BLOCK_SIZE;
      uvlc_write_block( stream, data, mb->num_coeff_y2 );

      /**************** Block Y3 ****************/
      data += MCU_BLOCK_SIZE;
      uvlc_write_block( stream, data, mb->num_coeff_y3 );

      /**************** Block CB ****************/
      data += MCU_BLOCK_SIZE;
      uvlc_write_block( stream, data, mb->num_coeff_cb );

      /**************** Block CR ****************/
      data += MCU_BLOCK_SIZE;
      uvlc_write_block( stream, data, mb->num_coeff_cr );
    }

    mb ++;
    num_macro_blocks --;
  }

  return C_OK;
}

C_RESULT uvlc_read_mb_layer( video_stream_t* stream, video_macroblock_t* mb, int32_t num_macro_blocks )
{
  int16_t* data;
  uint32_t code;

  memset( mb->data, 0, num_macro_blocks * 6 * MCU_BLOCK_SIZE * sizeof(int16_t) );
  while( num_macro_blocks > 0 )
  {
    mb->azq = 0;
    video_read_data( stream, (uint32_t*)&mb->azq, 1 );

    if( mb->azq == 0 )
    {
      video_read_data( stream, &code, 8 );

      mb->num_coeff_y0 = (code >> 0) & 1;
      mb->num_coeff_y1 = (code >> 1) & 1;
      mb->num_coeff_y2 = (code >> 2) & 1;
      mb->num_coeff_y3 = (code >> 3) & 1;
      mb->num_coeff_cb = (code >> 4) & 1;
      mb->num_coeff_cr = (code >> 5) & 1;

      mb->dquant = 0;
      if( (code >> 6) & 1 )
      {
        video_read_data( stream, &code, 2 );

        mb->dquant = (code < 2) ? ~code : code;
      }

      /**************** Block Y0 ****************/
      data = mb->data;
      uvlc_read_block( stream, data, &mb->num_coeff_y0 );

      /**************** Block Y1 ****************/
      data += MCU_BLOCK_SIZE;
      uvlc_read_block( stream, data, &mb->num_coeff_y1 );

      /**************** Block Y2 ****************/
      data += MCU_BLOCK_SIZE;
      uvlc_read_block( stream, data, &mb->num_coeff_y2 );

      /**************** Block Y3 ****************/
      data += MCU_BLOCK_SIZE;
      uvlc_read_block( stream, data, &mb->num_coeff_y3 );

      /**************** Block CB ****************/
      data += MCU_BLOCK_SIZE;
      uvlc_read_block( stream, data, &mb->num_coeff_cb );

      /**************** Block CR ****************/
      data += MCU_BLOCK_SIZE;
      uvlc_read_block( stream, data, &mb->num_coeff_cr );

    }

    mb ++;
    num_macro_blocks --;
  }

  return C_OK;
}

C_RESULT uvlc_write_picture_layer( video_controller_t* controller, video_stream_t* stream )
{
  uint32_t format = 0, resolution = 0, width, height;

  uvlc_codec_t* uvlc_codec = (uvlc_codec_t*) controller->video_codec;
  uvlc_picture_layer_t* picture_layer = &uvlc_codec->picture_layer;

  width   = controller->width;
  height  = controller->height;

  while( format == 0 )
  {
    if( width == QQCIF_WIDTH )
      format = UVLC_FORMAT_CIF;

    if( width == QQVGA_WIDTH )
      format = UVLC_FORMAT_VGA;

    width   >>= 1;
    height  >>= 1;

    resolution ++;
  }

  picture_layer->format     = format;
  picture_layer->resolution = resolution;

  video_write_data( stream, picture_layer->format, 2 );
  video_write_data( stream, picture_layer->resolution, 3 );
  video_write_data( stream, picture_layer->picture_type, 3 );
  video_write_data( stream, picture_layer->quant, 5 );
  video_write_data( stream, controller->num_frames, 32 );

  return C_OK;
}

C_RESULT uvlc_read_picture_layer( video_controller_t* controller, video_stream_t* stream )
{
  uint32_t width, height;

  uvlc_codec_t* uvlc_codec = (uvlc_codec_t*) controller->video_codec;
  uvlc_picture_layer_t* picture_layer = &uvlc_codec->picture_layer;

  picture_layer->format       = 0;
  picture_layer->resolution   = 0;
  picture_layer->picture_type = 0;
  picture_layer->quant        = 0;

  video_read_data( stream, &picture_layer->format, 2 );
  video_read_data( stream, &picture_layer->resolution, 3 );
  video_read_data( stream, &picture_layer->picture_type, 3 );
  video_read_data( stream, &picture_layer->quant, 5 );
  video_read_data( stream, &controller->num_frames, 32 );

  switch( picture_layer->format )
  {
    case UVLC_FORMAT_CIF:
      width   = QQCIF_WIDTH << (picture_layer->resolution-1);
      height  = QQCIF_HEIGHT << (picture_layer->resolution-1);
      break;

    case UVLC_FORMAT_VGA:
      width   = QQVGA_WIDTH << (picture_layer->resolution-1);
      height  = QQVGA_HEIGHT << (picture_layer->resolution-1);
      break;

    default:
      width   = 0;
      height  = 0;
      break;
  }

  video_controller_set_format( controller, width, height );

  return C_OK;
}

C_RESULT uvlc_read_block_unquantize( video_controller_t* controller, int16_t* data, int32_t quant, int32_t nc )
{
	video_stream_t* stream = &controller->in_stream;
	int32_t* zztable;
	int32_t index, code, run, last;

	zztable = &video_zztable_t81[0];
	code = run = last = 0;
	if (quant == TABLE_QUANTIZATION)
	{
	  // table quantization mode
	  video_read_data( stream, (uint32_t*) &code, 10 );
	  int16_t* p_iquant_table = (int16_t*)(&iquant_tab[0]);
	  code *= *p_iquant_table;
	  data[0] = code;

	  if( nc > 0 )
	  {
		uvlc_decode( stream, &run, &code, &last);
	    while (last == 0)
	    {
	      ASSERT( run < 64 );

	      zztable    += (run+1);
	      index       = *zztable;
		  code *= p_iquant_table[index];
		  data[index] = code;

		  uvlc_decode( stream, &run, &code, &last);
	    }
	  }
	}
	else
	{
	  // const quantization mode
	  // DC coeff
	  video_read_data( stream, (uint32_t*) &code, 10 );

      if( controller->picture_type == VIDEO_PICTURE_INTRA ) // intra
	  {
		code <<= 3;
	  }
	  else
	  {
		code = quant*( 2*code + 1 );
		if( quant & 1 )
			code -= 1;
	  }
	  data[0] = code;

	  if( nc > 0 )
	  {
		uvlc_decode( stream, &run, &code, &last);

		while( last == 0 )
		{
			ASSERT( run < 64 );

			zztable    += (run+1);
			index       = *zztable;

			code = quant*( 2*code + 1 );
			if( quant & 1 )
				code -= 1;

			data[index] = code;

			uvlc_decode( stream, &run, &code, &last);
		}
	  }
	}

	return C_OK;
}

uint16_t* uvlc_read_mb_layer_unquantize( video_controller_t* controller, video_macroblock_t* mb, uint16_t* out )
{
	int16_t* data;
	uint32_t code;

	video_zeromem32( (uint32_t*)mb->data, 6 * MCU_BLOCK_SIZE / 2 );

		mb->azq = 0;
		video_read_data( &controller->in_stream, (uint32_t*)&mb->azq, 1 );

		if( mb->azq == 0 )
		{
			video_read_data( &controller->in_stream, &code, 8 );

			mb->num_coeff_y0 = (code >> 0) & 1;
			mb->num_coeff_y1 = (code >> 1) & 1;
			mb->num_coeff_y2 = (code >> 2) & 1;
			mb->num_coeff_y3 = (code >> 3) & 1;
			mb->num_coeff_cb = (code >> 4) & 1;
			mb->num_coeff_cr = (code >> 5) & 1;

			mb->dquant = 0;
			if( (code >> 6) & 1 )
			{
				video_read_data( &controller->in_stream, &code, 2 );

				mb->dquant = (code < 2) ? ~code : code;
			}

			controller->quant += mb->dquant;

			/**************** Block Y0 ****************/
			data = mb->data;
			uvlc_read_block_unquantize( controller, data, controller->quant, mb->num_coeff_y0 );
			idct(data, out);
			out  += MCU_BLOCK_SIZE;

			/**************** Block Y1 ****************/
			data += MCU_BLOCK_SIZE;
			uvlc_read_block_unquantize( controller, data, controller->quant, mb->num_coeff_y1 );
			idct(data, out);
			out  += MCU_BLOCK_SIZE;

			/**************** Block Y2 ****************/
			data += MCU_BLOCK_SIZE;
			uvlc_read_block_unquantize( controller, data, controller->quant, mb->num_coeff_y2 );
			idct(data, out);
			out  += MCU_BLOCK_SIZE;

			/**************** Block Y3 ****************/
			data += MCU_BLOCK_SIZE;
			uvlc_read_block_unquantize( controller, data, controller->quant, mb->num_coeff_y3 );
			idct(data, out);
			out  += MCU_BLOCK_SIZE;

			/**************** Block CB ****************/
			data += MCU_BLOCK_SIZE;
			uvlc_read_block_unquantize( controller, data, controller->quant, mb->num_coeff_cb );
			idct(data, out);
			out  += MCU_BLOCK_SIZE;

			/**************** Block CR ****************/
			data += MCU_BLOCK_SIZE;
			uvlc_read_block_unquantize( controller, data, controller->quant, mb->num_coeff_cr );
			idct(data, out);
			out  += MCU_BLOCK_SIZE;
		}

		mb ++;

	return out;
}

C_RESULT uvlc_decode_blockline( video_controller_t* controller, vp_api_picture_t* picture, bool_t* got_image )
{
	video_codec_t* video_codec;
	vp_api_picture_t blockline = { 0 };
	int16_t *in = NULL;
	uint16_t *out = NULL;
	int32_t num_macro_blocks = 0;
	video_macroblock_t* macroblock = NULL;
	video_picture_context_t blockline_ctx;
	video_gob_t*  gobs;

	controller->mode  = VIDEO_DECODE;
	video_codec       = controller->video_codec;

	blockline                   = *picture;
	blockline.height            = MB_HEIGHT_Y;
	blockline.complete          = 1;
	blockline.vision_complete   = 0;

	picture->complete  = controller->picture_complete;

	blockline_ctx.y_woffset = blockline.y_line_size;
	blockline_ctx.c_woffset = blockline.cb_line_size;
	blockline_ctx.y_hoffset = blockline.y_line_size * MCU_HEIGHT;

	// At least a complete blockline is found
	while( !controller->picture_complete && controller->in_stream.index < (controller->in_stream.used >> 2) )
	{
		uvlc_unpack_controller( controller );

		if( !controller->picture_complete )
		{
			blockline.blockline  = controller->blockline;

			blockline_ctx.y_src     = picture->y_buf + blockline.blockline * MB_HEIGHT_Y * picture->y_line_size;
			blockline_ctx.cb_src    = picture->cb_buf + blockline.blockline * MB_HEIGHT_C * picture->cb_line_size;
			blockline_ctx.cr_src    = picture->cr_buf + blockline.blockline * MB_HEIGHT_C * picture->cr_line_size;

			picture->blockline  = controller->blockline;
			num_macro_blocks    = controller->mb_blockline;

			macroblock  = &controller->cache_mbs[0];
			gobs        = &controller->gobs[controller->blockline];
			out         = (uint16_t*) gobs->macroblocks->data;

			if( gobs->quant != controller->quant )
			{
				controller->quant = gobs->quant;
				video_quantizer_update( controller );
			}

			while( num_macro_blocks > 0 )
			{
				in = &macroblock->data[0];

				out = uvlc_read_mb_layer_unquantize( controller, macroblock, out );

				num_macro_blocks --;
			}

			video_blockline_from_macro_blocks(&blockline_ctx, gobs->macroblocks->data, controller->mb_blockline, picture->format);

			// Update controller according to video statistics
			video_controller_update( controller, controller->picture_complete );
		}
	}

	if( controller->picture_complete )
	{
		picture->complete   = controller->picture_complete;
		picture->blockline  = 0;

		controller->picture_complete  = 0;
		controller->in_stream.length  = 32;
		//controller->num_frames++;

		*got_image = TRUE;
	}
	else
	{
		controller->in_stream.used  = 0;
		controller->in_stream.index = 0;
	}

	return C_OK;
}

C_RESULT video_zeromem32( uint32_t* dst, uint32_t length )
{
  while( length )
  {
    *dst = 0;

    dst ++;
    length--;
  }

  return C_OK;
}

C_RESULT video_copy32(uint32_t* dst, uint32_t* src, uint32_t nb)
{
  uint32_t i;

  for( i = 0; i < nb; i++ )
  {
    dst[i] = src[i];
  }

  return C_OK;
}

C_RESULT video_copy32_swap(uint32_t* dst, uint32_t* src, uint32_t nb)
{
  uint32_t i;

  for( i = 0; i < nb; i++ )
  {
    dst[i] = bswap( src[i] );
  }

  return C_OK;
}

#ifndef HAS_VIDEO_BLOCKLINE_TO_MACRO_BLOCKS

// Convert a 8x8 block of 8 bits data to a 8x8 block of 16 bits data
static void copy_block_8_16(int16_t* dst, int32_t dst_offset, uint8_t* src, int32_t src_offset)
{
  uint32_t* src32 = (uint32_t*) src;
  uint32_t* dst32 = (uint32_t*) dst;

  uint32_t  src_offset32 = src_offset >> 2;
  uint32_t  dst_offset32 = dst_offset >> 1;

  uint32_t  temp;

  int32_t i;

  for( i = 0; i < MCU_BLOCK_SIZE; i += MCU_WIDTH, src32 += src_offset32, dst32 += dst_offset32 )
  {
    temp = *src32++;

    *dst32++ = ((temp << 8) & 0x00FF0000) | (temp & 0x000000FF);
    *dst32++ = ((temp >> 8) & 0x00FF0000) | ((temp >> 16) & 0x000000FF);

    temp = *src32++;

    *dst32++ = ((temp << 8) & 0x00FF0000) | (temp & 0x000000FF);
    *dst32++ = ((temp >> 8) & 0x00FF0000) | ((temp >> 16) & 0x000000FF);
  }

}

#endif

// Convert a 8x8 block of 16 bits data to a 8x8 block of 8 bits data
static void copy_block_16_8(uint8_t* dst, int32_t dst_offset, int16_t* src, int32_t src_offset)
{
  int32_t i;
  int16_t temp;

  for( i = 0; i < MCU_BLOCK_SIZE; i += MCU_WIDTH, dst += dst_offset, src += src_offset )
  {
    temp = *src++; if( temp > 0xff ) temp = 0xff; if(temp < 0) temp = 0; temp &= 0xff; *dst++ = (uint8_t) temp;
    temp = *src++; if( temp > 0xff ) temp = 0xff; if(temp < 0) temp = 0; temp &= 0xff; *dst++ = (uint8_t) temp;
    temp = *src++; if( temp > 0xff ) temp = 0xff; if(temp < 0) temp = 0; temp &= 0xff; *dst++ = (uint8_t) temp;
    temp = *src++; if( temp > 0xff ) temp = 0xff; if(temp < 0) temp = 0; temp &= 0xff; *dst++ = (uint8_t) temp;
    temp = *src++; if( temp > 0xff ) temp = 0xff; if(temp < 0) temp = 0; temp &= 0xff; *dst++ = (uint8_t) temp;
    temp = *src++; if( temp > 0xff ) temp = 0xff; if(temp < 0) temp = 0; temp &= 0xff; *dst++ = (uint8_t) temp;
    temp = *src++; if( temp > 0xff ) temp = 0xff; if(temp < 0) temp = 0; temp &= 0xff; *dst++ = (uint8_t) temp;
    temp = *src++; if( temp > 0xff ) temp = 0xff; if(temp < 0) temp = 0; temp &= 0xff; *dst++ = (uint8_t) temp;
  }
}

//
// Transform blockline in macro blocks
//
// Blockline:
//  _______
// | 1 | 2 |
// |___|___|  Y
// | 3 | 4 |
// |___|___|
//  ___
// | 5 |
// |___| Cb
//  ___
// | 6 |
// |___| Cr
//
// Dct cache:
//  _______________________
// | 1 | 2 | 3 | 4 | 5 | 6 | ...
// |___|___|___|___|___|___|
//

#ifndef HAS_VIDEO_BLOCKLINE_TO_MACRO_BLOCKS

C_RESULT video_blockline_to_macro_blocks(video_picture_context_t* ctx, int16_t* dst, int32_t num_macro_blocks)
{
  uint8_t* y_src        = ctx->y_src;
  uint8_t* cb_src       = ctx->cb_src;
  uint8_t* cr_src       = ctx->cr_src;

  while( num_macro_blocks > 0 )
  {
    // Current MB
    copy_block_8_16( dst,
                     0,
                     y_src,
                     ctx->y_woffset - MCU_WIDTH );

    dst += MCU_BLOCK_SIZE; // skip block 1

    copy_block_8_16( dst,
                     0,
                     y_src + MCU_WIDTH,
                     ctx->y_woffset - MCU_WIDTH );

    dst += MCU_BLOCK_SIZE; // skip block 2

    copy_block_8_16( dst,
                     0,
                     y_src + ctx->y_hoffset,
                     ctx->y_woffset - MCU_WIDTH );

    dst += MCU_BLOCK_SIZE; // skip block 3

    copy_block_8_16( dst,
                     0,
                     y_src + ctx->y_hoffset + MCU_WIDTH,
                     ctx->y_woffset - MCU_WIDTH );

    dst += MCU_BLOCK_SIZE; // skip block 4

    copy_block_8_16( dst,
                     0,
                     cb_src,
                     ctx->c_woffset - MCU_WIDTH );

    dst += MCU_BLOCK_SIZE;  // skip blocks 5

    copy_block_8_16( dst,
                     0,
                     cr_src,
                     ctx->c_woffset - MCU_WIDTH );

    dst += MCU_BLOCK_SIZE;  // skip blocks 6

    y_src   += MCU_WIDTH*2;
    cb_src  += MCU_WIDTH;
    cr_src  += MCU_WIDTH;

    num_macro_blocks --;
  }

  ctx->y_src  = y_src;
  ctx->cb_src = cb_src;
  ctx->cr_src = cr_src;

  return C_OK;
}

#endif

// Transform macro blocks in picture of specified format
static C_RESULT video_blockline_from_macro_blocks_yuv420(video_picture_context_t* ctx, int16_t* src, int32_t num_macro_blocks);
static C_RESULT video_blockline_from_macro_blocks_rgb565(video_picture_context_t* ctx, int16_t* src, int32_t num_macro_blocks);

C_RESULT video_blockline_from_macro_blocks(video_picture_context_t* ctx, int16_t* src, int32_t num_macro_blocks, enum PixelFormat format)
{
  C_RESULT res;

  switch(format)
  {
    case PIX_FMT_YUV420P:
      res = video_blockline_from_macro_blocks_yuv420(ctx, src, num_macro_blocks);
      break;
    case PIX_FMT_RGB565:
      res = video_blockline_from_macro_blocks_rgb565(ctx, src, num_macro_blocks);
      break;

    default:
		//PRINT("In file %s, in function %s, format %d not supported\n", __FILE__, __FUNCTION__, format);
      res = C_FAIL;
      break;
  }

  return res;
}

C_RESULT video_blockline_from_macro_blocks_yuv420(video_picture_context_t* ctx, int16_t* src, int32_t num_macro_blocks)
{
  uint8_t *y_dst, *cb_dst, *cr_dst;

  y_dst   = ctx->y_src;
  cb_dst  = ctx->cb_src;
  cr_dst  = ctx->cr_src;

  while( num_macro_blocks > 0 )
  {
    // Current MB
    copy_block_16_8( y_dst,
                     ctx->y_woffset - MCU_WIDTH,
                     src,
                     0 );

    src += MCU_BLOCK_SIZE;

    copy_block_16_8( y_dst + MCU_WIDTH,
                     ctx->y_woffset - MCU_WIDTH,
                     src,
                     0 );

    src += MCU_BLOCK_SIZE;

    copy_block_16_8( y_dst + ctx->y_hoffset,
                     ctx->y_woffset - MCU_WIDTH,
                     src,
                     0 );

    src += MCU_BLOCK_SIZE;

    copy_block_16_8( y_dst + ctx->y_hoffset + MCU_WIDTH,
                     ctx->y_woffset - MCU_WIDTH,
                     src,
                     0 );

    src += MCU_BLOCK_SIZE;

    copy_block_16_8( cb_dst,
                     ctx->c_woffset - MCU_WIDTH,
                     src,
                     0 );

    src += MCU_BLOCK_SIZE;

    copy_block_16_8( cr_dst,
                     ctx->c_woffset - MCU_WIDTH,
                     src,
                     0 );

    src += MCU_BLOCK_SIZE;

    y_dst   += MCU_WIDTH*2;
    cb_dst  += MCU_WIDTH;
    cr_dst  += MCU_WIDTH;

    num_macro_blocks--;
  }

  ctx->y_src  = y_dst;
  ctx->cb_src = cb_dst;
  ctx->cr_src = cr_dst;

  return C_OK;
}

#define MAKE_RGBA_565(r, g, b) ( ((r) << 11) | ((g) << 5) | (b) )

#if (TARGET_CPU_ARM == 1) && defined(_IPHONE)
static inline int32_t saturate8(int32_t x)
{
	usat(x, 7, 8);
	
	return x;
}

static inline uint32_t saturate5(int32_t x)
{
	usat(x, 5, 11);
	
	return x;
}

static inline uint32_t saturate6(int32_t x)
{
	usat(x, 6, 10);
	
	return x;
}

#else
 
// To make sure that you are bounding your inputs in the range of 0 & 255

static inline int32_t saturate8(int32_t x)
{
  if( x < 0 )
  {
    x = 0;
  }

  x >>= 8;

  return x > 0xFF ? 0xFF : x;
}

static inline uint16_t saturate5(int32_t x)
{
  if( x < 0 )
  {
    x = 0;
  }

  x >>= 11;

  return x > 0x1F ? 0x1F : x;
}

static inline uint16_t saturate6(int32_t x)
{
  if( x < 0 )
  {
    x = 0;
  }

  x >>= 10;

  return x > 0x3F ? 0x3F : x;
}
#endif

static C_RESULT video_blockline_from_macro_blocks_rgb565(video_picture_context_t* ctx, int16_t* src, int32_t num_macro_blocks)
{
  uint32_t y_up_read, y_down_read, cr_current, cb_current;
  int32_t u, v, vr, ug, vg, ub, r, g, b;
  int16_t *y_buf1, *y_buf2, *cr_buf, *cb_buf;
  uint16_t *dst_up, *dst_down;

  // Control variables
  int32_t line_size, block_size, y_woffset, y_hoffset;

  y_buf1  = src;
  y_buf2  = y_buf1 + MCU_WIDTH;

  cb_buf  = y_buf1 + MCU_BLOCK_SIZE * 4;
  cr_buf  = cb_buf + MCU_BLOCK_SIZE;

  // Our ptrs are 16 bits
  y_woffset = ctx->y_woffset / 2;
  y_hoffset = ctx->y_hoffset / 2;

  dst_up    = (uint16_t*) ctx->y_src;
  dst_down  = dst_up + y_woffset;

  line_size		= MCU_WIDTH / 2; // We compute two pixels at a time
  block_size	= MCU_HEIGHT / 2; // We compute two lines at a time

  while( num_macro_blocks > 0 )
  {
    // First quarter
    cb_current  = cb_buf[0];
    cr_current  = cr_buf[0];

    u   = cb_current - 128;
    ug  = 88 * u;
    ub  = 454 * u;
    v   = cr_current - 128;
    vg  = 183 * v;
    vr  = 359 * v;

    y_up_read   = y_buf1[0] << 8;
    y_down_read = y_buf2[0] << 8;

    r = saturate5((y_up_read + vr));
    g = saturate6((y_up_read - ug - vg));
    b = saturate5((y_up_read + ub));

    dst_up[0] = MAKE_RGBA_565(r, g, b);

    r = saturate5((y_down_read + vr));
    g = saturate6((y_down_read - ug - vg));
    b = saturate5((y_down_read + ub));

    dst_down[0] = MAKE_RGBA_565(r, g, b);

    y_up_read   = y_buf1[1] << 8;
    y_down_read = y_buf2[1] << 8;

    r = saturate5((y_up_read + vr));
    g = saturate6((y_up_read - ug - vg));
    b = saturate5((y_up_read + ub));

    dst_up[1] = MAKE_RGBA_565(r, g, b);

    r = saturate5((y_down_read + vr));
    g = saturate6((y_down_read - ug - vg));
    b = saturate5((y_down_read + ub));

    dst_down[1] = MAKE_RGBA_565(r, g, b);

    // Second quarter
    cr_current  = cr_buf[MCU_WIDTH / 2];
    cb_current  = cb_buf[MCU_WIDTH / 2];

    u   = cb_current - 128;
    ug  = 88 * u;
    ub  = 454 * u;
    v   = cr_current - 128;
    vg  = 183 * v;
    vr  = 359 * v;

    y_up_read   = y_buf1[MCU_BLOCK_SIZE] << 8;
    y_down_read = y_buf2[MCU_BLOCK_SIZE] << 8;

    r = saturate5((y_up_read + vr));
    g = saturate6((y_up_read - ug - vg));
    b = saturate5((y_up_read + ub));

    dst_up[MCU_WIDTH] = MAKE_RGBA_565(r, g, b);

    r = saturate5((y_down_read + vr));
    g = saturate6((y_down_read - ug - vg));
    b = saturate5((y_down_read + ub));

    dst_down[MCU_WIDTH] = MAKE_RGBA_565(r, g, b);

    y_up_read   = y_buf1[MCU_BLOCK_SIZE + 1] << 8;
    y_down_read = y_buf2[MCU_BLOCK_SIZE + 1] << 8;

    r = saturate5((y_up_read + vr));
    g = saturate6((y_up_read - ug - vg));
    b = saturate5((y_up_read + ub));

    dst_up[MCU_WIDTH+1] = MAKE_RGBA_565(r, g, b);

    r = saturate5((y_down_read + vr));
    g = saturate6((y_down_read - ug - vg));
    b = saturate5((y_down_read + ub));

    dst_down[MCU_WIDTH+1] = MAKE_RGBA_565(r, g, b);

    // Third quarter
    cr_current  = cr_buf[MCU_BLOCK_SIZE/2];
    cb_current  = cb_buf[MCU_BLOCK_SIZE/2];

    u   = cb_current - 128;
    ug  = 88 * u;
    ub  = 454 * u;
    v   = cr_current - 128;
    vg  = 183 * v;
    vr  = 359 * v;

    y_up_read   = y_buf1[MCU_BLOCK_SIZE*2] << 8;
    y_down_read = y_buf2[MCU_BLOCK_SIZE*2] << 8;

    r = saturate5((y_up_read + vr));
    g = saturate6((y_up_read - ug - vg));
    b = saturate5((y_up_read + ub));

    dst_up[y_hoffset] = MAKE_RGBA_565(r, g, b);

    r = saturate5((y_down_read + vr));
    g = saturate6((y_down_read - ug - vg));
    b = saturate5((y_down_read + ub));

    dst_down[y_hoffset] = MAKE_RGBA_565(r, g, b);

    y_up_read   = y_buf1[MCU_BLOCK_SIZE*2 + 1] << 8;
    y_down_read = y_buf2[MCU_BLOCK_SIZE*2 + 1] << 8;

    r = saturate5((y_up_read + vr));
    g = saturate6((y_up_read - ug - vg));
    b = saturate5((y_up_read + ub));

    dst_up[y_hoffset + 1] = MAKE_RGBA_565(r, g, b);

    r = saturate5((y_down_read + vr));
    g = saturate6((y_down_read - ug - vg));
    b = saturate5((y_down_read + ub));

    dst_down[y_hoffset + 1] = MAKE_RGBA_565(r, g, b);

    // Fourth quarter
    cr_current  = cr_buf[MCU_BLOCK_SIZE/2 + MCU_WIDTH/2];
    cb_current  = cb_buf[MCU_BLOCK_SIZE/2 + MCU_WIDTH/2];

    u   = cb_current - 128;
    ug  = 88 * u;
    ub  = 454 * u;
    v   = cr_current - 128;
    vg  = 183 * v;
    vr  = 359 * v;

    y_up_read   = y_buf1[MCU_BLOCK_SIZE*3] << 8;
    y_down_read = y_buf2[MCU_BLOCK_SIZE*3] << 8;

    r = saturate5((y_up_read + vr));
    g = saturate6((y_up_read - ug - vg));
    b = saturate5((y_up_read + ub));

    dst_up[y_hoffset + MCU_WIDTH] = MAKE_RGBA_565(r, g, b);

    r = saturate5((y_down_read + vr));
    g = saturate6((y_down_read - ug - vg));
    b = saturate5((y_down_read + ub));

    dst_down[y_hoffset + MCU_WIDTH] = MAKE_RGBA_565(r, g, b);

    y_up_read   = y_buf1[MCU_BLOCK_SIZE*3 + 1] << 8;
    y_down_read = y_buf2[MCU_BLOCK_SIZE*3 + 1] << 8;

    r = saturate5((y_up_read + vr));
    g = saturate6((y_up_read - ug - vg));
    b = saturate5((y_up_read + ub));

    dst_up[y_hoffset + MCU_WIDTH + 1] = MAKE_RGBA_565(r, g, b);

    r = saturate5((y_down_read + vr));
    g = saturate6((y_down_read - ug - vg));
    b = saturate5((y_down_read + ub));

    dst_down[y_hoffset + MCU_WIDTH + 1] = MAKE_RGBA_565(r, g, b);

    cr_buf    += 1;
    cb_buf    += 1;
    y_buf1    += 2;
    y_buf2    += 2;
    dst_up    += 2;
    dst_down  += 2;

    line_size--;

    if( line_size == 0 ) // We computed one line of a luma-block
    {
      dst_up    += y_woffset*2 - MCU_WIDTH;
      dst_down  += y_woffset*2 - MCU_WIDTH;

      block_size--;

      if( block_size == 0 )
      {
        y_buf1  = cr_buf + MCU_BLOCK_SIZE/2 + MCU_WIDTH/2;
        y_buf2  = y_buf1 + MCU_WIDTH;

        cb_buf  = y_buf1 + MCU_BLOCK_SIZE * 4;
        cr_buf  = cb_buf + MCU_BLOCK_SIZE;

        block_size = MCU_WIDTH / 2; // We compute two lines at a time

        dst_up  += 2*MCU_WIDTH - y_hoffset;
        dst_down = dst_up + y_woffset;

        num_macro_blocks--;
      }
      else
      {
        y_buf1  = y_buf2;
        y_buf2 += MCU_WIDTH;

        cr_buf += MCU_WIDTH / 2;
        cb_buf += MCU_WIDTH / 2;
      }

      line_size	= MCU_WIDTH / 2; // We compute two pixels at a time
    }
  }

  ctx->y_src = (uint8_t*) dst_up;

  return C_OK;
}

extern C_RESULT video_utils_set_format( uint32_t width, uint32_t height );

C_RESULT video_controller_update( video_controller_t* controller, bool_t complete )
{
  video_codec_t* video_codec = controller->video_codec;

  controller->current_bits  += controller->in_stream.index << 5; // Index is an index in a int32_t array (32 bits)

  if( complete )
  {
    controller->num_frames   += 1;
    controller->output_bits   = controller->current_bits;
    controller->current_bits  = 0;
  }

  video_codec->update( controller );

  return C_OK;
}

C_RESULT video_controller_set_mode( video_controller_t* controller, uint32_t mode )
{
  controller->mode = mode;

  return C_OK;
}

C_RESULT video_controller_set_bitrate( video_controller_t* controller, uint32_t target )
{
  controller->target_bitrate = target;

  return C_OK;
}

static void video_realloc_buffers( video_controller_t* controller, int32_t num_prev_blockline )
{
  int32_t i, j;
  int16_t* cache;
  video_gob_t* gob;
  video_macroblock_t* mb;

  // Realloc global cache (YUV420 format)
  controller->cache = (int16_t*) aligned_realloc( controller->cache,
                                                        3 * controller->width * controller->height * sizeof(int16_t) / 2,
                                                        VLIB_ALLOC_ALIGN );
  memset( controller->cache, 0, 3 * controller->width * controller->height * sizeof(int16_t) / 2 );

  // Realloc buffers
  cache = controller->cache;
  i = controller->num_blockline;

  controller->gobs = (video_gob_t*) realloc(controller->gobs, i * sizeof(video_gob_t));
  memset( controller->gobs, 0, i * sizeof(video_gob_t) );

  gob = &controller->gobs[0];
  for(; i > 0; i-- )
  {
    j = controller->mb_blockline;

    if( --num_prev_blockline < 0 )
      gob->macroblocks = NULL;

    gob->macroblocks = (video_macroblock_t*) realloc( gob->macroblocks, j * sizeof(video_macroblock_t));
    memset( gob->macroblocks, 0, j * sizeof(video_macroblock_t));

    mb = &gob->macroblocks[0];
    for(; j > 0; j-- )
    {
      mb->data = cache;
      cache   += MCU_BLOCK_SIZE*6;
      mb ++;
    }

    gob ++;
  }
}

C_RESULT video_controller_cleanup( video_controller_t* controller )
{
  int32_t i;
  video_gob_t* gob;

  if( controller->gobs != NULL )
  {
    gob = &controller->gobs[0];

    for(  i = controller->num_blockline; i > 0; i-- )
    {
      free( gob->macroblocks );

      gob ++;
    }

    free( controller->gobs );
  }

  if( controller->cache != NULL )
  {
    aligned_free( controller->cache );
  }

  return C_OK;
}

C_RESULT video_controller_set_format( video_controller_t* controller, int32_t width, int32_t height )
{
  int32_t num_prev_blockline;

  ASSERT( (width != 0) && (height != 0) );

  if( width != controller->width || controller->height != height )
  {
    controller->width   = width;
    controller->height  = height;

    num_prev_blockline = controller->num_blockline;

    controller->num_blockline = height >> 4;
    controller->mb_blockline  = width >> 4;

    video_realloc_buffers( controller, num_prev_blockline );

    video_utils_set_format( width, height );
  }

  return C_OK;
}

C_RESULT  video_controller_set_picture_type( video_controller_t* controller, uint32_t type )
{
  controller->picture_type = type;

  return C_OK;
}

C_RESULT  video_controller_set_motion_estimation( video_controller_t* controller, bool_t use_me )
{
  controller->use_me = use_me;

  return C_OK;
}

C_RESULT video_quantizer_init( video_controller_t* controller )
{
  // Init quantizer's value
  // This value is between 1 and 31

  int32_t quant = controller->quant;

  if( quant < 1 )
    quant = 1;
  else if( quant >= 32 )
    quant = 31;

  if( controller->picture_type == VIDEO_PICTURE_INTRA )
  {
    controller->Qp    = 0;
    controller->invQp = (1 << 16) / (2*quant);
  }
  else // VIDEO_PICTURE_INTER
  {
    controller->Qp    = quant / 2;
    controller->invQp = (1 << 16) / (2*quant);
  }

  controller->dquant = 0;

  return C_OK;
}

C_RESULT video_quantizer_update( video_controller_t* controller )
{
  // Update quantizer's value
  int32_t quant = controller->quant;

  if( quant < 1 )
    quant = 1;
  else if( quant >= 32 )
    quant = 31;

  if( controller->picture_type == VIDEO_PICTURE_INTRA )
  {
    controller->Qp    = 0;
    controller->invQp = (1 << 16) / (2*quant);
  }
  else // VIDEO_PICTURE_INTER
  {
    controller->Qp    = quant / 2;
    controller->invQp = (1 << 16) / (2*quant);
  }

  controller->dquant = 0;

  return C_OK;
}

C_RESULT video_quantize( video_controller_t* controller, video_macroblock_t* macroblock, int32_t num_macro_blocks )
{
  int32_t sum_y, sum_c, dc0, dc1, dc2, dc3, dcb, dcr;
  int16_t *y0;

  y0  = macroblock->data;

  while( num_macro_blocks > 0 )
  {
    if( controller->do_azq == TRUE )
    {
      int16_t *y1, *y2, *y3, *cb, *cr;

      y1  = y0 + MCU_BLOCK_SIZE;
      y2  = y1 + MCU_BLOCK_SIZE;
      y3  = y2 + MCU_BLOCK_SIZE;
      cb  = y3 + MCU_BLOCK_SIZE;
      cr  = cb + MCU_BLOCK_SIZE;

      // Test for azq (all zero quantized) in luma blocks
      dc0 = *y0;
      dc1 = *y1;
      dc2 = *y2;
      dc3 = *y3;
      dcb = *cb;
      dcr = *cr;

      sum_y = dc0 + dc1 + dc2 + dc3;
      sum_c = dcb + dcr;
    }
    else
    {
      sum_y = 0x7F000000;
      sum_c = 0x7F000000;
    }

    macroblock->azq = (sum_y < controller->aq) & (sum_c < controller->bq);
    macroblock->dquant = controller->dquant;

    // Perform quantification on coefficients if necessary
    if( !macroblock->azq )
    {
      RTMON_USTART(VIDEO_VLIB_DOQUANTIZE);
      if( controller->picture_type == VIDEO_PICTURE_INTRA ) // intra
      {
        y0 = do_quantize_intra_mb(y0, controller->invQp, &macroblock->num_coeff_y0);
      }
      else
      {
        y0 = do_quantize_inter_mb(y0, controller->Qp, controller->invQp, &macroblock->num_coeff_y0);
      }
      RTMON_USTOP(VIDEO_VLIB_DOQUANTIZE);
    }

    macroblock++;
    num_macro_blocks--;

    if( macroblock->azq )
    {
      y0  = macroblock->data;
    }
  }

  return C_OK;
}

C_RESULT video_unquantize( video_controller_t* controller, video_macroblock_t* macroblock, int32_t num_macro_blocks )
{
  int16_t *dst;

  controller->quant += macroblock->dquant;

  dst  = macroblock->data;

  while( num_macro_blocks > 0 )
  {
    // TODO Check generated code

    if( !macroblock->azq )
      do_unquantize(dst, controller->picture_type, controller->quant, macroblock->num_coeff_y0);

    dst += MCU_BLOCK_SIZE;

    if( !macroblock->azq )
      do_unquantize(dst, controller->picture_type, controller->quant, macroblock->num_coeff_y1);

    dst += MCU_BLOCK_SIZE;

    if( !macroblock->azq )
      do_unquantize(dst, controller->picture_type, controller->quant, macroblock->num_coeff_y2);

    dst += MCU_BLOCK_SIZE;

    if( !macroblock->azq )
      do_unquantize(dst, controller->picture_type, controller->quant, macroblock->num_coeff_y3);

    dst += MCU_BLOCK_SIZE;

    if( !macroblock->azq )
      do_unquantize(dst, controller->picture_type, controller->quant, macroblock->num_coeff_cb);

    dst += MCU_BLOCK_SIZE;

    if( !macroblock->azq )
      do_unquantize(dst, controller->picture_type, controller->quant, macroblock->num_coeff_cr);

    dst += MCU_BLOCK_SIZE;

    macroblock ++;
    num_macro_blocks--;
  }

  return C_OK;
}

#ifndef HAS_DO_QUANTIZE_INTRA_MB
int16_t* do_quantize_intra_mb(int16_t* ptr, int32_t invQuant, int32_t* last_ptr)
{
  int32_t i, num_coeff;
  int32_t coeff, sign, last;

  for( i = 6; i > 0; i-- )
  {
    // LEVEL = (COF + 4)/(2*4) see III.3.2.3

    coeff = (*ptr + 4) >> 3;
    if( coeff == 0 )
      coeff = 1;

    *ptr++ = coeff;

    last = 1;
    num_coeff = MCU_BLOCK_SIZE-1;

    while( num_coeff > 0 )
    {
      coeff = *ptr;

      if( coeff != 0 )
      {
        // |LEVEL| = |COF| / (2 * QUANT) see III.3.2.2

        sign = coeff < 0;

        if( sign )
          coeff = -coeff;

        coeff *= invQuant;
        coeff >>= 16;

        if( sign )
          coeff = -coeff;

        if( coeff != 0 )
        {
          last++;
        }

        *ptr = coeff;
      }

      ptr++;
      num_coeff--;
    }

    *last_ptr++ = last;
  }

  return ptr;
}
#endif // HAS_DO_QUANTIZE_INTRA_MB

int16_t* do_quantize_inter_mb(int16_t* ptr, int32_t quant, int32_t invQuant, int32_t* last_ptr)
{
  int32_t i, num_coeff;
  int32_t coeff, sign, last;

  // LEVEL = ( |COF| - QUANT/2 ) / (2*QUANT) see III.3.2.1

  for( i = 6; i > 0; i-- )
  {
    last = 0;
    num_coeff = MCU_BLOCK_SIZE;

    while( num_coeff > 0 )
    {
      coeff = *ptr;

      if( coeff != 0 )
      {
        sign = coeff < 0;

        if( sign )
          coeff = -coeff;

        coeff -= quant;
        coeff *= invQuant;
        coeff >>= 16;

        if( sign )
          coeff = -coeff;

        if( coeff != 0 )
        {
          last++;
        }

      }

      ptr++;
      num_coeff--;
    }

    *last_ptr++ = last;
  }

  return ptr;
}

#ifndef HAS_DO_UNQUANTIZE
C_RESULT do_unquantize(int16_t* ptr, int32_t picture_type, int32_t quant, int32_t num_coeff)
{
  int32_t coeff;

  if (quant == TABLE_QUANTIZATION)
  {
	// table quantization mode
	int16_t* p_iquant_table = (int16_t*)(&iquant_tab[0]);
	do
	{
	  coeff = *ptr;
      if( coeff )
      {
        coeff *= (*p_iquant_table);

        *ptr = coeff;
        num_coeff--;
      }
      p_iquant_table++;
      ptr ++;
    } while( num_coeff > 0 );
  }
  else
  {
    // constant quantization mode
    if( picture_type == VIDEO_PICTURE_INTRA ) // intra
    {
      coeff = *ptr;
      *ptr  = (coeff << 3); // see III.3.2

      ptr ++;
      num_coeff --;
    }

    while( num_coeff > 0 )
    {
      coeff = *ptr;

      if( coeff )
      {
        coeff = quant*( 2*coeff + 1 );
        if( quant & 1 )
          coeff -= 1;

        *ptr = coeff;

        num_coeff--;
      }

      ptr ++;
    }
  }

  return C_OK;
}
#endif
static uint32_t num_references = 0;

C_RESULT video_utils_init( video_controller_t* controller )
{
  if( num_references == 0 )
  {
  }

  num_references ++;

  return C_OK;
}

C_RESULT video_utils_close( video_controller_t* controller )
{
  if( num_references > 0 )
  {
    num_references --;
  }

  return C_OK;
}

C_RESULT video_utils_set_format( uint32_t width, uint32_t height )
{
  return C_OK;
}
int32_t video_zztable_t81[MCU_BLOCK_SIZE] = {
  0,  1,   8, 16,  9,  2,  3, 10,
  17, 24, 32, 25, 18, 11,  4,  5,
  12, 19, 26, 33, 40, 48, 41, 34,
  27, 20, 13,  6,  7, 14, 21, 28,
  35, 42, 49, 56, 57, 50, 43, 36,
  29, 22, 15, 23, 30, 37, 44, 51,
  58, 59, 52, 45, 38, 31, 39, 46,
  53, 60, 61, 54, 47, 55, 62, 63,
};

#define FIX_0_298631336  ((INT32)  2446)	/* FIX(0.298631336) */
#define FIX_0_390180644  ((INT32)  3196)	/* FIX(0.390180644) */
#define FIX_0_541196100  ((INT32)  4433)	/* FIX(0.541196100) */
#define FIX_0_765366865  ((INT32)  6270)	/* FIX(0.765366865) */
#define FIX_0_899976223  ((INT32)  7373)	/* FIX(0.899976223) */
#define FIX_1_175875602  ((INT32)  9633)	/* FIX(1.175875602) */
#define FIX_1_501321110  ((INT32)  12299)	/* FIX(1.501321110) */
#define FIX_1_847759065  ((INT32)  15137)	/* FIX(1.847759065) */
#define FIX_1_961570560  ((INT32)  16069)	/* FIX(1.961570560) */
#define FIX_2_053119869  ((INT32)  16819)	/* FIX(2.053119869) */
#define FIX_2_562915447  ((INT32)  20995)	/* FIX(2.562915447) */
#define FIX_3_072711026  ((INT32)  25172)	/* FIX(3.072711026) */

#define INT32       int
#define DCTELEM     int
#define DCTSIZE     8
#define DCTSIZE2    64
#define CONST_BITS  13
#define PASS1_BITS  1
#define ONE	((INT32) 1)
#define MULTIPLY(var,const)  ((var) * (const))
#define DESCALE(x,n)  RIGHT_SHIFT((x) + (ONE << ((n)-1)), n)
#define RIGHT_SHIFT(x,shft)	((x) >> (shft))

#ifndef HAS_FDCT_COMPUTE
void fdct(const unsigned short* in, short* out)
{
  INT32 tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
  INT32 tmp10, tmp11, tmp12, tmp13;
  INT32 z1, z2, z3, z4, z5;
  int ctr;
  // SHIFT_TEMPS

  int data[DCTSIZE * DCTSIZE];
  int i, j;
  int* dataptr = data;

  for( i = 0; i < DCTSIZE; i++ )
  {
    for( j = 0; j < DCTSIZE; j++ )
    {
      int temp;

      temp = in[i*DCTSIZE + j];
      dataptr[i*DCTSIZE + j] = temp;
    }
  }

  /* Pass 1: process rows. */
  /* Note results are scaled up by sqrt(8) compared to a true DCT; */
  /* furthermore, we scale the results by 2**PASS1_BITS. */

  dataptr = data;
  for (ctr = DCTSIZE-1; ctr >= 0; ctr--) {
    tmp0 = dataptr[0] + dataptr[7];
    tmp7 = dataptr[0] - dataptr[7];
    tmp1 = dataptr[1] + dataptr[6];
    tmp6 = dataptr[1] - dataptr[6];
    tmp2 = dataptr[2] + dataptr[5];
    tmp5 = dataptr[2] - dataptr[5];
    tmp3 = dataptr[3] + dataptr[4];
    tmp4 = dataptr[3] - dataptr[4];

    /* Even part per LL&M figure 1 --- note that published figure is faulty;
     * rotator "sqrt(2)*c1" should be "sqrt(2)*c6".
     */

    tmp10 = tmp0 + tmp3;
    tmp13 = tmp0 - tmp3;
    tmp11 = tmp1 + tmp2;
    tmp12 = tmp1 - tmp2;

    dataptr[0] = (DCTELEM) ((tmp10 + tmp11) << PASS1_BITS);
    dataptr[4] = (DCTELEM) ((tmp10 - tmp11) << PASS1_BITS);

    z1 = MULTIPLY(tmp12 + tmp13, FIX_0_541196100);
    dataptr[2] = (DCTELEM) DESCALE(z1 + MULTIPLY(tmp13, FIX_0_765366865), CONST_BITS-PASS1_BITS);
    dataptr[6] = (DCTELEM) DESCALE(z1 + MULTIPLY(tmp12, - FIX_1_847759065), CONST_BITS-PASS1_BITS);

    /* Odd part per figure 8 --- note paper omits factor of sqrt(2).
     * cK represents cos(K*pi/16).
     * i0..i3 in the paper are tmp4..tmp7 here.
     */

    z1 = tmp4 + tmp7;
    z2 = tmp5 + tmp6;
    z3 = tmp4 + tmp6;
    z4 = tmp5 + tmp7;
    z5 = MULTIPLY(z3 + z4, FIX_1_175875602); /* sqrt(2) * c3 */

    tmp4 = MULTIPLY(tmp4, FIX_0_298631336); /* sqrt(2) * (-c1+c3+c5-c7) */
    tmp5 = MULTIPLY(tmp5, FIX_2_053119869); /* sqrt(2) * ( c1+c3-c5+c7) */
    tmp6 = MULTIPLY(tmp6, FIX_3_072711026); /* sqrt(2) * ( c1+c3+c5-c7) */
    tmp7 = MULTIPLY(tmp7, FIX_1_501321110); /* sqrt(2) * ( c1+c3-c5-c7) */
    z1 = MULTIPLY(z1, - FIX_0_899976223); /* sqrt(2) * (c7-c3) */
    z2 = MULTIPLY(z2, - FIX_2_562915447); /* sqrt(2) * (-c1-c3) */
    z3 = MULTIPLY(z3, - FIX_1_961570560); /* sqrt(2) * (-c3-c5) */
    z4 = MULTIPLY(z4, - FIX_0_390180644); /* sqrt(2) * (c5-c3) */

    z3 += z5;
    z4 += z5;

    dataptr[7] = (DCTELEM) DESCALE(tmp4 + z1 + z3, CONST_BITS-PASS1_BITS);
    dataptr[5] = (DCTELEM) DESCALE(tmp5 + z2 + z4, CONST_BITS-PASS1_BITS);
    dataptr[3] = (DCTELEM) DESCALE(tmp6 + z2 + z3, CONST_BITS-PASS1_BITS);
    dataptr[1] = (DCTELEM) DESCALE(tmp7 + z1 + z4, CONST_BITS-PASS1_BITS);

    dataptr += DCTSIZE;		/* advance pointer to next row */
  }

  /* Pass 2: process columns.
   * We remove the PASS1_BITS scaling, but leave the results scaled up
   * by an overall factor of 8.
   */

  dataptr = data;
  for (ctr = DCTSIZE-1; ctr >= 0; ctr--) {
    tmp0 = dataptr[DCTSIZE*0] + dataptr[DCTSIZE*7];
    tmp7 = dataptr[DCTSIZE*0] - dataptr[DCTSIZE*7];
    tmp1 = dataptr[DCTSIZE*1] + dataptr[DCTSIZE*6];
    tmp6 = dataptr[DCTSIZE*1] - dataptr[DCTSIZE*6];
    tmp2 = dataptr[DCTSIZE*2] + dataptr[DCTSIZE*5];
    tmp5 = dataptr[DCTSIZE*2] - dataptr[DCTSIZE*5];
    tmp3 = dataptr[DCTSIZE*3] + dataptr[DCTSIZE*4];
    tmp4 = dataptr[DCTSIZE*3] - dataptr[DCTSIZE*4];

    /* Even part per LL&M figure 1 --- note that published figure is faulty;
     * rotator "sqrt(2)*c1" should be "sqrt(2)*c6".
     */

    tmp10 = tmp0 + tmp3;
    tmp13 = tmp0 - tmp3;
    tmp11 = tmp1 + tmp2;
    tmp12 = tmp1 - tmp2;

    dataptr[DCTSIZE*0] = (DCTELEM) DESCALE(tmp10 + tmp11, PASS1_BITS);
    dataptr[DCTSIZE*4] = (DCTELEM) DESCALE(tmp10 - tmp11, PASS1_BITS);

    z1 = MULTIPLY(tmp12 + tmp13, FIX_0_541196100);
    dataptr[DCTSIZE*2] = (DCTELEM) DESCALE(z1 + MULTIPLY(tmp13, FIX_0_765366865), CONST_BITS+PASS1_BITS);
    dataptr[DCTSIZE*6] = (DCTELEM) DESCALE(z1 + MULTIPLY(tmp12, - FIX_1_847759065), CONST_BITS+PASS1_BITS);

    /* Odd part per figure 8 --- note paper omits factor of sqrt(2).
     * cK represents cos(K*pi/16).
     * i0..i3 in the paper are tmp4..tmp7 here.
     */

    z1 = tmp4 + tmp7;
    z2 = tmp5 + tmp6;
    z3 = tmp4 + tmp6;
    z4 = tmp5 + tmp7;
    z5 = MULTIPLY(z3 + z4, FIX_1_175875602); /* sqrt(2) * c3 */

    tmp4 = MULTIPLY(tmp4, FIX_0_298631336); /* sqrt(2) * (-c1+c3+c5-c7) */
    tmp5 = MULTIPLY(tmp5, FIX_2_053119869); /* sqrt(2) * ( c1+c3-c5+c7) */
    tmp6 = MULTIPLY(tmp6, FIX_3_072711026); /* sqrt(2) * ( c1+c3+c5-c7) */
    tmp7 = MULTIPLY(tmp7, FIX_1_501321110); /* sqrt(2) * ( c1+c3-c5-c7) */
    z1 = MULTIPLY(z1, - FIX_0_899976223); /* sqrt(2) * (c7-c3) */
    z2 = MULTIPLY(z2, - FIX_2_562915447); /* sqrt(2) * (-c1-c3) */
    z3 = MULTIPLY(z3, - FIX_1_961570560); /* sqrt(2) * (-c3-c5) */
    z4 = MULTIPLY(z4, - FIX_0_390180644); /* sqrt(2) * (c5-c3) */

    z3 += z5;
    z4 += z5;

    dataptr[DCTSIZE*7] = (DCTELEM) DESCALE(tmp4 + z1 + z3, CONST_BITS+PASS1_BITS);
    dataptr[DCTSIZE*5] = (DCTELEM) DESCALE(tmp5 + z2 + z4, CONST_BITS+PASS1_BITS);
    dataptr[DCTSIZE*3] = (DCTELEM) DESCALE(tmp6 + z2 + z3, CONST_BITS+PASS1_BITS);
    dataptr[DCTSIZE*1] = (DCTELEM) DESCALE(tmp7 + z1 + z4, CONST_BITS+PASS1_BITS);

    dataptr++;  /* advance pointer to next column */
  }

  for( i = 0; i < DCTSIZE; i++ )
    for( j = 0; j < DCTSIZE; j++ )
      out[i*DCTSIZE + j] = data[i*DCTSIZE + j] >> 3;
}
#endif // HAS_FDCT_COMPUTE

#ifndef HAS_IDCT_COMPUTE
void idct(const short* in, unsigned short* out)
{
  INT32 tmp0, tmp1, tmp2, tmp3;
  INT32 tmp10, tmp11, tmp12, tmp13;
  INT32 z1, z2, z3, z4, z5;
  int* wsptr;
  int* outptr;
  const short* inptr;
  int ctr;
  int workspace[DCTSIZE2];	/* buffers data between passes */
  int data[DCTSIZE2];
  // SHIFT_TEMPS

  /* Pass 1: process columns from input, store into work array. */
  /* Note results are scaled up by sqrt(8) compared to a true IDCT; */
  /* furthermore, we scale the results by 2**PASS1_BITS. */

  inptr = in;
  wsptr = workspace;
  for (ctr = DCTSIZE; ctr > 0; ctr--) {
    /* Due to quantization, we will usually find that many of the input
     * coefficients are zero, especially the AC terms.  We can exploit this
     * by short-circuiting the IDCT calculation for any column in which all
     * the AC terms are zero.  In that case each output is equal to the
     * DC coefficient (with scale factor as needed).
     * With typical images and quantization tables, half or more of the
     * column DCT calculations can be simplified this way.
     */

    if( inptr[DCTSIZE*1] == 0 && inptr[DCTSIZE*2] == 0 &&
        inptr[DCTSIZE*3] == 0 && inptr[DCTSIZE*4] == 0 &&
        inptr[DCTSIZE*5] == 0 && inptr[DCTSIZE*6] == 0 &&
        inptr[DCTSIZE*7] == 0 ) {
      /* AC terms all zero */
      int dcval = inptr[DCTSIZE*0] << PASS1_BITS;

      wsptr[DCTSIZE*0] = dcval;
      wsptr[DCTSIZE*1] = dcval;
      wsptr[DCTSIZE*2] = dcval;
      wsptr[DCTSIZE*3] = dcval;
      wsptr[DCTSIZE*4] = dcval;
      wsptr[DCTSIZE*5] = dcval;
      wsptr[DCTSIZE*6] = dcval;
      wsptr[DCTSIZE*7] = dcval;

      inptr++;  /* advance pointers to next column */
      wsptr++;
      continue;
    }

    /* Even part: reverse the even part of the forward DCT. */
    /* The rotator is sqrt(2)*c(-6). */

    z2 = inptr[DCTSIZE*2];
    z3 = inptr[DCTSIZE*6];

    z1 = MULTIPLY(z2 + z3, FIX_0_541196100);
    tmp2 = z1 + MULTIPLY(z3, - FIX_1_847759065);
    tmp3 = z1 + MULTIPLY(z2, FIX_0_765366865);

    z2 = inptr[DCTSIZE*0];
    z3 = inptr[DCTSIZE*4];

    tmp0 = (z2 + z3) << CONST_BITS;
    tmp1 = (z2 - z3) << CONST_BITS;

    tmp10 = tmp0 + tmp3;
    tmp13 = tmp0 - tmp3;
    tmp11 = tmp1 + tmp2;
    tmp12 = tmp1 - tmp2;

    /* Odd part per figure 8; the matrix is unitary and hence its
     * transpose is its inverse.  i0..i3 are y7,y5,y3,y1 respectively.
     */

    tmp0 = inptr[DCTSIZE*7];
    tmp1 = inptr[DCTSIZE*5];
    tmp2 = inptr[DCTSIZE*3];
    tmp3 = inptr[DCTSIZE*1];

    z1 = tmp0 + tmp3;
    z2 = tmp1 + tmp2;
    z3 = tmp0 + tmp2;
    z4 = tmp1 + tmp3;
    z5 = MULTIPLY(z3 + z4, FIX_1_175875602); /* sqrt(2) * c3 */

    tmp0 = MULTIPLY(tmp0, FIX_0_298631336); /* sqrt(2) * (-c1+c3+c5-c7) */
    tmp1 = MULTIPLY(tmp1, FIX_2_053119869); /* sqrt(2) * ( c1+c3-c5+c7) */
    tmp2 = MULTIPLY(tmp2, FIX_3_072711026); /* sqrt(2) * ( c1+c3+c5-c7) */
    tmp3 = MULTIPLY(tmp3, FIX_1_501321110); /* sqrt(2) * ( c1+c3-c5-c7) */
    z1 = MULTIPLY(z1, - FIX_0_899976223); /* sqrt(2) * (c7-c3) */
    z2 = MULTIPLY(z2, - FIX_2_562915447); /* sqrt(2) * (-c1-c3) */
    z3 = MULTIPLY(z3, - FIX_1_961570560); /* sqrt(2) * (-c3-c5) */
    z4 = MULTIPLY(z4, - FIX_0_390180644); /* sqrt(2) * (c5-c3) */

    z3 += z5;
    z4 += z5;

    tmp0 += z1 + z3;
    tmp1 += z2 + z4;
    tmp2 += z2 + z3;
    tmp3 += z1 + z4;

    /* Final output stage: inputs are tmp10..tmp13, tmp0..tmp3 */

    wsptr[DCTSIZE*0] = (int) DESCALE(tmp10 + tmp3, CONST_BITS-PASS1_BITS);
    wsptr[DCTSIZE*7] = (int) DESCALE(tmp10 - tmp3, CONST_BITS-PASS1_BITS);
    wsptr[DCTSIZE*1] = (int) DESCALE(tmp11 + tmp2, CONST_BITS-PASS1_BITS);
    wsptr[DCTSIZE*6] = (int) DESCALE(tmp11 - tmp2, CONST_BITS-PASS1_BITS);
    wsptr[DCTSIZE*2] = (int) DESCALE(tmp12 + tmp1, CONST_BITS-PASS1_BITS);
    wsptr[DCTSIZE*5] = (int) DESCALE(tmp12 - tmp1, CONST_BITS-PASS1_BITS);
    wsptr[DCTSIZE*3] = (int) DESCALE(tmp13 + tmp0, CONST_BITS-PASS1_BITS);
    wsptr[DCTSIZE*4] = (int) DESCALE(tmp13 - tmp0, CONST_BITS-PASS1_BITS);

    inptr++;  /* advance pointers to next column */
    wsptr++;
  }

  /* Pass 2: process rows from work array, store into output array. */
  /* Note that we must descale the results by a factor of 8 == 2**3, */
  /* and also undo the PASS1_BITS scaling. */

  wsptr = workspace;
  outptr = data;
  for (ctr = 0; ctr < DCTSIZE; ctr++) {
    /* Even part: reverse the even part of the forward DCT. */
    /* The rotator is sqrt(2)*c(-6). */

    z2 = (INT32) wsptr[2];
    z3 = (INT32) wsptr[6];

    z1 = MULTIPLY(z2 + z3, FIX_0_541196100);
    tmp2 = z1 + MULTIPLY(z3, - FIX_1_847759065);
    tmp3 = z1 + MULTIPLY(z2, FIX_0_765366865);

    tmp0 = ((INT32) wsptr[0] + (INT32) wsptr[4]) << CONST_BITS;
    tmp1 = ((INT32) wsptr[0] - (INT32) wsptr[4]) << CONST_BITS;

    tmp10 = tmp0 + tmp3;
    tmp13 = tmp0 - tmp3;
    tmp11 = tmp1 + tmp2;
    tmp12 = tmp1 - tmp2;

    /* Odd part per figure 8; the matrix is unitary and hence its
     * transpose is its inverse.  i0..i3 are y7,y5,y3,y1 respectively.
     */

    tmp0 = (INT32) wsptr[7];
    tmp1 = (INT32) wsptr[5];
    tmp2 = (INT32) wsptr[3];
    tmp3 = (INT32) wsptr[1];

    z1 = tmp0 + tmp3;
    z2 = tmp1 + tmp2;
    z3 = tmp0 + tmp2;
    z4 = tmp1 + tmp3;
    z5 = MULTIPLY(z3 + z4, FIX_1_175875602); /* sqrt(2) * c3 */

    tmp0 = MULTIPLY(tmp0, FIX_0_298631336); /* sqrt(2) * (-c1+c3+c5-c7) */
    tmp1 = MULTIPLY(tmp1, FIX_2_053119869); /* sqrt(2) * ( c1+c3-c5+c7) */
    tmp2 = MULTIPLY(tmp2, FIX_3_072711026); /* sqrt(2) * ( c1+c3+c5-c7) */
    tmp3 = MULTIPLY(tmp3, FIX_1_501321110); /* sqrt(2) * ( c1+c3-c5-c7) */
    z1 = MULTIPLY(z1, - FIX_0_899976223); /* sqrt(2) * (c7-c3) */
    z2 = MULTIPLY(z2, - FIX_2_562915447); /* sqrt(2) * (-c1-c3) */
    z3 = MULTIPLY(z3, - FIX_1_961570560); /* sqrt(2) * (-c3-c5) */
    z4 = MULTIPLY(z4, - FIX_0_390180644); /* sqrt(2) * (c5-c3) */

    z3 += z5;
    z4 += z5;

    tmp0 += z1 + z3;
    tmp1 += z2 + z4;
    tmp2 += z2 + z3;
    tmp3 += z1 + z4;

    /* Final output stage: inputs are tmp10..tmp13, tmp0..tmp3 */

    outptr[0] = (tmp10 + tmp3) >> ( CONST_BITS+PASS1_BITS+3 );
    outptr[7] = (tmp10 - tmp3) >> ( CONST_BITS+PASS1_BITS+3 );
    outptr[1] = (tmp11 + tmp2) >> ( CONST_BITS+PASS1_BITS+3 );
    outptr[6] = (tmp11 - tmp2) >> ( CONST_BITS+PASS1_BITS+3 );
    outptr[2] = (tmp12 + tmp1) >> ( CONST_BITS+PASS1_BITS+3 );
    outptr[5] = (tmp12 - tmp1) >> ( CONST_BITS+PASS1_BITS+3 );
    outptr[3] = (tmp13 + tmp0) >> ( CONST_BITS+PASS1_BITS+3 );
    outptr[4] = (tmp13 - tmp0) >> ( CONST_BITS+PASS1_BITS+3 );

    wsptr += DCTSIZE; /* advance pointer to next row */
    outptr += DCTSIZE;
  }

  for(ctr = 0; ctr < DCTSIZE2; ctr++)
    out[ctr] = data[ctr];
}
#endif // HAS_IDCT_COMPUTE

#ifndef HAS_FDCT_COMPUTE
int16_t* video_fdct_compute(int16_t* in, int16_t* out, int32_t num_macro_blocks)
{
  while( num_macro_blocks > 0 )
  {
    fdct((uint16_t*)in, out);

    in  += MCU_BLOCK_SIZE;
    out += MCU_BLOCK_SIZE;

    fdct((uint16_t*)in, out);

    in  += MCU_BLOCK_SIZE;
    out += MCU_BLOCK_SIZE;

    fdct((uint16_t*)in, out);

    in  += MCU_BLOCK_SIZE;
    out += MCU_BLOCK_SIZE;

    fdct((uint16_t*)in, out);

    in  += MCU_BLOCK_SIZE;
    out += MCU_BLOCK_SIZE;

    fdct((uint16_t*)in, out);

    in  += MCU_BLOCK_SIZE;
    out += MCU_BLOCK_SIZE;

    fdct((uint16_t*)in, out);

    in  += MCU_BLOCK_SIZE;
    out += MCU_BLOCK_SIZE;

    num_macro_blocks--;
  }

  return out;
}
#endif // HAS_FDCT_COMPUTE

#ifndef HAS_IDCT_COMPUTE
int16_t* video_idct_compute(int16_t* in, int16_t* out, int32_t num_macro_blocks)
{
  while( num_macro_blocks > 0 )
  {
    idct(in, (uint16_t*)out);

    in  += MCU_BLOCK_SIZE;
    out += MCU_BLOCK_SIZE;

    idct(in, (uint16_t*)out);

    in  += MCU_BLOCK_SIZE;
    out += MCU_BLOCK_SIZE;

    idct(in, (uint16_t*)out);

    in  += MCU_BLOCK_SIZE;
    out += MCU_BLOCK_SIZE;

    idct(in, (uint16_t*)out);

    in  += MCU_BLOCK_SIZE;
    out += MCU_BLOCK_SIZE;

    idct(in, (uint16_t*)out);

    in  += MCU_BLOCK_SIZE;
    out += MCU_BLOCK_SIZE;

    idct(in, (uint16_t*)out);

    in  += MCU_BLOCK_SIZE;
    out += MCU_BLOCK_SIZE;

    num_macro_blocks--;
  }

  return out;
}
#endif // HAS_IDCT_COMPUTE
