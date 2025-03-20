#include "pch.h"
#include "sample-1Dlg.h"

IGraphicSession *pGraphic = nullptr;
display_handle display = nullptr;

shader_handle solidShader = nullptr;
buffer_handle solidVertexBuf = nullptr;

ColorRGBA clrBlue = {0, 0, 1, 1.f};

static const auto SLICE_COUNT = 40;
static const auto TEXTURE_VERTEX_COUNT = SLICE_COUNT * 3;
struct SolidVertexDesc {
	float x, y;
};
struct PSConstBuffer{
	float size;
	float offsetAlpha;
	float moveX;
	float moveY;
};

void initSolidShader()
{
	ShaderInformation shaderInfo;

	VertexInputDesc desc;
	desc.type = VERTEX_INPUT_TYPE::POSITION;
	desc.size = 8;
	shaderInfo.vertexDesc.push_back(desc);


	std::wstring dir = GetShaderDirectory();
	shaderInfo.vsFile = dir + L"solid-vs.cso";
	shaderInfo.psFile = dir + L"solid-ps.cso";
	shaderInfo.vsBufferSize = sizeof(XMMATRIX);
	shaderInfo.psBufferSize = sizeof(PSConstBuffer);

	solidShader = pGraphic->CreateShader(shaderInfo);
	assert(solidShader);

	BufferDesc bufDesc;
	bufDesc.bufferType = D3D11_BIND_VERTEX_BUFFER;
	bufDesc.itemCount = TEXTURE_VERTEX_COUNT;
	bufDesc.sizePerItem = sizeof(SolidVertexDesc);
	solidVertexBuf = pGraphic->CreateGraphicBuffer(bufDesc);
}

void Csample1Dlg::initGraphic(HWND hWnd)
{
	pGraphic = graphic::CreateGraphicSession();
	AUTO_GRAPHIC_CONTEXT(pGraphic);

	bool bOK = pGraphic->InitializeGraphic(0);
	assert(bOK);

	initSolidShader();

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

SolidVertexDesc outputVertex[TEXTURE_VERTEX_COUNT];

#define RAD(val) ((val) * 0.0174532925199432957692369076848f) // 将角度转换为弧度
#define DEG(val) ((val) * 57.295779513082320876798154814105f)

void CreateTriangleList() // 参考obs函数 OBSBasicPreview::DrawSelectedItem
{
	int index = 0;

	float angle = 180;
	for (int i = 0; i < SLICE_COUNT; i++) {
		outputVertex[index++] = {sin(RAD(angle)) / 2 + 0.5f, cos(RAD(angle)) / 2 + 0.5f};

		angle += 360 / SLICE_COUNT;
		outputVertex[index++] = {sin(RAD(angle)) / 2 + 0.5f, cos(RAD(angle)) / 2 + 0.5f};

		outputVertex[index++] = {0.5f, 1.0f};
	}
}

float SOLID_SIZE = 200;
float moveX = 80;
float moveY = 80;
void Csample1Dlg::RenderSolid(SIZE canvas, CPoint pos)
{
	AUTO_GRAPHIC_CONTEXT(pGraphic);

	//------------------------------------ 顶点数组 ------------------------------------
	CreateTriangleList();

	//------------------------------------- 像素着色器常量计算 -----------------------------------
	static PSConstBuffer psb = {0};
	psb.size = SOLID_SIZE;
	psb.moveX = moveX;
	psb.moveY = moveY;

	bool leftButtonDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000);
	if (leftButtonDown) {
		psb.offsetAlpha = 0.99f; // 0.4 -> 0
	} else {
		psb.offsetAlpha -= 0.15f;
		if (psb.offsetAlpha < 0.f)
			psb.offsetAlpha = 0.f;
	}

	//------------------------------------- 矩阵 -----------------------------------
	std::vector<WorldVector> worldList; 
	{
		WorldVector vec;
		vec.type = WORLD_TYPE::VECTOR_SCALE; 
		vec.x = SOLID_SIZE;
		vec.y = SOLID_SIZE;
		worldList.push_back(vec);
	}
	{
		WorldVector vec;
		vec.type = WORLD_TYPE::VECTOR_MOVE;
		vec.x = moveX;
		vec.y = moveY;
		worldList.push_back(vec);
	}

	XMMATRIX worldMatrix = GetWorldMatrix(&worldList);
	XMMATRIX matrixWVP = TransposedOrthoMatrixWVP2(canvas, true, worldMatrix);

	//------------------------------------------------------------------------
	pGraphic->SetGraphicBuffer(solidVertexBuf, outputVertex, sizeof(outputVertex));
	pGraphic->SetVSConstBuffer(solidShader, &matrixWVP, sizeof(matrixWVP));
	pGraphic->SetPSConstBuffer(solidShader, &psb, sizeof(PSConstBuffer));

	std::vector<texture_handle> textures = {};
	pGraphic->DrawTexture(textures, VIDEO_FILTER_TYPE::VIDEO_FILTER_LINEAR, solidShader, solidVertexBuf);
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
		pGraphic->SetBlendState(VIDEO_BLEND_TYPE::BLEND_NORMAL);
		pGraphic->SetRasterizerState(D3D11_CULL_MODE::D3D11_CULL_FRONT);

		RenderSolid(SIZE(rcWindow.right, rcWindow.bottom), CPoint(0, 0));

		pGraphic->EndRender();
	}
}