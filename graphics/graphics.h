#ifndef H_LIB_CCD_GRAPHICS_H
#define H_LIB_CCD_GRAPHICS_H
#include "..\window.h"
#include "ffimport.h"

struct INetPacket {

};
class IMuxer {

};
struct AVCodec;
struct AVCodecContext;

class CEncoderVideo : public IMuxer {
private:

	AVCodec *codec;
	AVCodecContext *c;
	int srcFormat;

	SwsContext *conv;
	bool m_gotpacket;
	AVPacket pktn;


	std::vector<AVPacket*> table;
public:
	bool ok();

	bool encode_texture(ITexture *t);
	bool gotpacket();

	INetPacket output();

	bool init(int w, int h, int depth);
	int changerate(int bitratemin, int bitratemax);
	int resize(int w, int h);
	void setdepth(int pix_fmt);

	

	CEncoderVideo();
	~CEncoderVideo();
};
class CDecoderWindow {
private:
public:

};
#endif //H_LIB_CCD_GRAPHICS_H