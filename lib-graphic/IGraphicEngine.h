#pragma once
#include <Windows.h>
#include <string>
#include <memory>
#include <vector>
#include <lib-graphic/IGraphicSession.h>

namespace graphic {

// this returned list includes "microsfot basic"
GRAPHIC_API std::shared_ptr<std::vector<GraphicCardDesc>> EnumGraphicCard();

GRAPHIC_API IGraphicSession *CreateGraphicSession();

// before calling it, you should destroy all object created by IGraphicSession
GRAPHIC_API void DestroyGraphicSession(IGraphicSession *&graphic);

//------------------------------------------------------------------------------------------------------
GRAPHIC_API XMMATRIX MatrixTranspose(const XMMATRIX &temp);

GRAPHIC_API XMMATRIX GetWorldMatrix(const std::vector<WorldVector> *worldList);

// orthogonal matrix
GRAPHIC_API void TransposedOrthoMatrixWVP(const SIZE &canvas, bool convertCoord,
					  const std::vector<WorldVector> *worldList,
					  XMMATRIX &outputMatrix);

// perspective matrix
GRAPHIC_API void TransposedPerspectiveMatrixWVP(const SIZE &canvas,
						const std::vector<WorldVector> *worldList,
						CameraDesc camera, XMMATRIX &outputMatrix);

// debug function (for BGRA or BGRX)
GRAPHIC_API bool SaveDebugBitmap(const wchar_t *path, const uint8_t *data, int linesize, int width,
				 int height, int pixelSize, bool flip);

} // namespace graphic
