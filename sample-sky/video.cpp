#include "pch.h"
#include "sample-1Dlg.h"

IGraphicSession *pGraphic = nullptr;
shader_handle shader = nullptr;
long indexId = INVALID_INDEX_ID;
display_handle display = nullptr;
texture_handle texImg = nullptr;

ColorRGBA clrBlack = {0, 0, 0, 1.f};
ColorRGBA clrBlue = {0, 0, 1, 1.f};

// 6个面 每个面四个顶点（每个顶点对应纹理坐标不一样 所以只能每个面单独定义一次顶点）
static const auto TEXTURE_VERTEX_COUNT = 6 * 4;
static const auto TEXTURE_INDEX_COUNT = 36;

struct TextureVertexDesc {
	XMFLOAT3 Pos;
	XMFLOAT2 Tex;
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
	desc.size = 12;
	shaderInfo.vertexDesc.push_back(desc);

	desc.type = VERTEX_INPUT_TYPE::TEXTURE_COORD;
	desc.size = 8;
	shaderInfo.vertexDesc.push_back(desc);

	std::wstring dir = GetShaderDirectory();

	shaderInfo.vsFile = dir + L"sky-vs.cso";
	shaderInfo.psFile = dir + L"sky-ps.cso";

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
	TextureVertexDesc vertices[] = {
		{XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f)},
		{XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f)},
		{XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f)},
		{XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f)},

		{XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f)},
		{XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f)},
		{XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f)},
		{XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f)},

		{XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f)},
		{XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f)},
		{XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f)},
		{XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f)},

		{XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f)},
		{XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f)},
		{XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f)},
		{XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f)},

		{XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f)},
		{XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f)},
		{XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f)},
		{XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f)},

		{XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.20f, 0.666666f)},
		{XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(0.5f, 0.666666f)},
		{XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(0.5f, 0.333333f)},
		{XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(0.25f, 0.333333f)},
	};

	pGraphic->SetVertexBuffer(shader, vertices, sizeof(vertices));

#if 1
	// 以下三角形 立方体外侧是正面
	WORD indices[] = {3,  1,  0,  2,  1,  3,

			  6,  4,  5,  7,  4,  6,

			  11, 9,  8,  10, 9,  11,

			  14, 12, 13, 15, 12, 14,

			  19, 17, 16, 18, 17, 19,

			  22, 20, 21, 23, 20, 22};

#else
	WORD indices[] = {3,  1,  0,  2,  1,  3,

			  6,  4,  5,  7,  4,  6,

			  11, 9,  8,  10, 9,  11,

			  14, 12, 13, 15, 12, 14,

			  19, 17, 16, 18, 17, 19,

			  22, 20, 21, 23, 20, 22};
#endif

	pGraphic->SetIndexBuffer(shader, indexId, indices, indexDesc.sizePerIndex * indexDesc.indexCount);
}

void Csample1Dlg::initGraphic(HWND hWnd)
{
	pGraphic = graphic::CreateGraphicSession();
	AUTO_GRAPHIC_CONTEXT(pGraphic);

	bool bOK = pGraphic->InitializeGraphic(0);
	assert(bOK);

	initShader();

	texImg = pGraphic->OpenImageTexture(L"res/sky.png");

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
	vec.y = getRotate();
	worldList.push_back(vec);

	CameraDesc camera;
	if (0) {
		camera.eyePos = {0.0f, 0.f, 0.f};
		camera.eyeUpDir = {0.0f, 1.0f, 0.0f};
		camera.lookAt = {0.0f, 0.0f, 1.f};
	} else {
		camera.eyePos = {0.0f, 2.f, -3.f};
		camera.eyeUpDir = {0.0f, 1.0f, 0.0f};
		camera.lookAt = {0.0f, 0.0f, 0.f};
	}

	TransposedPerspectiveMatrixWVP(canvas, &worldList, camera, matrixWVP);

	pGraphic->SetVSConstBuffer(shader, &(matrixWVP[0][0]), sizeof(matrixWVP));
	pGraphic->DrawTexture(shader, VIDEO_FILTER_TYPE::VIDEO_FILTER_LINEAR, textures,
			      D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, indexId);
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
		pGraphic->SetRasterizerState(D3D11_CULL_MODE::D3D11_CULL_BACK); // 剔除三角形背面的画面

		RenderTexture(texImg, SIZE(rcWindow.right, rcWindow.bottom), rcWindow);

		pGraphic->EndRender();
	}
}