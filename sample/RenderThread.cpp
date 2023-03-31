#include "pch.h"
#include "MFCDemoDlg.h"
#include <assert.h>
#include <vector>
#include <memory>
#include <unordered_map>

#include "CustomVideoFrame.h"
#include "RenderHelper.h"
#include "VideoConvertToRGB.h"
#include "VideoConvertToYUV.h"

static const auto DRAW_LINE_DISPLAY_DURATION = 20 * 1000; // ms
static const auto BORDER_THICKNESS = 3;

wchar_t testStr[] = L"WHHH YYY yyyWWWW世界观最好了呵呵但是呢又是为什么 WSH\n"
		    "01234为什么56789\n"
		    "wang hui hello world! How are 大家好 you? are you ok? hhh 结束";

ColorRGBA clrWhite = {1.f, 1.f, 1.f, 1.f};
ColorRGBA clrGrey = {0.3f, 0.3f, 0.3f, 1.f};
ColorRGBA clrBlack = {0, 0, 0, 1.f};
ColorRGBA clrBlackAlpha = {0, 0, 0, 0.7f};
ColorRGBA clrRed = {1.0, 0, 0, 1.f};
ColorRGBA clrGreen = {0, 1.f, 0, 0.8f};
ColorRGBA clrBlue = {0, 0, 1.f, 0.8f};
ColorRGBA clrYellow = {1.f, 1.f, 0.f, 1.f};

display_handle display = nullptr;
texture_handle texCanvas = nullptr;
texture_handle texCanvas2 = nullptr;
texture_handle texGirlRotated = nullptr;
texture_handle texGirlOrigin = nullptr;
texture_handle texGrid = nullptr;
texture_handle texShared = nullptr;
texture_handle texAlpha = nullptr;
texture_handle texImg = nullptr;
texture_handle texImg2 = nullptr;
texture_handle texHightlight = nullptr;
texture_handle texForWrite = nullptr;
font_handle fontFormat = nullptr;
font_handle fontFormat2 = nullptr;
geometry_handle drawingPath = nullptr;
std::vector<geometry_handle> finishedPathList;
bool pathListChanged = false;

struct texRegionInfo {
	texture_handle tex = nullptr;
	RECT region = {0, 0, 0, 0};
	bool selected = false;
	bool fullscreen = false;
};
std::vector<RECT> gridRegion;
std::vector<texRegionInfo> texRegions;

std::shared_ptr<VideoConvertToRGB> pYUVToRGB = nullptr;
AVFrame *preFrame = nullptr;

bool bFullscreenCrop = false;
bool g_clearBk = false;
extern float g_lineStride;
extern float g_blurValue;
extern bool g_checkboxPreMultAlpha;

void InitRenderRect(RECT rc, int numH, int numV);

bool RenderCustomYUV(SIZE canvasSize, RECT rc);

void TestConvertToYUV();
void TestCopyTexture(texture_handle tex);
void TestCopyDisplay(display_handle display);

