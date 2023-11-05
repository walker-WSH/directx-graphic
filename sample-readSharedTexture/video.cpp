#include "pch.h"
#include "sample-1Dlg.h"

HANDLE sharedHandle = (HANDLE)0X00001; // the id of shared texture

IGraphicSession *pGraphic = nullptr;
texture_handle sharedTex = nullptr;
texture_handle readTex = nullptr;

void Csample1Dlg::initGraphic(HWND hWnd)
{
	pGraphic = graphic::CreateGraphicSession();
	AUTO_GRAPHIC_CONTEXT(pGraphic);

	bool bOK = pGraphic->InitializeGraphic(0);
	assert(bOK);

	sharedTex = pGraphic->OpenSharedTexture(sharedHandle);
	assert(sharedTex);

	if (sharedTex) {
		auto info = pGraphic->GetTextureInfo(sharedTex);
		info.usage = TEXTURE_USAGE::READ_TEXTURE;

		readTex = pGraphic->CreateTexture(info, CREATE_TEXTURE_FLAG_SHARED_MUTEX);
		assert(readTex);
	}
}

void Csample1Dlg::uninitGraphic()
{
	{
		AUTO_GRAPHIC_CONTEXT(pGraphic);
		pGraphic->DestroyAllGraphicObject();
	}

	graphic::DestroyGraphicSession(pGraphic);
}

void Csample1Dlg::render()
{
	AUTO_GRAPHIC_CONTEXT(pGraphic);

	if (!pGraphic->IsGraphicBuilt()) {
		if (!pGraphic->ReBuildGraphic())
			return;
	}

	if (!sharedTex || !readTex)
		return;

	if (pGraphic->CopyTexture(readTex, sharedTex)) {

		D3D11_MAPPED_SUBRESOURCE data;
		if (pGraphic->MapTexture(readTex, MAP_TEXTURE_FEATURE::FOR_READ_TEXTURE, &data)) {

			/*
			data.pData;
			data.RowPitch
			*/

			pGraphic->UnmapTexture(readTex);
		}
	}
}