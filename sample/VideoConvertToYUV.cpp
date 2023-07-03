#include "pch.h"
#include "VideoConvertToYUV.h"
#include "RenderHelper.h"

VideoConvertToYUV::VideoConvertToYUV(VideoConvertSettings params) : original_video_info(params)
{
	assert((params.width % 2) == 0);
	assert((params.height % 2) == 0);
}

VideoConvertToYUV::~VideoConvertToYUV()
{
	UninitConvertion();
}

bool VideoConvertToYUV::InitConvertion()
{
	UninitConvertion();

	InitMatrix(original_video_info.color_range, original_video_info.color_space);

	if (!InitPlane()) {
		UninitConvertion();
		return false;
	}

	return true;
}

void VideoConvertToYUV::UninitConvertion()
{
	AUTO_GRAPHIC_CONTEXT(original_video_info.graphic);

	for (auto &item : video_plane_list) {
		if (item.canvas_tex)
			original_video_info.graphic->DestroyGraphicObject(item.canvas_tex);

		if (item.read_tex)
			original_video_info.graphic->DestroyGraphicObject(item.read_tex);
	}

	video_plane_list.clear();
}

bool VideoConvertToYUV::ConvertVideo(texture_handle src_tex)
{
	AUTO_GRAPHIC_CONTEXT(original_video_info.graphic);

	auto texInfo = original_video_info.graphic->GetTextureInfo(src_tex);
	if (texInfo.width != original_video_info.width || texInfo.height != original_video_info.height) {
		assert(false);
		return false;
	}

	std::vector<texture_handle> textures{src_tex};
	for (const auto &item : video_plane_list) {
		if (!original_video_info.graphic->BeginRenderCanvas(item.canvas_tex)) {
			assert(false);
			return false;
		}

		SIZE canvas(item.width, item.height);
		SIZE texSize(texInfo.width, texInfo.height);
		RECT drawDest(0, 0, item.width, item.height);

		XMMATRIX matrixWVP;
		TransposedOrthoMatrixWVP(canvas, true, nullptr, matrixWVP);

		TextureVertexDesc outputVertex[TEXTURE_VERTEX_COUNT];
		FillTextureVertex(0.f, 0.f, (float)item.width, (float)item.height, false, false, 0, 0, 0, 0,
				  outputVertex);

		original_video_info.graphic->SetVertexBuffer(item.shader, outputVertex, sizeof(outputVertex));

		original_video_info.graphic->SetVSConstBuffer(item.shader, &matrixWVP, sizeof(matrixWVP));

		original_video_info.graphic->SetPSConstBuffer(item.shader, &item.ps_const_buffer,
							      sizeof(ShaderConstBufferForToYUV));

		original_video_info.graphic->DrawTexture(item.shader, VIDEO_FILTER_TYPE::VIDEO_FILTER_POINT, textures);
		original_video_info.graphic->EndRender();

		// copy it to read video
		original_video_info.graphic->CopyTexture(item.read_tex, item.canvas_tex);
	}

	return true;
}

bool VideoConvertToYUV::MapConvertedYUV(void *output_data[AV_NUM_DATA_POINTERS],
					uint32_t output_linesize[AV_NUM_DATA_POINTERS])
{
	AUTO_GRAPHIC_CONTEXT(original_video_info.graphic);

	for (size_t i = 0; i < AV_NUM_DATA_POINTERS; i++) {
		output_data[i] = nullptr;
		output_linesize[i] = 0;
	}

	if (texutre_reading) {
		assert(false && "you are calling it repeatly! you must fix your code");
		return false;
	}

	bool map_failed = false;
	std::vector<texture_handle> maped_texs;

	int index = 0;
	for (const auto &item : video_plane_list) {
		D3D11_MAPPED_SUBRESOURCE info;
		if (original_video_info.graphic->MapTexture(item.read_tex, MAP_TEXTURE_FEATURE::FOR_READ_TEXTURE,
							    &info)) {
			maped_texs.push_back(item.read_tex);

			output_data[index] = info.pData;
			output_linesize[index] = info.RowPitch;

			++index;
		} else {
			map_failed = true;
			assert(false);
			break;
		}
	}

	if (map_failed) {
		for (auto &item : maped_texs)
			original_video_info.graphic->UnmapTexture(item);

		maped_texs.clear();
	}

	texutre_reading = !maped_texs.empty();
	return texutre_reading;
}

void VideoConvertToYUV::UnmapConvertedYUV()
{
	AUTO_GRAPHIC_CONTEXT(original_video_info.graphic);

	if (!texutre_reading) {
		assert(false && "invalid calling! you must fix your code");
		return;
	}

	texutre_reading = false;
	for (const auto &item : video_plane_list)
		original_video_info.graphic->UnmapTexture(item.read_tex);
}

