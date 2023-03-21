SamplerState sampleType : register(s0);
Texture2D image0 : register(t0);

#define PI 3.1415926f

cbuffer ConstBuffer
{
	int texWidth;  // 纹理宽度
	int texHeight; // 纹理高度
	int originPositionX;
	int originPositionY;
	int targetPositionX;
	int targetPositionY;
	float radius;
	float curve; // default 1
};

struct PixelInputType {
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};

float DistToLine(float2 p, float2 a, float2 b)
{
	float2 pa = p - a;
	float2 ba = b - a;
	float t = clamp(dot(pa, ba) / dot(ba, ba), 0.f, 1.f);
	return length(pa - ba * t);
}

float2 stretchFun(float2 tex)
{
	float2 textureCoord = float2(tex.x * texWidth, tex.y * texHeight);
	float2 originPosition = float2(originPositionX, originPositionY);
	float2 targetPosition = float2(targetPositionX, targetPositionY);

	float dis = DistToLine(textureCoord, originPosition, targetPosition);
	float infect = dis / radius;
	infect = clamp(infect, 0.0, 1.0);
	infect = (sin(infect * PI - PI / 2.f) + 1) / 2.f;
	infect = clamp(infect, 0.0, 1.0);

	float2 direction = targetPosition - originPosition;
	float directRatio = radius / distance(targetPosition, originPosition);
	directRatio = clamp(directRatio, 0.f, 1.f);
	direction *= directRatio;

	float weight = 1.0 - infect;
	float2 offset = direction * weight;
	float2 result = textureCoord - offset;

	return float2(result.x / texWidth, result.y / texHeight);
}

float4 main(PixelInputType input) : SV_TARGET
{
	float4 textureColor = image0.Sample(sampleType, stretchFun(input.tex));
	return textureColor;
}