extern int gridX;
extern int gridY;
unsigned __stdcall CMFCDemoDlg::ThreadFuncNormalRender(void *pParam)
{
	HRESULT hr = CoInitialize(NULL);

	CMFCDemoDlg *self = reinterpret_cast<CMFCDemoDlg *>(pParam);

	auto ret = open_file();
	assert(ret == 0);

	getRotatedTexture(texGirlOrigin, texGirlRotated);

	texRegionInfo info;
	for (size_t i = 0; i < 1; i++) {
		info.tex = texAlpha;
		texRegions.push_back(info);

		info.tex = texGirlRotated;
		texRegions.push_back(info);

		info.tex = texGirlOrigin;
		texRegions.push_back(info);

		info.tex = texImg;
		texRegions.push_back(info);

		info.tex = texCanvas;
		texRegions.push_back(info);

		info.tex = texCanvas2;
		texRegions.push_back(info);

		info.tex = texImg2;
		texRegions.push_back(info);

		info.tex = texGrid;
		texRegions.push_back(info);

		info.tex = texForWrite;
		texRegions.push_back(info);
	}

	int64_t frameInterval = 1000 / 30;
	int64_t startCaptureTime = GetTickCount64();
	int64_t capturedCount = 0;

	while (!self->m_bExit) {
		if (!self->m_asyncTask.RunAllTask()) {
			int64_t nextCaptureTime = startCaptureTime + capturedCount * frameInterval;
			int64_t currentTime = GetTickCount64();
			int64_t sleepTime = 0;
			if (currentTime < nextCaptureTime) {
				sleepTime = nextCaptureTime - currentTime;
				sleepTime = min(sleepTime, int64_t(100));
			}

			Sleep((DWORD)sleepTime);
		}

		if (self->m_nResizeState > 1)
			continue;

		++capturedCount;

		RECT rc;
		::GetClientRect(self->m_hWnd, &rc);

		rc.right = (rc.right / 2) * 2;
		rc.bottom = (rc.bottom / 2) * 2;

		InitRenderRect(rc, gridX, gridY);

		AUTO_GRAPHIC_CONTEXT(pGraphic);

		SIZE canvasSize(rc.right - rc.left, rc.bottom - rc.top);
		pGraphic->SetDisplaySize(display, canvasSize.cx, canvasSize.cy);

		if (!pGraphic->IsGraphicBuilt()) {
			if (!pGraphic->ReBuildGraphic())
				continue;
		}

		getRotatedTexture(texGirlOrigin, texGirlRotated);

		auto canvsInfo = pGraphic->GetTextureInfo(texCanvas);
		if (pGraphic->BeginRenderCanvas(texCanvas)) {
			RenderCustomYUV(SIZE(canvsInfo.width, canvsInfo.height),
					RECT(0, 0, canvsInfo.width, canvsInfo.height));
			pGraphic->EndRender();
		}

		auto api = pGraphic->OpenGeometryInterface(texCanvas2);
		if (api) {
			api->DrawGaussianBlur(texCanvas, g_blurValue);
			pGraphic->CloseGeometryInterface(texCanvas2);
		}

		IGeometryInterface *d2dDisplay = nullptr;
		if (pGraphic->BeginRenderWindow(display, &d2dDisplay)) {
			pGraphic->ClearBackground(&clrGrey);
			pGraphic->SetBlendState(VIDEO_BLEND_TYPE::BLEND_NORMAL);

			if (texShared) {
				RenderTexture(std::vector<texture_handle>{texShared}, canvasSize,
					      RECT(20, 50, rc.right - 20,
						   rc.bottom - 20)); // 渲染共享纹理
			}

			texture_handle fullTex = 0;
			RECT rcSelected = {0, 0, 0, 0};
			for (auto &item : texRegions) {
				if (!item.tex)
					continue;

				if (item.fullscreen)
					fullTex = item.tex;

				if (!item.region.right || !item.region.bottom)
					break; // no region to draw it

				if (item.selected) {
					rcSelected = item.region;

					auto temp = pGraphic->GetTextureInfo(item.tex);

					MosaicParam mosaic;
					mosaic.texWidth = temp.width;
					mosaic.texHeight = temp.height;
					mosaic.mosaicSizeCX = 20;
					mosaic.mosaicSizeCY = 20;

					RenderTexture(std::vector<texture_handle>{item.tex}, canvasSize, item.region,
						      VIDEO_SHADER_TYPE::SHADER_TEXTURE_MOSAIC, &mosaic);
				} else {
					RenderTexture(std::vector<texture_handle>{item.tex}, canvasSize, item.region);
				}
			}

			// draw grid
			if (d2dDisplay) {
				int index = 0;
				for (auto &item : gridRegion) {
					wchar_t text[200];
					swprintf_s(text, L" Region Test %d ", ++index);
					auto textSize = pGraphic->CalcTextSize(text, fontFormat);

					D2D1_RECT_F temp;
					temp.left = (float)item.left;
					temp.top = (float)item.top;
					temp.right = (float)item.right;
					temp.bottom = (float)item.bottom;

					D2D1_RECT_F textRegion;
					textRegion.left = (float)item.left;
					textRegion.bottom = (float)item.bottom;
					textRegion.right = textRegion.left + textSize.width;
					textRegion.top = textRegion.bottom - textSize.height;

					if (textRegion.right > temp.right)
						textRegion.right = temp.right;

					if (textRegion.bottom > temp.bottom)
						textRegion.bottom = temp.bottom;

					d2dDisplay->FillRectangle(textRegion, &clrBlackAlpha);
					d2dDisplay->DrawString(text, 0, fontFormat, &clrWhite, &textRegion);
					d2dDisplay->DrawRectangle(temp, &clrGreen, 1.f, LINE_DASH_STYLE::LINE_SOLID);
				}

				if (rcSelected.right > 0 && rcSelected.bottom > 0) {
					D2D1_RECT_F temp;
					temp.left = (float)rcSelected.left;
					temp.top = (float)rcSelected.top;
					temp.right = (float)rcSelected.right;
					temp.bottom = (float)rcSelected.bottom;
					d2dDisplay->DrawRectangle(temp, &clrRed, BORDER_THICKNESS,
								  LINE_DASH_STYLE::LINE_SOLID);
				}

				d2dDisplay->FlushGeometry();
			}

			if (fullTex) {
				pGraphic->SetBlendState(VIDEO_BLEND_TYPE::BLEND_NORMAL);
				pGraphic->SwitchRenderTarget(g_checkboxPreMultAlpha);
				if (g_checkboxPreMultAlpha) {
					RenderTexture(std::vector<texture_handle>{fullTex}, canvasSize, rc,
						      VIDEO_SHADER_TYPE::SHADER_TEXTURE_SRGB);
				} else {
					RenderTexture(std::vector<texture_handle>{fullTex}, canvasSize, rc);
				}
			}

			if (g_clearBk)
				pGraphic->ClearBackground(&clrWhite);

			if (d2dDisplay) {
				auto crtTime = GetTickCount64();
				ColorRGBA clrLine = {1.f, 0, 0, 1.f};
				for (auto itr = finishedPathList.begin(); itr != finishedPathList.end();) {
					auto item = *itr;
					bool remove = false;

					auto startTime = (DWORD64)item->GetUserData();
					auto percent = double(crtTime - startTime) / double(DRAW_LINE_DISPLAY_DURATION);
					if (percent > 1.0) {
						percent = 1.0;
						remove = true;
					}
					if (percent < 0.0)
						percent = 0.0;

					clrLine.alpha = float(1.0 - percent);
					d2dDisplay->DrawGeometry(item, &clrLine, g_lineStride,
								 LINE_DASH_STYLE::LINE_SOLID);

					if (remove) {
						pGraphic->DestroyGraphicObject(item);
						itr = finishedPathList.erase(itr);
						continue;
					}

					++itr;
				}

				if (drawingPath) {
					d2dDisplay->DrawGeometry(drawingPath, &clrBlack, g_lineStride,
								 LINE_DASH_STYLE::LINE_SOLID);
				}

				d2dDisplay->DrawString(testStr, wcslen(testStr), fontFormat2, &clrGreen);
			}

			pGraphic->EndRender(d2dDisplay);
		}

		if (self->m_bSaveImage) {
			self->m_bSaveImage = false;
			TestConvertToYUV();
			TestCopyTexture(texCanvas);
			TestCopyDisplay(display);
		}
	}

	if (SUCCEEDED(hr))
		CoUninitialize();

	return 0;
}

