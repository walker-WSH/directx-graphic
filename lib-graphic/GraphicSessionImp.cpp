#include "GraphicSessionImp.h"
#include "common/ListHelper.hpp"

namespace graphic {

DX11GraphicSession::DX11GraphicSession()
{
	InitializeCriticalSection(&m_lockOperation);

	AUTO_GRAPHIC_CONTEXT(this);

	std::vector<LINE_DASH_STYLE> temp = {
		LINE_DASH_STYLE::LINE_SOLID,        LINE_DASH_STYLE::LINE_DASH,
		LINE_DASH_STYLE::LINE_DOT,          LINE_DASH_STYLE::LINE_DASH_DOT,
		LINE_DASH_STYLE::LINE_DASH_DOT_DOT,
	};

	for (auto &item : temp) {
		m_mapLineStyle[item] = new D2DLineStyle(*this, item);
	}
}

DX11GraphicSession::~DX11GraphicSession()
{
	if (!m_listObject.empty())
		LOG_WARN("not all objects are released");

	DeleteCriticalSection(&m_lockOperation);
}

bool DX11GraphicSession::InitializeGraphic(uint32_t adapterIdx)
{
	LOG_TRACE();
	CHECK_GRAPHIC_CONTEXT;

	m_uAdapterIdx = adapterIdx;
	LOG_INFO("init graphic module with adapterIdx: %u", adapterIdx);

	HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
	if (FAILED(hr))
		LOG_INFO("CoInitializeEx failed with %X", hr);

	return BuildAllDX();
}

void DX11GraphicSession::UnInitializeGraphic()
{
	LOG_TRACE();
	CHECK_GRAPHIC_CONTEXT;

	ReleaseAllDX(false);

	if (!m_listObject.empty()) {
		LOG_WARN("you should destory all graphic objects");
		DestroyAllGraphicObject();
	}

	CoUninitialize();
}

void DX11GraphicSession::RegisterCallback(std::weak_ptr<IGraphicCallback> cb)
{
	CHECK_GRAPHIC_CONTEXT;
	m_pGraphicCallbacks.push_back(cb);
}

void DX11GraphicSession::UnRegisterCallback(IGraphicCallback *cb)
{
	CHECK_GRAPHIC_CONTEXT;

	auto itr = m_pGraphicCallbacks.begin();
	while (itr != m_pGraphicCallbacks.end()) {
		auto temp = itr->lock();
		if (!temp || temp.get() == cb) {
			itr = m_pGraphicCallbacks.erase(itr);
			continue;
		}

		++itr;
	}
}

bool DX11GraphicSession::IsGraphicBuilt()
{
	CHECK_GRAPHIC_CONTEXT;

	if (!m_bBuildSuccessed.load())
		return false;

	auto hr = m_pDX11Device->GetDeviceRemovedReason();
	if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
		return false;

	return true;
}

uint32_t DX11GraphicSession::MaxVideoSize()
{
	CHECK_GRAPHIC_CONTEXT;
	return m_uMaxTextureSize;
}

bool DX11GraphicSession::ReBuildGraphic()
{
	CHECK_GRAPHIC_CONTEXT;
	ReleaseAllDX(true);
	return BuildAllDX();
}

void DX11GraphicSession::DestroyGraphicObject(IGraphicObject *&hdl)
{
	CHECK_GRAPHIC_CONTEXT;
	CHECK_GRAPHIC_OBJECT_VALID((*this), hdl, DX11GraphicBase, obj, return);

	obj->ReleaseGraphic(false);
	delete hdl;
	hdl = nullptr;
}

void DX11GraphicSession::DestroyAllGraphicObject()
{
	LOG_TRACE();
	CHECK_GRAPHIC_CONTEXT;

	// Here we copy it to "temp" because delete-obj will make it to be changed!
	auto temp = m_listObject;
	for (auto &item : temp) {
		auto obj = item.first;
		LOG_INFO("destroy graphic object: %s, %X", obj->GetObjectName(), (void *)obj);

		obj->ReleaseGraphic(false);
		delete obj;
	}

	assert(m_listObject.empty());
	m_listObject.clear();
}

texture_handle DX11GraphicSession::OpenSharedTexture(HANDLE hSharedHanle)
{
	CHECK_GRAPHIC_CONTEXT;

	DX11Texture2D *tex = new DX11Texture2D(*this, hSharedHanle);
	if (!tex->IsBuilt()) {
		delete tex;
		return nullptr;
	}

	return tex;
}

texture_handle DX11GraphicSession::OpenImageTexture(const WCHAR *fullPath)
{
	CHECK_GRAPHIC_CONTEXT;

	DX11Texture2D *tex = new DX11Texture2D(*this, fullPath);
	if (!tex->IsBuilt()) {
		LOG_WARN("failed to load image texture %X", tex);
		delete tex;
		assert(false);
		return nullptr;
	}

	return tex;
}

texture_handle DX11GraphicSession::CreateTexture(const TextureInformation &info, int flags)
{
	CHECK_GRAPHIC_CONTEXT;

	assert(TEXTURE_USAGE::SHARED_TEXTURE != info.usage);
	assert(TEXTURE_USAGE::STATIC_IMAGE_FILE != info.usage);
	assert(info.width <= m_uMaxTextureSize);
	assert(info.height <= m_uMaxTextureSize);

	DX11Texture2D *tex = new DX11Texture2D(*this, info, flags);
	if (!tex->IsBuilt()) {
		LOG_WARN("failed to create texture %X %dx%d format:%d usage:%s", tex, info.width,
			 info.height, (int)info.format,
			 DX11Texture2D::mapTextureUsage[info.usage].c_str());

		delete tex;
		assert(false);
		return nullptr;
	}

	return tex;
}

TextureInformation DX11GraphicSession::GetTextureInfo(texture_handle hdl)
{
	CHECK_GRAPHIC_CONTEXT;
	CHECK_GRAPHIC_OBJECT_VALID((*this), hdl, DX11Texture2D, obj, return TextureInformation());

	return TextureInformation(obj->m_descTexture.Width, obj->m_descTexture.Height,
				  obj->m_descTexture.Format, obj->m_textureInfo.usage);
}

HANDLE DX11GraphicSession::GetSharedHandle(texture_handle hdl)
{
	CHECK_GRAPHIC_CONTEXT;
	CHECK_GRAPHIC_OBJECT_VALID((*this), hdl, DX11Texture2D, obj, return 0);

	return obj->m_hSharedHandle;
}

bool DX11GraphicSession::CopyTexture(texture_handle dest, texture_handle src,
				     TextureCopyRegion *region)
{
	CHECK_GRAPHIC_CONTEXT;
	CHECK_GRAPHIC_OBJECT_VALID((*this), dest, DX11Texture2D, destTex, return false);
	CHECK_GRAPHIC_OBJECT_VALID((*this), src, DX11Texture2D, srcTex, return false);

	if (!m_bBuildSuccessed.load()) {
		LOG_WARN("DX is not built");
		return false;
	}

	std::string reason;
	if (!IsTextureInfoSame(&destTex->m_descTexture, &srcTex->m_descTexture, region, reason)) {
		LOG_WARN("failed to copy texture because %s", reason.c_str());
		assert(false);
		return false;
	}

	srcTex->LockTexture();
	destTex->LockTexture();

	CopyTextureInner(destTex->m_pTexture2D, srcTex->m_pTexture2D, region);

	srcTex->UnlockTexture();
	destTex->UnlockTexture();

	return true;
}

bool DX11GraphicSession::CopyDisplay(texture_handle dest, display_handle src,
				     TextureCopyRegion *region)
{
	CHECK_GRAPHIC_CONTEXT;
	CHECK_GRAPHIC_OBJECT_VALID((*this), dest, DX11Texture2D, destTex, return false);
	CHECK_GRAPHIC_OBJECT_VALID((*this), src, DX11SwapChain, srcDisplay, return false);

	if (!m_bBuildSuccessed.load()) {
		LOG_WARN("DX is not built");
		return false;
	}

	std::string reason;
	if (!IsTextureInfoSame(&destTex->m_descTexture, &(srcDisplay->m_descTexture), region,
			       reason)) {
		LOG_WARN("failed to copy display because %s", reason.c_str());
		assert(false);
		return false;
	}

	destTex->LockTexture();
	CopyTextureInner(destTex->m_pTexture2D, srcDisplay->m_pSwapBackTexture2D, region);
	destTex->UnlockTexture();

	return true;
}

bool DX11GraphicSession::MapTexture(texture_handle hdl, MAP_TEXTURE_FEATURE type,
				    D3D11_MAPPED_SUBRESOURCE *mapData)
{
	CHECK_GRAPHIC_CONTEXT;
	CHECK_GRAPHIC_OBJECT_VALID((*this), hdl, DX11Texture2D, obj, return false);

	if (!m_bBuildSuccessed.load()) {
		LOG_WARN("DX is not built");
		return false;
	}

	D3D11_MAP method = D3D11_MAP(type);
	HRESULT hr = m_pDeviceContext->Map(obj->m_pTexture2D.Get(), 0, method, 0, mapData);
	if (FAILED(hr)) {
		CHECK_DX_ERROR(hr, "mapTexture %X, for %s", (void *)hdl,
			       (MAP_TEXTURE_FEATURE::FOR_READ_TEXTURE == type) ? "read" : "write");
		assert(false);
		return false;
	}

	EnterContext(std::source_location::current());
	return true;
}

void DX11GraphicSession::UnmapTexture(texture_handle hdl)
{
	CHECK_GRAPHIC_CONTEXT;
	CHECK_GRAPHIC_OBJECT_VALID((*this), hdl, DX11Texture2D, obj, return);

	m_pDeviceContext->Unmap(obj->m_pTexture2D.Get(), 0);
	LeaveContext(std::source_location::current());
}

font_handle DX11GraphicSession::CreateTextFont(const TextFormatDesc &desc)
{
	CHECK_GRAPHIC_CONTEXT;

	assert(IsGraphicBuilt());
	auto ret = new D2DTextFormat(*this, desc);
	if (!ret->IsBuilt()) {
		delete ret;
		return nullptr;
	}

	return ret;
}

geometry_handle DX11GraphicSession::CreateGeometry(const std::vector<D2D1_POINT_2F> &points,
						   GEOMETRY_TYPE type,
						   GEOMETRY_FRONT_END_STYLE style)
{
	CHECK_GRAPHIC_CONTEXT;

	assert(IsGraphicBuilt());
	auto ret = new D2DGeometry(*this, points, type, style);
	if (!ret->IsBuilt()) {
		delete ret;
		return nullptr;
	}

	return ret;
}

D2D1_SIZE_F DX11GraphicSession::CalcTextSize(const wchar_t *text, font_handle font)
{
	CHECK_GRAPHIC_CONTEXT;
	CHECK_GRAPHIC_OBJECT_VALID((*this), font, D2DTextFormat, pTextFormat,
				   return D2D1_SIZE_F(0.f, 0.f));

	if (!m_bBuildSuccessed.load()) {
		LOG_WARN("DX is not built");
		return D2D1_SIZE_F(0.f, 0.f);
	}

	ComPtr<IDWriteTextLayout> pTextLayout;
	auto hr = m_pDWriteFactory->CreateTextLayout(text, (UINT32)wcslen(text),
						     pTextFormat->m_pTextFormat.Get(),
						     (float)m_uMaxTextureSize,
						     (float)m_uMaxTextureSize, &pTextLayout);
	if (FAILED(hr)) {
		assert(false);
		return D2D1_SIZE_F{0.f, 0.f};
	}

	DWRITE_TEXT_METRICS textMetrics;
	pTextLayout->GetMetrics(&textMetrics);

	auto width = ceil(textMetrics.widthIncludingTrailingWhitespace);
	auto height = ceil(textMetrics.height);

	return D2D1::SizeF((float)width, (float)height);
}

ComPtr<ID2D1StrokeStyle> DX11GraphicSession::GetLineStyle(LINE_DASH_STYLE style)
{
	CHECK_GRAPHIC_CONTEXT;

	if (style == LINE_DASH_STYLE::LINE_NO_STYLE)
		return nullptr;

	auto itr = m_mapLineStyle.find(style);
	if (itr != m_mapLineStyle.end())
		return itr->second->m_pStrokeStyle.Get();
	else
		return nullptr;
}

ComPtr<ID2D1Effect> DX11GraphicSession::GetD2DEffect(D2D_EFFECT_TYPE effect)
{
	CHECK_GRAPHIC_CONTEXT;
	assert(FindItemInMap(m_mapD2DEffect, effect));
	return m_mapD2DEffect[effect];
}

IGeometryInterface *DX11GraphicSession::OpenGeometryInterface(shader_handle hdl)
{
	CHECK_GRAPHIC_CONTEXT;
	CHECK_GRAPHIC_OBJECT_VALID((*this), hdl, DX11Texture2D, obj, return nullptr);

	if (TEXTURE_USAGE::CANVAS_TARGET != obj->m_textureInfo.usage) {
		LOG_WARN("request d2d interface from invalid texture %X, usage is not canvas", hdl);
		assert(false && "unsupported interface");
		return nullptr;
	}

	if (!obj->IsInterfaceValid()) {
		assert(false);
		return nullptr;
	}

	obj->BeginDrawD2D();
	return obj;
}

void DX11GraphicSession::CloseGeometryInterface(shader_handle hdl)
{
	CHECK_GRAPHIC_CONTEXT;
	CHECK_GRAPHIC_OBJECT_VALID((*this), hdl, DX11Texture2D, obj, return);

	if (TEXTURE_USAGE::CANVAS_TARGET != obj->m_textureInfo.usage) {
		LOG_WARN("request d2d interface from invalid texture %X, usage is not canvas", hdl);
		assert(false && "unsupported interface");
		return;
	}

	if (!obj->IsInterfaceValid()) {
		assert(false);
		return;
	}

	auto hr = obj->EndDrawD2D();
	HandleDirectResult(hr);
}

display_handle DX11GraphicSession::CreateDisplay(HWND hWnd)
{
	CHECK_GRAPHIC_CONTEXT;

	auto ret = new DX11SwapChain(*this, hWnd);
	if (!ret->IsBuilt()) {
		LOG_WARN("failed to init display handle %X for HWND %X", ret, (void *)hWnd);
		delete ret;
		assert(false);
		return nullptr;
	}

	return ret;
}

void DX11GraphicSession::SetDisplaySize(display_handle hdl, uint32_t width, uint32_t height)
{
	CHECK_GRAPHIC_CONTEXT;
	CHECK_GRAPHIC_OBJECT_VALID((*this), hdl, DX11SwapChain, obj, return);

	obj->SetDisplaySize(width, height);
}

DisplayInformation DX11GraphicSession::GetDisplayInfo(display_handle hdl)
{
	CHECK_GRAPHIC_CONTEXT;
	CHECK_GRAPHIC_OBJECT_VALID((*this), hdl, DX11SwapChain, obj, return DisplayInformation());

	return DisplayInformation(obj->m_hWnd, obj->m_descTexture.Width, obj->m_descTexture.Height,
				  obj->m_descTexture.Format);
}

shader_handle DX11GraphicSession::CreateShader(const ShaderInformation &info)
{
	CHECK_GRAPHIC_CONTEXT;

	assert(!info.vertexDesc.empty());
	auto ret = new DX11Shader(*this, &info);
	if (!ret->IsBuilt()) {
		LOG_WARN("failed to init shader handle %X", ret);
		delete ret;
		assert(false);
		return nullptr;
	}

	return ret;
}

ComPtr<IDXGIFactory1> DX11GraphicSession::D3DFactory()
{
	CHECK_GRAPHIC_CONTEXT;
	return m_pDX11Factory;
}

ComPtr<ID3D11Device> DX11GraphicSession::D3DDevice()
{
	CHECK_GRAPHIC_CONTEXT;
	return m_pDX11Device;
}

ComPtr<ID3D11DeviceContext> DX11GraphicSession::D3DContext()
{
	CHECK_GRAPHIC_CONTEXT;
	return m_pDeviceContext;
}

ComPtr<ID2D1Factory1> DX11GraphicSession::D2DFactory()
{
	CHECK_GRAPHIC_CONTEXT;
	return m_pD2DFactory;
}

ComPtr<IDWriteFactory1> DX11GraphicSession::D2DWriteFactory()
{
	CHECK_GRAPHIC_CONTEXT;
	return m_pDWriteFactory;
}

ComPtr<ID2D1DeviceContext> DX11GraphicSession::D2DDeviceContext()
{
	CHECK_GRAPHIC_CONTEXT;
	return m_pD2DDeviceContext;
}

void DX11GraphicSession::PushObject(DX11GraphicBase *obj)
{
	CHECK_GRAPHIC_CONTEXT;
	m_listObject[obj] = true;
}

void DX11GraphicSession::RemoveObject(DX11GraphicBase *obj)
{
	CHECK_GRAPHIC_CONTEXT;
	RemoveItemInMap(m_listObject, obj);
}

void DX11GraphicSession::ReleaseAllDX(bool isForRebuild)
{
	LOG_TRACE();
	CHECK_GRAPHIC_CONTEXT;

	if (m_bDuringRendering) {
		assert(false && "invalid calling, you must fix your code");
		LOG_WARN(
			"release DX during render loop. CurrentRenderTarget:%X CurrentSwapChain:%X",
			(void *)m_pCurrentRenderTarget, (void *)m_pCurrentSwapChain);

		m_bDuringRendering = false;
		m_pD3DTarget = nullptr;
		m_pCurrentRenderTarget = nullptr;
		m_pCurrentSwapChain = nullptr;
		LeaveContext(std::source_location::current());
	}

	for (auto &item : m_listObject)
		item.first->ReleaseGraphic(isForRebuild);

	m_bBuildSuccessed = false;

	ReleaseD2D();

	m_mapRasterizer.clear();

	m_pAdapter = nullptr;
	m_pDX11Factory = nullptr;

	m_pDX11Device = nullptr;
	m_pDeviceContext = nullptr;

	m_pBlendStateNormal = nullptr;
	m_pBlendStatePreMultAlpha = nullptr;
	m_pSampleStateAnisotropic = m_pSampleStatePoint = m_pSampleStateLinear = m_pSampleStateLinearrRepeat = nullptr;
}

bool DX11GraphicSession::BuildAllDX()
{
	LOG_TRACE();
	CHECK_GRAPHIC_CONTEXT;

	HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&m_pDX11Factory));
	if (FAILED(hr)) {
		LOG_WARN("CreateDXGIFactory1 failed with %x", hr);
		return false;
	}

	hr = m_pDX11Factory->EnumAdapters1(m_uAdapterIdx, &m_pAdapter);
	if (FAILED(hr)) {
		LOG_WARN("EnumAdapters1 failed with %x for adapter: %u", hr, m_uAdapterIdx);
		return false;
	}

	DXGI_ADAPTER_DESC descAdapter;
	m_pAdapter->GetDesc(&descAdapter);

	D3D_FEATURE_LEVEL levelUsed = D3D_FEATURE_LEVEL_10_0;
	hr = D3D11CreateDevice(m_pAdapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, nullptr,
			       D3D11_CREATE_DEVICE_BGRA_SUPPORT, featureLevels.data(),
			       (uint32_t)featureLevels.size(), D3D11_SDK_VERSION,
			       m_pDX11Device.ReleaseAndGetAddressOf(), &levelUsed,
			       m_pDeviceContext.ReleaseAndGetAddressOf());

	if (FAILED(hr)) {
		CHECK_DX_ERROR(hr, "D3D11CreateDevice");
		return false;
	}

	// https://learn.microsoft.com/zh-cn/windows/win32/direct3d11/overviews-direct3d-11-devices-downlevel-intro
	if (levelUsed >= D3D_FEATURE_LEVEL_11_0) {
		m_uMaxTextureSize = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;
	} else {
		m_uMaxTextureSize = D3D10_REQ_TEXTURE2D_U_OR_V_DIMENSION;
	}

	if (!InitBlendState()) {
		LOG_WARN("InitBlendState failed");
		return false;
	}

	if (!InitSamplerState()) {
		LOG_WARN("InitSamplerState failed");
		return false;
	}

	if (!InitRasterizerState()) {
		LOG_WARN("InitRasterizerState failed");
		return false;
	}

	if (!BuildD2D()) {
		LOG_WARN("BuildD2D failed");
		return false;
	}

	m_bBuildSuccessed = true;

	for (auto &item : m_listObject)
		item.first->BuildGraphic();

	for (auto &item : m_pGraphicCallbacks) {
		auto cb = item.lock();
		if (cb)
			cb->OnBuildSuccessed(descAdapter);
	}

	const auto ONE_MB_BYTES = 1024 * 1024;
	LOG_INFO("successed to build DX11. device: %s \n"
		 "\t driver version: %s \n"
		 "\t D3D feature level: 0X%X \n"
		 "\t HighPart: 0X%X \n"
		 "\t LowPart: 0X%X \n"
		 "\t VendorId: 0X%X \n"
		 "\t DeviceId: 0X%X \n"
		 "\t DedicatedVideoMemory: %llu MB \n"
		 "\t DedicatedSystemMemory: %llu MB \n"
		 "\t SharedSystemMemory: %llu MB \n",
		 str::w2u(descAdapter.Description).c_str(),
		 DXGraphic::GetAdapterDriverVersion(m_pAdapter).c_str(), levelUsed,
		 descAdapter.AdapterLuid.HighPart, descAdapter.AdapterLuid.LowPart,
		 descAdapter.VendorId, descAdapter.DeviceId,
		 (DWORD64)descAdapter.DedicatedVideoMemory / ONE_MB_BYTES,
		 (DWORD64)descAdapter.DedicatedSystemMemory / ONE_MB_BYTES,
		 (DWORD64)descAdapter.SharedSystemMemory / ONE_MB_BYTES);

	return true;
}

