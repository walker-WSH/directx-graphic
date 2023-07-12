#ifndef MSAA_SAMPLES
#define MSAA_SAMPLES 8 // TODO : 外部传入
#endif

SamplerState sampleType : register(s0);
Texture2DMS<float4, MSAA_SAMPLES> image0 : register(t0); // only supported from ps-5.0

// 感觉效果也一般

struct PixelInputType {
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};

float4 main(PixelInputType input) : SV_TARGET
{
	uint width = 0;
	uint height = 0;
	uint numberOfSamples = 1;
	image0.GetDimensions(width, height, numberOfSamples);

	uint2 coords = uint2(input.tex.x * width, input.tex.y * height);
	float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);

	for (unsigned int i = 0; i < numberOfSamples; ++i) {
		color += image0.Load(coords, i);
	}

	return float4(color * rcp((float)numberOfSamples));
}