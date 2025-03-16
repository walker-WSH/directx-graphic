#include "pch.h"
#include "sample-1Dlg.h"

IGraphicSession *pGraphic = nullptr;
shader_handle shader = nullptr;
buffer_handle vertexBuf = nullptr;
display_handle display = nullptr;
texture_handle texImg = nullptr;

ColorRGBA clrBlack = {0, 0, 0, 1.f};
ColorRGBA clrBlue = {0, 0, 1, 1.f};

static const auto TEXTURE_VERTEX_COUNT = 4;
struct TextureVertexDesc {
	float x, y, z;
	float u, v;
};

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
	shaderInfo.vsFile = dir + L"rotate-vs.cso";
	shaderInfo.psFile = dir + L"rotate-ps.cso";
	shaderInfo.vsBufferSize = sizeof(matrixWVP);
	shaderInfo.psBufferSize = 0;

	shader = pGraphic->CreateShader(shaderInfo);
	assert(shader);

	BufferDesc bufDesc;
	bufDesc.bufferType = D3D11_BIND_VERTEX_BUFFER;
	bufDesc.itemCount = TEXTURE_VERTEX_COUNT;
	bufDesc.sizePerItem = sizeof(TextureVertexDesc);
	vertexBuf = pGraphic->CreateGraphicBuffer(bufDesc);
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

void Csample1Dlg::RenderTexture(texture_handle tex, SIZE canvas, RECT drawDest)
{
	AUTO_GRAPHIC_CONTEXT(pGraphic);

	auto texInfo = pGraphic->GetTextureInfo(tex);
	std::vector<texture_handle> textures = {tex};

	TextureVertexDesc outputVertex[TEXTURE_VERTEX_COUNT];
	float l = 0.f;
	float t = 0.f;
	float r = (float)texInfo.width;
	float b = (float)texInfo.height;
	float leftUV = 0.f;
	float topUV = 0.f;
	float rightUV = 1.f;
	float bottomUV = 1.f;
	outputVertex[0] = {l, t, 0.f, leftUV, topUV};
	outputVertex[1] = {r, t, 0.f, rightUV, topUV};
	outputVertex[2] = {l, b, 0.f, leftUV, bottomUV};
	outputVertex[3] = {r, b, 0.f, rightUV, bottomUV};

	XMMATRIX matrixWVP;
	TransposedOrthoMatrixWVP(canvas, true, &m_worldList, matrixWVP);
	m_worldMatrix = GetWorldMatrix(&m_worldList);

	pGraphic->SetGraphicBuffer(vertexBuf, outputVertex, sizeof(outputVertex));
	pGraphic->SetVSConstBuffer(shader, &matrixWVP, sizeof(matrixWVP));
	pGraphic->DrawTexture(textures, VIDEO_FILTER_TYPE::VIDEO_FILTER_LINEAR, shader, vertexBuf);
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