bool DX11GraphicSession::InitBlendState()
{
	CHECK_GRAPHIC_CONTEXT;

	D3D11_BLEND_DESC blendStateDescription = {};
	blendStateDescription.RenderTarget[0].BlendEnable = TRUE;
	blendStateDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendStateDescription.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendStateDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	blendStateDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	HRESULT hr = m_pDX11Device->CreateBlendState(&blendStateDescription,
						     m_pBlendStateNormal.ReleaseAndGetAddressOf());
	if (FAILED(hr)) {
		CHECK_DX_ERROR(hr, "CreateBlendState failed for normal blend");
		assert(false);
		return false;
	}

	blendStateDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	hr = m_pDX11Device->CreateBlendState(&blendStateDescription,
					     m_pBlendStatePreMultAlpha.ReleaseAndGetAddressOf());
	if (FAILED(hr)) {
		CHECK_DX_ERROR(hr, "CreateBlendState failed for preMultAlpha");
		assert(false);
		return false;
	}

	return true;
}

bool DX11GraphicSession::InitSamplerState()
{
	CHECK_GRAPHIC_CONTEXT;

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MaxLOD = FLT_MAX;

	//------------------------------------------------------------------------------------------
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = 16;
	HRESULT hr = m_pDX11Device->CreateSamplerState(&samplerDesc,
						       m_pSampleStateAnisotropic.ReleaseAndGetAddressOf());
	if (FAILED(hr)) {
		CHECK_DX_ERROR(hr, "CreateSamplerState D3D11_FILTER_ANISOTROPIC");
		assert(false);
		return false;
	}

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.MaxAnisotropy = 0;
	hr = m_pDX11Device->CreateSamplerState(&samplerDesc, m_pSampleStateLinear.ReleaseAndGetAddressOf());
	if (FAILED(hr)) {
		CHECK_DX_ERROR(hr, "CreateSamplerState D3D11_FILTER_MIN_MAG_MIP_LINEAR");
		assert(false);
		return false;
	}

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.MaxAnisotropy = 0;
	hr = m_pDX11Device->CreateSamplerState(&samplerDesc, m_pSampleStatePoint.ReleaseAndGetAddressOf());
	if (FAILED(hr)) {
		CHECK_DX_ERROR(hr, "CreateSamplerState D3D11_FILTER_MIN_MAG_MIP_POINT");
		assert(false);
		return false;
	}

	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.MaxAnisotropy = 0;
	hr = m_pDX11Device->CreateSamplerState(&samplerDesc, m_pSampleStateLinear.ReleaseAndGetAddressOf());
	if (FAILED(hr)) {
		CHECK_DX_ERROR(hr, "CreateSamplerState D3D11_FILTER_MIN_MAG_MIP_LINEAR_REPEAT");
		assert(false);
		return false;
	}

	return true;
}

