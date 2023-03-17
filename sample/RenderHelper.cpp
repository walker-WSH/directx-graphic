#include "pch.h"
#include "RenderHelper.h"
#include "VideoConvertToRGB.h"
#include "VideoConvertToYUV.h"
#include "IGraphicSession.h"

IGraphicSession *pGraphic = nullptr;
std::map<VIDEO_SHADER_TYPE, shader_handle> shaders;

std::wstring GetShaderDirectory();
void InitShader()
{
	float matrixWVP[4][4];
	ShaderInformation shaderInfo;

	VertexInputDesc desc;
	desc.type = VERTEX_INPUT_TYPE::SV_POSITION;
	desc.size = 16;
	shaderInfo.vertexDesc.push_back(desc);

	desc.type = VERTEX_INPUT_TYPE::TEXTURE_COORD;
	desc.size = 8;
	shaderInfo.vertexDesc.push_back(desc);

	std::wstring dir = GetShaderDirectory();

	{
		shaderInfo.vsFile = dir + L"default-vs.cso";
		shaderInfo.psFile = dir + L"default-ps.cso";
		shaderInfo.vsBufferSize = sizeof(matrixWVP);
		shaderInfo.psBufferSize = 0;
		shaderInfo.vertexCount = TEXTURE_VERTEX_COUNT;
		shaderInfo.perVertexSize = sizeof(TextureVertexDesc);
		shader_handle shader = pGraphic->CreateShader(shaderInfo);
		assert(shader);
		shaders[VIDEO_SHADER_TYPE::SHADER_TEXTURE] = shader;
	}

	{
		shaderInfo.vsFile = dir + L"default-vs.cso";
		shaderInfo.psFile = dir + L"mosaic-ps.cso";
		shaderInfo.vsBufferSize = sizeof(matrixWVP);
		shaderInfo.psBufferSize = sizeof(MosaicParam);
		shaderInfo.vertexCount = TEXTURE_VERTEX_COUNT;
		shaderInfo.perVertexSize = sizeof(TextureVertexDesc);
		shader_handle shader = pGraphic->CreateShader(shaderInfo);
		assert(shader);
		shaders[VIDEO_SHADER_TYPE::SHADER_TEXTURE_MOSAIC] = shader;
	}

	{
		shaderInfo.vsFile = dir + L"default-vs.cso";
		shaderInfo.psFile = dir + L"mosaic-subregion-ps.cso";
		shaderInfo.vsBufferSize = sizeof(matrixWVP);
		shaderInfo.psBufferSize = sizeof(MosaicParam);
		shaderInfo.vertexCount = TEXTURE_VERTEX_COUNT;
		shaderInfo.perVertexSize = sizeof(TextureVertexDesc);
		shader_handle shader = pGraphic->CreateShader(shaderInfo);
		assert(shader);
		shaders[VIDEO_SHADER_TYPE::SHADER_TEXTURE_MOSAIC_SUB] = shader;
	}

	{
		shaderInfo.vsFile = dir + L"default-vs.cso";
		shaderInfo.psFile = dir + L"bulge-ps.cso";
		shaderInfo.vsBufferSize = sizeof(matrixWVP);
		shaderInfo.psBufferSize = sizeof(BulgeParam);
		shaderInfo.vertexCount = TEXTURE_VERTEX_COUNT;
		shaderInfo.perVertexSize = sizeof(TextureVertexDesc);
		shader_handle shader = pGraphic->CreateShader(shaderInfo);
		assert(shader);
		shaders[VIDEO_SHADER_TYPE::SHADER_TEXTURE_BULGE] = shader;
	}

	{
		ShaderInformation shaderInfo;

		VertexInputDesc desc;
		desc.type = VERTEX_INPUT_TYPE::SV_POSITION;
		desc.size = 16;
		shaderInfo.vertexDesc.push_back(desc);

		shaderInfo.vsFile = dir + L"fill-rect-vs.cso";
		shaderInfo.psFile = dir + L"fill-rect-ps.cso";
		shaderInfo.vsBufferSize = sizeof(matrixWVP);
		shaderInfo.psBufferSize = sizeof(ColorRGBA);
		shaderInfo.vertexCount = TEXTURE_VERTEX_COUNT;
		shaderInfo.perVertexSize = sizeof(ColorVertexDesc);
		shader_handle shader = pGraphic->CreateShader(shaderInfo);
		assert(shader);
		shaders[VIDEO_SHADER_TYPE::SHADER_FILL_RECT] = shader;
	}

	{
		shaderInfo.vsFile = dir + L"default-vs.cso";
		shaderInfo.psFile = dir + L"ps-rgb-to-y.cso";
		shaderInfo.vsBufferSize = sizeof(matrixWVP);
		shaderInfo.psBufferSize = sizeof(VideoConvertToYUV::ShaderConstBufferForToYUV);
		shaderInfo.vertexCount = TEXTURE_VERTEX_COUNT;
		shaderInfo.perVertexSize = sizeof(TextureVertexDesc);
		shader_handle shader = pGraphic->CreateShader(shaderInfo);
		assert(shader);
		shaders[VIDEO_SHADER_TYPE::SHADER_TO_Y_PLANE] = shader;
	}

	{
		shaderInfo.vsFile = dir + L"default-vs.cso";
		shaderInfo.psFile = dir + L"ps-rgb-to-uv.cso";
		shaderInfo.vsBufferSize = sizeof(matrixWVP);
		shaderInfo.psBufferSize = sizeof(VideoConvertToYUV::ShaderConstBufferForToYUV);
		shaderInfo.vertexCount = TEXTURE_VERTEX_COUNT;
		shaderInfo.perVertexSize = sizeof(TextureVertexDesc);
		shader_handle shader = pGraphic->CreateShader(shaderInfo);
		assert(shader);
		shaders[VIDEO_SHADER_TYPE::SHADER_TO_UV_PLANE] = shader;
	}

	{
		shaderInfo.vsFile = dir + L"default-vs.cso";
		shaderInfo.psFile = dir + L"ps-to-rgb-i420.cso";
		shaderInfo.vsBufferSize = sizeof(matrixWVP);
		shaderInfo.psBufferSize = sizeof(VideoConvertToRGB::ShaderConstBufferForToRGB);
		shaderInfo.vertexCount = TEXTURE_VERTEX_COUNT;
		shaderInfo.perVertexSize = sizeof(TextureVertexDesc);
		shader_handle shader = pGraphic->CreateShader(shaderInfo);
		assert(shader);
		shaders[VIDEO_SHADER_TYPE::SHADER_I420_TO_RGB] = shader;
	}

	{
		shaderInfo.vsFile = dir + L"default-vs.cso";
		shaderInfo.psFile = dir + L"ps-to-rgb-nv12.cso";
		shaderInfo.vsBufferSize = sizeof(matrixWVP);
		shaderInfo.psBufferSize = sizeof(VideoConvertToRGB::ShaderConstBufferForToRGB);
		shaderInfo.vertexCount = TEXTURE_VERTEX_COUNT;
		shaderInfo.perVertexSize = sizeof(TextureVertexDesc);
		shader_handle shader = pGraphic->CreateShader(shaderInfo);
		assert(shader);
		shaders[VIDEO_SHADER_TYPE::SHADER_NV12_TO_RGB] = shader;
	}

	{
		shaderInfo.vsFile = dir + L"default-vs.cso";
		shaderInfo.psFile = dir + L"ps-to-rgb-yuy2.cso";
		shaderInfo.vsBufferSize = sizeof(matrixWVP);
		shaderInfo.psBufferSize = sizeof(VideoConvertToRGB::ShaderConstBufferForToRGB);
		shaderInfo.vertexCount = TEXTURE_VERTEX_COUNT;
		shaderInfo.perVertexSize = sizeof(TextureVertexDesc);
		shader_handle shader = pGraphic->CreateShader(shaderInfo);
		assert(shader);
		shaders[VIDEO_SHADER_TYPE::SHADER_YUYV_TO_RGB] = shader;
	}

	{
		shaderInfo.vsFile = dir + L"default-vs.cso";
		shaderInfo.psFile = dir + L"ps-to-rgb-uyvy.cso";
		shaderInfo.vsBufferSize = sizeof(matrixWVP);
		shaderInfo.psBufferSize = sizeof(VideoConvertToRGB::ShaderConstBufferForToRGB);
		shaderInfo.vertexCount = TEXTURE_VERTEX_COUNT;
		shaderInfo.perVertexSize = sizeof(TextureVertexDesc);
		shader_handle shader = pGraphic->CreateShader(shaderInfo);
		assert(shader);
		shaders[VIDEO_SHADER_TYPE::SHADER_UYVY_TO_RGB] = shader;
	}
}

