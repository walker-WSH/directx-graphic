#pragma once
#include <stack>
#include <mutex>
#include <map>
#include <vector>
#include <assert.h>
#include <Windows.h>
#include <GraphicBase.h>
#include <IGraphicSession.h>
#include "EnumAdapter.h"
#include "GraphicBase.h"
#include "DX11Shader.h"
#include "DX11Texture2D.h"
#include "DX11SwapChain.h"
#include "D2DTextFormat.h"
#include "D2DGeometry.h"
#include "D2DLineStyle.h"

namespace graphic {

// 保证了以下规则：
// DX11GraphicSession 的操作函数（RunTaskXXX） 必须在 EnterContext和LeaveContext 之间执行
// 同一个线程中，不同的DX11GraphicSession实例  必须保证上一个实例leave了 下一个实例才可以enter
// 同一个DX11GraphicSession实例，在不同线程中可以多线程访问（entercontext时有锁）
#define CHECK_GRAPHIC_CONTEXT CheckContext(std::source_location::current())
#define CHECK_GRAPHIC_CONTEXT_EX(x) x.CheckContext(std::source_location::current())

class DX11GraphicSession : public IGraphicSession {
	friend class IAutoGraphicContext;

public:
	DX11GraphicSession();
	virtual ~DX11GraphicSession();

	//----------------------------- IGraphicSession -------------------------
	virtual void RegisterCallback(std::weak_ptr<IGraphicCallback> cb);
	virtual void UnRegisterCallback(IGraphicCallback *cb);

	virtual bool InitializeGraphic(uint32_t adapterIdx = 0);
	virtual void UnInitializeGraphic();

	virtual uint32_t MaxVideoSize();
	virtual bool IsGraphicBuilt();
	virtual bool ReBuildGraphic();

	virtual void DestroyGraphicObject(IGraphicObject *&hdl);
	virtual void DestroyAllGraphicObject();

	// d2d
	virtual font_handle CreateTextFont(const TextFormatDesc &desc);
	virtual geometry_handle CreateGeometry(const std::vector<D2D1_POINT_2F> &points, GEOMETRY_TYPE type,
					       GEOMETRY_FRONT_END_STYLE style);
	virtual D2D1_SIZE_F CalcTextSize(const wchar_t *text, font_handle font);
	virtual IGeometryInterface *OpenGeometryInterface(shader_handle targetTex);
	virtual void CloseGeometryInterface(shader_handle targetTex);

	// display
	virtual display_handle CreateDisplay(HWND hWnd);
	virtual void SetDisplaySize(display_handle hdl, uint32_t width, uint32_t height);
	virtual DisplayInformation GetDisplayInfo(display_handle hdl);

	// shader
	virtual shader_handle CreateShader(const ShaderInformation &info);
	virtual void SetVertexBuffer(shader_handle hdl, const void *buffer, size_t size);
	virtual void SetVSConstBuffer(shader_handle hdl, const void *vsBuffer, size_t vsSize);
	virtual void SetPSConstBuffer(shader_handle hdl, const void *psBuffer, size_t psSize);

	// texture
	virtual texture_handle OpenSharedTexture(HANDLE hSharedHanle);
	virtual texture_handle OpenImageTexture(const WCHAR *fullPath);
	virtual texture_handle CreateTexture(const TextureInformation &info);
	virtual TextureInformation GetTextureInfo(texture_handle tex);
	virtual HANDLE GetSharedHandle(texture_handle tex);
	virtual bool CopyTexture(texture_handle dest, texture_handle src, TextureCopyRegion *region = nullptr);
	virtual bool CopyDisplay(texture_handle dest, display_handle src, TextureCopyRegion *region = nullptr);
	virtual bool MapTexture(texture_handle tex, MAP_TEXTURE_FEATURE type, D3D11_MAPPED_SUBRESOURCE *mapData);
	virtual void UnmapTexture(texture_handle tex);

	// render
	virtual bool BeginRenderCanvas(texture_handle hdl, IGeometryInterface **geometryInterface = nullptr);
	virtual bool BeginRenderWindow(display_handle hdl, IGeometryInterface **geometryInterface = nullptr);
	virtual void SwitchRenderTarget(bool enableSRGB);
	virtual void ClearBackground(const ColorRGBA *bkClr);
	virtual void SetBlendState(VIDEO_BLEND_TYPE type);
	virtual void DrawTopplogy(shader_handle hdl, D3D11_PRIMITIVE_TOPOLOGY type);
	virtual void DrawTexture(shader_handle hdl, VIDEO_FILTER_TYPE flt, const std::vector<texture_handle> &textures);
	virtual void EndRender(IGeometryInterface *geometryInterface = nullptr);

