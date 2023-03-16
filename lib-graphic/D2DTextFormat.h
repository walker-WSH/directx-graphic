#pragma once
#include <GraphicBase.h>

namespace graphic {

class DX11GraphicSession;
class D2DRenderTarget;

struct D2DTextFormat : public DX11GraphicBase {
	friend class DX11GraphicSession;
	friend class D2DRenderTarget;

public:
	D2DTextFormat(DX11GraphicSession &graphic, const TextFormatDesc &desc);

	virtual bool BuildGraphic();
	virtual void ReleaseGraphic(bool isForRebuild);
	virtual bool IsBuilt();

protected:
	DWRITE_TEXT_ALIGNMENT SwitchAlignH(TEXT_ALIGNMENT_TYPE alg);
	DWRITE_PARAGRAPH_ALIGNMENT SwitchAlignV(TEXT_ALIGNMENT_TYPE alg);

private:
	TextFormatDesc m_textFormat;
	ComPtr<IDWriteTextFormat> m_pTextFormat = nullptr;
	ComPtr<IDWriteInlineObject> m_pTextCutShow = nullptr;
};

} // namespace graphic
