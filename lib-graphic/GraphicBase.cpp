#include "GraphicBase.h"
#include <GraphicSessionImp.h>

// system
#pragma comment(lib, "DXGI.lib")
#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

// dxsdk
#pragma comment(lib, "D3DX11.lib")

// d2d
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "windowscodecs.lib")

namespace graphic {

DX11GraphicBase::DX11GraphicBase(DX11GraphicSession &graphic, const char *name)
	: m_graphicSession(graphic), m_strName(name ? name : "EmptyName")
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphicSession);
	LOG_INFO("\"%s\" graphic object is created: %X", m_strName.c_str(), (void *)this);
	m_graphicSession.PushObject(this);
}

DX11GraphicBase::~DX11GraphicBase()
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphicSession);
	LOG_INFO("\"%s\" graphic object is deleted %X", m_strName.c_str(), (void *)this);
	m_graphicSession.RemoveObject(this);
}

graphic_cb DX11GraphicBase::RegisterCallback(std::function<void(IGraphicObject *obj)> cbRebuilt,
					     std::function<void(IGraphicObject *obj)> cbReleased)
{
	static long graphicCllbackIndex = 1;

	GraphicCallback info;
	info.cbRebuilt = cbRebuilt;
	info.cbReleased = cbReleased;
	info.id = InterlockedIncrement(&graphicCllbackIndex);

	CHECK_GRAPHIC_CONTEXT_EX(m_graphicSession);
	m_callbacks.push_back(info);
	return info.id;
}

void DX11GraphicBase::UnregisterCallback(graphic_cb hdl)
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphicSession);
	for (auto itr = m_callbacks.begin(); itr != m_callbacks.end(); ++itr) {
		if (itr->id = hdl) {
			m_callbacks.erase(itr);
			break;
		}
	}
}

void DX11GraphicBase::ClearCallback()
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphicSession);
	m_callbacks.clear();
}

void DX11GraphicBase::SetUserData(void *data)
{
	// Here we do not need CHECK_GRAPHIC_CONTEXT_EX
	m_userData = data;
}

void *DX11GraphicBase::GetUserData()
{
	// Here we do not need CHECK_GRAPHIC_CONTEXT_EX
	return m_userData;
}

void DX11GraphicBase::NotifyReleaseEvent(bool isForRebuild)
{
	if (!isForRebuild)
		return;

	CHECK_GRAPHIC_CONTEXT_EX(m_graphicSession);
	for (auto &item : m_callbacks) {
		if (item.cbReleased)
			item.cbReleased(this);
	}
}

void DX11GraphicBase::NotifyRebuildEvent()
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphicSession);
	for (auto &item : m_callbacks) {
		if (item.cbRebuilt)
			item.cbRebuilt(this);
	}
}

} // namespace graphic
