#include "IGraphicDefine.h"
#include "GraphicSessionImp.h"
#include "EnumAdapter.h"
#include <stack>
#include <mutex>
#include <assert.h>

namespace graphic {

GRAPHIC_API std::shared_ptr<std::vector<GraphicCardDesc>> EnumGraphicCard()
{
	std::shared_ptr<std::vector<GraphicCardDesc>> pReturnList(new std::vector<GraphicCardDesc>);

	DXGraphic::EnumD3DAdapters(nullptr,
				   [pReturnList](void *userdata, ComPtr<IDXGIFactory1>, ComPtr<IDXGIAdapter1> adapter,
						 const DXGI_ADAPTER_DESC &desc, const char *version) {
					   GraphicCardDesc info;
					   info.graphicName = desc.Description;
					   info.driverVersion = version;

					   info.adapterLuid = desc.AdapterLuid;
					   info.vendorId = desc.VendorId;
					   info.deviceId = desc.DeviceId;

					   info.dedicatedVideoMemory = desc.DedicatedVideoMemory;
					   info.dedicatedSystemMemory = desc.DedicatedSystemMemory;
					   info.sharedSystemMemory = desc.SharedSystemMemory;

					   pReturnList->push_back(info);
					   return true;
				   });

	return pReturnList;
}

GRAPHIC_API IGraphicSession *CreateGraphicSession()
{
	LOG_TRACE();
	return new DX11GraphicSession();
}

GRAPHIC_API void DestroyGraphicSession(IGraphicSession *&graphic)
{
	if (graphic) {
		LOG_TRACE();
		delete graphic;
		graphic = nullptr;
	}
}

} // namespace graphic