unsigned __stdcall CMFCDemoDlg::ThreadFuncForSubRegionMosic(void *pParam)
{
	HRESULT hr = CoInitialize(NULL);

	CMFCDemoDlg *self = reinterpret_cast<CMFCDemoDlg *>(pParam);

	TextureInformation canvasInfo;
	canvasInfo.usage = TEXTURE_USAGE::CANVAS_TARGET;
	canvasInfo.format = DXGI_FORMAT_B8G8R8A8_UNORM;

	texture_handle canvasTex = 0;  // 窗口画面 先画到这上面 再present到窗口
	texture_handle canvasPath = 0; // path的mask

	{
		RECT rc;
		::GetClientRect(self->m_hWnd, &rc);

		canvasInfo.width = rc.right;
		canvasInfo.height = rc.bottom;

		AUTO_GRAPHIC_CONTEXT(pGraphic);
		canvasPath = pGraphic->CreateTexture(canvasInfo);
		canvasTex = pGraphic->CreateTexture(canvasInfo);
	}

	int64_t frameInterval = 1000 / 30;
	int64_t startCaptureTime = GetTickCount64();
	int64_t capturedCount = 0;
	while (!self->m_bExit) {
		if (!self->m_asyncTask.RunAllTask()) {
			int64_t nextCaptureTime = startCaptureTime + capturedCount * frameInterval;
			int64_t currentTime = GetTickCount64();
			int64_t sleepTime = 0;
			if (currentTime < nextCaptureTime) {
				sleepTime = nextCaptureTime - currentTime;
				sleepTime = min(sleepTime, int64_t(100));
			}

			Sleep((DWORD)sleepTime);
		}

		if (self->m_nResizeState > 1)
			continue;

		++capturedCount;

		RECT rc;
		::GetClientRect(self->m_hWnd, &rc);

		rc.right = (rc.right / 2) * 2;
		rc.bottom = (rc.bottom / 2) * 2;

		AUTO_GRAPHIC_CONTEXT(pGraphic);

		SIZE wndSize(rc.right - rc.left, rc.bottom - rc.top);
		pGraphic->SetDisplaySize(display, wndSize.cx, wndSize.cy);

		if (!pGraphic->IsGraphicBuilt()) {
			if (!pGraphic->ReBuildGraphic())
				continue;
		}

		bool reinitPathTex = false;
		canvasInfo = pGraphic->GetTextureInfo(canvasTex);
		if (canvasInfo.width != rc.right || canvasInfo.height != rc.bottom) {
			pGraphic->DestroyGraphicObject(canvasPath);
			pGraphic->DestroyGraphicObject(canvasTex);

			canvasInfo.width = rc.right;
			canvasInfo.height = rc.bottom;

			canvasPath = pGraphic->CreateTexture(canvasInfo);
			canvasTex = pGraphic->CreateTexture(canvasInfo);

			reinitPathTex = true;
		}

		if (pathListChanged) {
			reinitPathTex = true;
			pathListChanged = false;
		}

		if (reinitPathTex) {
			IGeometryInterface *d2d = nullptr;
			if (pGraphic->BeginRenderCanvas(canvasPath, &d2d)) {
				ColorRGBA clrBk = {0, 0, 0, 0};
				ColorRGBA clrSubRegion = {1.f, 0, 0, 1.f};

				pGraphic->ClearBackground(&clrBk);
				pGraphic->SetBlendState(VIDEO_BLEND_TYPE::BLEND_NORMAL);

				for (const auto &item : finishedPathList) {
					d2d->DrawGeometry(item, &clrSubRegion, g_lineStride,
							  LINE_DASH_STYLE::LINE_SOLID);
				}

				pGraphic->EndRender(d2d);
			}
		}

		IGeometryInterface *d2d = nullptr;
		if (pGraphic->BeginRenderCanvas(canvasTex, &d2d)) {
			ColorRGBA clr = {0, 0, 0, 1.f};
			pGraphic->ClearBackground(&clr);
			pGraphic->SetBlendState(VIDEO_BLEND_TYPE::BLEND_NORMAL);

			RECT left = rc;
			left.right = rc.right / 2;
			RenderTexture(std::vector<texture_handle>{texGirlOrigin}, wndSize, left);

			RECT right = rc;
			right.left = rc.right / 2;
			RenderTexture(std::vector<texture_handle>{texGrid}, wndSize, right);

			if (drawingPath) {
				d2d->DrawGeometry(drawingPath, &clrRed, g_lineStride, LINE_DASH_STYLE::LINE_SOLID);
			}

			pGraphic->EndRender(d2d);
		}

		if (pGraphic->BeginRenderWindow(display)) {
			pGraphic->ClearBackground(&clrGrey);
			pGraphic->SetBlendState(VIDEO_BLEND_TYPE::BLEND_DISABLED);

			MosaicParam mosaic;
			mosaic.texWidth = rc.right;
			mosaic.texHeight = rc.bottom;
			mosaic.mosaicSizeCX = 20;
			mosaic.mosaicSizeCY = 20;
			RenderTexture(std::vector<texture_handle>{canvasTex, canvasPath}, wndSize, rc,
				      VIDEO_SHADER_TYPE::SHADER_TEXTURE_MOSAIC_SUB, &mosaic);

			pGraphic->EndRender();
		}
	}

	if (SUCCEEDED(hr))
		CoUninitialize();

	return 0;
}

