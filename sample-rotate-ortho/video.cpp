#include "pch.h"
#include "sample-1Dlg.h"

IGraphicSession *pGraphic = nullptr;
display_handle display = nullptr;

shader_handle texShader = nullptr;
buffer_handle texVertexBuf = nullptr;
texture_handle texImg = nullptr;
texture_handle texOver = nullptr;

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
	texOver = pGraphic->OpenImageTexture(L"overflow.png");

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

void Csample1Dlg::RenderTexture(texture_handle tex, SIZE canvas, RECT drawDest, bool overflow)
{
	AUTO_GRAPHIC_CONTEXT(pGraphic);

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
	if (overflow) {
		CPoint lt, rb;
		GetTextureScreenPos(lt, rb);

		auto x = rb.x - lt.x;
		auto y = rb.y - lt.y;

		float cntX = float(x) / 40;
		float cntY = float(y) / 40.f;

		rightUV *= cntX;
		bottomUV *= cntY;
	}
	outputVertex[0] = {l, t, 0.f, leftUV, topUV};
	outputVertex[1] = {r, t, 0.f, rightUV, topUV};
	outputVertex[2] = {l, b, 0.f, leftUV, bottomUV};
	outputVertex[3] = {r, b, 0.f, rightUV, bottomUV};

	//------------------------------------------ 生成世界坐标转换以及wvp矩阵 -------------------------------------
	XMMATRIX matrixWVP;
	if (overflow) {
		std::vector<WorldVector> worldList = m_worldList;
		if (!worldList.empty() && worldList[0].type != WORLD_TYPE::VECTOR_SCALE) {
			worldList[0].x = worldList[0].y = worldList[0].z = 1.f; // overflow 不执行缩放
		}

		auto worldMatrix = GetWorldMatrix(&worldList);
		matrixWVP = TransposedOrthoMatrixWVP2(canvas, true, worldMatrix);

	} else {
		m_worldMatrix = GetWorldMatrix(&m_worldList);
		matrixWVP = TransposedOrthoMatrixWVP2(canvas, true, m_worldMatrix);
	}

	//------------------------------------------- 渲染 ------------------------------------
	pGraphic->SetGraphicBuffer(texVertexBuf, outputVertex, sizeof(outputVertex));
	pGraphic->SetVSConstBuffer(texShader, &matrixWVP, sizeof(matrixWVP));

	std::vector<texture_handle> textures = {tex};
	pGraphic->DrawTexture(textures, overflow ? VIDEO_FILTER_TYPE::VIDEO_FILTER_LINEAR_REPEAT : VIDEO_FILTER_TYPE::VIDEO_FILTER_LINEAR, texShader, texVertexBuf);
}

float SOLID_SIZE = 20.f;
void Csample1Dlg::RenderSolid(SIZE canvas, CPoint pos)
{
	AUTO_GRAPHIC_CONTEXT(pGraphic);

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

	std::vector<WorldVector> tempWorldList;
	{
		WorldVector vec;
		vec.type = WORLD_TYPE::VECTOR_MOVE; // 计算得到纹理的左上角位置 
		vec.x = pos.x;
		vec.y = pos.y;
		tempWorldList.push_back(vec);
	}
	XMMATRIX worldMatrix = GetWorldMatrix(&tempWorldList);
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

		RenderTexture(texImg, SIZE(rcWindow.right, rcWindow.bottom), rcWindow, false);

		if (GetAsyncKeyState(VK_RBUTTON) & 0x8000) {
			RenderTexture(texOver, SIZE(rcWindow.right, rcWindow.bottom), rcWindow, true);
		}

		if (m_selected) {
			CPoint lt, rb;
			GetTextureScreenPos(lt, rb);

			RenderSolid(SIZE(rcWindow.right, rcWindow.bottom), lt);
			RenderSolid(SIZE(rcWindow.right, rcWindow.bottom), CPoint(rb.x - SOLID_SIZE, rb.y - SOLID_SIZE));
		}

		pGraphic->EndRender();
	}
}