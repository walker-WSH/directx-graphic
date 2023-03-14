#pragma once
#include <Windows.h>
#include <string>
#include <memory>
#include <vector>
#include <IGraphicSession.h>

namespace graphic {

// this returned list includes "microsfot basic"
GRAPHIC_API std::shared_ptr<std::vector<GraphicCardDesc>> EnumGraphicCard();

GRAPHIC_API IGraphicSession *CreateGraphicSession();

// before calling it, you should destroy all object created by IGraphicSession
GRAPHIC_API void DestroyGraphicSession(IGraphicSession *&graphic);

} // namespace graphic
