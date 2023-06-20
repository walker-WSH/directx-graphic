#include "pch.h"
#include "sample-1Dlg.h"

IGraphicSession *pGraphic = nullptr;
shader_handle shader = nullptr;
display_handle display = nullptr;
texture_handle texImg = nullptr;

ColorRGBA clrBlack = {0, 0, 0, 1.f};
ColorRGBA clrBlue = {0, 0, 1, 1.f};

static const auto TEXTURE_VERTEX_COUNT = 4;
static const auto INPUT_POS_TYPE = VERTEX_INPUT_TYPE::POSITION;
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
	desc.type = INPUT_POS_TYPE;
	desc.size = 16;
	shaderInfo.vertexDesc.push_back(desc);

	desc.type = VERTEX_INPUT_TYPE::TEXTURE_COORD;
	desc.size = 8;
	shaderInfo.vertexDesc.push_back(desc);

	std::wstring dir = GetShaderDirectory();

	shaderInfo.vsFile = dir + L"default-vs.cso";
	shaderInfo.psFile = dir + L"default-ps.cso";

	shaderInfo.vsBufferSize = sizeof(matrixWVP);
	shaderInfo.psBufferSize = 0;

	shaderInfo.vertexCount = TEXTURE_VERTEX_COUNT;
	shaderInfo.perVertexSize = sizeof(TextureVertexDesc);

	shader = pGraphic->CreateShader(shaderInfo);
	assert(shader);
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

float Csample1Dlg::getRotate()
{
	auto ret = (float)m_sliderRotate.GetPos();
	ATLTRACE("--------------------- %f \n", ret);
	return ret;
}

void Csample1Dlg::RenderTexture(texture_handle tex, SIZE canvas, RECT drawDest)
{
	AUTO_GRAPHIC_CONTEXT(pGraphic);

	auto texInfo = pGraphic->GetTextureInfo(tex);
	std::vector<texture_handle> textures = {tex};

	float matrixWVP[4][4];
	std::vector<WorldVector> worldList;

	auto half_width = float(drawDest.right) / float(drawDest.bottom);

	{
		WorldVector vec;
		vec.type = WORLD_TYPE::VECTOR_MOVE;
		vec.x = -half_width;
		worldList.push_back(vec);
	}
	{
		WorldVector vec;
		vec.type = WORLD_TYPE::VECTOR_ROTATE;
		vec.y = getRotate();
		worldList.push_back(vec);
	}
	{
		WorldVector vec;
		vec.type = WORLD_TYPE::VECTOR_MOVE;
		vec.x = half_width;
		worldList.push_back(vec);
	}

	{
		WorldVector vec;
		vec.type = WORLD_TYPE::VECTOR_SCALE;
		vec.x = vec.y = 0.5f;
		//worldList.push_back(vec);
	}

	TextureVertexDesc outputVertex[TEXTURE_VERTEX_COUNT];
	float leftUV = 0.f;
	float topUV = 0.f;
	float rightUV = 1.f;
	float bottomUV = 1.f;
	const bool use3D = true;
	if (use3D) {
		auto half_width = float(drawDest.right) / float(drawDest.bottom);
		auto half_height = 1.f;

		outputVertex[0] = {-half_width, half_height, 0, 1.f, leftUV, topUV};
		outputVertex[1] = {half_width, half_height, 0, 1.f, rightUV, topUV};
		outputVertex[2] = {-half_width, -half_height, 0, 1.f, leftUV, bottomUV};
		outputVertex[3] = {half_width, -half_height, 0, 1.f, rightUV, bottomUV};

		// should use this project matrix for 3D render
		TransposedPerspectiveMatrixWVP(canvas, &worldList, matrixWVP);

	} else {
		outputVertex[0] = {-(float)texInfo.width / 2, (float)texInfo.height / 2, 0, 1.f, leftUV, topUV};
		outputVertex[1] = {(float)texInfo.width / 2, (float)texInfo.height / 2, 0, 1.f, rightUV, topUV};
		outputVertex[2] = {-(float)texInfo.width / 2, -(float)texInfo.height / 2, 0, 1.f, leftUV, bottomUV};
		outputVertex[3] = {(float)texInfo.width / 2, -(float)texInfo.height / 2, 0, 1.f, rightUV, bottomUV};

		// should use this orthogonal project matrix for 2D render
		TransposedOrthoMatrixWVP(canvas, false, &worldList, matrixWVP);
	}

	pGraphic->SetVertexBuffer(shader, outputVertex, sizeof(outputVertex));
	pGraphic->SetVSConstBuffer(shader, &(matrixWVP[0][0]), sizeof(matrixWVP));
	pGraphic->DrawTexture(shader, VIDEO_FILTER_TYPE::VIDEO_FILTER_LINEAR, textures);
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