void FillTextureVertex(float left, float top, float right, float bottom, bool flipH, bool flipV, float cropLeft,
		       float cropTop, float cropRight, float cropBtm,
		       TextureVertexDesc outputVertex[TEXTURE_VERTEX_COUNT])
{
	// 纹理采样的区域
	float leftUV = 0.f + cropLeft;
	float topUV = 0.f + cropTop;
	float rightUV = 1.f - cropRight;
	float bottomUV = 1.f - cropBtm;

	if (flipH)
		SWAP_VALUE(leftUV, rightUV);

	if (flipV)
		SWAP_VALUE(topUV, bottomUV);

	outputVertex[0] = {left, top, 0, 1.f, leftUV, topUV};
	outputVertex[1] = {right, top, 0, 1.f, rightUV, topUV};
	outputVertex[2] = {left, bottom, 0, 1.f, leftUV, bottomUV};
	outputVertex[3] = {right, bottom, 0, 1.f, rightUV, bottomUV};
}

void FillColorVertex(float left, float top, float right, float bottom,
		     ColorVertexDesc outputVertex[TEXTURE_VERTEX_COUNT])
{
	outputVertex[0] = {left, top, 0, 1.f};
	outputVertex[1] = {right, top, 0, 1.f};
	outputVertex[2] = {left, bottom, 0, 1.f};
	outputVertex[3] = {right, bottom, 0, 1.f};
}

