#pragma once
#include "lib-graphic/IGraphicDefine.h"
#include <DirectXMath.h>
#include <array>
#include <vector>

#define SWAP_VALUE(x, y)       \
	{                      \
		auto temp = x; \
		x = y;         \
		y = temp;      \
	}

namespace matrix {
enum class VIDEO_RANGE_TYPE {
	VIDEO_RANGE_FULL,
	VIDEO_RANGE_PARTIAL,
};

enum class VIDEO_COLOR_SPACE {
	VIDEO_CS_601,
	VIDEO_CS_709,
};

struct VideoConvertionMatrix {
	enum VIDEO_COLOR_SPACE const color_space;

	std::array<float, 3> float_range_min;
	std::array<float, 3> float_range_max;

	// index.0 is for partial color range while index.1 is for full range
	std::array<std::array<float, 16>, 2> matrix;
};

const std::array<VideoConvertionMatrix, 2> format_info = {{
	{VIDEO_COLOR_SPACE::VIDEO_CS_601,
	 {16.0f / 255.0f, 16.0f / 255.0f, 16.0f / 255.0f},
	 {235.0f / 255.0f, 240.0f / 255.0f, 240.0f / 255.0f},
	 {{{1.164384f, 0.000000f, 1.596027f, -0.874202f, 1.164384f, -0.391762f, -0.812968f,
	    0.531668f, 1.164384f, 2.017232f, 0.000000f, -1.085631f, 0.000000f, 0.000000f, 0.000000f,
	    1.000000f},
	   {1.000000f, 0.000000f, 1.407520f, -0.706520f, 1.000000f, -0.345491f, -0.716948f,
	    0.533303f, 1.000000f, 1.778976f, 0.000000f, -0.892976f, 0.000000f, 0.000000f, 0.000000f,
	    1.000000f}}}},

	{VIDEO_COLOR_SPACE::VIDEO_CS_709,
	 {16.0f / 255.0f, 16.0f / 255.0f, 16.0f / 255.0f},
	 {235.0f / 255.0f, 240.0f / 255.0f, 240.0f / 255.0f},
	 {{{1.164384f, 0.000000f, 1.792741f, -0.972945f, 1.164384f, -0.213249f, -0.532909f,
	    0.301483f, 1.164384f, 2.112402f, 0.000000f, -1.133402f, 0.000000f, 0.000000f, 0.000000f,
	    1.000000f},
	   {1.000000f, 0.000000f, 1.581000f, -0.793600f, 1.000000f, -0.188062f, -0.469967f,
	    0.330305f, 1.000000f, 1.862906f, 0.000000f, -0.935106f, 0.000000f, 0.000000f, 0.000000f,
	    1.000000f}}}},
}};

/*
color_matrix: input/output, its for yuv->rgb
color_range_min: it should be null while getting matrix for rgb->yuv
color_range_max: it should be null while getting matrix for rgb->yuv
*/
bool GetVideoMatrix(enum VIDEO_RANGE_TYPE color_range, enum VIDEO_COLOR_SPACE color_space,
		    std::array<float, 16> *color_matrix, std::array<float, 3> *color_range_min,
		    std::array<float, 3> *color_range_max);

/*
color_matrix: input/output, it is returned by GetVideoMatrix
after handled by this function, color_matrix is for rgb->yuv
--------------------------------
00 01 02 03 // for U plane
10 11 12 13 // for Y plane
20 21 22 23 // for V plane
30 31 32 33
--------------------------------
*/
void VideoMatrixINV(std::array<float, 16> &color_matrix);

} // namespace matrix
