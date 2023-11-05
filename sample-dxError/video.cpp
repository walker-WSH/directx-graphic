#include "pch.h"
#include "sample-1Dlg.h"

class DXCallback;

std::shared_ptr<DXCallback> cb = std::make_shared<DXCallback>();
IGraphicSession *pGraphic = nullptr;
display_handle display = nullptr;


class DXCallback : public IGraphicCallback {
public:
	virtual ~DXCallback() = default;

	virtual void OnGraphicError(HRESULT hr)
	{ 
		OutputDebugStringA(__FUNCTION__);
	}

	virtual void OnRequestRebuild() 
	{ 
		OutputDebugStringA(__FUNCTION__);
	}

	virtual void OnBuildSuccessed(const DXGI_ADAPTER_DESC &desc) 
	{
		OutputDebugStringA(__FUNCTION__);
	}
};

void Csample1Dlg::initGraphic(HWND hWnd)
{
	pGraphic = graphic::CreateGraphicSession();
	AUTO_GRAPHIC_CONTEXT(pGraphic);

	pGraphic->RegisterCallback(cb);

	bool bOK = pGraphic->InitializeGraphic(0);
	assert(bOK);

	display = pGraphic->CreateDisplay(hWnd);
	assert(display);
}

void Csample1Dlg::uninitGraphic()
{
	{
		AUTO_GRAPHIC_CONTEXT(pGraphic);
		pGraphic->DestroyAllGraphicObject();
		pGraphic->UnRegisterCallback(cb.get());
	}

	graphic::DestroyGraphicSession(pGraphic);
}

void Csample1Dlg::render()
{
	AUTO_GRAPHIC_CONTEXT(pGraphic);

	RECT rcWindow;
	::GetClientRect(m_hWnd, &rcWindow);

	pGraphic->SetDisplaySize(display, 
				rcWindow.right - rcWindow.left,
				rcWindow.bottom - rcWindow.top);

	if (!pGraphic->IsGraphicBuilt()) {
		if (!pGraphic->ReBuildGraphic())
			return;
	}

	if (pGraphic->BeginRenderWindow(display)) {
		pGraphic->EndRender();
	}
}