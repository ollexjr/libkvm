#include "graphics.h"
#include "ffimport.h"

#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avdevice.lib")
#pragma comment(lib, "avfilter.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "postproc.lib")
#pragma comment(lib, "swscale.lib")

int make_even(int val) {
	return (val % 2 == 0) ? val : (val - 1);
}

bool g_print_log = false;
void my_log_callback(void *ptr, int level, const char *fmt, va_list vargs){
	if (!g_print_log)
		return;
	vprintf(fmt, vargs);
}
void print_av_error(int code) {
	char buf[1024];
	av_strerror(code, buf, 1024);
	printf("av_strerror: error: %s\n", buf);
	//av_make_error_string(buf, 1024, code);
	//printf("avcodec: error: %s\n", buf);
}


void encode_outfile(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *outpacket, FILE *outfile) {

}
void encode_frame(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *dst) {
	int ret;

	/* send the frame to the encoder */
	ret = avcodec_send_frame(enc_ctx, frame);
	if (ret < 0) {
		fprintf(stderr, "error sending a frame for encoding\n");
		exit(1);
	}
	while (ret >= 0) {
		ret = avcodec_receive_packet(enc_ctx, dst);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			return; //needs more input
		else if (ret < 0) {
			fprintf(stderr, "error during encoding\n");
		}

		printf("encoded frame %3 (size=%5d)\n", dst->pts, dst->size);
		//fwrite(pkt->data, 1, pkt->size, outfile);
		av_packet_unref(dst);
	}
}

bool CEncoderVideo::ok()
{
	return false;
}

bool CEncoderVideo::encode_texture(ITexture * tex) {


	AVFrame* rgbFrame = av_frame_alloc();
	switch (tex->format) {
	case 4:
		rgbFrame->format = AV_PIX_FMT_RGB32;
		break;
	case 3:
		rgbFrame->format = AV_PIX_FMT_RGB24;
		break;
	}
	rgbFrame->width = tex->width;
	rgbFrame->height = tex->height;

	AVFrame* yuvFrame = av_frame_alloc();
	yuvFrame->format = c->pix_fmt;
	yuvFrame->width = c->width;
	yuvFrame->height = c->height;

	//int num_bytes = av_image_get_buffer_size(AV_PIX_FMT_RGB32, tex->width, tex->height, 32);
	//printf("Bytes: %d\n", num_bytes);
	//_pictureFrameData = (uint8_t*)av_malloc(num_bytes);
	//int numBytes = av_image_alloc(picture->data, picture->linesize, tex->width, tex->height, AV_PIX_FMT_RGB24, 32);
	//tag the arrays
	//memcpy(tex->buffer, "DEADBEEF", strlen("DEADBEEF"));
	/* Init frame data pointing to existing texture buffer */

	int ret = 0;
	int numBytes = av_image_fill_arrays(
		rgbFrame->data, rgbFrame->linesize,
		tex->buffer, //????
		AV_PIX_FMT_RGB32,//AV_PIX_FMT_RGB32, //source format
		tex->width, tex->height, //source width and height
		1); //source image alignment
	if (ret < 0) {
		av_frame_free(&rgbFrame);
		print_av_error(ret);
		return false;
	}

	//alloc destination?
	ret = av_image_alloc(yuvFrame->data, yuvFrame->linesize, c->width, c->height, c->pix_fmt, 32);
	if (ret < 0) {
		av_frame_free(&rgbFrame);
		print_av_error(ret);
		return false;
	}

	//convert RGB to YUV
	ret = sws_scale(
		this->conv,
		rgbFrame->data,
		rgbFrame->linesize,

		0,
		tex->height,

		yuvFrame->data,
		yuvFrame->linesize);

	//ret == slice height
	if (ret < 0) {
		av_frame_free(&rgbFrame);
		av_frame_free(&yuvFrame);
		print_av_error(ret);
		return false;
	}
	//printf("NewFrame: %dy/%dy\n", ret, rgbFrame->height);


	m_gotpacket = false;

	//av_packet_unref(&pktn);
	//av_packet_unref(&pktn);
	av_init_packet(&pktn);
	//avcodec_send_frame(this->c, yuvFrame);

	ret = avcodec_send_frame(this->c, yuvFrame);
	if (ret < 0) {
		//fprintf(stderr, "Error sending a frame for encoding\n");
		av_frame_free(&rgbFrame);
		av_frame_free(&yuvFrame);
		print_av_error(ret);
		printf("error sending frame\n");
		return false;
	}
	
	printf("trying to get packet...");
	//overwrite stored packet
	while (ret >= 0) { 
		ret = avcodec_receive_packet(this->c, &this->pktn);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
			av_frame_free(&yuvFrame);
			av_frame_free(&rgbFrame);
			fprintf(stderr, "nothing to get\n");
			return false; //needs more input
		}
		else if (ret < 0) {
			av_frame_free(&yuvFrame);
			av_frame_free(&rgbFrame);
			fprintf(stderr, "error during encoding\n");
			return false;
		}
		m_gotpacket = true;
		printf("[encoded] frame %3 (size=%5d)...", pktn.pts, pktn.size);
		av_packet_unref(&pktn);
		//fwrite(pkt->data, 1, pkt->size, outfile);
	}
	printf("error?\n");
	av_frame_free(&yuvFrame);
	av_frame_free(&rgbFrame);
}

