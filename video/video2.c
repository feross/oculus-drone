#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "libavcodec/avcodec.h"
#include "libavutil/channel_layout.h"
#include "libavutil/common.h"
#include "libavutil/imgutils.h"
#include "libavutil/mathematics.h"
#include "libavutil/samplefmt.h"






void h264_encode_decode_(const char *filein, const char *fileout) {
	codecDecode = avcodec_find_decoder(CODEC_ID_H264);
	if (!codecDecode) {
		fprintf(stderr, "codec not found\n");
		exit(1);
	}

	ctxDecode= avcodec_alloc_context();
	avcodec_get_context_defaults(ctxDecode);
	ctxDecode->flags2 |= CODEC_FLAG2_FAST;
	ctxDecode->pix_fmt = PIX_FMT_YUV420P;
	ctxDecode->width = clip_width;
	ctxDecode->height = clip_height; 
	ctxDecode->dsp_mask = (FF_MM_MMX | FF_MM_MMXEXT | FF_MM_SSE);

	if (avcodec_open(ctxDecode, codecDecode) < 0) {
		fprintf(stderr, "could not open codec\n");
		exit(1);
	}

	pictureDecoded= avcodec_alloc_frame();
	avcodec_get_frame_defaults(pictureDecoded);
	pic_size = avpicture_get_size(PIX_FMT_YUV420P, clip_width, clip_height);

	decodedOut = (uint8_t *)malloc(pic_size);
	fout = fopen(fileout, "wb");
	if (!fout) {
		fprintf(stderr, "could not open %s\n", fileout);
		exit(1);
	}


	codecEncode = avcodec_find_encoder(CODEC_ID_H264);
	if (!codecEncode) {
		printf("codec not found\n");
		exit(1);
	}

	ctxEncode= avcodec_alloc_context();
	ctxEncode->coder_type = 0; // coder = 1
	ctxEncode->flags|=CODEC_FLAG_LOOP_FILTER; // flags=+loop
	ctxEncode->me_cmp|= 1; // cmp=+chroma, where CHROMA = 1
	ctxEncode->partitions|=X264_PART_I8X8+X264_PART_I4X4+X264_PART_P8X8+X264_PART_B8X8; // partitions=+parti8x8+parti4x4+partp8x8+partb8x8
	ctxEncode->me_method=ME_HEX; // me_method=hex
	ctxEncode->me_subpel_quality = 0; // subq=7
	ctxEncode->me_range = 16; // me_range=16
	ctxEncode->gop_size = 30*3; // g=250
	ctxEncode->keyint_min = 30; // keyint_min=25
	ctxEncode->scenechange_threshold = 40; // sc_threshold=40
	ctxEncode->i_quant_factor = 0.71; // i_qfactor=0.71
	ctxEncode->b_frame_strategy = 1; // b_strategy=1
	ctxEncode->qcompress = 0.6; // qcomp=0.6
	ctxEncode->qmin = 0; // qmin=10
	ctxEncode->qmax = 69; // qmax=51
	ctxEncode->max_qdiff = 4; // qdiff=4
	ctxEncode->max_b_frames = 3; // bf=3
	ctxEncode->refs = 3; // refs=3
	ctxEncode->directpred = 1; // directpred=1
	ctxEncode->trellis = 1; // trellis=1
	ctxEncode->flags2|=CODEC_FLAG2_FASTPSKIP; // flags2=+bpyramid+mixed_refs+wpred+dct8x8+fastpskip
	ctxEncode->weighted_p_pred = 0; // wpredp=2
	ctxEncode->bit_rate = 32000;
	ctxEncode->width = clip_width;
	ctxEncode->height = clip_height;
	ctxEncode->time_base.num = 1;
	ctxEncode->time_base.den = 30;
	ctxEncode->pix_fmt = PIX_FMT_YUV420P; 
	ctxEncode->dsp_mask = (FF_MM_MMX | FF_MM_MMXEXT | FF_MM_SSE);
	ctxEncode->rc_lookahead = 0;
	ctxEncode->max_b_frames = 0;
	ctxEncode->b_frame_strategy =1;
	ctxEncode->chromaoffset = 0;
	ctxEncode->thread_count =1;
	ctxEncode->bit_rate = (int)(128000.f * 0.80f);
	ctxEncode->bit_rate_tolerance = (int) (128000.f * 0.20f);
	ctxEncode->gop_size = 30*3; // Each 3 seconds

	/* open codec for encoder*/
	if (avcodec_open(ctxEncode, codecEncode) < 0) {
		printf("could not open codec\n");
		exit(1);
	}

	//open file to read
	fin = fopen(filein, "rb");
	if (!fin) {
		printf("could not open %s\n", filein);
		exit(1);
	}

	/* alloc image and output buffer for encoder*/
	pictureEncoded= avcodec_alloc_frame();
	avcodec_get_frame_defaults(pictureEncoded);

	//encoderOutSize = 100000;
	encoderOut = (uint8_t *)malloc(100000);
	//int size = ctxEncode->width * ctxEncode->height;
	picEncodeBuf = (uint8_t *)malloc(3*pic_size/2); /* size for YUV 420 */
	pictureEncoded->data[0] = picEncodeBuf;
	pictureEncoded->data[1] = pictureEncoded->data[0] + pic_size;
	pictureEncoded->data[2] = pictureEncoded->data[1] + pic_size / 4;
	pictureEncoded->linesize[0] = ctxEncode->width;
	pictureEncoded->linesize[1] = ctxEncode->width / 2;
	pictureEncoded->linesize[2] = ctxEncode->width / 2; 

	//encode and decode loop
	for(int i=0;i<30;i++) 
	{
		fflush(stdout);
	//read qcif 1 frame to buufer
		fread(pictureEncoded->data[0],ctxEncode->width * ctxEncode->height, 1, fin);
		fread(pictureEncoded->data[1],ctxEncode->width * ctxEncode->height/4, 1, fin);
		fread(pictureEncoded->data[2],ctxEncode->width * ctxEncode->height/4, 1, fin); 
		pictureEncoded->pts = AV_NOPTS_VALUE;

	/* encode frame */
		encoderOutSize = avcodec_encode_video(ctxEncode, encoderOut, 100000, pictureEncoded);
		printf("encoding frame %3d (size=%5d)\n", i, encoderOutSize);
		if(encoderOutSize <= 0)
			continue;

	//send encoderOut to decoder
		avpkt.size = encoderOutSize;
		avpkt.data = encoderOut;
	//decode frame
		len = avcodec_decode_video2(ctxDecode, pictureDecoded, &got_picture, &avpkt);
		if (len < 0) {
			printf("Error while decoding frame %d\n", frame);
			exit(1);
		}
		if (got_picture) {
			printf("len = %d saving frame %3d\n", len, frame);
			fflush(stdout);

			avpicture_layout((AVPicture *)pictureDecoded, ctxDecode->pix_fmt
				, clip_width, clip_height, decodedOut, pic_size);
			fwrite(decodedOut, pic_size, 1, fout);
			frame++;
		}
	}

	fclose(fout);
	fclose(fin);

	avcodec_close(ctxEncode);
	avcodec_close(ctxDecode);
	av_free(ctxEncode);
	av_free(ctxDecode);
	av_free(pictureEncoded); 
	av_free(pictureDecoded); 


}



int main(int argc, char **argv)
{

/* must be called before using avcodec lib */
	avcodec_init();

/* register all the codecs */
	avcodec_register_all();

	h264_encode_decode("Foreman.qcif","Decoded.yuv");

	return 0;
}