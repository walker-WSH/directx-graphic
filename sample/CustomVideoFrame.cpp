#include "pch.h"
#include <assert.h>
#include <memory>
#include "CustomVideoFrame.h"

#if 0
int width = 1920;
int height = 1080;

int lenY = width * height;
int lenU = (width / 2) * (height / 2);
int lenV = (width / 2) * (height / 2);

std::shared_ptr<uint8_t> i420Y(new uint8_t[lenY]);
std::shared_ptr<uint8_t> i420U(new uint8_t[lenU]);
std::shared_ptr<uint8_t> i420V(new uint8_t[lenV]);

bool readVideo()
{
	FILE *fp = nullptr;
	fopen_s(&fp, 1080p.i420, "rb+");
	if (!fp) {
		assert(false);
		return false;
	}

	fread(i420Y.get(), lenY, 1, fp);
	fread(i420U.get(), lenU, 1, fp);
	fread(i420V.get(), lenV, 1, fp);

	fclose(fp);
	return true;
}

#endif

//------------------------------------------------------------------------------------
AVFormatContext *input_ctx = nullptr;
AVCodecContext *decoder_ctx = nullptr;
AVStream *video = nullptr;
int video_stream = 0;
struct SwsContext *sws_ctx = nullptr;

int open_file()
{
	int ret;

	/* open the input file */
	if (avformat_open_input(&input_ctx, "test.wmv", nullptr, nullptr) != 0) {
		return -1;
	}

	if (avformat_find_stream_info(input_ctx, nullptr) < 0) {
		return -1;
	}

	/* find the video stream information */
	AVCodec *test = nullptr;
	ret = av_find_best_stream(input_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, &test, 0);
	if (ret < 0) {
		return -1;
	}

	video_stream = ret;
	video = input_ctx->streams[video_stream];

	AVCodecParameters *const codecpar = video->codecpar;
	const AVCodec *const decoder =
		avcodec_find_decoder(codecpar->codec_id); // fix discarded-qualifiers

	decoder_ctx = avcodec_alloc_context3(decoder);
	avcodec_parameters_to_context(decoder_ctx, codecpar);

	if (avcodec_parameters_to_context(decoder_ctx, video->codecpar) < 0)
		return -1;

	if ((ret = avcodec_open2(decoder_ctx, decoder, nullptr)) < 0) {
		return -1;
	}

	return 0;
}

void close_file()
{
	if (sws_ctx) {
		sws_freeContext(sws_ctx);
		sws_ctx = 0;
	}

	if (decoder_ctx) {
		avcodec_free_context(&decoder_ctx);
		decoder_ctx = 0;
	}

	if (input_ctx) {
		avformat_close_input(&input_ctx);
		input_ctx = 0;
	}
}

AVFrame *decode_frame()
{
	AVPacket packet = {0};
	int ret;

	while (true) {
		ret = av_read_frame(input_ctx, &packet);
		if (ret < 0) {
			return nullptr;
		}

		if (video_stream = packet.stream_index)
			break;

		av_packet_unref(&packet);
	}

	AVFrame *frame = av_frame_alloc();
	int got_frame = 0;

	while (!got_frame) {
		ret = avcodec_send_packet(decoder_ctx, &packet);
		if (ret == 0)
			ret = avcodec_receive_frame(decoder_ctx, frame);

		got_frame = (ret == 0);

		if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
			ret = 0;

		if (ret < 0) {
			break;
		}
	}

	av_packet_unref(&packet);

	if (got_frame) {
		if (!sws_ctx) {
			sws_ctx = sws_getContext(frame->width, frame->height,
						 (enum AVPixelFormat)frame->format, frame->width,
						 frame->height, TEST_RENDER_YUV_FORMAT, SWS_BICUBIC,
						 nullptr, nullptr, nullptr);
			assert(sws_ctx);
		}

		AVFrame *output = av_frame_alloc();
		output->format = TEST_RENDER_YUV_FORMAT;
		output->width = frame->width;
		output->height = frame->height;

		int ret = av_frame_get_buffer(output, 1);
		if (ret < 0) {
			assert(false);
			return nullptr;
		}

		sws_scale(sws_ctx, (const uint8_t *const *)frame->data, frame->linesize, 0,
			  frame->height, output->data, output->linesize);

		av_frame_free(&frame);
		return output;

	} else {
		av_frame_free(&frame);
		return nullptr;
	}
}

bool SaveBitmapFile(const wchar_t *path, const uint8_t *data, int linesize, int width, int height,
		    int pixelSize, bool flip)
{
	if (!path || !data)
		return false;

	HANDLE hWrite = CreateFile(path, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
				   FILE_ATTRIBUTE_NORMAL, nullptr);
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

	WriteFile(hWrite, &fileHead, sizeof(BITMAPFILEHEADER), &dwNumOfWrite, nullptr);
	WriteFile(hWrite, &infoHead, sizeof(BITMAPINFOHEADER), &dwNumOfWrite, nullptr);

	for (size_t i = 0; i < (size_t)height; ++i) {
		if (flip)
			WriteFile(hWrite, data + ((size_t)height - 1 - i) * linesize, dwStride,
				  &dwNumOfWrite, nullptr);
		else
			WriteFile(hWrite, data + i * (size_t)linesize, dwStride, &dwNumOfWrite,
				  nullptr);
	}

	CloseHandle(hWrite);
	return true;
}

void WritePlaneData(HANDLE hWrite, const void *data, int32_t srcLinesize, int32_t dstLinesize,
		    int32_t counts)
{
	DWORD dwCount = 0;
	if (srcLinesize == dstLinesize) {
		WriteFile(hWrite, data, dstLinesize * counts, &dwCount, nullptr);
	} else {
		assert(srcLinesize >= dstLinesize);
		for (size_t i = 0; i < (size_t)counts; i++) {
			WriteFile(hWrite, (uint8_t *)data + i * srcLinesize, dstLinesize, &dwCount,
				  nullptr);
		}
	}
}

void SaveI420(uint32_t width, uint32_t height, void *data[AV_NUM_DATA_POINTERS],
	      uint32_t linesize[AV_NUM_DATA_POINTERS])
{
	wchar_t path[MAX_PATH];
	swprintf_s(path, L"d:/%dx%d.yuv420p", width, height);

	HANDLE hWrite = CreateFile(path, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
				   FILE_ATTRIBUTE_NORMAL, nullptr);
	if (!hWrite || hWrite == INVALID_HANDLE_VALUE) {
		assert(false);
		return;
	}

	WritePlaneData(hWrite, data[0], linesize[0], width, height);
	WritePlaneData(hWrite, data[1], linesize[1], width / 2, height / 2);
	WritePlaneData(hWrite, data[2], linesize[2], width / 2, height / 2);

	CloseHandle(hWrite);
}

void SaveNV12(uint32_t width, uint32_t height, void *data[AV_NUM_DATA_POINTERS],
	      uint32_t linesize[AV_NUM_DATA_POINTERS])
{
	wchar_t path[MAX_PATH];
	swprintf_s(path, L"d:/%dx%d.nv12", width, height);

	HANDLE hWrite = CreateFile(path, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
				   FILE_ATTRIBUTE_NORMAL, nullptr);
	if (!hWrite || hWrite == INVALID_HANDLE_VALUE) {
		assert(false);
		return;
	}

	WritePlaneData(hWrite, data[0], linesize[0], width, height);
	WritePlaneData(hWrite, data[1], linesize[1], width, height / 2);

	CloseHandle(hWrite);
}
