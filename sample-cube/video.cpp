﻿#include "pch.h"
#include "sample-1Dlg.h"

IGraphicSession *pGraphic = nullptr;
shader_handle shader = nullptr;
long indexId = INVALID_INDEX_ID;
display_handle display = nullptr;
texture_handle texImg = nullptr;

ColorRGBA clrBlack = {0, 0, 0, 1.f};
ColorRGBA clrBlue = {0, 0, 1, 1.f};

static const auto TEXTURE_VERTEX_COUNT = 8;
static const auto TEXTURE_INDEX_COUNT = 36;
struct TextureVertexDesc {
	float x, y, z, w;
	float r, g, b, a;
};

std::wstring GetShaderDirectory()
{
	WCHAR dir[MAX_PATH] = {};
	GetModuleFileNameW(0, dir, MAX_PATH);
	PathRemoveFileSpecW(dir);
	return std::wstring(dir) + std::wstring(L"\\HLSL\\");
}

void initShader()
{
	float matrixWVP[4][4];
	ShaderInformation shaderInfo;

	VertexInputDesc desc;
	desc.type = VERTEX_INPUT_TYPE::POSITION;
	desc.size = 16;
	shaderInfo.vertexDesc.push_back(desc);

	desc.type = VERTEX_INPUT_TYPE::COLOR;
	desc.size = 16;
	shaderInfo.vertexDesc.push_back(desc);

	std::wstring dir = GetShaderDirectory();

	shaderInfo.vsFile = dir + L"cube-vs.cso";
	shaderInfo.psFile = dir + L"cube-ps.cso";

	shaderInfo.vsBufferSize = sizeof(matrixWVP);
	shaderInfo.psBufferSize = 0;

	shaderInfo.vertexCount = TEXTURE_VERTEX_COUNT;
	shaderInfo.perVertexSize = sizeof(TextureVertexDesc);

	shader = pGraphic->CreateShader(shaderInfo);
	assert(shader);

	IndexItemDesc indexDesc;
	indexDesc.sizePerIndex = sizeof(WORD);
	indexDesc.indexCount = TEXTURE_INDEX_COUNT;
	indexId = pGraphic->CreateIndexBuffer(shader, indexDesc);

	//---------------------------------------------------------------------------------------
	float scale = 1.f;

	TextureVertexDesc outputVertex[TEXTURE_VERTEX_COUNT];
	outputVertex[0] = {-scale, scale, -scale, 1.f, 0, 0, 0, 1.f};
	outputVertex[1] = {scale, scale, -scale, 1.f, 0, 0, 0, 1.f};
	outputVertex[2] = {scale, scale, scale, 1.f, 1.f, 0, 0, 1.f};
	outputVertex[3] = {-scale, scale, scale, 1.f, 1.f, 0, 0, 1.f};

	outputVertex[4] = {-scale, -scale, -scale, 1.f, 0, 1, 0, 1.f};
	outputVertex[5] = {scale, -scale, -scale, 1.f, 0, 1, 0, 1.f};
	outputVertex[6] = {scale, -scale, scale, 1.f, 0, 1, 0, 1.f};
	outputVertex[7] = {-scale, -scale, scale, 1.f, 0, 1, 0, 1.f};

	pGraphic->SetVertexBuffer(shader, outputVertex, sizeof(outputVertex));

	//---------------------------------------------------------------------------------------
	WORD indices[TEXTURE_INDEX_COUNT] = {
		3, 1, 0, 2, 1, 3,

		0, 5, 4, 1, 5, 0,

		3, 4, 7, 0, 4, 3,

		1, 6, 5, 2, 6, 1,

		2, 7, 6, 3, 7, 2,

		6, 4, 5, 7, 4, 6,
	};

	pGraphic->SetIndexBuffer(shader, indexId, indices, indexDesc.sizePerIndex * indexDesc.indexCount);
}

void Csample1Dlg::initGraphic(HWND hWnd)
{
	pGraphic = graphic::CreateGraphicSession();
	AUTO_GRAPHIC_CONTEXT(pGraphic);

	bool bOK = pGraphic->InitializeGraphic(0);
	assert(bOK);

	initShader();

	texImg = pGraphic->OpenImageTexture(L"test.jpg");

	display = pGraphic->CreateDisplay(hWnd);
	assert(display);

	RECT rcWindow;
	::GetClientRect(hWnd, &rcWindow);
	pGraphic->SetDisplaySize(display, rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top);
}

void Csample1Dlg::uninitGraphic()
{
	{
		AUTO_GRAPHIC_CONTEXT(pGraphic);
		pGraphic->DestroyAllGraphicObject();
	}

	graphic::DestroyGraphicSession(pGraphic);
}

float Csample1Dlg::getRotate()
{
	auto ret = (float)m_sliderRotate.GetPos();
	ATLTRACE("--------------------- %f \n", ret);
	return ret;
}

void Csample1Dlg::RenderTexture(texture_handle tex, SIZE canvas, RECT drawDest)
{
	AUTO_GRAPHIC_CONTEXT(pGraphic);

	std::vector<texture_handle> textures = {tex};

	float matrixWVP[4][4];
	std::vector<WorldVector> worldList;
	WorldVector vec;
	vec.type = WORLD_TYPE::VECTOR_ROTATE;
	vec.x = getRotate();
	worldList.push_back(vec);

	CameraDesc camera;
	camera.eyePos = {0.0f, 2.f, -2.f};
	camera.eyeUpDir = {0.0f, 1.0f, 1.f};
	camera.lookAt = {0.0f, 0.0f, 0.0f};

	TransposedPerspectiveMatrixWVP(canvas, &worldList, camera, matrixWVP);

	pGraphic->SetVSConstBuffer(shader, &(matrixWVP[0][0]), sizeof(matrixWVP));
	pGraphic->DrawTexture(shader, VIDEO_FILTER_TYPE::VIDEO_FILTER_LINEAR, textures,
			      D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, indexId);
}

void Csample1Dlg::render()
{
	AUTO_GRAPHIC_CONTEXT(pGraphic);

	RECT rcWindow;
	::GetClientRect(m_hWnd, &rcWindow);

	pGraphic->SetDisplaySize(display, rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top);

	if (!pGraphic->IsGraphicBuilt()) {
		if (!pGraphic->ReBuildGraphic())
			return;
	}

	if (pGraphic->BeginRenderWindow(display)) {
		pGraphic->ClearBackground(&clrBlue);
		pGraphic->SetBlendState(VIDEO_BLEND_TYPE::BLEND_DISABLED);

		RenderTexture(texImg, SIZE(rcWindow.right, rcWindow.bottom), rcWindow);

		pGraphic->EndRender();
	}
}