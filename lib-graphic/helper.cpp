#include "IGraphicEngine.h"
#include "IGraphicDefine.h"
#include <assert.h>

namespace graphic {

bool SaveDebugBitmap(const wchar_t *path, const uint8_t *data, int linesize, int width, int height, int pixelSize,
		     bool flip)
{
	if (!path || !data)
		return false;

	HANDLE hWrite = CreateFile(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (!hWrite || hWrite == INVALID_HANDLE_VALUE)
		return false;

	DWORD dwStride = width * pixelSize;
	DWORD dwNumOfWrite = 0;

	BITMAPFILEHEADER fileHead;
	memset(&fileHead, 0, sizeof(fileHead));
	fileHead.bfType = 'MB';
	fileHead.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	fileHead.bfSize = fileHead.bfOffBits + height * dwStride;

	BITMAPINFOHEADER infoHead;
	memset(&infoHead, 0, sizeof(infoHead));
	infoHead.biSize = sizeof(BITMAPINFOHEADER);
	infoHead.biWidth = width;
	infoHead.biHeight = height;
	infoHead.biPlanes = 1;
	infoHead.biBitCount = pixelSize * 8;
	infoHead.biCompression = 0;
	infoHead.biSizeImage = height * dwStride;

	WriteFile(hWrite, &fileHead, sizeof(BITMAPFILEHEADER), &dwNumOfWrite, NULL);
	WriteFile(hWrite, &infoHead, sizeof(BITMAPINFOHEADER), &dwNumOfWrite, NULL);

	for (int i = 0; i < height; ++i) {
		if (flip)
			WriteFile(hWrite, data + (height - 1 - i) * linesize, dwStride, &dwNumOfWrite, NULL);
		else
			WriteFile(hWrite, data + i * linesize, dwStride, &dwNumOfWrite, NULL);
	}

	CloseHandle(hWrite);
	return true;
}

} // namespace graphic