#pragma once
#include "VideoConvertDefine.h"

class VideoConvertToYUV {
public:
	// Note: byte alignment as 16 bytes
	struct ShaderConstBufferForToYUV {
		DirectX::XMFLOAT4 color_vec0 = {};
		DirectX::XMFLOAT4 color_vec1 = {}; // this param may not be used for some case
	};

	VideoConvertToYUV(VideoConvertSettings params);
	virtual ~VideoConvertToYUV();

	bool InitConvertion();
	void UninitConvertion();

	bool ConvertVideo(texture_handle src_tex);

	bool MapConvertedYUV(void *output_data[AV_NUM_DATA_POINTERS],
			     uint32_t output_linesize[AV_NUM_DATA_POINTERS]);
	void UnmapConvertedYUV();

private:
	void InitMatrix(enum VIDEO_RANGE_TYPE color_range, enum VIDEO_COLOR_SPACE color_space);

	bool InitPlane();
	void SetPlanarI420();
	void SetPlanarNV12();

private:
	struct video_plane_info {
		uint32_t width = 0;
		uint32_t height = 0;
		enum DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;

		uint32_t expect_linesize = 0;
		uint32_t real_linesize = 0;

		texture_handle canvas_tex = 0;
		texture_handle read_tex = 0;

		shader_handle shader = nullptr; // does not need to free
		buffer_handle vertex_buf = nullptr;
		ShaderConstBufferForToYUV ps_const_buffer;
	};

	DirectX::XMFLOAT4 color_vec_y = {};
	DirectX::XMFLOAT4 color_vec_u = {};
	DirectX::XMFLOAT4 color_vec_v = {};

	const VideoConvertSettings original_video_info;
	std::vector<video_plane_info> video_plane_list;
	bool texutre_reading = false;
};