bool DX11GraphicSession::InitRasterizerState()
{
	CD3D11_RASTERIZER_DESC rd(CD3D11_DEFAULT{});

	rd.FrontCounterClockwise = FALSE; // 顶点顺时针为三角形的正面
	rd.AntialiasedLineEnable = TRUE;
	rd.MultisampleEnable = FALSE;
	rd.FillMode = D3D11_FILL_SOLID;

	std::vector<D3D11_CULL_MODE> stateList = {
		D3D11_CULL_MODE::D3D11_CULL_NONE,
		D3D11_CULL_MODE::D3D11_CULL_FRONT,
		D3D11_CULL_MODE::D3D11_CULL_BACK,
	};
	for (const auto &item : stateList) {
		rd.CullMode = item;

		ComPtr<ID3D11RasterizerState> state;
		auto hr = m_pDX11Device->CreateRasterizerState(&rd, state.ReleaseAndGetAddressOf());
		if (FAILED(hr)) {
			CHECK_DX_ERROR(hr, "CreateRasterizerState");
			assert(false);
			return false;
		}

		m_mapRasterizer[item] = state;
	}

	return true;
}

void DX11GraphicSession::SwitchRenderTarget(bool enableSRGB)
{
	CHECK_GRAPHIC_CONTEXT;

	if (m_pD3DTarget) {
		auto target = m_pD3DTarget->D3DTarget(enableSRGB);
		if (target != m_pCurrentRenderTarget) {
			m_pCurrentRenderTarget = target;
			m_pDeviceContext->OMSetRenderTargets(1, &m_pCurrentRenderTarget, nullptr);
		}
	}
}

