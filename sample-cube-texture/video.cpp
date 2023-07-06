#include "pch.h"
#include "sample-1Dlg.h"
#include <string>
#include <format>

const auto IMAGE_COUNT = 6;

IGraphicSession *pGraphic = nullptr;
shader_handle shader = nullptr;
long indexId = INVALID_INDEX_ID;
display_handle display = nullptr;
texture_handle texCube = nullptr;

ColorRGBA clrBlack = {0, 0, 0, 1.f};
ColorRGBA clrBlue = {0, 0, 1, 1.f};

// 6个面 每个面四个顶点（每个顶点对应纹理坐标不一样 所以只能每个面单独定义一次顶点）
static const auto TEXTURE_VERTEX_COUNT = IMAGE_COUNT * 4;
static const auto TEXTURE_INDEX_COUNT = 36;

struct TextureVertexDesc {
	XMFLOAT3 Pos;
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

	std::wstring dir = GetShaderDirectory();

	shaderInfo.vsFile = dir + L"box-vs.cso";
	shaderInfo.psFile = dir + L"box-ps.cso";

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
	TextureVertexDesc vertices[TEXTURE_VERTEX_COUNT];
	float w2 = 1.f, h2 = 1.f, d2 = 1.f;

	// 右面(+X面)
	vertices[0].Pos = XMFLOAT3(w2, -h2, -d2);
	vertices[1].Pos = XMFLOAT3(w2, h2, -d2);
	vertices[2].Pos = XMFLOAT3(w2, h2, d2);
	vertices[3].Pos = XMFLOAT3(w2, -h2, d2);
	// 左面(-X面)
	vertices[4].Pos = XMFLOAT3(-w2, -h2, d2);
	vertices[5].Pos = XMFLOAT3(-w2, h2, d2);
	vertices[6].Pos = XMFLOAT3(-w2, h2, -d2);
	vertices[7].Pos = XMFLOAT3(-w2, -h2, -d2);
	// 顶面(+Y面)
	vertices[8].Pos = XMFLOAT3(-w2, h2, -d2);
	vertices[9].Pos = XMFLOAT3(-w2, h2, d2);
	vertices[10].Pos = XMFLOAT3(w2, h2, d2);
	vertices[11].Pos = XMFLOAT3(w2, h2, -d2);
	// 底面(-Y面)
	vertices[12].Pos = XMFLOAT3(w2, -h2, -d2);
	vertices[13].Pos = XMFLOAT3(w2, -h2, d2);
	vertices[14].Pos = XMFLOAT3(-w2, -h2, d2);
	vertices[15].Pos = XMFLOAT3(-w2, -h2, -d2);
	// 背面(+Z面)
	vertices[16].Pos = XMFLOAT3(w2, -h2, d2);
	vertices[17].Pos = XMFLOAT3(w2, h2, d2);
	vertices[18].Pos = XMFLOAT3(-w2, h2, d2);
	vertices[19].Pos = XMFLOAT3(-w2, -h2, d2);
	// 正面(-Z面)
	vertices[20].Pos = XMFLOAT3(-w2, -h2, -d2);
	vertices[21].Pos = XMFLOAT3(-w2, h2, -d2);
	vertices[22].Pos = XMFLOAT3(w2, h2, -d2);
	vertices[23].Pos = XMFLOAT3(w2, -h2, -d2);

	pGraphic->SetVertexBuffer(shader, vertices, sizeof(vertices));

	WORD indices[] = {
		0,  1,  2,  2,  3,  0,  // 右面(+X面)
		4,  5,  6,  6,  7,  4,  // 左面(-X面)
		8,  9,  10, 10, 11, 8,  // 顶面(+Y面)
		12, 13, 14, 14, 15, 12, // 底面(-Y面)
		16, 17, 18, 18, 19, 16, // 背面(+Z面)
		20, 21, 22, 22, 23, 20  // 正面(-Z面)
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

	std::vector<texture_handle> images;
	images.reserve(IMAGE_COUNT);

	TextureInformation texDesc;
	for (auto i = 0; i < IMAGE_COUNT; i++) {
		//auto path = std::format(L"res/daylight{}.png", i);
		auto path = std::format(L"res/test{}.jpg", i);
		auto texImg = pGraphic->OpenImageTexture(path.c_str());
		images.push_back(texImg);

		auto texInfo = pGraphic->GetTextureInfo(texImg);
		if (i == 0) {
			texDesc = texInfo;
		} else {
			assert(texInfo.width == texDesc.width);
			assert(texInfo.height == texDesc.height);
			assert(texInfo.format == texDesc.format);
		}
	}

	texDesc.usage = TEXTURE_USAGE::CUBE_TEXTURE;
	texCube = pGraphic->CreateTexture(texDesc);

	TextureCopyRegion sub;
	sub.srcRight = texDesc.width;
	sub.srcBottom = texDesc.height;

	for (auto i = 0; i < IMAGE_COUNT; i++) {
		sub.destArraySliceIndex = i;
		pGraphic->CopyTexture(texCube, images[i], &sub);
	}

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

	eyePosZ = max(eyePosZ, -98);
	eyePosZ = min(eyePosZ, 98);
	float eyeZ = (float)eyePosZ / 100.f;

	CameraDesc camera;
	camera.eyePos = {0.0f, 0.0f, eyeZ};
	camera.eyeUpDir = {0.0f, 1.0f, 0.0f};
	camera.lookAt = {0.0f, 0.0f, 10.f};

	std::vector<WorldVector> worldTemp = worldList;
	if (m_bLBDown) {
		worldTemp.push_back(get_rotate());
	}

	XMMATRIX matrixWVP;
	TransposedPerspectiveMatrixWVP(canvas, &worldTemp, camera, matrixWVP);

	pGraphic->SetVSConstBuffer(shader, &matrixWVP, sizeof(matrixWVP));

	std::vector<texture_handle> textures = {tex};
	pGraphic->DrawTexture(shader, VIDEO_FILTER_TYPE::VIDEO_FILTER_ANISOTROPIC, textures,
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
		pGraphic->SetRasterizerState(
			D3D11_CULL_MODE::D3D11_CULL_NONE); // 剔除三角形其中一面的画面，剔除哪一面 也和设置的顶点顺逆时针有关

		RenderTexture(texCube, SIZE(rcWindow.right, rcWindow.bottom), rcWindow);

		pGraphic->EndRender();
	}
}