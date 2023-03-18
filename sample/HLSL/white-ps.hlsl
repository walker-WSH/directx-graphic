SamplerState sampleType : register(s0);
Texture2D image0 : register(t0);

cbuffer ConstBuffer
{
	float intensity; // default 1.f
	float3 reserve;
};

struct PixelInputType {
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};

float4 main(PixelInputType input) : SV_TARGET
{
	float4 textureColor = image0.Sample(sampleType, input.tex);
	float4 clr = textureColor * intensity;
	clr = clamp(clr, 0.f, 1.f);
	clr.a = textureColor.a;
	return clr;
}