void DX11GraphicSession::SetRenderContext(ID3DRenderTarget *target, uint32_t width, uint32_t height,
					  ComPtr<IDXGISwapChain> swapChain)
{
	CHECK_GRAPHIC_CONTEXT;

	m_pD3DTarget = target;

	SwitchRenderTarget(false);
	SetViewports(0.f, 0.f, (float)width, (float)height);

	assert(!m_bDuringRendering);
	m_bDuringRendering = true;
	m_pCurrentSwapChain = swapChain.Get();
	EnterContext(std::source_location::current());
}

void DX11GraphicSession::SetViewports(float left, float top, float width, float height)
{
	CHECK_GRAPHIC_CONTEXT;

	if (!m_pDeviceContext) {
		assert(false);
		return;
	}

	D3D11_VIEWPORT vp;
	memset(&vp, 0, sizeof(vp));

	vp.MinDepth = 0.f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = left;
	vp.TopLeftY = top;
	vp.Width = width;
	vp.Height = height;

	m_pDeviceContext->RSSetViewports(1, &vp);
}

void DX11GraphicSession::UpdateShaderBuffer(ComPtr<ID3D11Buffer> buffer, const void *data,
					    size_t size)
{
	CHECK_GRAPHIC_CONTEXT;

	D3D11_BUFFER_DESC desc;
	buffer->GetDesc(&desc);

	if (desc.ByteWidth == size)
		m_pDeviceContext->UpdateSubresource(buffer.Get(), 0, nullptr, data, 0, 0);
	else {
		assert(false);
		LOG_WARN(
			"failed to update shader buffer because of invalid buffer size. expect:%d real:%d",
			(int)desc.ByteWidth, (int)size);
	}
}

