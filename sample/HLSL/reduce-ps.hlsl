SamplerState sampleType : register(s0);
Texture2D image0 : register(t0);

cbuffer ConstBuffer
{
	int texWidth;  // 纹理宽度
	int texHeight; // 纹理高度
	int centerX;
	int centerY;
	int radius;
	int power; // default 2
	float2 reserve;
};

struct PixelInputType {
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};

float4 main(PixelInputType input) : SV_TARGET
{
	float2 circleCenter = float2(centerX, centerY);
	float2 curCoord = float2(input.tex.x * texWidth, input.tex.y * texHeight);

	float currentDistance = distance(circleCenter, curCoord);
	if (currentDistance < radius) {
		float weight = sqrt(currentDistance / radius);
		weight = sqrt(weight);
		curCoord = circleCenter + (curCoord - circleCenter) / weight;

		float2 newTexUV = float2(curCoord.x / (float)texWidth, curCoord.y / (float)texHeight);
		float4 textureColor = image0.Sample(sampleType, newTexUV);
		return textureColor;

	} else if (currentDistance - radius <= 1) { // 画个边框
		float4 textureColor = float4(1, 0, 0, 1);
		return textureColor;

	} else {
		float4 textureColor = image0.Sample(sampleType, input.tex);
		return textureColor;
	}
}