extern int g_nCenterX;
extern int g_nCenterY;
extern int g_nRadius;
extern float g_fWeight;

extern float g_nMoveRadius;
extern float g_nMoveCurve; // default 1

unsigned __stdcall CMFCDemoDlg::ThreadFuncForBulge(void *pParam)
{
	HRESULT hr = CoInitialize(NULL);

	CMFCDemoDlg *self = reinterpret_cast<CMFCDemoDlg *>(pParam);

	texture_handle bulgeTex = 0; // 窗口画面 先画到这上面 再present到窗口

	int64_t frameInterval = 1000 / 30;
	int64_t startCaptureTime = GetTickCount64();
	int64_t capturedCount = 0;
	while (!self->m_bExit) {
		if (!self->m_asyncTask.RunAllTask()) {
			int64_t nextCaptureTime = startCaptureTime + capturedCount * frameInterval;
			int64_t currentTime = GetTickCount64();
			int64_t sleepTime = 0;
			if (currentTime < nextCaptureTime) {
				sleepTime = nextCaptureTime - currentTime;
				sleepTime = min(sleepTime, int64_t(100));
			}

			Sleep((DWORD)sleepTime);
		}

		if (self->m_nResizeState > 1)
			continue;

		++capturedCount;

		RECT rc;
		::GetClientRect(self->m_hWnd, &rc);

		rc.right = (rc.right / 2) * 2;
		rc.bottom = (rc.bottom / 2) * 2;

		AUTO_GRAPHIC_CONTEXT(pGraphic);

		SIZE wndSize(rc.right - rc.left, rc.bottom - rc.top);
		pGraphic->SetDisplaySize(display, wndSize.cx, wndSize.cy);

		if (!pGraphic->IsGraphicBuilt()) {
			if (!pGraphic->ReBuildGraphic())
				continue;
		}

		auto info = pGraphic->GetTextureInfo(texGrid);
		if (!bulgeTex) {
			info.usage = TEXTURE_USAGE::CANVAS_TARGET;
			bulgeTex = pGraphic->CreateTexture(info);
		}

		BulgeParam psParam;
		psParam.texWidth = info.width;
		psParam.texHeight = info.height;
		psParam.centerX = g_nCenterX;
		psParam.centerY = g_nCenterY;
		psParam.radius = (float)g_nRadius;
		psParam.intensity = g_fWeight;
		psParam.intensityDiv100 = g_fWeight / 100.f;

		if (pGraphic->BeginRenderCanvas(bulgeTex)) {
			ColorRGBA clr = {0, 0, 0, 1.f};
			pGraphic->ClearBackground(&clr);
			pGraphic->SetBlendState(VIDEO_BLEND_TYPE::BLEND_NORMAL);

			RenderBulgeTexture(std::vector<texture_handle>{texGrid}, SIZE(info.width, info.height),
					   RECT(0, 0, info.width, info.height), &psParam);

			pGraphic->EndRender();
		}

		if (pGraphic->BeginRenderWindow(display)) {
			pGraphic->ClearBackground(&clrGrey);
			pGraphic->SetBlendState(VIDEO_BLEND_TYPE::BLEND_DISABLED);

			RECT left = rc;
			left.right = rc.right / 2;
			RenderTexture(std::vector<texture_handle>{texGrid}, wndSize, left);

			RECT right = rc;
			right.left = rc.right / 2;
			RenderTexture(std::vector<texture_handle>{bulgeTex}, wndSize, right);

			pGraphic->EndRender();
		}
	}

	if (SUCCEEDED(hr))
		CoUninitialize();

	return 0;
}