extern bool bFullscreenCrop;
void RenderTexture(std::vector<texture_handle> texs, SIZE canvas, RECT drawDest, VIDEO_SHADER_TYPE shaderType,
		   const MosaicParam *mosaic)
{
	AUTO_GRAPHIC_CONTEXT(pGraphic);

	TextureInformation texInfo = pGraphic->GetTextureInfo(texs.at(0));
	shader_handle shader = shaders[shaderType];

	RECT realDrawDest = drawDest;
	float cropL = 0;
	float cropT = 0;
	float cropR = 0;
	float cropB = 0;
	if (!bFullscreenCrop) // fit to screen
	{
		// 根据图片等比例缩放 确认实际的渲染区域
		auto cx = drawDest.right - drawDest.left;
		auto cy = drawDest.bottom - drawDest.top;
		float wndRadio = float(cx) / float(cy);
		float frameRadio = float(texInfo.width) / float(texInfo.height);

		// 确认是按照宽度缩放 还是按照高度缩放
		if (wndRadio > frameRadio) {
			float radio = float(cy) / float(texInfo.height);
			auto destCx = radio * texInfo.width;

			realDrawDest.left += ((drawDest.right - drawDest.left) - (LONG)destCx) / 2;
			realDrawDest.right = realDrawDest.left + (LONG)destCx;
		} else {
			float radio = float(cx) / float(texInfo.width);
			auto destCy = radio * texInfo.height;

			realDrawDest.top += ((drawDest.bottom - drawDest.top) - (LONG)destCy) / 2;
			realDrawDest.bottom = realDrawDest.top + (LONG)destCy;
		}
	} else { // crop to ensure fullscreen

		// 根据图片等比例缩放 确认实际的渲染区域
		auto cx = drawDest.right - drawDest.left;
		auto cy = drawDest.bottom - drawDest.top;
		float wndRadio = float(cx) / float(cy);
		float frameRadio = float(texInfo.width) / float(texInfo.height);

		// 确认是按照宽度缩放 还是按照高度缩放
		if (wndRadio < frameRadio) {
			float radio = float(cy) / float(texInfo.height);
			auto destCx = radio * texInfo.width;

			realDrawDest.left += ((drawDest.right - drawDest.left) - (LONG)destCx) / 2;
			realDrawDest.right = realDrawDest.left + (LONG)destCx;

			cropL = cropR = float(abs(realDrawDest.left - drawDest.left)) /
					float(realDrawDest.right - realDrawDest.left);
		} else {
			float radio = float(cx) / float(texInfo.width);
			auto destCy = radio * texInfo.height;

			realDrawDest.top += ((drawDest.bottom - drawDest.top) - (LONG)destCy) / 2;
			realDrawDest.bottom = realDrawDest.top + (LONG)destCy;

			cropT = cropB = float(abs(realDrawDest.top - drawDest.top)) /
					float(realDrawDest.bottom - realDrawDest.top);
		}

		realDrawDest = drawDest;
	}

	float matrixWVP[4][4];
	TransposeMatrixWVP(canvas, true, WorldDesc(), matrixWVP);

	TextureVertexDesc outputVertex[TEXTURE_VERTEX_COUNT];
	FillTextureVertex((float)realDrawDest.left, (float)realDrawDest.top, (float)realDrawDest.right,
			  (float)realDrawDest.bottom, false, false, cropL, cropT, cropR, cropB, outputVertex);

	pGraphic->SetVertexBuffer(shader, outputVertex, sizeof(outputVertex));
	pGraphic->SetVSConstBuffer(shader, &(matrixWVP[0][0]), sizeof(matrixWVP));
	if (mosaic) {
		pGraphic->SetPSConstBuffer(shader, mosaic, sizeof(MosaicParam));
	}

	pGraphic->DrawTexture(shader, VIDEO_FILTER_TYPE::VIDEO_FILTER_LINEAR, texs);
}

