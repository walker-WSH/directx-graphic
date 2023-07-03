#include "IGraphicEngine.h"
#include "IGraphicDefine.h"
#include <assert.h>

namespace graphic {

XMMATRIX GetWorldMatrix(const std::vector<WorldVector> *worldList);
XMMATRIX GetOrthoMatrix(SIZE canvas, bool convertCoord);

GRAPHIC_API void TransposedOrthoMatrixWVP(const SIZE &canvas, bool convertCoord,
					  const std::vector<WorldVector> *worldList, XMMATRIX &outputMatrix)
{
	XMMATRIX worldMatrix = GetWorldMatrix(worldList);
	XMMATRIX orthoMatrix = GetOrthoMatrix(canvas, convertCoord);

	XMMATRIX temp = worldMatrix * orthoMatrix;
	outputMatrix = XMMatrixTranspose(temp);
}

GRAPHIC_API void TransposedPerspectiveMatrixWVP(const SIZE &canvas, const std::vector<WorldVector> *worldList,
						CameraDesc camera, XMMATRIX &outputMatrix)
{
	XMMATRIX worldMatrix = GetWorldMatrix(worldList);
	XMMATRIX viewMatrix = XMMatrixLookAtLH(camera.eyePos, camera.lookAt, camera.eyeUpDir);
	XMMATRIX projMatrix = XMMatrixPerspectiveFovLH(XM_PIDIV2, // 90 - degree
						       (float)canvas.cx / (float)canvas.cy, 0.1f, 2000.0f);

	XMMATRIX temp;
	temp = worldMatrix * viewMatrix;
	temp = temp * projMatrix;

	outputMatrix = XMMatrixTranspose(temp);
}

//-----------------------------------------------------------------------------------------------------------
XMMATRIX GetOrthoMatrix(SIZE canvas, bool convertCoord)
{
	const FLOAT zn = -100.f;
	const FLOAT zf = 100.f;

	XMMATRIX ret = XMMatrixOrthographicLH((float)canvas.cx, (float)canvas.cy, zn, zf);

	// convert coord of vertex to windows coord whose left-top is (0, 0)
	if (convertCoord) {
		ret.r[1].m128_f32[1] = -ret.r[1].m128_f32[1];
		ret.r[3].m128_f32[0] = -1.0f;
		ret.r[3].m128_f32[1] = 1.0f;
	}

	return ret;
}

float ConvertAngleToRadian(float angle)
{
	auto radian = double(angle) * XM_PI / 180.0;
	return (float)radian;
}

XMMATRIX GetRotateMatrix(const WorldVector &param)
{
	XMMATRIX rotate = XMMatrixIdentity();

	if (param.x.has_value()) {
		XMMATRIX temp = XMMatrixRotationX(ConvertAngleToRadian(param.x.value_or(0.f)));
		rotate *= temp;
	}

	if (param.y.has_value()) {
		XMMATRIX temp = XMMatrixRotationY(ConvertAngleToRadian(param.y.value_or(0.f)));
		rotate *= temp;
	}

	if (param.z.has_value()) {
		XMMATRIX temp = XMMatrixRotationZ(ConvertAngleToRadian(param.z.value_or(0.f)));
		rotate *= temp;
	}

	return rotate;
}

XMMATRIX GetWorldMatrix(const std::vector<WorldVector> *worldList)
{
	XMMATRIX outputWorldMatrix = XMMatrixIdentity();
	if (!worldList)
		return outputWorldMatrix;

	for (const auto &item : (*worldList)) {
		switch (item.type) {
		case WORLD_TYPE::VECTOR_MOVE: {
			XMMATRIX temp =
				XMMatrixTranslation(item.x.value_or(0.f), item.y.value_or(0.f), item.z.value_or(0.f));
			outputWorldMatrix *= temp;
		} break;

		case WORLD_TYPE::VECTOR_SCALE: {
			XMMATRIX temp =
				XMMatrixScaling(item.x.value_or(1.f), item.y.value_or(1.f), item.z.value_or(1.f));
			outputWorldMatrix *= temp;
		} break;

		case WORLD_TYPE::VECTOR_ROTATE: {
			XMMATRIX temp = GetRotateMatrix(item);
			outputWorldMatrix *= temp;
		} break;

		default:
			assert(false);
			break;
		}
	}

	return outputWorldMatrix;
}

} // namespace graphic