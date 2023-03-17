SamplerState sampleType : register(s0);
Texture2D image0 : register(t0);

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

float2 stretchFun(float2 tex)
{
	float2 textureCoord = float2(tex.x * texWidth, tex.y * texHeight);
	float2 originPosition = float2(originPositionX, originPositionY);
	float2 targetPosition = float2(targetPositionX, targetPositionY);
	float2 direction = targetPosition - originPosition;

	float infect = distance(textureCoord, originPosition) / radius;

	infect = pow(infect, curve); // 默认 curve 为1,这个值越大,拉伸到指定点越圆润,越小越尖
	infect = 1.0 - infect;
	infect = clamp(infect, 0.0, 1.0);

	float2 offset = direction * infect;
	float2 result = textureCoord - offset;

	return float2(result.x / texWidth, result.y / texHeight);
}

float4 main(PixelInputType input) : SV_TARGET
{
	// // 画个边框
	float2 textureCoord = float2(input.tex.x * texWidth, input.tex.y * texHeight);
	float2 originPosition = float2(originPositionX, originPositionY);
	float2 targetPosition = float2(targetPositionX, targetPositionY);
	float dis = distance(textureCoord, originPosition);
	if (dis >= radius && dis < (radius + 1)) {
		float4 textureColor = float4(1, 0, 0, 1);
		return textureColor;
	}

	float4 textureColor = image0.Sample(sampleType, stretchFun(input.tex));
	return textureColor;
}
