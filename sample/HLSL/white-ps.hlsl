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

	float R = textureColor.r;
	float G = textureColor.g;
	float B = textureColor.b;

	float Y = 0.256788 * R + 0.504129 * G + 0.097906 * B + 16.f / 255.f;
	float U = -0.148223 * R - 0.290993 * G + 0.439216 * B + 128.f / 255.f;
	float V = 0.439216 * R - 0.367788 * G - 0.071427 * B + 128.f / 255.f;
	Y *= intensity; // ¡¡∂»«ø∂»
	Y = clamp(Y, 0.f, 1.f);
	U = clamp(U, 0.f, 1.f);
	V = clamp(V, 0.f, 1.f);

	float r = 1.164383 * (Y - 16.f / 255.f) + 1.596027 * (V - 128.f / 255.f);
	float g = 1.164383 * (Y - 16.f / 255.f) - 0.391762 * (U - 128.f / 255.f) - 0.812968 * (V - 128.f / 255.f);
	float b = 1.164383 * (Y - 16.f / 255.f) + 2.017232 * (U - 128.f / 255.f);
	r = clamp(r, 0.f, 1.f);
	g = clamp(g, 0.f, 1.f);
	b = clamp(b, 0.f, 1.f);

	return float4(r, g, b, textureColor.a);
}
