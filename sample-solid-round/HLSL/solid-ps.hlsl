SamplerState sampleType : register(s0);
Texture2D image0 : register(t0);

cbuffer MatrixBuffer
{
	float size;
	float offsetAlpha;
	float moveX;
	float moveY;
};

struct PixelInputType {
	float4 position : SV_POSITION;
};

float4 main(PixelInputType input) : SV_TARGET
{
	float3 pos = float3(input.position.xyz);
	float3 farPt = float3(moveX, moveY, 0);
	float3 center = float3(moveX + size / 2, moveY + size / 2, 0);

	float r = distance(farPt, center);
	float dist = distance(pos, center);
	float a = saturate(dist / r + offsetAlpha);

	float4 textureColor = float4(1.f, 0, 0, a);
	return textureColor;
}