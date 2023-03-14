#include "DX11Shader.h"
#include "GraphicSessionImp.h"
#include <d3dcompiler.h>

namespace graphic {

DX11Shader::DX11Shader(DX11GraphicSession &graphic, const ShaderInformation *info)
	: DX11GraphicBase(graphic, "shader"), m_shaderInfo(*info)
{
	auto vsFile = winHelper::ExtractFileName(str::w2u(info->vsFile.c_str()).c_str());
	auto psFile = winHelper::ExtractFileName(str::w2u(info->psFile.c_str()).c_str());

	LOG_INFO("shader object is created: %X. \n"
		 "\t vs:%s \n"
		 "\t ps:%s \n"
		 "\t perVertexSize:%d vertexCount:%d \n"
		 "\t vsBufferSize:%d psBufferSize:%d \n",
		 this, vsFile.c_str(), psFile.c_str(), info->perVertexSize, info->vertexCount, info->vsBufferSize,
		 info->psBufferSize);

	BuildGraphic();
}

bool DX11Shader::BuildGraphic()
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphic);

	ComPtr<ID3D10Blob> vertexShaderBuffer;
	ComPtr<ID3D10Blob> pixelShaderBuffer;

	HRESULT hr = D3DReadFileToBlob(m_shaderInfo.vsFile.c_str(), vertexShaderBuffer.Assign());
	if (FAILED(hr)) {
		CHECK_DX_ERROR(hr, "D3DReadFileToBlob %X", this);
		assert(false);
		return false;
	}

	hr = D3DReadFileToBlob(m_shaderInfo.psFile.c_str(), pixelShaderBuffer.Assign());
	if (FAILED(hr)) {
		CHECK_DX_ERROR(hr, "D3DReadFileToBlob %X", this);
		assert(false);
		return false;
	}

	hr = m_graphic.D3DDevice()->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(),
						       vertexShaderBuffer->GetBufferSize(), NULL,
						       m_pVertexShader.Assign());
	if (FAILED(hr)) {
		CHECK_DX_ERROR(hr, "CreateVertexShader %X", this);
		assert(false);
		return false;
	}

	hr = m_graphic.D3DDevice()->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(),
						      pixelShaderBuffer->GetBufferSize(), NULL,
						      m_pPixelShader.Assign());
	if (FAILED(hr)) {
		CHECK_DX_ERROR(hr, "CreatePixelShader %X", this);
		assert(false);
		return false;
	}

	std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc = GetInputLayout();
	hr = m_graphic.D3DDevice()->CreateInputLayout(inputLayoutDesc.data(), (uint32_t)inputLayoutDesc.size(),
						      vertexShaderBuffer->GetBufferPointer(),
						      vertexShaderBuffer->GetBufferSize(), m_pInputLayout.Assign());
	if (FAILED(hr)) {
		CHECK_DX_ERROR(hr, "CreateInputLayout %X", this);
		assert(false);
		return false;
	}

	D3D11_BUFFER_DESC vertexBufferDesc = {};
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = m_shaderInfo.vertexCount * m_shaderInfo.perVertexSize;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	hr = m_graphic.D3DDevice()->CreateBuffer(&vertexBufferDesc, NULL, m_pVertexBuffer.Assign());
	if (FAILED(hr)) {
		CHECK_DX_ERROR(hr, "DX-CreateBuffer size:%u %X", vertexBufferDesc.ByteWidth, this);
		assert(false);
		return false;
	}

	if (m_shaderInfo.vsBufferSize > 0) {
		D3D11_BUFFER_DESC CBufferDesc = {};
		CBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		CBufferDesc.ByteWidth = m_shaderInfo.vsBufferSize;
		CBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		hr = m_graphic.D3DDevice()->CreateBuffer(&CBufferDesc, NULL, m_pVSConstBuffer.Assign());
		if (FAILED(hr)) {
			CHECK_DX_ERROR(hr, "DX-CreateBuffer size:%u %X", CBufferDesc.ByteWidth, this);
			assert(false);
			return false;
		}
	}

	if (m_shaderInfo.psBufferSize > 0) {
		D3D11_BUFFER_DESC CBufferDesc = {};
		CBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		CBufferDesc.ByteWidth = m_shaderInfo.psBufferSize;
		CBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		hr = m_graphic.D3DDevice()->CreateBuffer(&CBufferDesc, NULL, m_pPSConstBuffer.Assign());
		if (FAILED(hr)) {
			CHECK_DX_ERROR(hr, "DX-CreateBuffer size:%u %X", CBufferDesc.ByteWidth, this);
			assert(false);
			return false;
		}
	}

	return true;
}

void DX11Shader::ReleaseGraphic()
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphic);

	m_pVertexShader = nullptr;
	m_pVSConstBuffer = nullptr;

	m_pPixelShader = nullptr;
	m_pPSConstBuffer = nullptr;

	m_pInputLayout = nullptr;
	m_pVertexBuffer = nullptr;
}

bool DX11Shader::IsBuilt()
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphic);
	return m_pVertexShader && m_pPixelShader && m_pInputLayout && m_pVertexBuffer;
}

void DX11Shader::GetSemanticName(VERTEX_INPUT_TYPE type, D3D11_INPUT_ELEMENT_DESC &desc)
{
	switch (type) {
	case VERTEX_INPUT_TYPE::NORMAL:
		desc.SemanticName = "NORMAL";
		break;

	case VERTEX_INPUT_TYPE::COLOR:
		desc.SemanticName = "COLOR";
		break;

	case VERTEX_INPUT_TYPE::POSITION:
		desc.SemanticName = "POSITION";
		break;

	case VERTEX_INPUT_TYPE::SV_POSITION:
		desc.SemanticName = "SV_POSITION";
		break;

	case VERTEX_INPUT_TYPE::TEXTURE_COORD:
		desc.SemanticName = "TEXCOORD";
		break;

	default:
		assert(false);
		break;
	}
}

void DX11Shader::GetSemanticFormat(uint32_t size, D3D11_INPUT_ELEMENT_DESC &desc)
{
	switch (size) {
	case 4:
		desc.Format = DXGI_FORMAT_R32_FLOAT;
		break;

	case 8:
		desc.Format = DXGI_FORMAT_R32G32_FLOAT;
		break;

	case 12:
		desc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		break;

	case 16:
		desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		break;

	default:
		assert(false);
		break;
	}
}

std::vector<D3D11_INPUT_ELEMENT_DESC> DX11Shader::GetInputLayout()
{
	std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc;

	uint32_t offset = 0;
	std::map<VERTEX_INPUT_TYPE, uint32_t> indexs;

	for (const auto &item : m_shaderInfo.vertexDesc) {
		D3D11_INPUT_ELEMENT_DESC desc = {};
		desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		desc.AlignedByteOffset = offset;
		desc.SemanticIndex = indexs[item.type];
		GetSemanticName(item.type, desc);
		GetSemanticFormat(item.size, desc);

		inputLayoutDesc.push_back(desc);

		offset += item.size;
		indexs[item.type] = indexs[item.type] + 1;
	}

	assert(!inputLayoutDesc.empty());
	return inputLayoutDesc;
}

} // namespace graphic