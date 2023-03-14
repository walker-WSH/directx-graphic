#include "pch.h"
#include "RenderHelper.h"
#include <dxsdk/include/d3dx10math.h>

// dxsdk
#pragma comment(lib, "D3DX10.lib")

float ConvertAngleToRadian(float angle)
{
	auto radian = double(angle) * D3DX_PI / 180.0;
	return (float)radian;
}

D3DXMATRIX GetWorldMatrix(const WorldDesc &wd)
{
	D3DXMATRIX outputWorldMatrix;
	D3DXMatrixIdentity(&outputWorldMatrix);

	if (wd.rotate.has_value()) {
		D3DXMATRIX rotate;
		D3DXMatrixIdentity(&rotate);

		const auto &value = wd.rotate.value();
		if (value.x > 0) {
			D3DXMATRIX temp;
			D3DXMatrixRotationX(&temp, ConvertAngleToRadian(value.x));
			rotate *= temp;
		}

		if (value.y > 0) {
			D3DXMATRIX temp;
			D3DXMatrixRotationY(&temp, ConvertAngleToRadian(value.y));
			rotate *= temp;
		}

		if (value.z > 0) {
			D3DXMATRIX temp;
			D3DXMatrixRotationZ(&temp, ConvertAngleToRadian(value.z));
			rotate *= temp;
		}

		outputWorldMatrix *= rotate;
	}

	if (wd.move.has_value()) {
		D3DXMATRIX temp;
		D3DXMatrixIdentity(&temp);
		D3DXMatrixTranslation(&temp, wd.move.value().x, wd.move.value().y, wd.move.value().z);
		outputWorldMatrix *= temp;
	}

	if (wd.scale.has_value()) {
		D3DXMATRIX temp;
		D3DXMatrixIdentity(&temp);
		D3DXMatrixScaling(&temp, wd.scale.value().x, wd.scale.value().y, wd.scale.value().z);
		outputWorldMatrix *= temp;
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

void TransposeMatrixWVP(const SIZE &canvas, bool convertCoord, WorldDesc wd, float outputMatrix[4][4])
{
	D3DXMATRIX worldMatrix = GetWorldMatrix(wd);
	D3DXMATRIX orthoMatrix = GetOrthoMatrix(canvas, convertCoord);

	D3DXMATRIX wvpMatrix = worldMatrix * orthoMatrix;
	D3DXMatrixTranspose(&wvpMatrix, &wvpMatrix);

	void *src = &(wvpMatrix.m[0][0]);
	memmove(&(outputMatrix[0][0]), src, sizeof(float) * 16);
}
