#ifndef MSAA_SAMPLES
#define MSAA_SAMPLES 8 // TODO : 外部传入
#endif

// TODO : 外部传入
#define TEXTURE_WIDTH 1460
#define TEXTURE_HEIGHT 831

SamplerState sampleType : register(s0);
Texture2DMS<float4, MSAA_SAMPLES> image0 : register(t0); // only supported from ps-5.0

struct PixelInputType {
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};

float4 main(PixelInputType input) : SV_TARGET
{
	uint2 coords = uint2(input.tex.x * TEXTURE_WIDTH, input.tex.y * TEXTURE_HEIGHT);
	float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);

#if MSAA_SAMPLES <= 1
	[unroll]
#endif
	for (unsigned int i = 0; i < MSAA_SAMPLES; ++i) {
		color += image0.Load(coords, i);
	}

	return float4(color * rcp((float)MSAA_SAMPLES));
}