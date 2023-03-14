#include "DX11Texture2D.h"
#include "GraphicSessionImp.h"
#include <d3dcompiler.h>
#include <dxsdk/include/D3DX11tex.h>

namespace graphic {

std::map<TEXTURE_USAGE, std::string> DX11Texture2D::mapTextureUsage = {
	{TEXTURE_USAGE::UNKNOWN, "unknownTextureUsage"},  {TEXTURE_USAGE::CANVAS_TARGET, "canvasTexture"},
	{TEXTURE_USAGE::READ_TEXTURE, "readTexture"},     {TEXTURE_USAGE::WRITE_TEXTURE, "writeTexture"},
	{TEXTURE_USAGE::SHARED_TEXTURE, "sharedTexture"}, {TEXTURE_USAGE::STATIC_IMAGE_FILE, "imageTexture"},
};

DX11Texture2D::DX11Texture2D(DX11GraphicSession &graphic, const TextureInformation &info)
	: DX11GraphicBase(graphic, mapTextureUsage[info.usage].c_str()),
	  D2DRenderTarget(graphic, false),
	  m_textureInfo(info)
{
	BuildGraphic();
}

DX11Texture2D::DX11Texture2D(DX11GraphicSession &graphic, HANDLE handle)
	: DX11GraphicBase(graphic, mapTextureUsage[TEXTURE_USAGE::SHARED_TEXTURE].c_str()),
	  D2DRenderTarget(graphic, false),
	  m_hSharedHandle(handle)
{
	m_textureInfo.usage = TEXTURE_USAGE::SHARED_TEXTURE;
	BuildGraphic();
}

DX11Texture2D::DX11Texture2D(DX11GraphicSession &graphic, const WCHAR *fullPath)
	: DX11GraphicBase(graphic, mapTextureUsage[TEXTURE_USAGE::STATIC_IMAGE_FILE].c_str()),
	  D2DRenderTarget(graphic, false),
	  m_strImagePath(fullPath)
{
	m_textureInfo.usage = TEXTURE_USAGE::STATIC_IMAGE_FILE;
	BuildGraphic();
}

bool DX11Texture2D::BuildGraphic()
{
	CHECK_GRAPHIC_CONTEXT_EX(DX11GraphicBase::m_graphicSession);

	bool bSuccessed = false;
	switch (m_textureInfo.usage) {
	case TEXTURE_USAGE::CANVAS_TARGET:
		bSuccessed = InitTargetTexture();
		break;

	case TEXTURE_USAGE::READ_TEXTURE:
		bSuccessed = InitReadTexture();
		break;

	case TEXTURE_USAGE::WRITE_TEXTURE:
		bSuccessed = InitWriteTexture();
		break;

	case TEXTURE_USAGE::SHARED_TEXTURE:
		bSuccessed = InitSharedTexture();
		break;

	case TEXTURE_USAGE::STATIC_IMAGE_FILE:
		bSuccessed = InitImageTexture();
		break;

	default:
		assert(false);
		break;
	}

	if (bSuccessed)
		m_pTexture2D->GetDesc(&m_descTexture);
	else
		ReleaseGraphic();

	return bSuccessed;
}

void DX11Texture2D::ReleaseGraphic()
{
	CHECK_GRAPHIC_CONTEXT_EX(DX11GraphicBase::m_graphicSession);

	ZeroMemory(&m_descTexture, sizeof(D3D11_TEXTURE2D_DESC));
	m_pTexture2D = nullptr;
	m_pRenderTargetView = nullptr;
	m_pTextureResView = nullptr;

	D2DRenderTarget::ReleaseD2D();
}

bool DX11Texture2D::InitWriteTexture()
{
	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = m_textureInfo.width;
	desc.Height = m_textureInfo.height;
	desc.Format = m_textureInfo.format;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.SampleDesc.Count = 1;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.Usage = D3D11_USAGE_DYNAMIC;

	HRESULT hr =
		DX11GraphicBase::m_graphicSession.D3DDevice()->CreateTexture2D(&desc, nullptr, m_pTexture2D.Assign());
	if (FAILED(hr)) {
		CHECK_DX_ERROR(hr, "CreateTexture2D for write, %ux%u format:%d %X", m_textureInfo.width,
			       m_textureInfo.height, (int)m_textureInfo.format, this);
		assert(false);
		return false;
	}

	if (!InitResourceView())
		return false;

	D3D11_MAPPED_SUBRESOURCE map;
	hr = DX11GraphicBase::m_graphicSession.D3DContext()->Map(m_pTexture2D, 0, D3D11_MAP_WRITE_DISCARD, 0, &map);
	if (FAILED(hr)) {
		CHECK_DX_ERROR(hr, "mapWriteTexture %X", this);
		assert(false);
		return false;
	}

	DX11GraphicBase::m_graphicSession.D3DContext()->Unmap(m_pTexture2D, 0);
	return true;
}

bool DX11Texture2D::InitReadTexture()
{
	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = m_textureInfo.width;
	desc.Height = m_textureInfo.height;
	desc.Format = m_textureInfo.format;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_STAGING;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

	HRESULT hr =
		DX11GraphicBase::m_graphicSession.D3DDevice()->CreateTexture2D(&desc, nullptr, m_pTexture2D.Assign());
	if (FAILED(hr)) {
		CHECK_DX_ERROR(hr, "CreateTexture2D for read, %ux%u format:%d %X", m_textureInfo.width,
			       m_textureInfo.height, (int)m_textureInfo.format, this);
		assert(false);
		return false;
	}

	D3D11_MAPPED_SUBRESOURCE map;
	hr = DX11GraphicBase::m_graphicSession.D3DContext()->Map(m_pTexture2D, 0, D3D11_MAP_READ, 0, &map);
	if (FAILED(hr)) {
		CHECK_DX_ERROR(hr, "mapReadTexture");
		assert(false);
		return false;
	}

	DX11GraphicBase::m_graphicSession.D3DContext()->Unmap(m_pTexture2D, 0);
	return true;
}

bool DX11Texture2D::InitTargetTexture()
{
	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = m_textureInfo.width;
	desc.Height = m_textureInfo.height;
	desc.Format = m_textureInfo.format;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

	desc.MiscFlags |= D3D11_RESOURCE_MISC_SHARED;
	if (DXGI_FORMAT_B8G8R8A8_UNORM == m_textureInfo.format)
		desc.MiscFlags |= D3D11_RESOURCE_MISC_GDI_COMPATIBLE;

	HRESULT hr =
		DX11GraphicBase::m_graphicSession.D3DDevice()->CreateTexture2D(&desc, nullptr, m_pTexture2D.Assign());
	if (FAILED(hr)) {
		CHECK_DX_ERROR(hr, "CreateTexture2D for target, %ux%u format:%d %X", m_textureInfo.width,
			       m_textureInfo.height, (int)m_textureInfo.format, this);
		assert(false);
		return false;
	}

	hr = DX11GraphicBase::m_graphicSession.D3DDevice()->CreateRenderTargetView(m_pTexture2D, nullptr,
										   m_pRenderTargetView.Assign());
	if (FAILED(hr)) {
		CHECK_DX_ERROR(hr, "CreateRenderTargetView %X", this);
		assert(false);
		return false;
	}

	if (!InitResourceView())
		return false;

	ComPtr<IDXGIResource> pDXGIRes = nullptr;
	hr = m_pTexture2D->QueryInterface(__uuidof(IDXGIResource), (LPVOID *)(pDXGIRes.Assign()));
	if (FAILED(hr)) {
		CHECK_DX_ERROR(hr, "query-IDXGIResource %X", this);
		assert(false);
		return false;
	}

	hr = pDXGIRes->GetSharedHandle(&m_hSharedHandle);
	if (FAILED(hr)) {
		CHECK_DX_ERROR(hr, "GetSharedHandle %X", this);
		assert(false);
		return false;
	}

	if (m_textureInfo.format == D2D_COMPATIBLE_FORMAT) {
		ComPtr<IDXGISurface1> sfc;
		hr = DX11GraphicBase::m_graphicSession.D3DDevice()->OpenSharedResource(
			m_hSharedHandle, __uuidof(IDXGISurface1), (LPVOID *)(sfc.Assign()));
		if (SUCCEEDED(hr))
			D2DRenderTarget::BuildD2D(sfc);
	}

	return true;
}

bool DX11Texture2D::InitSharedTexture()
{
	HRESULT hr = DX11GraphicBase::m_graphicSession.D3DDevice()->OpenSharedResource(
		(HANDLE)m_hSharedHandle, __uuidof(ID3D11Texture2D), (void **)m_pTexture2D.Assign());
	if (FAILED(hr)) {
		CHECK_DX_ERROR(hr, "OpenSharedResource HANDLE:%X %X", m_hSharedHandle, this);
		return false;
	}

	if (!InitResourceView())
		return false;

	return true;
}

bool DX11Texture2D::InitImageTexture()
{
	HRESULT hr = D3DX11CreateShaderResourceViewFromFile(DX11GraphicBase::m_graphicSession.D3DDevice(),
							    m_strImagePath.c_str(), NULL, NULL,
							    m_pTextureResView.Assign(), NULL);
	if (FAILED(hr)) {
		CHECK_DX_ERROR(hr, "D3DX11CreateShaderResourceViewFromFile %X", this);
		assert(false);
		return false;
	}

	ComPtr<ID3D11Resource> pResource;
	m_pTextureResView->GetResource(pResource.Assign());
	if (!pResource) {
		CHECK_DX_ERROR(hr, "m_pTextureResView->GetResource %X", this);
		assert(false);
		return false;
	}

	hr = pResource->QueryInterface<ID3D11Texture2D>(m_pTexture2D.Assign());
	if (FAILED(hr)) {
		CHECK_DX_ERROR(hr, "QueryInterfaceID3D11Texture2D %X", this);
		assert(false);
		return false;
	}

	return true;
}

bool DX11Texture2D::InitResourceView()
{
	D3D11_TEXTURE2D_DESC desc = {};
	m_pTexture2D->GetDesc(&desc);

	D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc{};
	viewDesc.Format = desc.Format;
	viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	viewDesc.Texture2D.MipLevels = 1;

	HRESULT hr = DX11GraphicBase::m_graphicSession.D3DDevice()->CreateShaderResourceView(
		m_pTexture2D, &viewDesc, m_pTextureResView.Assign());
	if (FAILED(hr)) {
		CHECK_DX_ERROR(hr, "CreateShaderResourceView %X", this);
		assert(false);
		return false;
	}

	return true;
}

} // namespace graphic