void RenderBulgeTexture(std::vector<texture_handle> texs, SIZE canvas, RECT drawDest, const BulgeParam *psParam)
{
	AUTO_GRAPHIC_CONTEXT(pGraphic);

	TextureInformation texInfo = pGraphic->GetTextureInfo(texs.at(0));
	shader_handle shader = shaders[VIDEO_SHADER_TYPE::SHADER_TEXTURE_BULGE];

	RECT realDrawDest = drawDest;

	float matrixWVP[4][4];
	TransposeMatrixWVP(canvas, true, WorldDesc(), matrixWVP);

	TextureVertexDesc outputVertex[TEXTURE_VERTEX_COUNT];
	FillTextureVertex((float)realDrawDest.left, (float)realDrawDest.top, (float)realDrawDest.right,
			  (float)realDrawDest.bottom, false, false, 0.f, 0.f, 0.f, 0.f, outputVertex);

	pGraphic->SetVertexBuffer(shader, outputVertex, sizeof(outputVertex));
	pGraphic->SetVSConstBuffer(shader, &(matrixWVP[0][0]), sizeof(matrixWVP));
	pGraphic->SetPSConstBuffer(shader, psParam, sizeof(BulgeParam));

	pGraphic->DrawTexture(shader, VIDEO_FILTER_TYPE::VIDEO_FILTER_LINEAR, texs);
}