bool DX11GraphicSession::GetResource(const std::vector<texture_handle> &textures,
				     std::vector<ID3D11ShaderResourceView *> &resources)
{
	CHECK_GRAPHIC_CONTEXT;

	resources.clear();

	for (auto &item : textures) {
		CHECK_GRAPHIC_OBJECT_VALID((*this), item, DX11Texture2D, tex, return false);

		if (!tex->IsBuilt() || !tex->m_pTextureResView)
			return false;

		resources.push_back(tex->m_pTextureResView.Get());
	}

	return !resources.empty();
}

bool DX11GraphicSession::ApplyShader(DX11Shader *shader, buffer_handle index)
{
	CHECK_GRAPHIC_CONTEXT;

	ID3D11Buffer *buffer[1];

	m_pDeviceContext->IASetInputLayout(shader->m_pInputLayout.Get());

	m_pDeviceContext->VSSetShader(shader->m_pVertexShader.Get(), nullptr, 0);
	if (shader->m_pVSConstBuffer) {
		buffer[0] = shader->m_pVSConstBuffer.Get();
		m_pDeviceContext->VSSetConstantBuffers(0, 1, buffer);
	}

	m_pDeviceContext->PSSetShader(shader->m_pPixelShader.Get(), nullptr, 0);
	if (shader->m_pPSConstBuffer) {
		buffer[0] = shader->m_pPSConstBuffer.Get();
		m_pDeviceContext->PSSetConstantBuffers(0, 1, buffer);
	}

	if (index) {
		CHECK_GRAPHIC_OBJECT_VALID((*this), index, DX11Buffer, indexBuffer, return false);
		return indexBuffer->ApplyBuffer();
	}

	return true;
}

