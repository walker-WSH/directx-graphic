#include "DX11Texture2D.h"
#include "IGraphicEngine.h"
#include "GraphicSessionImp.h"
#include <d3dcompiler.h>
#include <dxsdk/include/D3DX11tex.h>

namespace graphic {

std::map<TEXTURE_USAGE, std::string> DX11Texture2D::mapTextureUsage = {
	{TEXTURE_USAGE::UNKNOWN, "unknownTextureUsage"},
	{TEXTURE_USAGE::CANVAS_TARGET, "canvasTexture"},
	{TEXTURE_USAGE::READ_TEXTURE, "readTexture"},
	{TEXTURE_USAGE::WRITE_TEXTURE, "writeTexture"},
	{TEXTURE_USAGE::SHARED_TEXTURE, "sharedTexture"},
	{TEXTURE_USAGE::STATIC_IMAGE_FILE, "imageTexture"},
	{TEXTURE_USAGE::CUBE_TEXTURE, "cubeTexture"},
};

DX11Texture2D::DX11Texture2D(DX11GraphicSession &graphic, const TextureInformation &info, int flags)
	: DX11GraphicBase(graphic, mapTextureUsage[info.usage].c_str()),
	  D2DRenderTarget(graphic, false),
	  m_textureInfo(info),
	  m_nCreateFlags(flags)
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

ID3D11RenderTargetView *DX11Texture2D::D3DTarget(bool srgb)
{
	CHECK_GRAPHIC_CONTEXT_EX(DX11GraphicBase::m_graphicSession);
	assert(m_textureInfo.usage == TEXTURE_USAGE::CANVAS_TARGET);
	return m_pRenderTargetView.Get();
}

bool DX11Texture2D::BuildGraphic()
{
	CHECK_GRAPHIC_CONTEXT_EX(DX11GraphicBase::m_graphicSession);

	bool bSuccessed = false;
	switch (m_textureInfo.usage) {
	case TEXTURE_USAGE::CANVAS_TARGET:
	case TEXTURE_USAGE::CUBE_TEXTURE:
		bSuccessed = InitTargetTexture(m_textureInfo.usage == TEXTURE_USAGE::CUBE_TEXTURE);
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

	case TEXTURE_USAGE::READ_TEXTURE:
		bSuccessed = InitReadTexture();
		break;

	default:
		assert(false);
		break;
	}

	if (bSuccessed) {
		m_pTexture2D->GetDesc(&m_descTexture);
		NotifyRebuildEvent();
	} else {
		ReleaseGraphic(false);
	}

	return bSuccessed;
}

void DX11Texture2D::ReleaseGraphic(bool isForRebuild)
{
	CHECK_GRAPHIC_CONTEXT_EX(DX11GraphicBase::m_graphicSession);

	ZeroMemory(&m_descTexture, sizeof(D3D11_TEXTURE2D_DESC));
	m_pTexture2D = nullptr;
	m_pRenderTargetView = nullptr;
	m_pTextureResView = nullptr;

	D2DRenderTarget::ReleaseD2D();

	NotifyReleaseEvent(isForRebuild);
}

void DX11Texture2D::LockTexture()
{
	if (m_pKeyedMutex)
		m_pKeyedMutex->AcquireSync(0, INFINITE);
}

void DX11Texture2D::UnlockTexture()
{
	if (m_pKeyedMutex)
		m_pKeyedMutex->ReleaseSync(0);
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

	HRESULT hr = DX11GraphicBase::m_graphicSession.D3DDevice()->CreateTexture2D(
		&desc, nullptr, m_pTexture2D.ReleaseAndGetAddressOf());
	if (FAILED(hr)) {
		CHECK_DX_ERROR(hr, "CreateTexture2D for write, %ux%u format:%d %X",
			       m_textureInfo.width, m_textureInfo.height, (int)m_textureInfo.format,
			       this);
		assert(false);
		return false;
	}

	if (!InitResourceView())
		return false;

	D3D11_MAPPED_SUBRESOURCE map;
	hr = DX11GraphicBase::m_graphicSession.D3DContext()->Map(m_pTexture2D.Get(), 0,
								 D3D11_MAP_WRITE_DISCARD, 0, &map);
	if (FAILED(hr)) {
		CHECK_DX_ERROR(hr, "mapWriteTexture %X", this);
		assert(false);
		return false;
	}

	DX11GraphicBase::m_graphicSession.D3DContext()->Unmap(m_pTexture2D.Get(), 0);
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

	HRESULT hr = DX11GraphicBase::m_graphicSession.D3DDevice()->CreateTexture2D(
		&desc, nullptr, m_pTexture2D.ReleaseAndGetAddressOf());
	if (FAILED(hr)) {
		CHECK_DX_ERROR(hr, "CreateTexture2D for read, %ux%u format:%d %X",
			       m_textureInfo.width, m_textureInfo.height, (int)m_textureInfo.format,
			       this);
		assert(false);
		return false;
	}

	D3D11_MAPPED_SUBRESOURCE map;
	hr = DX11GraphicBase::m_graphicSession.D3DContext()->Map(m_pTexture2D.Get(), 0,
								 D3D11_MAP_READ, 0, &map);
	if (FAILED(hr)) {
		CHECK_DX_ERROR(hr, "mapReadTexture");
		assert(false);
		return false;
	}

	DX11GraphicBase::m_graphicSession.D3DContext()->Unmap(m_pTexture2D.Get(), 0);
	return true;
}

bool DX11Texture2D::InitTargetTexture(bool cube)
{
	if (cube && m_textureInfo.width != m_textureInfo.height) {
		LOG_WARN("invalid size for cube texture: %dx%d", m_textureInfo.width,
			 m_textureInfo.height);
		assert(false);
	}

	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = m_textureInfo.width;
	desc.Height = m_textureInfo.height;
	desc.Format = m_textureInfo.format;
	desc.MipLevels = 1;
	desc.ArraySize = cube ? 6 : 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

	/*
	desc.SampleDesc.Count = 1;      // 无MSAA
	desc.SampleDesc.Count = 2;      // 2x MSAA（性能与质量的平衡）
	desc.SampleDesc.Count = 4;      // 4x MSAA（最常用，效果明显）
	desc.SampleDesc.Count = 8;      // 8x MSAA（高质量，性能开销大）
	*/
	if (m_textureInfo.enableMSAA) {
		UINT qualityLevels; // 检查设备支持的MSAA级别
		DX11GraphicBase::m_graphicSession.D3DDevice()->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &qualityLevels);

		if (qualityLevels > 0) {
			desc.SampleDesc.Count = 4;
			desc.SampleDesc.Quality = qualityLevels - 1;
		} else { // 不支持4x MSAA，尝试2x
			DX11GraphicBase::m_graphicSession.D3DDevice()->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 2, &qualityLevels);
			desc.SampleDesc.Count = 2;
			desc.SampleDesc.Quality = qualityLevels - 1;
		}
	} else {
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
	}

	bool msaaEnabled = desc.SampleDesc.Count > 1;

	if (cube) {
		desc.MiscFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE; //  cube must include

	} else {
		if (!msaaEnabled) {
			if (m_nCreateFlags & CREATE_TEXTURE_FLAG_SHARED_MUTEX) {
				desc.MiscFlags |= D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX;
			} else {
				desc.MiscFlags |= D3D11_RESOURCE_MISC_SHARED;
			}
		}

		if (m_nCreateFlags & CREATE_TEXTURE_FLAG_GDI_SHARED) {
			if (DXGI_FORMAT_B8G8R8A8_UNORM == m_textureInfo.format) {
				desc.MiscFlags |= D3D11_RESOURCE_MISC_GDI_COMPATIBLE; //  cube must not include
			} else {
				assert(false && "unsupported format");
			}
		}
	}

	HRESULT hr = DX11GraphicBase::m_graphicSession.D3DDevice()->CreateTexture2D(
		&desc, nullptr, m_pTexture2D.ReleaseAndGetAddressOf());
	if (FAILED(hr)) {
		CHECK_DX_ERROR(hr, "CreateTexture2D for target, %ux%u format:%d %X",
			       m_textureInfo.width, m_textureInfo.height, (int)m_textureInfo.format,
			       this);
		assert(false);
		return false;
	}

	hr = DX11GraphicBase::m_graphicSession.D3DDevice()->CreateRenderTargetView(
		m_pTexture2D.Get(), nullptr, m_pRenderTargetView.ReleaseAndGetAddressOf());
	if (FAILED(hr)) {
		CHECK_DX_ERROR(hr, "CreateRenderTargetView %X", this);
		assert(false);
		return false;
	}

	if (!InitResourceView())
		return false;

	if (!msaaEnabled) {
		// 启用mass的共享纹理画布 其handle调用 OpenSharedResource 会crash

		ComPtr<IDXGIResource> pDXGIRes = nullptr;
		hr = m_pTexture2D->QueryInterface(__uuidof(IDXGIResource), (LPVOID *)(pDXGIRes.ReleaseAndGetAddressOf()));
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

		ComPtr<IDXGISurface1> sfc;
		hr = DX11GraphicBase::m_graphicSession.D3DDevice()->OpenSharedResource(m_hSharedHandle, __uuidof(IDXGISurface1), (LPVOID *)(sfc.ReleaseAndGetAddressOf()));
		if (SUCCEEDED(hr))
			D2DRenderTarget::BuildD2DFromDXGI(sfc, m_textureInfo.format);
	}
	
	return true;
}

