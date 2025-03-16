SamplerState sampleType : register(s0);
Texture2D image0 : register(t0);

struct PixelInputType {
	float4 position : SV_POSITION;
	float3 clr : COLOR0;
};

float4 main(PixelInputType input) : SV_TARGET
{
    float4 textureColor = float4(input.clr, 1.f);
	return textureColor;
}