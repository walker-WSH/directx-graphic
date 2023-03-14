#include "GraphicSessionImp.h"

namespace graphic {

static thread_local std::stack<DX11GraphicSession *> g_stackContexts;

class IAutoGraphicContext::impl {
public:
	impl(IGraphicSession *graphic, const std::source_location &location) : m_Location(location)
	{
		m_pGraphic = dynamic_cast<DX11GraphicSession *>(graphic);
		m_pGraphic->EnterContext(m_Location);
	}

	virtual ~impl() { m_pGraphic->LeaveContext(m_Location); }

private:
	std::source_location m_Location;
	DX11GraphicSession *m_pGraphic = nullptr;
};

IAutoGraphicContext::IAutoGraphicContext(IGraphicSession *graphic, const std::source_location &location)
{
	self = new impl(graphic, location);
}

IAutoGraphicContext::~IAutoGraphicContext()
{
	delete self;
}

//------------------------------------------------------------------------------------
void DX11GraphicSession::EnterContext(const std::source_location &location)
{
	if (!g_stackContexts.empty() && g_stackContexts.top() != this) {
		LOG_WARN("you are in another context!");
		return;
	}

	EnterCriticalSection(&m_lockOperation);
	g_stackContexts.push(this);
}

void DX11GraphicSession::LeaveContext(const std::source_location &location)
{
	if (g_stackContexts.empty()) {
		LOG_WARN("you are not in any context!");
		return;
	} else {
		if (g_stackContexts.top() != this) {
			LOG_WARN("you are in another context!");
			return;
		}
	}

	g_stackContexts.pop();
	LeaveCriticalSection(&m_lockOperation);
}

bool DX11GraphicSession::CheckContext(const std::source_location &location)
{
	bool ret = (!g_stackContexts.empty() && this == g_stackContexts.top());
	if (!ret)
		LOG_WARN("currently in invalid context from %s", location.function_name());

	return ret;
}

} // namespace graphic