bool DX11GraphicSession::BeginRenderCanvas(texture_handle hdl,
					   IGeometryInterface **geometryInterface)
{
	if (geometryInterface)
		*geometryInterface = nullptr;

	CHECK_GRAPHIC_CONTEXT;
	CHECK_GRAPHIC_OBJECT_VALID((*this), hdl, DX11Texture2D, obj, return false);

	if (m_bDuringRendering) {
		LOG_WARN(
			"trying to render during another loop. CurrentRenderTarget:%X CurrentSwapChain:%X",
			(void *)m_pCurrentRenderTarget, (void *)m_pCurrentSwapChain);
		assert(false);
		return false;
	}

	if (!m_bBuildSuccessed.load()) {
		LOG_WARN("graphic is not built");
		assert(false);
		return false;
	}

	if (obj->m_textureInfo.usage != TEXTURE_USAGE::CANVAS_TARGET) {
		LOG_WARN("send inalid canvas texture: %X", (void *)hdl);
		assert(false);
		return false;
	}

	if (!obj->IsBuilt())
		return false;

	SetRenderContext(obj, obj->m_descTexture.Width, obj->m_descTexture.Height, nullptr);

	if (geometryInterface && obj->IsInterfaceValid() && obj->BeginDrawD2D())
		*geometryInterface = obj;

	return true;
}

bool DX11GraphicSession::BeginRenderWindow(display_handle hdl,
					   IGeometryInterface **geometryInterface)
{
	if (geometryInterface)
		*geometryInterface = nullptr;

	CHECK_GRAPHIC_CONTEXT;
	CHECK_GRAPHIC_OBJECT_VALID((*this), hdl, DX11SwapChain, obj, return false);

	if (m_bDuringRendering) {
		LOG_WARN(
			"trying to render during another loop. CurrentRenderTarget:%X CurrentSwapChain:%X",
			(void *)m_pCurrentRenderTarget, (void *)m_pCurrentSwapChain);
		assert(false);
		return false;
	}

	if (!m_bBuildSuccessed.load()) {
		LOG_WARN("graphic is not built");
		assert(false);
		return false;
	}

	if (!IsWindow(obj->m_hWnd))
		return false;

	HRESULT hr = obj->TestResizeSwapChain();
	if (FAILED(hr)) {
		CHECK_DX_ERROR(hr, "TestResizeSwapChain");
		HandleDirectResult(hr);
		return false;
	}

	SetRenderContext(obj, obj->m_dwWidth, obj->m_dwHeight, obj->m_pSwapChain);

	if (geometryInterface && obj->IsInterfaceValid() && obj->BeginDrawD2D())
		*geometryInterface = obj;

	return true;
}

void DX11GraphicSession::ClearBackground(const ColorRGBA *bkClr)
{
	CHECK_GRAPHIC_CONTEXT;

	if (!m_pDeviceContext || !m_pCurrentRenderTarget) {
		LOG_WARN("there is no target set");
		assert(false);
		return;
	}

	float color[4] = {bkClr->red, bkClr->green, bkClr->blue, bkClr->alpha};
	m_pDeviceContext->ClearRenderTargetView(m_pCurrentRenderTarget, color);
}

void DX11GraphicSession::SetBlendState(VIDEO_BLEND_TYPE type)
{
	CHECK_GRAPHIC_CONTEXT;

#define SET_BLEND(x)                                                           \
	if (x) {                                                               \
		float blendFactor[4] = {1.0f, 1.0f, 1.0f, 1.0f};               \
		m_pDeviceContext->OMSetBlendState(x, blendFactor, 0xffffffff); \
	}

	if (!m_pDeviceContext || !m_pCurrentRenderTarget) {
		LOG_WARN("there is no target set");
		assert(false);
		return;
	}

	switch (type) {
	case VIDEO_BLEND_TYPE::BLEND_NORMAL:
		SET_BLEND(m_pBlendStateNormal.Get());
		break;

	case VIDEO_BLEND_TYPE::BLEND_PREMULTIPLIED: {
		SET_BLEND(m_pBlendStatePreMultAlpha.Get());
	} break;

	case VIDEO_BLEND_TYPE::BLEND_DISABLED:
	default:
		m_pDeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
		break;
	}
}

void DX11GraphicSession::SetRasterizerState(D3D11_CULL_MODE mode)
{
	CHECK_GRAPHIC_CONTEXT;

	if (!m_pDeviceContext) {
		assert(false);
		return;
	}

	auto itr = m_mapRasterizer.find(mode);
	if (itr == m_mapRasterizer.end()) {
		assert(false);
		return;
	}

	m_pDeviceContext->RSSetState(itr->second.Get());
}

void DX11GraphicSession::SetVSConstBuffer(shader_handle hdl, const void *vsBuffer, size_t vsSize)
{
	CHECK_GRAPHIC_CONTEXT;
	CHECK_GRAPHIC_OBJECT_VALID((*this), hdl, DX11Shader, shader, return);

	auto expectLen = shader->m_shaderInfo.vsBufferSize;
	if (expectLen == vsSize)
		UpdateShaderBuffer(shader->m_pVSConstBuffer, vsBuffer, vsSize);
	else {
		assert(false);
		LOG_WARN("invalid size for vs const buffer. expect:%d, real:%d", (int)expectLen,
			 (int)vsSize);
	}
}

void DX11GraphicSession::SetPSConstBuffer(shader_handle hdl, const void *psBuffer, size_t psSize)
{
	CHECK_GRAPHIC_CONTEXT;
	CHECK_GRAPHIC_OBJECT_VALID((*this), hdl, DX11Shader, shader, return);

	auto expectLen = shader->m_shaderInfo.psBufferSize;
	if (expectLen == psSize)
		UpdateShaderBuffer(shader->m_pPSConstBuffer, psBuffer, psSize);
	else {
		assert(false);
		LOG_WARN("invalid size for ps const buffer. expect:%d, real:%d", (int)expectLen,
			 (int)psSize);
	}
}

buffer_handle DX11GraphicSession::CreateGraphicBuffer(const BufferDesc &desc, const void *data)
{
	CHECK_GRAPHIC_CONTEXT;

	auto buffer = new DX11Buffer(*this, &desc, data);
	if (!buffer->IsBuilt()) {
		assert(false);
		LOG_WARN("failed to create buffer. type:%d count:%d size:%d", (int)desc.bufferType,
			 (int)desc.itemCount, (int)desc.sizePerItem);
		return nullptr;
	}

	return buffer;
}

void DX11GraphicSession::SetGraphicBuffer(buffer_handle hdl, const void *data, size_t size)
{
	CHECK_GRAPHIC_CONTEXT;
	CHECK_GRAPHIC_OBJECT_VALID((*this), hdl, DX11Buffer, buffer, return);

	buffer->SetBufferValue(data, size);
}