bool DX11Texture2D::InitSharedTexture()
{
	HRESULT hr = DX11GraphicBase::m_graphicSession.D3DDevice()->OpenSharedResource(
		(HANDLE)m_hSharedHandle, __uuidof(ID3D11Texture2D),
		(void **)m_pTexture2D.ReleaseAndGetAddressOf());
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
	auto fileName = winHelper::ExtractFileName(str::w2u(m_strImagePath.c_str()).c_str());

	ComPtr<ID3D11Texture2D> pImage = LoadImageTexture();
	if (!pImage) {
		LOG_WARN("failed to load input image (%s)", fileName.c_str());
		assert(false);
		return false;
	}

	D3D11_TEXTURE2D_DESC srcDesc = {};
	pImage->GetDesc(&srcDesc);

	m_textureInfo.width = srcDesc.Width;
	m_textureInfo.height = srcDesc.Height;
	m_textureInfo.format = srcDesc.Format;

	bool res = InitWriteTexture();
	if (!res) {
		LOG_WARN("failed to create write texture for input image (%s)", fileName.c_str());
		assert(false);
		return false;
	}

	D3D11_TEXTURE2D_DESC destDesc = {};
	m_pTexture2D->GetDesc(&destDesc);

	DX11GraphicBase::m_graphicSession.D3DContext()->CopySubresourceRegion(
		m_pTexture2D.Get(), D3D11CalcSubresource(0, 0, destDesc.MipLevels), 0, 0, 0,
		pImage.Get(), D3D11CalcSubresource(0, 0, srcDesc.MipLevels), nullptr);

	return true;
}