unsigned __stdcall CMFCDemoDlg::ThreadFuncRender(void *pParam)
{
	HRESULT hr = CoInitialize(NULL);
	CMFCDemoDlg *self = reinterpret_cast<CMFCDemoDlg *>(pParam);

	int64_t frameInterval = 1000 / 30;
	int64_t startCaptureTime = GetTickCount64();
	int64_t capturedCount = 0;
	while (!self->m_bExit) {
		if (!self->m_asyncTask.RunAllTask()) {
			int64_t nextCaptureTime = startCaptureTime + capturedCount * frameInterval;
			int64_t currentTime = GetTickCount64();
			int64_t sleepTime = 0;
			if (currentTime < nextCaptureTime) {
				sleepTime = nextCaptureTime - currentTime;
				sleepTime = min(sleepTime, int64_t(100));
			}

			Sleep((DWORD)sleepTime);
		}

		if (self->m_nResizeState > 1)
			continue;

		++capturedCount;

		RECT rc;
		::GetClientRect(self->m_hWnd, &rc);

		rc.right = (rc.right / 2) * 2;
		rc.bottom = (rc.bottom / 2) * 2;

		AUTO_GRAPHIC_CONTEXT(pGraphic);

		if (!pGraphic->IsGraphicBuilt()) {
			if (!pGraphic->ReBuildGraphic())
				continue;
		}

		pGraphic->SetDisplaySize(display, rc.right, rc.bottom);

		if (pGraphic->BeginRenderWindow(display)) {
			pGraphic->ClearBackground(&clrWhite);
			pGraphic->SetBlendState(VIDEO_BLEND_TYPE::BLEND_NORMAL);

			VIDEO_SHADER_TYPE shader;
			if (g_checkboxPreMultAlpha) {
				shader = VIDEO_SHADER_TYPE::SHADER_TEXTURE_SRGB;
			} else {
				shader = VIDEO_SHADER_TYPE::SHADER_TEXTURE;
			}

			pGraphic->SwitchRenderTarget(g_checkboxPreMultAlpha);

			RenderTexture(std::vector<texture_handle>{texAlpha}, SIZE(rc.right, rc.bottom),
				      RECT(0, 0, rc.right, rc.bottom), shader);

			RenderTexture(std::vector<texture_handle>{texForWrite}, SIZE(rc.right, rc.bottom),
				      RECT(0, 0, rc.right, rc.bottom), shader);

			pGraphic->EndRender();
		}
	}

	if (SUCCEEDED(hr))
		CoUninitialize();

	return 0;
}

