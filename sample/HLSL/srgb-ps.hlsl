SamplerState sampleType : register(s0);
Texture2D image0 : register(t0);

struct PixelInputType {
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};

float4 main(PixelInputType input) : SV_TARGET
{
	float4 textureColor = image0.Sample(sampleType, input.tex);
	return textureColor;
}