ComPtr<ID3D11Texture2D> DX11Texture2D::LoadImageTexture()
{
	ComPtr<ID3D11ShaderResourceView> pTextureResView = nullptr;
	ComPtr<ID3D11Resource> pResource = nullptr;
	ComPtr<ID3D11Texture2D> pTexture2D = nullptr;

	HRESULT hr = D3DX11CreateShaderResourceViewFromFile(
		DX11GraphicBase::m_graphicSession.D3DDevice().Get(), m_strImagePath.c_str(),
		nullptr, nullptr, pTextureResView.ReleaseAndGetAddressOf(), nullptr);
	if (FAILED(hr)) {
		CHECK_DX_ERROR(hr, "D3DX11CreateShaderResourceViewFromFile %X", this);
		assert(false);
		return nullptr;
	}

	pTextureResView->GetResource(pResource.ReleaseAndGetAddressOf());
	if (!pResource) {
		CHECK_DX_ERROR(hr, "m_pTextureResView->GetResource %X", this);
		assert(false);
		return nullptr;
	}

	hr = pResource->QueryInterface<ID3D11Texture2D>(pTexture2D.ReleaseAndGetAddressOf());
	if (FAILED(hr)) {
		CHECK_DX_ERROR(hr, "QueryInterfaceID3D11Texture2D %X", this);
		assert(false);
		return nullptr;
	}

	return pTexture2D;
}

bool DX11Texture2D::InitResourceView()
{
	D3D11_TEXTURE2D_DESC desc = {};
	m_pTexture2D->GetDesc(&desc);

	DXGI_FORMAT resourceFormat;
	switch (desc.Format) {
	case DXGI_FORMAT_R8G8B8A8_TYPELESS:
		resourceFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		break;
	case DXGI_FORMAT_B8G8R8A8_TYPELESS:
		resourceFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
		break;
	case DXGI_FORMAT_B8G8R8X8_TYPELESS:
		resourceFormat = DXGI_FORMAT_B8G8R8X8_UNORM;
		break;
	default:
		resourceFormat = desc.Format;
		break;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc{};
	viewDesc.Format = resourceFormat;
	viewDesc.Texture2D.MipLevels = 1;

	if (desc.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE) {
		viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	} else {
		if (m_textureInfo.enableMSAA) {
			viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
		} else {
			viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		}
	}

	HRESULT hr = DX11GraphicBase::m_graphicSession.D3DDevice()->CreateShaderResourceView(
		m_pTexture2D.Get(), &viewDesc, m_pTextureResView.ReleaseAndGetAddressOf());
	if (FAILED(hr)) {
		CHECK_DX_ERROR(hr, "CreateShaderResourceView %X", this);
		assert(false);
		return false;
	}

	if (desc.MiscFlags & D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX) {
		hr = m_pTexture2D->QueryInterface(__uuidof(IDXGIKeyedMutex),
						  (void **)m_pKeyedMutex.ReleaseAndGetAddressOf());
		if (FAILED(hr)) {
			CHECK_DX_ERROR(hr, "IDXGIKeyedMutex failed %X", this);
			assert(false);
			return false;
		}
	}

	return true;
}

} // namespace graphic
