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

void DX11GraphicBase::SetUserData(void *data)
{
	m_userData = data;
}

void *DX11GraphicBase::GetUserData()
{
	return m_userData;
}

} // namespace graphic
