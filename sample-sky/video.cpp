#include "pch.h"
#include "sample-1Dlg.h"

IGraphicSession *pGraphic = nullptr;
shader_handle shader = nullptr;
display_handle display = nullptr;
texture_handle texImg = nullptr;

ColorRGBA clrBlack = {0, 0, 0, 1.f};
ColorRGBA clrBlue = {0, 0, 1, 1.f};

static const auto TEXTURE_VERTEX_COUNT = 6 * 2 * 3;
struct TextureVertexDesc {
	float x, y, z, w;
	float u, v;
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

	//---------------------------------------------------------------------------------------
	auto x = 1.f;
	auto y = 1.f;
	auto z = 1.f;

	float leftUV = 0.f;
	float topUV = 0.f;
	float rightUV = 1.f;
	float bottomUV = 1.f;
	
	TextureVertexDesc outputVertex[TEXTURE_VERTEX_COUNT] = {
		{-x, y, z, 1.f, leftUV, topUV},     {-x, -y, z, 1.f, leftUV, bottomUV},
		{x, y, z, 1.f, rightUV, topUV},     {x, y, z, 1.f, rightUV, topUV},
		{-x, -y, z, 1.f, leftUV, bottomUV}, {x, -y, z, 1.f, rightUV, bottomUV},

		{x, y, z, 1.f, leftUV, topUV},      {x, y, -z, 1.f, leftUV, topUV},
		{x, -y, z, 1.f, leftUV, topUV},     {x, -y, z, 1.f, leftUV, bottomUV},
		{x, y, -z, 1.f, leftUV, bottomUV},  {x, -y, -z, 1.f, leftUV, bottomUV},

	};


	pGraphic->SetVertexBuffer(shader, outputVertex, sizeof(outputVertex));
}

void Csample1Dlg::initGraphic(HWND hWnd)
{
	pGraphic = graphic::CreateGraphicSession();
	AUTO_GRAPHIC_CONTEXT(pGraphic);

	bool bOK = pGraphic->InitializeGraphic(0);
	assert(bOK);

	initShader();

	texImg = pGraphic->OpenImageTexture(L"res/test.jpg");

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
	camera.eyePos = {0.0f, 1.5f, -3.f};
	camera.eyeUpDir = {0.0f, 1.0f, 0.0f};
	camera.lookAt = {0.0f, 0.0f, 0.f};

	TransposedPerspectiveMatrixWVP(canvas, &worldList, camera, matrixWVP);

	pGraphic->SetVSConstBuffer(shader, &(matrixWVP[0][0]), sizeof(matrixWVP));
	pGraphic->DrawTexture(shader, VIDEO_FILTER_TYPE::VIDEO_FILTER_LINEAR, textures,
			      D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
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
		pGraphic->SetRasterizerState(D3D11_CULL_MODE::D3D11_CULL_BACK);

		RenderTexture(texImg, SIZE(rcWindow.right, rcWindow.bottom), rcWindow);

		pGraphic->EndRender();
	}
}