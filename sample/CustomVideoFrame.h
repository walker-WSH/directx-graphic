#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/pixdesc.h>
#include <libavutil/hwcontext.h>
#include <libavutil/opt.h>
#include <libavutil/avassert.h>
#include <libavutil/imgutils.h>
#include <libavutil/parseutils.h>
#include <libswscale/swscale.h>
}

// RGB -> YUV
static enum AVPixelFormat TEST_TO_YUV_FORMAT = AVPixelFormat::AV_PIX_FMT_YUV420P;
//static enum AVPixelFormat TEST_TO_YUV_FORMAT = AVPixelFormat::AV_PIX_FMT_NV12;

// YUV -> RGB
//static enum AVPixelFormat TEST_RENDER_YUV_FORMAT = AVPixelFormat::AV_PIX_FMT_YUV420P;
//static enum AVPixelFormat TEST_RENDER_YUV_FORMAT = AVPixelFormat::AV_PIX_FMT_NV12;
//static enum AVPixelFormat TEST_RENDER_YUV_FORMAT = AVPixelFormat::AV_PIX_FMT_YUYV422;
static enum AVPixelFormat TEST_RENDER_YUV_FORMAT = AVPixelFormat::AV_PIX_FMT_UYVY422;

int open_file();
void close_file();

AVFrame *decode_frame();

bool SaveBitmapFile(const wchar_t *path, const uint8_t *data, int linesize, int width, int height,
		    int pixelSize, bool flip);

void SaveI420(uint32_t width, uint32_t height, void *data[AV_NUM_DATA_POINTERS],
	      uint32_t linesize[AV_NUM_DATA_POINTERS]);
void SaveNV12(uint32_t width, uint32_t height, void *data[AV_NUM_DATA_POINTERS],
	      uint32_t linesize[AV_NUM_DATA_POINTERS]);
