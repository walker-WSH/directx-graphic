#pragma once
#include <GraphicBase.h>
#include <windows.h>
#include <memory>
#include <map>

namespace graphic {

class DX11GraphicSession;

class DX11Shader : public DX11GraphicBase {
	friend class DX11GraphicSession;

public:
	DX11Shader(DX11GraphicSession &graphic, const ShaderInformation *info);

	virtual bool BuildGraphic();
	virtual void ReleaseGraphic(bool isForRebuild);
	virtual bool IsBuilt();

protected:
	std::vector<D3D11_INPUT_ELEMENT_DESC> GetInputLayout();
	void GetSemanticName(VERTEX_INPUT_TYPE type, D3D11_INPUT_ELEMENT_DESC &desc);
	void GetSemanticFormat(uint32_t size, D3D11_INPUT_ELEMENT_DESC &desc);

private:
	const ShaderInformation m_shaderInfo;

	// vertex info
	ComPtr<ID3D11InputLayout> m_pInputLayout = nullptr;

	// vertex shader
	ComPtr<ID3D11VertexShader> m_pVertexShader = nullptr;
	ComPtr<ID3D11Buffer> m_pVSConstBuffer = nullptr;

	// pixel shader
	ComPtr<ID3D11PixelShader> m_pPixelShader = nullptr;
	ComPtr<ID3D11Buffer> m_pPSConstBuffer = nullptr;
};

} // namespace graphic