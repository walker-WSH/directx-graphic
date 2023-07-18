#include "pch.h"
#include "sample-1Dlg.h"

IGraphicSession *pGraphic = nullptr;
shader_handle shader = nullptr;
buffer_handle vertexBuf = nullptr;
buffer_handle indexId = nullptr;
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

	shader = pGraphic->CreateShader(shaderInfo);
	assert(shader);

	BufferDesc bufDesc;
	bufDesc.bufferType = D3D11_BIND_VERTEX_BUFFER;
	bufDesc.itemCount = TEXTURE_VERTEX_COUNT;
	bufDesc.sizePerItem = sizeof(TextureVertexDesc);
	vertexBuf = pGraphic->CreateGraphicBuffer(bufDesc);

	bufDesc.bufferType = D3D11_BIND_INDEX_BUFFER;
	bufDesc.sizePerItem = sizeof(WORD);
	bufDesc.itemCount = TEXTURE_INDEX_COUNT;
	indexId = pGraphic->CreateGraphicBuffer(bufDesc);

	//---------------------------------------------------------------------------------------
	float stepY = 1.f / 3.f;
	float stepY_x_1 = stepY * 1.f;
	float stepY_x_2 = stepY * 2.f;

	TextureVertexDesc vertices[] = {
		// 图像0 顶部图像
		{XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(0.25f, 0.f)},
		{XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(0.25f, stepY_x_1)},
		{XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(0.5f, stepY_x_1)},
		{XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(0.5f, 0.f)},

		// 图像1 底部图像
		{XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.25f, 1.f)},
		{XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(0.25f, stepY_x_2)},
		{XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(0.5f, stepY_x_2)},
		{XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.5f, 1.f)},

		// 图像4
		{XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.75f, stepY_x_2)},
		{XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(1.f, stepY_x_2)},
		{XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(1.f, stepY_x_1)},
		{XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(0.75f, stepY_x_1)},

		// 图像2
		{XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.f, stepY_x_2)},
		{XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(0.25f, stepY_x_2)},
		{XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(0.25f, stepY_x_1)},
		{XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(0.f, stepY_x_1)},

		// 图像5
		{XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(0.5f, stepY_x_2)},
		{XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(0.25f, stepY_x_2)},
		{XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(0.25f, stepY_x_1)},
		{XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(0.5f, stepY_x_1)},

		// 图像3
		{XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.75f, stepY_x_2)},
		{XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(0.5f, stepY_x_2)},
		{XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(0.5f, stepY_x_1)},
		{XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(0.75f, stepY_x_1)},

	};

	pGraphic->SetGraphicBuffer(vertexBuf, vertices, sizeof(vertices));

	WORD indices[] = {3,  0,  1,  2,  3,  1, // 图像0

			  7,  5,  4,  6,  5,  7, // 图像1

			  11, 8,  9,  10, 11, 9, // 图像4

			  15, 12, 13, 14, 15, 13, // 图像2

			  19, 17, 16, 18, 17, 19, // 图像5

			  22, 21, 20, 23, 22, 20}; // 图像3

	pGraphic->SetGraphicBuffer(indexId, indices, sizeof(indices));
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
	pGraphic->SetDisplaySize(display, rcWindow.right - rcWindow.left,
				 rcWindow.bottom - rcWindow.top);
}

void Csample1Dlg::uninitGraphic()
{
	{
		AUTO_GRAPHIC_CONTEXT(pGraphic);
		pGraphic->DestroyAllGraphicObject();
	}

	graphic::DestroyGraphicSession(pGraphic);
}

void Csample1Dlg::RenderTexture(texture_handle tex, SIZE canvas, RECT drawDest)
{
	AUTO_GRAPHIC_CONTEXT(pGraphic);

	std::vector<texture_handle> textures = {tex};

	XMMATRIX matrixWVP;
	std::vector<WorldVector> worldList;
	WorldVector vec;
	vec.type = WORLD_TYPE::VECTOR_ROTATE;
	vec.x = (float)rotateX;
	vec.y = (float)rotateY;
	worldList.push_back(vec);

	eyePosZ = max(eyePosZ, -100);
	eyePosZ = min(eyePosZ, 100);
	float eyeZ = (float)eyePosZ / 100.f;

	CameraDesc camera;
	camera.eyePos = {0.0f, 0.0f, eyeZ};
	camera.eyeUpDir = {0.0f, 1.0f, 0.0f};
	camera.lookAt = {0.0f, 0.0f, 1.f};

	TransposedPerspectiveMatrixWVP(canvas, &worldList, camera, matrixWVP);

	pGraphic->SetVSConstBuffer(shader, &matrixWVP, sizeof(matrixWVP));

	// 如果使用别的采样 会有明显的两面接缝
	pGraphic->DrawTexture(textures, VIDEO_FILTER_TYPE::VIDEO_FILTER_POINT, shader, vertexBuf,
			      indexId, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Csample1Dlg::render()
{
	AUTO_GRAPHIC_CONTEXT(pGraphic);

	RECT rcWindow;
	::GetClientRect(m_hWnd, &rcWindow);

	pGraphic->SetDisplaySize(display, rcWindow.right - rcWindow.left,
				 rcWindow.bottom - rcWindow.top);

	if (!pGraphic->IsGraphicBuilt()) {
		if (!pGraphic->ReBuildGraphic())
			return;
	}

	if (pGraphic->BeginRenderWindow(display)) {
		pGraphic->ClearBackground(&clrBlue);
		pGraphic->SetBlendState(VIDEO_BLEND_TYPE::BLEND_DISABLED);
		pGraphic->SetRasterizerState(
			D3D11_CULL_MODE::D3D11_CULL_BACK); // 剔除三角形背面的画面

		RenderTexture(texImg, SIZE(rcWindow.right, rcWindow.bottom), rcWindow);

		pGraphic->EndRender();
	}
}