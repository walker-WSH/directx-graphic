#include "pch.h"
#include "sample-1Dlg.h"

IGraphicSession *pGraphic = nullptr;
display_handle display = nullptr;

shader_handle texShader = nullptr;
buffer_handle texVertexBuf = nullptr;
texture_handle texImg = nullptr;

shader_handle solidShader = nullptr;
buffer_handle solidVertexBuf = nullptr;

ColorRGBA clrBlack = {0, 0, 0, 1.f};
ColorRGBA clrBlue = {0, 0, 1, 1.f};

static const auto TEXTURE_VERTEX_COUNT = 4;
struct TextureVertexDesc {
	float x, y, z;
	float u, v;
};
struct SolidVertexDesc {
	float x, y, z;
	float r, g, b;
};

void initTexShader()
{
	ShaderInformation shaderInfo;

	VertexInputDesc desc;
	desc.type = VERTEX_INPUT_TYPE::POSITION;
	desc.size = 12;
	shaderInfo.vertexDesc.push_back(desc);
	desc.type = VERTEX_INPUT_TYPE::TEXTURE_COORD;
	desc.size = 8;
	shaderInfo.vertexDesc.push_back(desc);

	std::wstring dir = GetShaderDirectory();
	shaderInfo.vsFile = dir + L"rotate-vs.cso";
	shaderInfo.psFile = dir + L"rotate-ps.cso";
	shaderInfo.vsBufferSize = sizeof(XMMATRIX);
	shaderInfo.psBufferSize = 0;

	texShader = pGraphic->CreateShader(shaderInfo);
	assert(texShader);

	BufferDesc bufDesc;
	bufDesc.bufferType = D3D11_BIND_VERTEX_BUFFER;
	bufDesc.itemCount = TEXTURE_VERTEX_COUNT;
	bufDesc.sizePerItem = sizeof(TextureVertexDesc);
	texVertexBuf = pGraphic->CreateGraphicBuffer(bufDesc);
}

void initSolidShader()
{
	ShaderInformation shaderInfo;

	VertexInputDesc desc;
	desc.type = VERTEX_INPUT_TYPE::POSITION;
	desc.size = 12;
	shaderInfo.vertexDesc.push_back(desc);
	desc.type = VERTEX_INPUT_TYPE::COLOR;
	desc.size = 12;
	shaderInfo.vertexDesc.push_back(desc);

	std::wstring dir = GetShaderDirectory();
	shaderInfo.vsFile = dir + L"solid-vs.cso";
	shaderInfo.psFile = dir + L"solid-ps.cso";
	shaderInfo.vsBufferSize = sizeof(XMMATRIX);
	shaderInfo.psBufferSize = 0;

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

	initTexShader();
	initSolidShader();

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

void Csample1Dlg::RenderTexture(texture_handle tex, SIZE canvas, RECT drawDest)
{
	AUTO_GRAPHIC_CONTEXT(pGraphic);

	m_texInfo = pGraphic->GetTextureInfo(tex);
	std::vector<texture_handle> textures = {tex};

	TextureVertexDesc outputVertex[TEXTURE_VERTEX_COUNT];
	float l = 0.f;
	float t = 0.f;
	float r = (float)m_texInfo.width;
	float b = (float)m_texInfo.height;
	float leftUV = 0.f;
	float topUV = 0.f;
	float rightUV = 1.f;
	float bottomUV = 1.f;
	outputVertex[0] = {l, t, 0.f, leftUV, topUV};
	outputVertex[1] = {r, t, 0.f, rightUV, topUV};
	outputVertex[2] = {l, b, 0.f, leftUV, bottomUV};
	outputVertex[3] = {r, b, 0.f, rightUV, bottomUV};

	m_worldMatrix = GetWorldMatrix(&m_worldList);
	XMMATRIX matrixWVP = TransposedOrthoMatrixWVP2(canvas, true, m_worldMatrix);

	pGraphic->SetGraphicBuffer(texVertexBuf, outputVertex, sizeof(outputVertex));
	pGraphic->SetVSConstBuffer(texShader, &matrixWVP, sizeof(matrixWVP));
	pGraphic->DrawTexture(textures, VIDEO_FILTER_TYPE::VIDEO_FILTER_LINEAR, texShader, texVertexBuf);
}

void Csample1Dlg::RenderSolid(SIZE canvas)
{
	AUTO_GRAPHIC_CONTEXT(pGraphic);

	float SOLID_SIZE = 20.f;

	SolidVertexDesc outputVertex[TEXTURE_VERTEX_COUNT];
	float l = 0.f;
	float t = 0.f;
	float r = SOLID_SIZE;
	float b = SOLID_SIZE;
	float clr_r = 0.f;
	float clr_g = 1.f;
	float clr_b = 0.f;
	outputVertex[0] = {l, t, 0.f, clr_r, clr_g, clr_b};
	outputVertex[1] = {r, t, 0.f, clr_r, clr_g, clr_b};
	outputVertex[2] = {l, b, 0.f, clr_r, clr_g, clr_b};
	outputVertex[3] = {r, b, 0.f, clr_r, clr_g, clr_b};

	XMMATRIX worldMatrix = GetWorldMatrix(&m_worldList);
	XMMATRIX matrixWVP = TransposedOrthoMatrixWVP2(canvas, true, worldMatrix);

	pGraphic->SetGraphicBuffer(solidVertexBuf, outputVertex, sizeof(outputVertex));
	pGraphic->SetVSConstBuffer(solidShader, &matrixWVP, sizeof(matrixWVP));

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
		pGraphic->SetBlendState(VIDEO_BLEND_TYPE::BLEND_DISABLED);

		RenderTexture(texImg, SIZE(rcWindow.right, rcWindow.bottom), rcWindow);

		if (m_selected)
			RenderSolid(SIZE(rcWindow.right, rcWindow.bottom));

		pGraphic->EndRender();
	}
}