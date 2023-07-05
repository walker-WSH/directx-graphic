#pragma once
#include <assert.h>
#include "lib-graphic/IGraphicDefine.h"
#include "VideoConvertColor.h"

extern "C" {
#include <libavcodec/avcodec.h>
}

using namespace graphic;
using namespace matrix;

struct VideoConvertSettings {
	IGraphicSession *graphic = nullptr;

	uint32_t width = 0;
	uint32_t height = 0;

	// for YUV->RGB, "format" means the video format of source YUV.
	// for RGB->YUV, "format" means the video format of dest YUV.
	enum AVPixelFormat format = AVPixelFormat::AV_PIX_FMT_NONE;

	enum VIDEO_RANGE_TYPE color_range = VIDEO_RANGE_TYPE::VIDEO_RANGE_PARTIAL;
	enum VIDEO_COLOR_SPACE color_space = VIDEO_COLOR_SPACE::VIDEO_CS_601;
};
