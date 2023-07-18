#pragma once
#include <GraphicBase.h>
#include <windows.h>
#include <memory>
#include <map>

namespace graphic {

class DX11GraphicSession;

class DX11Buffer : public DX11GraphicBase {
	friend class DX11GraphicSession;

public:
	DX11Buffer(DX11GraphicSession &graphic, const BufferDesc *info, const void *data);

	const BufferDesc &GetBufferDesc() { return m_bufferInfo; }
	bool SetBufferValue(const void *data, size_t size);
	bool ApplyBuffer();

	virtual bool BuildGraphic();
	virtual void ReleaseGraphic(bool isForRebuild);
	virtual bool IsBuilt();

private:
	const BufferDesc m_bufferInfo;
	std::shared_ptr<char> m_pBufferData = nullptr;
	ComPtr<ID3D11Buffer> m_pBuffer = nullptr;
};

} // namespace graphic