extern int g_rotatePeriod;
texture_handle getRotatedTexture(texture_handle tex, texture_handle &canvasTex)
{
	AUTO_GRAPHIC_CONTEXT(pGraphic);

	auto oldInfo = pGraphic->GetTextureInfo(tex);

	if (!canvasTex) {
		TextureInformation info;
		info.width = info.height = max(oldInfo.width, oldInfo.height);
		info.format = oldInfo.format;
		info.usage = TEXTURE_USAGE::CANVAS_TARGET;

		canvasTex = pGraphic->CreateTexture(info);
	}

	auto canvasInfo = pGraphic->GetTextureInfo(canvasTex);
	if (pGraphic->BeginRenderCanvas(canvasTex)) {
		ColorRGBA clr{0, 0, 0, 1.0};
		pGraphic->ClearBackground(&clr);

		shader_handle shader = shaders[VIDEO_SHADER_TYPE::SHADER_TEXTURE];
		SIZE canvas(canvasInfo.width, canvasInfo.height);

		TextureVertexDesc outputVertex[TEXTURE_VERTEX_COUNT];
		FillTextureVertex(-(float)oldInfo.width / 2, (float)oldInfo.height / 2, (float)oldInfo.width / 2,
				  -(float)oldInfo.height / 2, false, false, 0.f, 0.f, 0.f, 0.f, outputVertex);

		const auto temp = (float)(GetTickCount64() % g_rotatePeriod);

		WorldVector rt;
		rt.z = (temp / g_rotatePeriod) * 360;

		WorldDesc wd;
		wd.rotate = rt;

		float matrixWVP[4][4];
		TransposeMatrixWVP(canvas, false, wd, matrixWVP);

		pGraphic->SetVertexBuffer(shader, outputVertex, sizeof(outputVertex));
		pGraphic->SetVSConstBuffer(shader, &(matrixWVP[0][0]), sizeof(matrixWVP));
		pGraphic->DrawTexture(shader, VIDEO_FILTER_TYPE::VIDEO_FILTER_LINEAR, std::vector{tex});

		pGraphic->EndRender();
	}

	return canvasTex;
}

void FillRectangleByDX11(SIZE canvas, RECT drawDest, ColorRGBA clr)
{
	AUTO_GRAPHIC_CONTEXT(pGraphic);

	VIDEO_SHADER_TYPE type = VIDEO_SHADER_TYPE::SHADER_FILL_RECT;
	float matrixWVP[4][4];
	TransposeMatrixWVP(canvas, true, WorldDesc(), matrixWVP);

	ColorVertexDesc outputVertex[TEXTURE_VERTEX_COUNT];
	FillColorVertex((float)drawDest.left, (float)drawDest.top, (float)drawDest.right, (float)drawDest.bottom,
			outputVertex);

	pGraphic->SetVertexBuffer(shaders[type], outputVertex, sizeof(outputVertex));
	pGraphic->SetVSConstBuffer(shaders[type], &(matrixWVP[0][0]), sizeof(matrixWVP));
	pGraphic->SetPSConstBuffer(shaders[type], &clr, sizeof(ColorRGBA));

	pGraphic->DrawTopplogy(shaders[type], D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
}

void RenderBorderByDX11(SIZE canvas, RECT drawDest, long borderSize, ColorRGBA clr)
{
	RECT rcLeft;
	RECT rcRight;
	RECT rcTop;
	RECT rcBottom;

	rcLeft.right = drawDest.left;
	rcLeft.left = drawDest.left - borderSize;
	rcLeft.top = drawDest.top - borderSize;
	rcLeft.bottom = drawDest.bottom + borderSize;

	rcRight.left = drawDest.right;
	rcRight.right = drawDest.right + borderSize;
	rcRight.top = drawDest.top - borderSize;
	rcRight.bottom = drawDest.bottom + borderSize;

	rcTop.left = drawDest.left - borderSize;
	rcTop.right = drawDest.right + borderSize;
	rcTop.top = drawDest.top - borderSize;
	rcTop.bottom = drawDest.top;

	rcBottom.left = drawDest.left - borderSize;
	rcBottom.right = drawDest.right + borderSize;
	rcBottom.top = drawDest.bottom;
	rcBottom.bottom = drawDest.bottom + borderSize;

	AUTO_GRAPHIC_CONTEXT(pGraphic);

	FillRectangleByDX11(canvas, rcLeft, clr);
	FillRectangleByDX11(canvas, rcRight, clr);
	FillRectangleByDX11(canvas, rcTop, clr);
	FillRectangleByDX11(canvas, rcBottom, clr);
}

std::wstring GetShaderDirectory()
{
	WCHAR dir[MAX_PATH] = {};
	GetModuleFileNameW(0, dir, MAX_PATH);
	PathRemoveFileSpecW(dir);
	return std::wstring(dir) + std::wstring(L"\\HLSL\\");
}
