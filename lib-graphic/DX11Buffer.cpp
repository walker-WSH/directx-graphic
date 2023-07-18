#include "DX11Buffer.h"
#include "GraphicSessionImp.h"
#include <d3dcompiler.h>

namespace graphic {

DX11Buffer::DX11Buffer(DX11GraphicSession &graphic, const BufferDesc *info, const void *data)
	: DX11GraphicBase(graphic, "buffer"), m_bufferInfo(*info)
{
	auto length = info->sizePerItem * info->itemCount;
	m_pBufferData = std::shared_ptr<char>(new char[length]);

	if (data) {
		memcpy(m_pBufferData.get(), data, length);
	}

	BuildGraphic();
}

bool DX11Buffer::SetBufferValue(const void *data, size_t size)
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphicSession);

	auto expected = m_bufferInfo.itemCount * m_bufferInfo.sizePerItem;
	if (expected == size) {
		memcpy(m_pBufferData.get(), data, size);
		m_graphicSession.UpdateShaderBuffer(m_pBuffer, data, size);

	} else {
		assert(false);
		LOG_WARN("invalid size for SetBufferValue. expect:%d, real:%d", (int)expected,
			 (int)size);
	}

	return true;
}

bool DX11Buffer::ApplyBuffer()
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphicSession);

	if (!m_pBuffer) {
		assert(false);
		return false;
	}

	if (m_bufferInfo.bufferType == D3D11_BIND_VERTEX_BUFFER) {
		uint32_t stride = m_bufferInfo.sizePerItem;
		uint32_t offset = 0;
		ID3D11Buffer *buffer[1] = {m_pBuffer};

		m_graphicSession.D3DContext()->IASetVertexBuffers(0, 1, buffer, &stride, &offset);

	} else if (m_bufferInfo.bufferType == D3D11_BIND_INDEX_BUFFER) {
		if (m_bufferInfo.sizePerItem == sizeof(WORD)) {
			m_graphicSession.D3DContext()->IASetIndexBuffer(m_pBuffer,
									DXGI_FORMAT_R16_UINT, 0);

		} else if (m_bufferInfo.sizePerItem == sizeof(DWORD)) {
			m_graphicSession.D3DContext()->IASetIndexBuffer(m_pBuffer,
									DXGI_FORMAT_R32_UINT, 0);
		} else {
			LOG_WARN("invalid index size:%d", (int)m_bufferInfo.sizePerItem);
			assert(false && "invalid index size");
			return false;
		}

	} else {
		LOG_WARN("unknown buffer type:%d", (int)m_bufferInfo.bufferType);
		assert(false && "unknown buffer type");
		return false;
	}

	return true;
}

bool DX11Buffer::BuildGraphic()
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphicSession);

	auto expected = m_bufferInfo.itemCount * m_bufferInfo.sizePerItem;
	if (expected <= 0) {
		assert(false);
		return false;
	}

	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = expected;
	bd.BindFlags = m_bufferInfo.bufferType;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = m_pBufferData.get();

	auto hr = m_graphicSession.D3DDevice()->CreateBuffer(&bd, &initData, &m_pBuffer);
	if (FAILED(hr)) {
		CHECK_DX_ERROR(hr, "DX-CreateBuffer for %d, size:%u type:%d %X",
			       m_bufferInfo.bufferType, expected, (int)m_bufferInfo.bufferType,
			       this);
		assert(false);
		return false;
	}

	NotifyRebuildEvent();
	return true;
}

void DX11Buffer::ReleaseGraphic(bool isForRebuild)
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphicSession);
	m_pBuffer = nullptr;
	NotifyReleaseEvent(isForRebuild);
}

bool DX11Buffer::IsBuilt()
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphicSession);
	return m_pBuffer;
}

} // namespace graphic