bool InitGraphic(HWND hWnd)
{
	auto listGraphic = graphic::EnumGraphicCard();
	assert(!listGraphic->empty() && "no valid graphic");
	if (listGraphic->empty())
		return false;

	AUTO_GRAPHIC_CONTEXT(pGraphic);

	bool bOK = pGraphic->InitializeGraphic(nullptr);
	assert(bOK);

	//------------------------------------------------------------------
	InitShader();

	//------------------------------------------------------------------
	display = pGraphic->CreateDisplay(hWnd);
	assert(display);
	RECT rc;
	::GetClientRect(hWnd, &rc);
	pGraphic->SetDisplaySize(display, rc.right - rc.left, rc.bottom - rc.top);

	//------------------------------------------------------------------
	texGirlOrigin = pGraphic->OpenImageTexture(L"testGirl.jpg");

	texGrid = pGraphic->OpenImageTexture(L"testWangge.jpg");

	texHightlight = pGraphic->OpenImageTexture(L"testHighlightShadow.jpg");

	texAlpha = pGraphic->OpenImageTexture(L"testAlpha.png");

	texImg = pGraphic->OpenImageTexture(L"test.jpg");

	texImg2 = pGraphic->OpenImageTexture(L"test.png");

	texShared = pGraphic->OpenSharedTexture((HANDLE)0X0000000040003282);

	//------------------------------- test texutres --------------------
	TextureInformation info;
	info.width = 1440;
	info.height = 1080;
	info.format = DXGI_FORMAT_B8G8R8A8_UNORM;

	info.usage = TEXTURE_USAGE::READ_TEXTURE;
	texture_handle tex1 = pGraphic->CreateTexture(info);
	pGraphic->DestroyGraphicObject(tex1);

	info.usage = TEXTURE_USAGE::WRITE_TEXTURE;
	texture_handle tex2 = pGraphic->CreateTexture(info);
	pGraphic->DestroyGraphicObject(tex2);

	// 注意：联系到shader的逻辑，要作为格式转换的画布，那么画布必须是BGRA格式
	info.usage = TEXTURE_USAGE::CANVAS_TARGET;
	texCanvas = pGraphic->CreateTexture(info);
	texCanvas2 = pGraphic->CreateTexture(info);

	info.usage = TEXTURE_USAGE::READ_TEXTURE;
	auto texForRead = pGraphic->CreateTexture(info);
	pGraphic->CopyTexture(texForRead, texCanvas);
	pGraphic->DestroyGraphicObject(texForRead);

	TextFormatDesc desc;
	desc.alignH = TEXT_ALIGNMENT_TYPE::ALIGNMENT_NEAR;
	desc.alignV = TEXT_ALIGNMENT_TYPE::ALIGNMENT_CENTER;
	desc.wordWrap = TEXT_WORD_WRAP::WORD_WRAPPING_NO_WRAP_WITH_ELLIPSIS;
	fontFormat = pGraphic->CreateTextFont(desc);

	desc.fontSize = 30.f;
	desc.bold = true;
	desc.italic = false;
	desc.alignH = TEXT_ALIGNMENT_TYPE::ALIGNMENT_CENTER;
	desc.alignV = TEXT_ALIGNMENT_TYPE::ALIGNMENT_FAR;
	desc.wordWrap = TEXT_WORD_WRAP::WORD_WRAPPING_WHOLE_WORD;
	fontFormat2 = pGraphic->CreateTextFont(desc);

	{ // test build D2D based on kinds of format of canvas
		info.width = 1280;
		info.height = 720;
		info.usage = TEXTURE_USAGE::CANVAS_TARGET;

		info.format = DXGI_FORMAT_B8G8R8A8_UNORM;
		pGraphic->CreateTexture(info);

		info.format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
		pGraphic->CreateTexture(info);

		info.format = DXGI_FORMAT_R8G8B8A8_UNORM;
		pGraphic->CreateTexture(info);

		info.format = DXGI_FORMAT_B8G8R8X8_UNORM;
		pGraphic->CreateTexture(info);
	}

	CreateTextFrame();
	return true;
}

void UnInitGraphic()
{
	AUTO_GRAPHIC_CONTEXT(pGraphic);

	if (pYUVToRGB) {
		pYUVToRGB->UninitConvertion();
		pYUVToRGB.reset();
	}

	close_file();
	if (preFrame)
		av_frame_free(&preFrame);

	pGraphic->DestroyAllGraphicObject();
	pGraphic->UnInitializeGraphic();
}

void InitRenderRect(RECT rc, int numH, int numV)
{
	int width = rc.right - rc.left;
	int height = rc.bottom - rc.top;

	int cx = width / numH;
	int cy = height / numV;

	gridRegion.clear();
	for (int j = 0; j < numV; j++) {
		for (int i = 0; i < numH; i++) {
			RECT temp;
			temp.left = cx * i;
			temp.top = cy * j;
			temp.right = temp.left + cx;
			temp.bottom = temp.top + cy;

			temp.left += BORDER_THICKNESS;
			temp.top += BORDER_THICKNESS;
			temp.right -= BORDER_THICKNESS;
			temp.bottom -= BORDER_THICKNESS;

			gridRegion.push_back(temp);
		}
	}

	// clear firstly
	for (auto &item : texRegions) {
		item.region = RECT();
	}

	auto itr = texRegions.begin();
	for (size_t i = 0; i < texRegions.size() && i < gridRegion.size(); i++) {
		itr->region = gridRegion[i];
		++itr;
	}
}

