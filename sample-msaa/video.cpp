#include "pch.h"
#include "sample-1Dlg.h"

IGraphicSession *pGraphic = nullptr;
shader_handle shader = nullptr;
shader_handle shaderTex = nullptr;
buffer_handle vertexBuf = nullptr;
buffer_handle vertexBufTex = nullptr;
buffer_handle indexId = nullptr;
texture_handle texCanvas = nullptr;
display_handle display = nullptr;

ColorRGBA clrBlack = {0, 0, 0, 1.f};
ColorRGBA clrBlue = {0, 0, 1, 1.f};
ColorRGBA clrTrans = {0, 0, 0, 0};

static const auto TEXTURE_VERTEX_COUNT = 8;
static const auto TEXTURE_INDEX_COUNT = 12;
struct TriangleVertexDesc {
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
};

static const auto CANVAS_VERTEX_COUNT = 4;
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
	std::wstring dir = GetShaderDirectory();

	{
		ShaderInformation shaderInfo;

		VertexInputDesc desc;
		desc.type = VERTEX_INPUT_TYPE::POSITION;
		desc.size = 12;
		shaderInfo.vertexDesc.push_back(desc);

		desc.type = VERTEX_INPUT_TYPE::COLOR;
		desc.size = 16;
		shaderInfo.vertexDesc.push_back(desc);

		shaderInfo.vsFile = dir + L"msaa-vs.cso";
		shaderInfo.psFile = dir + L"msaa-ps.cso";
		shaderInfo.vsBufferSize = sizeof(XMMATRIX);
		shaderInfo.psBufferSize = 0;

		shader = pGraphic->CreateShader(shaderInfo);

		BufferDesc bufDesc;
		bufDesc.bufferType = D3D11_BIND_VERTEX_BUFFER;
		bufDesc.itemCount = TEXTURE_VERTEX_COUNT;
		bufDesc.sizePerItem = sizeof(TriangleVertexDesc);
		vertexBuf = pGraphic->CreateGraphicBuffer(bufDesc);
	}

	{
		ShaderInformation shaderInfo;

		VertexInputDesc desc;
		desc.type = VERTEX_INPUT_TYPE::POSITION;
		desc.size = 16;
		shaderInfo.vertexDesc.push_back(desc);

		desc.type = VERTEX_INPUT_TYPE::TEXTURE_COORD;
		desc.size = 8;
		shaderInfo.vertexDesc.push_back(desc);

		std::wstring dir = GetShaderDirectory();

		shaderInfo.vsFile = dir + L"tex-vs.cso";
		shaderInfo.psFile = dir + L"tex-ps.cso";
		shaderInfo.vsBufferSize = sizeof(XMMATRIX);
		shaderInfo.psBufferSize = 0;
		shaderTex = pGraphic->CreateShader(shaderInfo);

		BufferDesc bufDesc;
		bufDesc.bufferType = D3D11_BIND_VERTEX_BUFFER;
		bufDesc.itemCount = CANVAS_VERTEX_COUNT;
		bufDesc.sizePerItem = sizeof(TextureVertexDesc);
		vertexBufTex = pGraphic->CreateGraphicBuffer(bufDesc);
	}

	BufferDesc bufDesc;
	bufDesc.bufferType = D3D11_BIND_INDEX_BUFFER;
	bufDesc.sizePerItem = sizeof(WORD);
	bufDesc.itemCount = TEXTURE_INDEX_COUNT;
	indexId = pGraphic->CreateGraphicBuffer(bufDesc);

	//---------------------------------------------------------------------------------------
	XMFLOAT4 red = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 green = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blue = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	XMFLOAT4 yellow = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 white = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 black = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	TriangleVertexDesc vertices[] = {
		{XMFLOAT3(-1.0f, 1.0f, 1.0f), red},    {XMFLOAT3(1.0f, 1.0f, 1.0f), green},
		{XMFLOAT3(1.0f, -1.0f, 1.0f), blue},   {XMFLOAT3(-1.0f, -1.0f, 1.0f), white},

		{XMFLOAT3(-1.0f, 1.0f, -1.0f), white}, {XMFLOAT3(1.0f, 1.0f, -1.0f), red},
		{XMFLOAT3(1.0f, -1.0f, -1.0f), black}, {XMFLOAT3(-1.0f, -1.0f, -1.0f), green},
	};

	pGraphic->SetGraphicBuffer(vertexBuf, vertices, sizeof(vertices));

	//---------------------------------------------------------------------------------------
	// 以下三角形 立方体外侧是正面
	WORD indices[] = {
		0, 1, 3, 1, 2, 3, 4, 5, 7, 5, 6, 7,
	};

	pGraphic->SetGraphicBuffer(indexId, indices, sizeof(indices));
}

