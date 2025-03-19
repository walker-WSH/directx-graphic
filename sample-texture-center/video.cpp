#include "pch.h"
#include "sample-1Dlg.h"

IGraphicSession *pGraphic = nullptr;
display_handle display = nullptr;

shader_handle texShader = nullptr;
buffer_handle texVertexBuf = nullptr;
texture_handle texImg = nullptr;

ColorRGBA clrBlack = {0, 0, 0, 1.f};

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

void Csample1Dlg::initGraphic(HWND hWnd)
{
	pGraphic = graphic::CreateGraphicSession();
	AUTO_GRAPHIC_CONTEXT(pGraphic);

	bool bOK = pGraphic->InitializeGraphic(0);
	assert(bOK);

	initTexShader();

	texImg = pGraphic->OpenImageTexture(L"test.jpg");
	m_texMainImgInfo = pGraphic->GetTextureInfo(texImg);

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

void CenterTexture(int winWidth, int winHeight, int texWidth, int texHeight)
{
	auto winRatio = float(winWidth) / float(winHeight);
	auto texRatio = float(texWidth) / float(texHeight);
	float ratio;
	if (winRatio > texRatio) {
		ratio = float(winHeight) / float(texHeight);
	} else {
		ratio = float(winWidth) / float(texWidth);
	}

	auto newTexCx = float(texWidth) * ratio;
	auto newTexCy = float(texHeight) * ratio;

	auto x = (float(winWidth) - newTexCx) / 2.f;
	auto y = (float(winHeight) - newTexCy) / 2.f;

	pGraphic->SetViewports(x, y, newTexCx, newTexCy);
}

void Csample1Dlg::RenderTexture(texture_handle tex, SIZE canvas)
{
	AUTO_GRAPHIC_CONTEXT(pGraphic);

	// 调整viewPort 让纹理居中显示
	CenterTexture(canvas.cx, canvas.cy, m_texMainImgInfo.width, m_texMainImgInfo.height);

	//---------------------------------------- 顶点数组赋值 ---------------------------------------
	TextureVertexDesc outputVertex[TEXTURE_VERTEX_COUNT];
	float l = 0.f;
	float t = 0.f;
	float r = (float)m_texMainImgInfo.width;
	float b = (float)m_texMainImgInfo.height;
	float leftUV = 0.f;
	float topUV = 0.f;
	float rightUV = 1.f;
	float bottomUV = 1.f;
	outputVertex[0] = {l, t, 0.f, leftUV, topUV};
	outputVertex[1] = {r, t, 0.f, rightUV, topUV};
	outputVertex[2] = {l, b, 0.f, leftUV, bottomUV};
	outputVertex[3] = {r, b, 0.f, rightUV, bottomUV};

	//------------------------------------------ 生成世界坐标转换以及wvp矩阵 -------------------------------------
	std::vector<WorldVector> worldList;
	XMMATRIX worldMatrix = GetWorldMatrix(&worldList);
	XMMATRIX matrixWVP = TransposedOrthoMatrixWVP2(SIZE(m_texMainImgInfo.width, m_texMainImgInfo.height), true, worldMatrix);

	//------------------------------------------- 渲染 ------------------------------------
	pGraphic->SetGraphicBuffer(texVertexBuf, outputVertex, sizeof(outputVertex));
	pGraphic->SetVSConstBuffer(texShader, &matrixWVP, sizeof(matrixWVP));

	std::vector<texture_handle> textures = {tex};
	pGraphic->DrawTexture(textures, VIDEO_FILTER_TYPE::VIDEO_FILTER_LINEAR, texShader, texVertexBuf);
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
		pGraphic->ClearBackground(&clrBlack);
		pGraphic->SetBlendState(VIDEO_BLEND_TYPE::BLEND_DISABLED);

		RenderTexture(texImg, SIZE(rcWindow.right, rcWindow.bottom));

		pGraphic->EndRender();
	}
}