bool RenderCustomYUV(SIZE canvasSize, RECT rc)
{
	AVFrame *newFrame = decode_frame();
	if (!newFrame)
		return false;

	if (preFrame)
		av_frame_free(&preFrame);

	preFrame = newFrame;
	if (!pYUVToRGB) {
		VideoConvertSettings params;
		params.graphic = pGraphic;
		params.width = preFrame->width;
		params.height = preFrame->height;
		params.format = (AVPixelFormat)preFrame->format;

		assert(params.format == TEST_RENDER_YUV_FORMAT);

		pYUVToRGB = std::make_shared<VideoConvertToRGB>(params);
		pYUVToRGB->InitConvertion();
	}

	pYUVToRGB->ConvertVideo(preFrame, canvasSize, rc);

	if (0) {
		FILE *fp = 0;
		fopen_s(&fp, "yuv", "wb+");
		if (fp) {
			for (auto i = 0; i < preFrame->height; i++) {
				fwrite(preFrame->data[0] + i * preFrame->linesize[0], preFrame->width, 1, fp);
			}

			for (auto i = 0; i < preFrame->height / 2; i++) {
				fwrite(preFrame->data[1] + i * preFrame->linesize[1], preFrame->width / 2, 1, fp);
			}

			for (auto i = 0; i < preFrame->height / 2; i++) {
				fwrite(preFrame->data[2] + i * preFrame->linesize[2], preFrame->width / 2, 1, fp);
			}

			fclose(fp);
		}
	}

	return true;
}

void TestConvertToYUV()
{
	auto info = pGraphic->GetDisplayInfo(display);

	TextureInformation destinfo;
	destinfo.width = info.width;
	destinfo.height = info.height;
	destinfo.format = info.format;
	destinfo.usage = TEXTURE_USAGE::CANVAS_TARGET;

	texture_handle dest = pGraphic->CreateTexture(destinfo);
	assert(dest);
	pGraphic->CopyDisplay(dest, display);

	VideoConvertSettings params;
	params.graphic = pGraphic;
	params.width = info.width;
	params.height = info.height;
	params.format = TEST_TO_YUV_FORMAT;

	std::shared_ptr<VideoConvertToYUV> toYUV = std::make_shared<VideoConvertToYUV>(params);
	toYUV->InitConvertion();

	if (toYUV->ConvertVideo(dest)) {
		void *output_data[AV_NUM_DATA_POINTERS] = {0};
		uint32_t output_linesize[AV_NUM_DATA_POINTERS] = {0};

		if (toYUV->MapConvertedYUV(output_data, output_linesize)) {
			if (TEST_TO_YUV_FORMAT == AVPixelFormat::AV_PIX_FMT_YUV420P) {
				SaveI420(info.width, info.height, output_data, output_linesize);

			} else if (TEST_TO_YUV_FORMAT == AVPixelFormat::AV_PIX_FMT_NV12) {
				SaveNV12(info.width, info.height, output_data, output_linesize);
			}

			toYUV->UnmapConvertedYUV();
		}
	}

	toYUV->UninitConvertion();

	pGraphic->DestroyGraphicObject(dest);
}

void TestCopyTexture(texture_handle tex)
{
	auto srcInfo = pGraphic->GetTextureInfo(tex);

	TextureInformation destinfo;
	destinfo.width = srcInfo.width;
	destinfo.height = srcInfo.height;
	destinfo.format = srcInfo.format;
	destinfo.usage = TEXTURE_USAGE::READ_TEXTURE;

	texture_handle dest = pGraphic->CreateTexture(destinfo);
	assert(dest);
	pGraphic->CopyTexture(dest, tex);

	D3D11_MAPPED_SUBRESOURCE data;
	if (pGraphic->MapTexture(dest, MAP_TEXTURE_FEATURE::FOR_READ_TEXTURE, &data)) {
		SaveBitmapFile(L"d:/copyTexture.bmp", (const uint8_t *)data.pData, data.RowPitch, destinfo.width,
			       destinfo.height, 4, true);
		pGraphic->UnmapTexture(dest);
	}

	pGraphic->DestroyGraphicObject(dest);
}

void TestCopyDisplay(display_handle display)
{
	auto srcInfo = pGraphic->GetDisplayInfo(display);

	TextureCopyRegion region;
	region.srcLeft = 0;
	region.srcTop = 0;
	region.srcRight = srcInfo.width / 2;
	region.srcBottom = srcInfo.height / 2;
	region.destLeft = 50;
	region.destTop = 100;

	TextureInformation destinfo;
	destinfo.width = srcInfo.width;
	destinfo.height = srcInfo.height;
	destinfo.format = srcInfo.format;
	destinfo.usage = TEXTURE_USAGE::READ_TEXTURE;

	texture_handle dest = pGraphic->CreateTexture(destinfo);
	assert(dest);
	pGraphic->CopyDisplay(dest, display, &region);

	D3D11_MAPPED_SUBRESOURCE data;
	if (pGraphic->MapTexture(dest, MAP_TEXTURE_FEATURE::FOR_READ_TEXTURE, &data)) {
		SaveBitmapFile(L"d:/copyDisplay.bmp", (const uint8_t *)data.pData, data.RowPitch, destinfo.width,
			       destinfo.height, 4, true);
		pGraphic->UnmapTexture(dest);
	}

	pGraphic->DestroyGraphicObject(dest);
}