	//---------------------------------------------------------------------------
	void EnterContext(const std::source_location &location);
	void LeaveContext(const std::source_location &location);
	bool CheckContext(const std::source_location &location);

	ComPtr<IDXGIFactory1> D3DFactory();
	ComPtr<ID3D11Device> D3DDevice();
	ComPtr<ID3D11DeviceContext> D3DContext();

	ComPtr<ID2D1Factory1> D2DFactory();
	ComPtr<IDWriteFactory1> D2DWriteFactory();
	ComPtr<ID2D1DeviceContext> D2DDeviceContext();
	ComPtr<ID2D1StrokeStyle> GetLineStyle(LINE_DASH_STYLE style);
	ComPtr<ID2D1Effect> GetD2DEffect(D2D_EFFECT_TYPE effect);

	void PushObject(DX11GraphicBase *obj);
	void RemoveObject(DX11GraphicBase *obj);
	bool IsGraphicObjectAlive(IGraphicObject *obj);
	bool IsDuringRender() { return m_bDuringRendering; }
	void HandleDirectResult(HRESULT hr, std::source_location location = std::source_location::current());

protected:
	bool BuildAllDX();
	void ReleaseAllDX(bool isForRebuild);
	bool InitBlendState();
	bool InitSamplerState();

	void SetRenderContext(ID3DRenderTarget *target, uint32_t width, uint32_t height,
			      ComPtr<IDXGISwapChain> swapChain);
	void UpdateShaderBuffer(ComPtr<ID3D11Buffer> buffer, const void *data, size_t size);
	bool GetResource(const std::vector<texture_handle> &textures,
			 std::vector<ID3D11ShaderResourceView *> &resources);
	void ApplyShader(DX11Shader *shader);

	bool IsTextureInfoSame(const D3D11_TEXTURE2D_DESC *dest, const D3D11_TEXTURE2D_DESC *src,
			       TextureCopyRegion *region, std::string &reason);
	void CopyTextureInner(ComPtr<ID3D11Texture2D> dest, ComPtr<ID3D11Texture2D> src, TextureCopyRegion *region);

	bool BuildD2D();
	void ReleaseD2D();

private:
	CRITICAL_SECTION m_lockOperation;

	std::vector<std::weak_ptr<IGraphicCallback>> m_pGraphicCallbacks;

	uint32_t m_uAdapterIdx = 0;
	bool m_bBuildSuccessed = false;
	std::atomic<uint32_t> m_uMaxTextureSize = D3D10_REQ_TEXTURE2D_U_OR_V_DIMENSION;

	ComPtr<ID2D1Factory1> m_pD2DFactory = nullptr;
	ComPtr<IDWriteFactory1> m_pDWriteFactory = nullptr;
	ComPtr<ID2D1DeviceContext> m_pD2DDeviceContext = nullptr;
	std::map<LINE_DASH_STYLE, D2DLineStyle *> m_mapLineStyle; // lifetime is managed in m_listObject
	std::map<D2D_EFFECT_TYPE, ComPtr<ID2D1Effect>> m_mapD2DEffect;

	ComPtr<IDXGIAdapter1> m_pAdapter = nullptr;
	ComPtr<IDXGIFactory1> m_pDX11Factory = nullptr;
	ComPtr<ID3D11Device> m_pDX11Device = nullptr;
	ComPtr<ID3D11DeviceContext> m_pDeviceContext = nullptr;
	ComPtr<ID3D11BlendState> m_pBlendStateNormal = nullptr;
	ComPtr<ID3D11BlendState> m_pBlendStatePreMultAlpha = nullptr;
	ComPtr<ID3D11SamplerState> m_pSampleStateAnisotropic = nullptr;
	ComPtr<ID3D11SamplerState> m_pSampleStateLinear = nullptr;
	ComPtr<ID3D11SamplerState> m_pSampleStatePoint = nullptr;
	std::map<DX11GraphicBase *, bool> m_listObject;

	ID3DRenderTarget *m_pD3DTarget = nullptr;
	ID3D11RenderTargetView *m_pCurrentRenderTarget = nullptr;
	IDXGISwapChain *m_pCurrentSwapChain = nullptr;
	volatile bool m_bDuringRendering = false;
};

} // namespace graphic
