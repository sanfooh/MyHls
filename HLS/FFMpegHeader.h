#pragma once
extern "C"
{
#include "libavutil/opt.h"
#include "libavcodec/avcodec.h"
#include "libavutil/channel_layout.h"
#include "libavutil/common.h"
#include "libavutil/imgutils.h"
#include "libavutil/mathematics.h"
#include "libavutil/samplefmt.h"
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
#include "libavutil/audio_fifo.h"
#include "libavfilter/avfilter.h"
#include "libavfilter/avfiltergraph.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h""

}
#include <string>
#define snprintf _snprintf
using namespace std;
string PrintfFfmpegError(int err)
{
	char errorBuff[80];
	av_make_error_string(errorBuff, 80, err);
	return errorBuff;
	
}

void InitialFFMPEG()
{
	av_register_all();
	avformat_network_init();
	avfilter_register_all();
}

#define FFCHECK(ret) if(ret<0) \
InitialFFMPEG(ret);