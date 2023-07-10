#pragma once
#include <GraphicBase.h>
#include <D2DRenderTarget.h>
#include <map>

namespace graphic {

class DX11GraphicSession;

class DX11Texture2D : public DX11GraphicBase, public D2DRenderTarget, public ID3DRenderTarget {
	friend class DX11GraphicSession;

public:
	static std::map<TEXTURE_USAGE, std::string> mapTextureUsage;

	DX11Texture2D(DX11GraphicSession &graphic, const TextureInformation &info, int flags);
	DX11Texture2D(DX11GraphicSession &graphic, HANDLE handle);
	DX11Texture2D(DX11GraphicSession &graphic, const WCHAR *fullPath);

	virtual ID3D11RenderTargetView *D3DTarget(bool srgb);

	virtual bool BuildGraphic();
	virtual void ReleaseGraphic(bool isForRebuild);
	virtual bool IsBuilt() { return m_pTexture2D; }

	void LockTexture();
	void UnlockTexture();

protected:
	bool InitWriteTexture();
	bool InitReadTexture();
	bool InitTargetTexture(bool cube);
	bool InitSharedTexture();
	bool InitImageTexture();
	ComPtr<ID3D11Texture2D> LoadImageTexture();
	bool InitResourceView();

protected:
	D3D11_TEXTURE2D_DESC m_descTexture = {};
	ComPtr<ID3D11Texture2D> m_pTexture2D = nullptr;
	ComPtr<ID3D11RenderTargetView> m_pRenderTargetView = nullptr;
	ComPtr<ID3D11ShaderResourceView> m_pTextureResView = nullptr;
	ComPtr<IDXGIKeyedMutex> m_pKeyedMutex = nullptr;

	HANDLE m_hSharedHandle = 0;
	std::wstring m_strImagePath = L"";
	TextureInformation m_textureInfo = {};
	const int m_nCreateFlags = 0;
};

} // namespace graphic