void DX11GraphicSession::DrawTopplogy(shader_handle hdl, buffer_handle vertex, buffer_handle index,
				      D3D11_PRIMITIVE_TOPOLOGY type)
{
	CHECK_GRAPHIC_CONTEXT;
	CHECK_GRAPHIC_OBJECT_VALID((*this), hdl, DX11Shader, shader, return);
	CHECK_GRAPHIC_OBJECT_VALID((*this), vertex, DX11Buffer, vertexBuffer, return);

	if (!ApplyShader(shader, index)) {
		assert(false);
		return;
	}

	vertexBuffer->ApplyBuffer();
	m_pDeviceContext->IASetPrimitiveTopology(type);

	if (index) {
		CHECK_GRAPHIC_OBJECT_VALID((*this), index, DX11Buffer, indexBuffer, return);
		m_pDeviceContext->DrawIndexed(indexBuffer->GetBufferDesc().itemCount, 0, 0);

	} else {
		m_pDeviceContext->Draw(vertexBuffer->GetBufferDesc().itemCount, 0);
	}
}

void DX11GraphicSession::DrawTexture(const std::vector<texture_handle> &textures,
				     VIDEO_FILTER_TYPE flt, shader_handle hdl, buffer_handle vertex,
				     buffer_handle index, D3D11_PRIMITIVE_TOPOLOGY type)
{
	CHECK_GRAPHIC_CONTEXT;
	CHECK_GRAPHIC_OBJECT_VALID((*this), hdl, DX11Shader, shader, return);
	CHECK_GRAPHIC_OBJECT_VALID((*this), vertex, DX11Buffer, vertexBuffer, return);

	std::vector<ID3D11ShaderResourceView *> resources;
	DX11Texture2D *resourceTex = NULL;

	if (!textures.empty()) {
		CHECK_GRAPHIC_OBJECT_VALID((*this), textures[0], DX11Texture2D, dest, return);
		resourceTex = dest;

		if (!GetResource(textures, resources)) {
			LOG_WARN("invalid texture for render");
			assert(false);
			return;
		}
	}

	if (!ApplyShader(shader, index)) {
		assert(false);
		return;
	}

	ID3D11SamplerState *sampleState = nullptr;
	switch (flt) {
	case VIDEO_FILTER_TYPE::VIDEO_FILTER_POINT:
		sampleState = m_pSampleStatePoint.Get();
		break;

	case VIDEO_FILTER_TYPE::VIDEO_FILTER_ANISOTROPIC:
		sampleState = m_pSampleStateAnisotropic.Get();
		break;

	case VIDEO_FILTER_TYPE::VIDEO_FILTER_LINEAR_REPEAT:
		sampleState = m_pSampleStateLinearrRepeat.Get();
		break;

	case VIDEO_FILTER_TYPE::VIDEO_FILTER_LINEAR:
	default:
		sampleState = m_pSampleStateLinear.Get();
		break;
	}

	if (sampleState)
		m_pDeviceContext->PSSetSamplers(0, 1, &sampleState);

	if (resourceTex)
		resourceTex->LockTexture();

	vertexBuffer->ApplyBuffer();
	m_pDeviceContext->IASetPrimitiveTopology(type);

	if (!resources.empty())
		m_pDeviceContext->PSSetShaderResources(0, (uint32_t)resources.size(), resources.data());

	if (index) {
		CHECK_GRAPHIC_OBJECT_VALID((*this), index, DX11Buffer, indexBuffer, return);
		m_pDeviceContext->DrawIndexed(indexBuffer->GetBufferDesc().itemCount, 0, 0);

	} else {
		m_pDeviceContext->Draw(vertexBuffer->GetBufferDesc().itemCount, 0);
	}

	if (resourceTex)
		resourceTex->UnlockTexture();
}

void DX11GraphicSession::FlushD3D()
{
	CHECK_GRAPHIC_CONTEXT;

	if (!m_bBuildSuccessed.load()) {
		LOG_WARN("graphic is not built");
		assert(false);
		return;
	}

	D3DContext()->Flush();
}

void DX11GraphicSession::EndRender(IGeometryInterface *geometryInterface)
{
	CHECK_GRAPHIC_CONTEXT;

	if (!m_bDuringRendering) {
		LOG_WARN("trying to end ender while DX is not built");
		assert(false);
		return;
	}

	HRESULT hr = S_OK;

	if (geometryInterface) {
		auto api = dynamic_cast<D2DRenderTarget *>(geometryInterface);
		assert(api);
		if (api)
			hr = api->EndDrawD2D();
	}

	if (m_pCurrentSwapChain) {
		hr = m_pCurrentSwapChain->Present(0, 0);
	}

	m_pD3DTarget = nullptr;
	m_pCurrentRenderTarget = nullptr;
	m_pCurrentSwapChain = nullptr;
	m_bDuringRendering = false;
	LeaveContext(std::source_location::current());

	HandleDirectResult(hr);
}

void DX11GraphicSession::HandleDirectResult(HRESULT hr, std::source_location location)
{
	CHECK_GRAPHIC_CONTEXT;

	if (FAILED(hr)) {
		bool rebuild = (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET ||
				hr == D2DERR_RECREATE_TARGET);

		LOG_WARN("Device Error %X from %s, rebuild: %s", hr, location.function_name(),
			 rebuild ? "yes" : "no");

		for (auto &item : m_pGraphicCallbacks) {
			auto cb = item.lock();
			if (cb)
				cb->OnGraphicError(hr);
		}
	}
}

bool DX11GraphicSession::IsGraphicObjectAlive(IGraphicObject *obj)
{
	CHECK_GRAPHIC_CONTEXT;
	auto temp = dynamic_cast<DX11GraphicBase *>(obj);
	return FindItemInMap(m_listObject, temp);
}