void Csample1Dlg::initGraphic(HWND hWnd)
{
	pGraphic = graphic::CreateGraphicSession();
	AUTO_GRAPHIC_CONTEXT(pGraphic);

	bool bOK = pGraphic->InitializeGraphic(0);
	assert(bOK);

	initShader();

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

void Csample1Dlg::RenderTexture(SIZE canvas, RECT drawDest, texture_handle tex)
{
	AUTO_GRAPHIC_CONTEXT(pGraphic);

	XMMATRIX matrixWVP;

	if (tex) {
		std::vector<texture_handle> textures = {tex};
		auto info = pGraphic->GetTextureInfo(tex);

		TextureVertexDesc outputVertex[CANVAS_VERTEX_COUNT];

		float leftUV = 0.f;
		float topUV = 0.f;
		float rightUV = 1.f;
		float bottomUV = 1.f;

		outputVertex[0] = {
			-(float)info.width / 2, (float)info.height / 2, 0, 1.f, leftUV, topUV};
		outputVertex[1] = {
			(float)info.width / 2, (float)info.height / 2, 0, 1.f, rightUV, topUV};
		outputVertex[2] = {
			-(float)info.width / 2, -(float)info.height / 2, 0, 1.f, leftUV, bottomUV};
		outputVertex[3] = {
			(float)info.width / 2, -(float)info.height / 2, 0, 1.f, rightUV, bottomUV};

		TransposedOrthoMatrixWVP(canvas, false, nullptr, matrixWVP);

		pGraphic->SetGraphicBuffer(vertexBufTex, outputVertex, sizeof(outputVertex));
		pGraphic->SetVSConstBuffer(shaderTex, &matrixWVP, sizeof(matrixWVP));

		pGraphic->DrawTexture(textures, VIDEO_FILTER_TYPE::VIDEO_FILTER_LINEAR, shaderTex,
				      vertexBufTex);

	} else {
		auto ret = (float)eyePos;
		auto percent = ret / 100.f;

		CameraDesc camera;
		camera.eyePos = {0.0f, 1.50f, -6.0f + percent};
		camera.eyeUpDir = {0.0f, 1.0f, 0.0f};
		camera.lookAt = {0.0f, 0.0f, 1.0f};

		TransposedPerspectiveMatrixWVP(canvas, nullptr, camera, matrixWVP);

		pGraphic->SetVSConstBuffer(shader, &matrixWVP, sizeof(matrixWVP));
		pGraphic->DrawTopplogy(
			shader, vertexBuf, indexId,
			D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
}

void Csample1Dlg::render()
{
	AUTO_GRAPHIC_CONTEXT(pGraphic);

	RECT rcWindow;
	::GetClientRect(m_hWnd, &rcWindow);

	auto cx = rcWindow.right - rcWindow.left;
	auto cy = rcWindow.bottom - rcWindow.top;

	pGraphic->SetDisplaySize(display, cx, cy);

	if (!pGraphic->IsGraphicBuilt()) {
		if (!pGraphic->ReBuildGraphic())
			return;
	}

	if (!texCanvas) {
		TextureInformation vcif;
		vcif.width = cx;
		vcif.height = cy;
		vcif.format = DXGI_FORMAT_B8G8R8A8_UNORM;
		vcif.usage = TEXTURE_USAGE::CANVAS_TARGET;
		vcif.sampleCount = 8;
		texCanvas = pGraphic->CreateTexture(vcif);
	}

	if (pGraphic->BeginRenderCanvas(texCanvas)) {
		pGraphic->ClearBackground(&clrTrans);
		pGraphic->SetBlendState(VIDEO_BLEND_TYPE::BLEND_NORMAL);
		RenderTexture(SIZE(cx, cy), rcWindow, nullptr);
		pGraphic->EndRender();
	}

	if (pGraphic->BeginRenderWindow(display)) {
		pGraphic->ClearBackground(&clrBlue);
		pGraphic->SetBlendState(VIDEO_BLEND_TYPE::BLEND_NORMAL);
		RenderTexture(SIZE(cx, cy), rcWindow, texCanvas);
		pGraphic->EndRender();
	}
}