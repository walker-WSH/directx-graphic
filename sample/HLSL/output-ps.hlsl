SamplerState sampleType : register(s0);
Texture2D image0 : register(t0);

RWTexture2D<float4> g_Output1 : register(u1);
RWTexture2D<float4> g_Output2 : register(u2);

struct PixelInputType {
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};

float4 main(PixelInputType input) : SV_TARGET
{
	float4 textureColor = image0.Sample(sampleType, input.tex);

	g_Output1[input.tex] = textureColor;
	g_Output2[input.tex] = float4(1.f, 0, 0, 1.f);

	return textureColor;
}
