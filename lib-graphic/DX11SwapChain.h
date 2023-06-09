#pragma once
#include <GraphicBase.h>
#include <D2DRenderTarget.h>

namespace graphic {

static const auto SWAPCHAIN_TEXTURE_FORMAT = GRAPHIC_FORMAT::GF_RGBA;

class DX11GraphicSession;

struct DX11SwapChain : public DX11GraphicBase, public D2DRenderTarget, public ID3DRenderTarget {
	friend class DX11GraphicSession;

public:
	DX11SwapChain(DX11GraphicSession &graphic, HWND hWnd);

	void SetDisplaySize(uint32_t width, uint32_t height);
	HRESULT TestResizeSwapChain();

	virtual ID3D11RenderTargetView *D3DTarget(bool srgb);

	virtual bool BuildGraphic();
	virtual void ReleaseGraphic(bool isForRebuild);
	virtual bool IsBuilt();

private:
	HRESULT InitSwapChain();
	HRESULT CreateTargetView();

private:
	const HWND m_hWnd = 0;
	uint32_t m_dwWidth = 0;
	uint32_t m_dwHeight = 0;

	DXGI_FORMAT m_dxgiFormatTexture = GetResourceViewFormat(SWAPCHAIN_TEXTURE_FORMAT);
	DXGI_FORMAT m_dxgiFormatView = GetResourceViewFormat(SWAPCHAIN_TEXTURE_FORMAT);
	DXGI_FORMAT m_dxgiFormatViewLinear = GetResourceViewLinearFormat(SWAPCHAIN_TEXTURE_FORMAT);

	D3D11_TEXTURE2D_DESC m_descTexture;
	ComPtr<IDXGISwapChain> m_pSwapChain = nullptr;
	ComPtr<ID3D11Texture2D> m_pSwapBackTexture2D = nullptr;
	ComPtr<ID3D11RenderTargetView> m_pRenderTargetView = nullptr;
	ComPtr<ID3D11RenderTargetView> m_pRenderTargetViewLinear = nullptr;
};

} // namespace graphic
