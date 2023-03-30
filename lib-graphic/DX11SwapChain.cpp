#include "DX11SwapChain.h"
#include "GraphicSessionImp.h"

namespace graphic {

DX11SwapChain::DX11SwapChain(DX11GraphicSession &graphic, HWND hWnd)
	: DX11GraphicBase(graphic, "swapchain"), D2DRenderTarget(graphic, true), m_hWnd(hWnd)
{
	RECT rc;
	GetClientRect(hWnd, &rc);

	m_dwWidth = rc.right - rc.left;
	m_dwHeight = rc.bottom - rc.top;

	BuildGraphic();
}

bool DX11SwapChain::BuildGraphic()
{
	CHECK_GRAPHIC_CONTEXT_EX(DX11GraphicBase::DX11GraphicBase::m_graphicSession);

	HRESULT hr = InitSwapChain();
	if (FAILED(hr)) {
		CHECK_DX_ERROR(hr, "InitSwapChain %X", this);
		ReleaseGraphic(false);
		return false;
	}

	NotifyRebuildEvent();
	return true;
}

void DX11SwapChain::ReleaseGraphic(bool isForRebuild)
{
	CHECK_GRAPHIC_CONTEXT_EX(DX11GraphicBase::DX11GraphicBase::m_graphicSession);

	ZeroMemory(&m_descTexture, sizeof(D3D11_TEXTURE2D_DESC));
	m_pSwapChain = nullptr;
	m_pSwapBackTexture2D = nullptr;
	m_pRenderTargetView = nullptr;
	m_pRenderTargetViewLinear = nullptr;

	D2DRenderTarget::ReleaseD2D();

	NotifyReleaseEvent(isForRebuild);
}

bool DX11SwapChain::IsBuilt()
{
	CHECK_GRAPHIC_CONTEXT_EX(DX11GraphicBase::m_graphicSession);
	return m_pSwapChain && m_pSwapBackTexture2D && m_pRenderTargetView && m_pRenderTargetViewLinear;
}

void DX11SwapChain::SetDisplaySize(uint32_t width, uint32_t height)
{
	m_dwWidth = width;
	m_dwHeight = height;
}

HRESULT DX11SwapChain::TestResizeSwapChain()
{
	CHECK_GRAPHIC_CONTEXT_EX(DX11GraphicBase::m_graphicSession);

	if (!m_pSwapChain || !m_dwWidth || !m_dwHeight)
		return S_FALSE;

	if (m_descTexture.Width != m_dwWidth || m_descTexture.Height != m_dwHeight) {
		D2DRenderTarget::ReleaseD2D();

		ID3D11RenderTargetView *pRenderView = NULL;
		DX11GraphicBase::m_graphicSession.D3DContext()->OMSetRenderTargets(1, &pRenderView, NULL);

		m_pRenderTargetViewLinear = nullptr;
		m_pRenderTargetView = nullptr;
		m_pSwapBackTexture2D = nullptr;
		ZeroMemory(&m_descTexture, sizeof(D3D11_TEXTURE2D_DESC));

		HRESULT hr = m_pSwapChain->ResizeBuffers(1, m_dwWidth, m_dwHeight, m_dxgiFormatTexture, 0);
		if (FAILED(hr)) {
			CHECK_DX_ERROR(hr, "ResizeBuffers %ux%u %X", m_dwWidth, m_dwHeight, this);
			assert(false && "m_pSwapChain->ResizeBuffers failed");
			return hr;
		}

		return CreateTargetView();
	}

	return S_OK;
}

HRESULT DX11SwapChain::InitSwapChain()
{
	CHECK_GRAPHIC_CONTEXT_EX(DX11GraphicBase::m_graphicSession);

	assert(m_dwWidth > 0 && m_dwHeight > 0);

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferDesc.Width = m_dwWidth;
	sd.BufferDesc.Height = m_dwHeight;
	sd.BufferDesc.Format = m_dxgiFormatTexture;
	sd.SampleDesc.Count = 1;
	sd.BufferCount = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = m_hWnd;
	sd.Windowed = TRUE;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;

	HRESULT hr = DX11GraphicBase::m_graphicSession.D3DFactory()->CreateSwapChain(
		DX11GraphicBase::m_graphicSession.D3DDevice(), &sd, m_pSwapChain.Assign());
	if (FAILED(hr)) {
		CHECK_DX_ERROR(hr, "CreateSwapChain %ux%u %X", m_dwWidth, m_dwHeight, this);
		assert(false);
		return hr;
	}

	return CreateTargetView();
}

HRESULT DX11SwapChain::CreateTargetView()
{
	HRESULT hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
					     reinterpret_cast<void **>(m_pSwapBackTexture2D.Assign()));
	if (FAILED(hr)) {
		CHECK_DX_ERROR(hr, "m_pSwapChain->GetBuffer %X", this);
		assert(false);
		return hr;
	}
	m_pSwapBackTexture2D->GetDesc(&m_descTexture);

	ComPtr<IDXGISurface1> sfc;
	hr = m_pSwapChain->GetBuffer(0, __uuidof(IDXGISurface1), reinterpret_cast<void **>(sfc.Assign()));
	assert(SUCCEEDED(hr));
	if (SUCCEEDED(hr)) {
		// Here we ignore the result of building shared D2D, because we can not ensure it is valid on different format.
		D2DRenderTarget::BuildD2DFromDXGI(sfc, m_descTexture.Format);
	}

	//---------------------------------------------------------------------------------------------------------
	D3D11_RENDER_TARGET_VIEW_DESC rtv;
	rtv.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtv.Texture2D.MipSlice = 0;

	rtv.Format = m_dxgiFormatView;
	hr = DX11GraphicBase::m_graphicSession.D3DDevice()->CreateRenderTargetView(m_pSwapBackTexture2D, &rtv,
										   m_pRenderTargetView.Assign());
	if (FAILED(hr)) {
		CHECK_DX_ERROR(hr, "CreateRenderTargetView %X", this);
		assert(false);
		return hr;
	}

	rtv.Format = m_dxgiFormatViewLinear;
	hr = DX11GraphicBase::m_graphicSession.D3DDevice()->CreateRenderTargetView(m_pSwapBackTexture2D, &rtv,
										   m_pRenderTargetViewLinear.Assign());
	if (FAILED(hr)) {
		CHECK_DX_ERROR(hr, "CreateRenderTargetView SRGB %X", this);
		assert(false);
		return hr;
	}

	return S_OK;
}

} // namespace graphic
