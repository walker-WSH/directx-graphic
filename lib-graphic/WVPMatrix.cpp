#include "IGraphicEngine.h"
#include "IGraphicDefine.h"
#include <assert.h>
#include <dxsdk/include/d3dx10math.h>

// dxsdk
#pragma comment(lib, "D3DX10.lib")

namespace graphic {

float ConvertAngleToRadian(float angle)
{
	auto radian = double(angle) * D3DX_PI / 180.0;
	return (float)radian;
}

D3DXMATRIX GetWorldMatrix(const std::vector<WorldVector> *worldList)
{
	D3DXMATRIX outputWorldMatrix;
	D3DXMatrixIdentity(&outputWorldMatrix);

	if (worldList) {
		for (const auto &item : (*worldList)) {
			switch (item.type) {
			case WORLD_TYPE::VECTOR_MOVE: {
				D3DXMATRIX temp;
				D3DXMatrixIdentity(&temp);
				D3DXMatrixTranslation(&temp, item.x.value_or(0.f), item.y.value_or(0.f),
						      item.z.value_or(0.f));
				outputWorldMatrix *= temp;
			} break;

			case WORLD_TYPE::VECTOR_SCALE: {
				D3DXMATRIX temp;
				D3DXMatrixIdentity(&temp);
				D3DXMatrixScaling(&temp, item.x.value_or(1.f), item.y.value_or(1.f),
						  item.z.value_or(1.f));
				outputWorldMatrix *= temp;
			} break;

			case WORLD_TYPE::VECTOR_ROTATE: {
				D3DXMATRIX temp;
				D3DXMATRIX rotate;
				D3DXMatrixIdentity(&rotate);

				D3DXMatrixRotationX(&temp, ConvertAngleToRadian(item.x.value_or(0.f)));
				rotate *= temp;

				D3DXMatrixRotationY(&temp, ConvertAngleToRadian(item.y.value_or(0.f)));
				rotate *= temp;

				D3DXMatrixRotationZ(&temp, ConvertAngleToRadian(item.z.value_or(0.f)));
				rotate *= temp;

				outputWorldMatrix *= rotate;
			} break;

			default:
				assert(false);
				break;
			}
		}
	}

	outputWorldMatrix.m[0][2] = -outputWorldMatrix.m[0][2];
	outputWorldMatrix.m[1][2] = -outputWorldMatrix.m[1][2];
	outputWorldMatrix.m[2][2] = -outputWorldMatrix.m[2][2];
	outputWorldMatrix.m[3][2] = -outputWorldMatrix.m[3][2];

	return outputWorldMatrix;
}

D3DXMATRIX GetOrthoMatrix(SIZE canvas, bool convertCoord)
{
	FLOAT zn = -100.f;
	FLOAT zf = 100.f;

	D3DXMATRIX orthoMatrix;
	D3DXMatrixOrthoLH(&orthoMatrix, (float)canvas.cx, (float)canvas.cy, zn, zf);

	// convert coord of vertex to windows coord whose left-top is (0, 0)
	if (convertCoord) {
		orthoMatrix.m[1][1] = -orthoMatrix.m[1][1];
		orthoMatrix.m[3][0] = -1.0f;
		orthoMatrix.m[3][1] = 1.0f;
	}

	return orthoMatrix;
}

void TransposedOrthoMatrixWVP(const SIZE &canvas, bool convertCoord, const std::vector<WorldVector> *worldList,
			float outputMatrix[4][4])
{
	D3DXMATRIX worldMatrix = GetWorldMatrix(worldList);
	D3DXMATRIX orthoMatrix = GetOrthoMatrix(canvas, convertCoord);

	D3DXMATRIX wvpMatrix = worldMatrix * orthoMatrix;
	D3DXMatrixTranspose(&wvpMatrix, &wvpMatrix);

	void *src = &(wvpMatrix.m[0][0]);
	memmove(&(outputMatrix[0][0]), src, sizeof(float) * 16);
}

void TransposedPerspectiveMatrixWVP(const SIZE &canvas, const std::vector<WorldVector> *worldList, CameraDesc camera,
				   float outputMatrix[4][4])
{
	D3DXMATRIX worldMatrix = GetWorldMatrix(worldList);

	D3DXVECTOR3 eyePos(camera.eyePos.x, camera.eyePos.y, camera.eyePos.z);
	D3DXVECTOR3 eyeUpDir(camera.eyeUpDir.x, camera.eyeUpDir.y, camera.eyeUpDir.z);
	D3DXVECTOR3 lookAt(camera.lookAt.x, camera.lookAt.y, camera.lookAt.z);
	D3DXMATRIX viewMatrix;
	D3DXMatrixLookAtLH(&viewMatrix, &eyePos, &lookAt, &eyeUpDir);

	D3DXMATRIX projMatrix;
	D3DXMatrixPerspectiveFovLH(&projMatrix,
				   XM_PIDIV2, // 90 - degree
				   (float)canvas.cx / (float)canvas.cy, 0.1f, 2000.0f);

	D3DXMATRIX wvpMatrix;
	wvpMatrix = worldMatrix * viewMatrix;
	wvpMatrix = wvpMatrix * projMatrix;
	D3DXMatrixTranspose(&wvpMatrix, &wvpMatrix);

	void *src = &(wvpMatrix.m[0][0]);
	memmove(&(outputMatrix[0][0]), src, sizeof(float) * 16);
}

} // namespace graphic