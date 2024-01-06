SamplerState sampleType : register(s0);
Texture2D image0 : register(t0);

struct PixelInputType {
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};

float4 main(PixelInputType input) : SV_TARGET
{
	float4 textureColor = image0.Sample(sampleType, input.tex);
	
	// ¼ì²âÑÀ³ÝÑÕÉ«
	if (textureColor.r > 0.35 && textureColor.g > 0.35 && textureColor.b > 0.35){
			float t1 = abs(textureColor.r - textureColor.g);
			float t2 = abs(textureColor.g - textureColor.b);
			if (t1 < 0.2 && t2 < 0.16){
				return float4(0.0, 1.0, 0, 1.0);
			}
	}

	return textureColor;
}