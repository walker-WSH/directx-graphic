#pragma once
#include <GraphicBase.h>
#include <D2DRenderTarget.h>

namespace graphic {

class DX11GraphicSession;

struct DX11SwapChain : public DX11GraphicBase, public D2DRenderTarget {
	friend class DX11GraphicSession;

public:
	DX11SwapChain(DX11GraphicSession &graphic, HWND hWnd);

	void SetDisplaySize(uint32_t width, uint32_t height);
	HRESULT TestResizeSwapChain();

	virtual bool BuildGraphic();
	virtual void ReleaseGraphic(bool isForRebuild);
	virtual bool IsBuilt();

private:
	HRESULT InitSwapChain();
	HRESULT CreateTargetView();

private:
	HWND m_hWnd = 0;
	uint32_t m_dwWidth = 0;
	uint32_t m_dwHeight = 0;

	D3D11_TEXTURE2D_DESC m_descTexture;
	ComPtr<IDXGISwapChain> m_pSwapChain = nullptr;
	ComPtr<ID3D11Texture2D> m_pSwapBackTexture2D = nullptr;
	ComPtr<ID3D11RenderTargetView> m_pRenderTargetView = nullptr;
};

} // namespace graphic