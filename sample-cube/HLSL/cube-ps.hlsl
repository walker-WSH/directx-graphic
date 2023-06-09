SamplerState sampleType : register(s0);
Texture2D image0 : register(t0);

struct PixelInputType {
	float4 position : SV_POSITION;
	float4 color : COLOR0;
};

float4 main(PixelInputType input) : SV_TARGET
{
	return input.color;
}