bool DX11GraphicSession::IsTextureInfoSame(const D3D11_TEXTURE2D_DESC *dest,
					   const D3D11_TEXTURE2D_DESC *src,
					   TextureCopyRegion *region, std::string &reason)
{
	if (region) {
		if (region->srcLeft < region->srcRight && region->srcLeft >= 0 &&
		    region->srcRight <= src->Width && region->srcTop < region->srcBottom &&
		    region->srcTop >= 0 && region->srcBottom <= src->Height) {
		} else {
			reason = "invalid src region";
			return false;
		}

		if (region->destLeft >= 0 && region->destLeft < dest->Width &&
		    region->destTop >= 0 && region->destTop < dest->Height) {
		} else {
			reason = "invalid dest region";
			return false;
		}

	} else {
		if (dest->Width != src->Width) {
			reason = "different width";
			return false;
		}

		if (dest->Height != src->Height) {
			reason = "different height";
			return false;
		}
	}

	if (dest->Format != src->Format) {
		reason = "different format";
		return false;
	}

	return true;
}

void DX11GraphicSession::CopyTextureInner(ComPtr<ID3D11Texture2D> dest, ComPtr<ID3D11Texture2D> src,
					  TextureCopyRegion *region)
{
	CHECK_GRAPHIC_CONTEXT;
	assert(dest && src);

	D3D11_TEXTURE2D_DESC destDesc;
	dest->GetDesc(&destDesc);

	D3D11_TEXTURE2D_DESC srcDesc;
	src->GetDesc(&srcDesc);

	if (region) {
		D3D11_BOX sourceRegion;
		sourceRegion.left = region->srcLeft;
		sourceRegion.top = region->srcTop;
		sourceRegion.right = region->srcRight;
		sourceRegion.bottom = region->srcBottom;
		sourceRegion.front = 0;
		sourceRegion.back = 1;

		if (sourceRegion.right <= 0) {
			sourceRegion.right = srcDesc.Width;
		}

		if (sourceRegion.bottom <= 0) {
			sourceRegion.bottom = srcDesc.Height;
		}

		if (region->destArraySliceIndex < 0 ||
		    region->destArraySliceIndex >= destDesc.ArraySize) {
			LOG_WARN("invalid dest array slice index: %d on %d",
				 region->destArraySliceIndex, destDesc.ArraySize);
			assert(false);
		}

		m_pDeviceContext->CopySubresourceRegion(
			dest.Get(),
			D3D11CalcSubresource(0, region->destArraySliceIndex, destDesc.MipLevels),
			region->destLeft, region->destTop, 0, src.Get(),
			D3D11CalcSubresource(0, 0, srcDesc.MipLevels), &sourceRegion);

	} else {
		if (srcDesc.MipLevels == destDesc.MipLevels) {
			m_pDeviceContext->CopyResource(dest.Get(), src.Get());

		} else {
			m_pDeviceContext->CopySubresourceRegion(
				dest.Get(), D3D11CalcSubresource(0, 0, destDesc.MipLevels), 0, 0, 0,
				src.Get(), D3D11CalcSubresource(0, 0, srcDesc.MipLevels), nullptr);
		}
	}
}

bool DX11GraphicSession::BuildD2D()
{
	LOG_TRACE();
	CHECK_GRAPHIC_CONTEXT;

	HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, __uuidof(ID2D1Factory1),
				       reinterpret_cast<void **>(m_pD2DFactory.ReleaseAndGetAddressOf()));
	if (FAILED(hr)) {
		LOG_WARN("D2D1CreateFactory failed 0x%x", hr);
		assert(false);
		return false;
	}

	hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory1),
				 reinterpret_cast<IUnknown **>(m_pDWriteFactory.ReleaseAndGetAddressOf()));
	if (FAILED(hr)) {
		LOG_WARN("DWriteCreateFactory failed 0x%x", hr);
		assert(false);
		return false;
	}

	ComPtr<IDXGIDevice1> pDxgiDevice = nullptr;
	hr = m_pDX11Device->QueryInterface(__uuidof(IDXGIDevice1), (void **)&pDxgiDevice);
	if (FAILED(hr)) {
		LOG_WARN("IDXGIDevice1 failed 0x%x", hr);
		assert(false);
		return false;
	}

	ComPtr<ID2D1Device> pD2DDevice = nullptr;
	hr = m_pD2DFactory->CreateDevice(pDxgiDevice.Get(), &pD2DDevice);
	if (FAILED(hr)) {
		LOG_WARN("ID2D1Device create failed 0x%x", hr);
		assert(false);
		return false;
	}

	hr = pD2DDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
					     &m_pD2DDeviceContext);
	if (FAILED(hr)) {
		LOG_WARN("CreateDeviceContext D2D failed 0x%x", hr);
		assert(false);
		return false;
	}
	m_pD2DDeviceContext->SetUnitMode(D2D1_UNIT_MODE_PIXELS);

	//------------------------------------------ create effects ------------------------------------------------
	std::map<D2D_EFFECT_TYPE, IID> effectList = {
		{D2D_EFFECT_TYPE::D2D_EFFECT_GAUSSIAN_BLUR, D2D_CLSID_D2D1GaussianBlur},
		{D2D_EFFECT_TYPE::D2D_EFFECT_DIRECT_BLUR, D2D_CLSID_D2D1DirectionalBlur},
		{D2D_EFFECT_TYPE::D2D_EFFFECT_HIGHLIGHT, D2D_CLSID_D2D1HighlightsShadows},
		{D2D_EFFECT_TYPE::D2D_EFFFECT_CHROMAKEY, D2D_CLSID_D2D1ChromaKey},
	};

	ComPtr<ID2D1Effect> pEffect = nullptr;
	for (const auto &item : effectList) {
		hr = m_pD2DDeviceContext->CreateEffect(item.second, &pEffect);
		if (FAILED(hr)) {
			LOG_WARN("Create d2d effect failed 0x%x", hr);
			assert(false);
			return false;
		}
		m_mapD2DEffect[item.first] = pEffect;
	}

	return true;
}

void DX11GraphicSession::ReleaseD2D()
{
	LOG_TRACE();
	CHECK_GRAPHIC_CONTEXT;

	m_mapD2DEffect.clear();
	m_pD2DDeviceContext = nullptr;
	m_pDWriteFactory = nullptr;
	m_pD2DFactory = nullptr;
}
} // namespace graphic