bool g_bSmooth = false;
bool g_bEndClosed = false;
void UpdatePath(std::vector<CPoint> pt, bool finished)
{
	AUTO_GRAPHIC_CONTEXT(pGraphic);

	if (drawingPath)
		pGraphic->DestroyGraphicObject(drawingPath);

	if (pt.size() >= 2) {
		std::vector<D2D1_POINT_2F> points;
		for (auto &item : pt)
			points.push_back(D2D1_POINT_2F(float(item.x), float(item.y)));

		GEOMETRY_TYPE type = g_bSmooth ? GEOMETRY_TYPE::BEZIER_CURVE : GEOMETRY_TYPE::MULP_POINTS;
		GEOMETRY_FRONT_END_STYLE style = g_bEndClosed ? GEOMETRY_FRONT_END_STYLE::FRONT_END_CLOSE
							      : GEOMETRY_FRONT_END_STYLE::FRONT_END_OPEN;
		auto temp = pGraphic->CreateGeometry(points, type, style);

		if (finished) {
			temp->SetUserData((void *)GetTickCount64());
			finishedPathList.push_back(temp);
			pathListChanged = true;
		} else {
			drawingPath = temp;
		}
	}
}

void CMFCDemoDlg::OnBnClickedClearAllDraw()
{
	m_asyncTask.PushVideoTask([]() {
		AUTO_GRAPHIC_CONTEXT(pGraphic);

		if (drawingPath)
			pGraphic->DestroyGraphicObject(drawingPath);

		for (auto &item : finishedPathList) {
			pGraphic->DestroyGraphicObject(item);
		}

		finishedPathList.clear();
		drawingPath = nullptr;
		pathListChanged = true;
	});
}

void CMFCDemoDlg::OnBnClickedClearFullBk()
{
	g_clearBk = !g_clearBk;
}

void CMFCDemoDlg::OnBnClickedDrawCurve()
{
	if (!m_bToDrawCurve) {
		g_clearBk = true;
	} else {
		g_clearBk = false;
	}

	m_bToDrawCurve = !m_bToDrawCurve;
	m_vCurrentPoints.clear();

	g_bSmooth = m_checkboxSmooth.GetCheck();
	auto pts = m_vCurrentPoints;
	m_asyncTask.PushVideoTask([pts]() { UpdatePath(pts, false); });
}

void CMFCDemoDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (m_bToDrawCurve) {
		m_bIsDrawing = true;
		SetCapture();

		m_vCurrentPoints.clear();
		m_vCurrentPoints.push_back(point);

		g_bSmooth = m_checkboxSmooth.GetCheck();
		auto pts = m_vCurrentPoints;
		m_asyncTask.PushVideoTask([pts]() { UpdatePath(pts, false); });

		return;
	}

	for (auto &item : texRegions) {
		if (item.fullscreen) {
			item.fullscreen = false;
			break;
		}
	}

	for (auto &item : texRegions) {
		auto rc = item.region;
		if (point.x > rc.left && point.x < rc.right && point.y > rc.top && point.y < rc.bottom) {
			item.selected = true;
		} else {
			item.selected = false;
		}
	}

	CDialogEx::OnLButtonDown(nFlags, point);
}

void CMFCDemoDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_bIsDrawing) {
		m_vCurrentPoints.push_back(point);

		g_bSmooth = m_checkboxSmooth.GetCheck();
		auto pts = m_vCurrentPoints;
		m_asyncTask.PushVideoTask([pts]() { UpdatePath(pts, false); });
	}

	CDialogEx::OnMouseMove(nFlags, point);
}

void CMFCDemoDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_bIsDrawing) {
		m_bIsDrawing = false;
		ReleaseCapture();

		m_vCurrentPoints.push_back(point);

		g_bSmooth = m_checkboxSmooth.GetCheck();
		auto pts = m_vCurrentPoints;
		m_asyncTask.PushVideoTask([pts]() { UpdatePath(pts, true); });

		return;
	}

	CDialogEx::OnLButtonUp(nFlags, point);
}

void CMFCDemoDlg::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	for (auto &item : texRegions) {
		if (item.selected) {
			item.fullscreen = !item.fullscreen;
			break;
		}
	}

	CDialogEx::OnLButtonDblClk(nFlags, point);
}

void CMFCDemoDlg::OnRButtonDown(UINT nFlags, CPoint point)
{
	DWORD styles = (DWORD)GetWindowLongPtr(m_hWnd, GWL_STYLE);
	if (styles & WS_MAXIMIZE)
		ShowWindow(SW_RESTORE);
	else
		ShowWindow(SW_MAXIMIZE);

	CDialogEx::OnRButtonDown(nFlags, point);
}

void CMFCDemoDlg::OnMButtonDown(UINT nFlags, CPoint point)
{
	bFullscreenCrop = !bFullscreenCrop;
	CDialogEx::OnMButtonDown(nFlags, point);
}