void VideoConvertToYUV::InitMatrix(enum VIDEO_RANGE_TYPE color_range, enum VIDEO_COLOR_SPACE color_space)
{
	std::array<float, 16> color_matrix{};
	GetVideoMatrix(color_range, color_space, &color_matrix, nullptr, nullptr);
	VideoMatrixINV(color_matrix);

	// Y plane
	color_vec_y.x = color_matrix[4];
	color_vec_y.y = color_matrix[5];
	color_vec_y.z = color_matrix[6];
	color_vec_y.w = color_matrix[7];

	// U plane
	color_vec_u.x = color_matrix[0];
	color_vec_u.y = color_matrix[1];
	color_vec_u.z = color_matrix[2];
	color_vec_u.w = color_matrix[3];

	// V plane
	color_vec_v.x = color_matrix[8];
	color_vec_v.y = color_matrix[9];
	color_vec_v.z = color_matrix[10];
	color_vec_v.w = color_matrix[11];
}

bool VideoConvertToYUV::InitPlane()
{
	for (auto &item : video_plane_list)
		item = video_plane_info();

	switch (original_video_info.format) {
	case AV_PIX_FMT_YUV420P:
		SetPlanarI420();
		break;

	case AV_PIX_FMT_NV12:
		SetPlanarNV12();
		break;

	default:
		assert(false && "unsupported format");
		return false;
	}

	AUTO_GRAPHIC_CONTEXT(original_video_info.graphic);

	for (auto &item : video_plane_list) {
		if (item.width && item.height) {
			TextureInformation info;
			info.width = item.width;
			info.height = item.height;
			info.format = item.format;

			info.usage = TEXTURE_USAGE::CANVAS_TARGET;
			item.canvas_tex = original_video_info.graphic->CreateTexture(info);
			if (!item.canvas_tex) {
				assert(false);
				return false;
			}

			info.usage = TEXTURE_USAGE::READ_TEXTURE;
			item.read_tex = original_video_info.graphic->CreateTexture(info);
			if (!item.read_tex) {
				assert(false);
				return false;
			}
		}
	}

	return true;
}

void VideoConvertToYUV::SetPlanarI420()
{
	video_plane_list.push_back(video_plane_info());
	video_plane_list.push_back(video_plane_info());
	video_plane_list.push_back(video_plane_info());

	video_plane_list[0].width = original_video_info.width;
	video_plane_list[0].height = original_video_info.height;
	video_plane_list[0].format = DXGI_FORMAT_R8_UNORM;
	video_plane_list[0].shader = shaders[VIDEO_SHADER_TYPE::SHADER_TO_Y_PLANE];
	video_plane_list[0].ps_const_buffer.color_vec0 = color_vec_y;
	video_plane_list[0].expect_linesize = video_plane_list[0].width * 1;

	video_plane_list[1].width = (original_video_info.width + 1) / 2;
	video_plane_list[1].height = (original_video_info.height + 1) / 2;
	video_plane_list[1].format = DXGI_FORMAT_R8_UNORM;
	video_plane_list[1].shader = shaders[VIDEO_SHADER_TYPE::SHADER_TO_Y_PLANE];
	video_plane_list[1].ps_const_buffer.color_vec0 = color_vec_u;
	video_plane_list[1].expect_linesize = video_plane_list[1].width * 1;

	video_plane_list[2].width = (original_video_info.width + 1) / 2;
	video_plane_list[2].height = (original_video_info.height + 1) / 2;
	video_plane_list[2].format = DXGI_FORMAT_R8_UNORM;
	video_plane_list[2].shader = shaders[VIDEO_SHADER_TYPE::SHADER_TO_Y_PLANE];
	video_plane_list[2].ps_const_buffer.color_vec0 = color_vec_v;
	video_plane_list[2].expect_linesize = video_plane_list[2].width * 1;
}

void VideoConvertToYUV::SetPlanarNV12()
{
	video_plane_list.push_back(video_plane_info());
	video_plane_list.push_back(video_plane_info());

	video_plane_list[0].width = original_video_info.width;
	video_plane_list[0].height = original_video_info.height;
	video_plane_list[0].format = DXGI_FORMAT_R8_UNORM;
	video_plane_list[0].shader = shaders[VIDEO_SHADER_TYPE::SHADER_TO_Y_PLANE];
	video_plane_list[0].ps_const_buffer.color_vec0 = color_vec_y;
	video_plane_list[0].expect_linesize = video_plane_list[0].width * 1;

	video_plane_list[1].width = (original_video_info.width + 1) / 2;
	video_plane_list[1].height = (original_video_info.height + 1) / 2;
	video_plane_list[1].format = DXGI_FORMAT_R8G8_UNORM;
	video_plane_list[1].shader = shaders[VIDEO_SHADER_TYPE::SHADER_TO_UV_PLANE];
	video_plane_list[1].ps_const_buffer.color_vec0 = color_vec_u;
	video_plane_list[1].ps_const_buffer.color_vec1 = color_vec_v;
	video_plane_list[1].expect_linesize = video_plane_list[1].width * 2;
}