bool CEncoderVideo::gotpacket(){
	return this->m_gotpacket;
}

//avcodec_receive_packet();
//avpicture_fill((AVPicture*)picture, tex->buffer, AV_PIX_FMT_RGB32, tex->width, tex->height);
//memcpy(picture->data, t->buffer, t->width * t->height * t->format);
//picture->data = 
//ret = avcodec_receive_packet(this->c, output);

//ret = av_image_alloc(picture->data, picture->linesize, c->width, c->height, c->pix_fmt, 0);
//if (ret < 0)
//{
//	printf("Could not allocate raw picture buffer\n");
//	return;
//}

// ret = av_frame_get_buffer(picture, 0);
//if (ret < 0) {
//	printf("CEncoder -> could not alloc the frame data\n");
//	return;
//}


//avcodec_fill_audio_frame()
//avpicture_fill();
//swsContext = sws_getContext(WIDTH, HEIGHT, AV_PIX_FMT_RGBA, WIDTH, HEIGHT, AV_PIX_FMT_YUV, SWS_BICUBIC, 0, 0, 0);
//ret = av_frame_make_writable(picture);
//if (ret < 0) {
//	print_av_error(ret);
//	return;
//}

INetPacket CEncoderVideo::output()
{
	return INetPacket();
}

int CEncoderVideo::resize(int w, int h)
{
	//if (!this->c) {
	//	this->init(w, h, this->setdepth);
	//}

	if (this->c) {
		this->c->width = w;
		this->c->height = h;
	}
	return true;
}

void CEncoderVideo::setdepth(int pix_fmt)
{
	this->srcFormat = pix_fmt;
}

void codec_set_option(AVCodecContext * av, const char * name, const char * value) {
	int ret = av_opt_set(av, name, value, 0);
	if (ret < 0) {
		printf("Failed to set: %s\n\t", name);
		print_av_error(ret);
	}
}

bool CEncoderVideo::init(int w, int h, int depth){
	w = make_even(w);
	h = make_even(h);

	this->srcFormat = depth;

	//av_log_set_level(AV_LOG_DEBUG);
	av_log_set_level(AV_LOG_DEBUG);
	av_log_set_callback(my_log_callback);
	//avcodec_register(this->codec);
	/* get the libvpx encoder */
	//AV_CODEC_ID_MPEG4

	this->c = NULL;

	/*
	codec = avcodec_find_encoder_by_name("libvpx");
	if (!codec) {
		fprintf(stderr, "Codec '%s' not found\n", "libvpx");
		return false;
	}/**/

	//this->codec = avcodec_find_encoder(AV_CODEC_ID_VP9);
	this->codec = avcodec_find_encoder(AV_CODEC_ID_H264);
	if (!codec) {
		printf("codec not found\n");
		return false;
	}

	this->c = avcodec_alloc_context3(this->codec);
	if (!c) {
		fprintf(stderr, "Could not allocate video codec context\n");
		return false;
	}

	c->bit_rate = 400000;
	c->width = w;
	c->height = h;

	c->time_base = { 1, 25 };
	c->framerate = { 25, 1 };
	c->gop_size = 10;
	c->max_b_frames = 1;
	c->pix_fmt = AV_PIX_FMT_YUV420P; //YUVA 4:2:0p


	codec_set_option(c, "deadline", "realtime");
	codec_set_option(c, "b", "1M");
	codec_set_option(c, "minrate", "1M");
	codec_set_option(c, "maxrate", "1M");


	avcodec_open2(this->c, this->codec, NULL);
	if (!c) {
		fprintf(stderr, "Could not OPEN video codec context\n");
		return false;
	}


	//av_set_options_string(this->c, "quality", ")
	//AV_PIX_FMT_YUVA420P
	this->conv = sws_getContext(
		w, h,
		AV_PIX_FMT_RGB32,
		w, h,
		c->pix_fmt,
		SWS_BICUBIC,
		NULL, NULL, NULL);
}

CEncoderVideo::CEncoderVideo() {
	//this->init();
}

CEncoderVideo::~CEncoderVideo()
{
	avcodec_free_context(&this->c);
}
