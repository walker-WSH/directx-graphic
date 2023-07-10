#pragma once
#include <lib-graphic/IGraphicDefine.h>
#include <lib-graphic/IGeometryInterface.h>

namespace graphic {

class IGraphicSession {
public:
	virtual ~IGraphicSession() = default;

	virtual void RegisterCallback(std::weak_ptr<IGraphicCallback> cb) = 0;
	virtual void UnRegisterCallback(IGraphicCallback *cb) = 0;

	// graphic : Select graphic automatically if it's null. NVIDIA > AMD > INTEL > BAISC > ANY
	virtual bool InitializeGraphic(uint32_t adapterIdx = 0) = 0;
	virtual void UnInitializeGraphic() = 0;

	virtual uint32_t MaxVideoSize() = 0;
	virtual bool IsGraphicBuilt() = 0;
	virtual bool ReBuildGraphic() = 0;

	virtual void DestroyGraphicObject(IGraphicObject *&hdl) = 0;
	virtual void DestroyAllGraphicObject() = 0;

	// d2d
	virtual font_handle CreateTextFont(const TextFormatDesc &desc) = 0;
	virtual geometry_handle CreateGeometry(const std::vector<D2D1_POINT_2F> &points, GEOMETRY_TYPE type,
					       GEOMETRY_FRONT_END_STYLE style) = 0;
	virtual D2D1_SIZE_F CalcTextSize(const wchar_t *text, font_handle font) = 0;
	virtual IGeometryInterface *OpenGeometryInterface(shader_handle targetTex) = 0;
	virtual void CloseGeometryInterface(shader_handle targetTex) = 0;

	// display
	virtual display_handle CreateDisplay(HWND hWnd) = 0;
	virtual void SetDisplaySize(display_handle hdl, uint32_t width, uint32_t height) = 0;
	virtual DisplayInformation GetDisplayInfo(display_handle hdl) = 0;

	// shader
	virtual shader_handle CreateShader(const ShaderInformation &info) = 0;
	virtual long CreateIndexBuffer(shader_handle hdl, const IndexItemDesc &desc) = 0;
	virtual void SetVertexBuffer(shader_handle hdl, const void *buffer, size_t size) = 0;
	virtual void SetVSConstBuffer(shader_handle hdl, const void *vsBuffer, size_t vsSize) = 0;
	virtual void SetPSConstBuffer(shader_handle hdl, const void *psBuffer, size_t psSize) = 0;
	virtual void SetIndexBuffer(shader_handle hdl, long index_id, const void *data, size_t size) = 0;

	// texture
	virtual texture_handle OpenSharedTexture(HANDLE hSharedHanle) = 0;
	virtual texture_handle OpenImageTexture(const WCHAR *fullPath) = 0;
	virtual texture_handle CreateTexture(const TextureInformation &info, int textureCreateFlags = 0) = 0;
	virtual TextureInformation GetTextureInfo(texture_handle tex) = 0;
	virtual HANDLE GetSharedHandle(texture_handle tex) = 0;
	virtual bool CopyTexture(texture_handle dest, texture_handle src, TextureCopyRegion *region = nullptr) = 0;
	virtual bool CopyDisplay(texture_handle dest, display_handle src, TextureCopyRegion *region = nullptr) = 0;
	virtual bool MapTexture(texture_handle tex, MAP_TEXTURE_FEATURE type, D3D11_MAPPED_SUBRESOURCE *) = 0;
	virtual void UnmapTexture(texture_handle tex) = 0;

	// render
	virtual bool BeginRenderCanvas(texture_handle hdl, IGeometryInterface **geometryInterface = nullptr) = 0;
	virtual bool BeginRenderWindow(display_handle hdl, IGeometryInterface **geometryInterface = nullptr) = 0;

	virtual void SwitchRenderTarget(bool enableSRGB) = 0;
	virtual void ClearBackground(const ColorRGBA *bkClr) = 0;
	virtual void SetBlendState(VIDEO_BLEND_TYPE type) = 0;
	virtual void SetRasterizerState(D3D11_CULL_MODE mode) = 0;
	virtual void DrawTopplogy(shader_handle hdl, D3D11_PRIMITIVE_TOPOLOGY type, long indexId = INVALID_INDEX_ID) = 0;
	virtual void DrawTexture(shader_handle hdl, VIDEO_FILTER_TYPE flt, const std::vector<texture_handle> &,
				 D3D11_PRIMITIVE_TOPOLOGY type = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
				 long indexId = INVALID_INDEX_ID) = 0;

	virtual void EndRender(IGeometryInterface *geometryInterface = nullptr) = 0;
};

} // namespace graphic
