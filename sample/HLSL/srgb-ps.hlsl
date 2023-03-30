SamplerState sampleType : register(s0);
Texture2D image0 : register(t0);

struct PixelInputType {
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};

float3 SRGB_to_linearRGB(float3 srgbColor)
{
	return pow(srgbColor, 2.2f);
}

float3 linearRGB_to_SRGB(float3 rgb)
{
	return pow(rgb, 1.0 / 2.2);
}

float4 main(PixelInputType input) : SV_TARGET
{
	// Note: Here the image0 is based on RGB while renderTarget is sRGB
	float4 clr = image0.Sample(sampleType, input.tex);
	return float4(SRGB_to_linearRGB(clr.rgb), clr.a);
}
