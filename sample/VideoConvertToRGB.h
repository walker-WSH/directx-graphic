#pragma once
#include "VideoConvertDefine.h"

class VideoConvertToRGB {
public:
	// Note: byte alignment as 16 bytes
	struct ShaderConstBufferForToRGB {
		DirectX::XMFLOAT4 color_vec0 = {};
		DirectX::XMFLOAT4 color_vec1 = {};
		DirectX::XMFLOAT4 color_vec2 = {};
		DirectX::XMFLOAT4 color_range_min = {}; //only x,y,z valid
		DirectX::XMFLOAT4 color_range_max = {}; //only x,y,z valid

		float width = 0.0;
		float height = 0.0;
		float half_width = 0.0;
		uint32_t full_range = 0;
	};

	VideoConvertToRGB(VideoConvertSettings params);
	virtual ~VideoConvertToRGB();

	bool InitConvertion();
	void UninitConvertion();

	void ConvertVideo(const AVFrame *src_frame, SIZE canvas, RECT dest);

private:
	void UpdateVideo(const AVFrame *av_frame);
	std::vector<texture_handle> GetTextures();

	void InitMatrix(enum VIDEO_RANGE_TYPE color_range, enum VIDEO_COLOR_SPACE color_space);

	bool InitPlane();
	void SetPlanarI420();
	void SetPlanarNV12();
	void SetPacked422Info(enum AVPixelFormat fmt);

private:
	struct video_plane_info {
		uint32_t width = 0;
		uint32_t height = 0;
		enum DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
		texture_handle texture = 0;
	};

	const VideoConvertSettings original_video_info;
	shader_handle convert_shader = 0; // does not need to free
	std::vector<video_plane_info> video_plane_list;
	ShaderConstBufferForToRGB ps_const_buffer;
};
