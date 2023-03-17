SamplerState sampleType : register(s0);
Texture2D image0 : register(t0);

cbuffer ConstBuffer
{
	int texWidth;  // 纹理宽度
	int texHeight; // 纹理高度
	int centerX;
	int centerY;
	float radius;
	float intensity;       // [0.f, 100.f]
	float intensityDiv100; // intensity / 100.f
	float reserve;
};

struct PixelInputType {
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};

float4 main(PixelInputType input) : SV_TARGET
{
	float2 circleCenter = float2(centerX, centerY);
	float2 curCoord = float2(input.tex.x * texWidth, input.tex.y * texHeight);

	float dis = distance(circleCenter, curCoord);
	if (dis < radius) {
		float temp = pow(dis, 2) / pow(radius, 2);
		float k = 1.f - (1.f - temp) * intensityDiv100;
		k = clamp(k, 0.f, 1.f);
		curCoord = circleCenter + (curCoord - circleCenter) * k;

		float2 newTexUV = float2(curCoord.x / (float)texWidth, curCoord.y / (float)texHeight);
		float4 textureColor = image0.Sample(sampleType, newTexUV);
		return textureColor;

	} else if (dis - radius <= 1) { // 画个边框
		float4 textureColor = float4(1, 0, 0, 1);
		return textureColor;

	} else {
		float4 textureColor = image0.Sample(sampleType, input.tex);
		return textureColor;
	}
}
