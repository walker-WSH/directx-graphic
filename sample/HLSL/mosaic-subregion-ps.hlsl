SamplerState sampleType : register(s0);
Texture2D image0 : register(t0);
Texture2D image1 : register(t1);

cbuffer ConstBuffer
{
	int texWidth;
	int texHeight;
	int mosaicSizeCX;
	int mosaicSizeCY;
};

struct PixelInputType {
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};

float4 main(PixelInputType input) : SV_TARGET
{
	float2 intXY = float2(input.tex.x * texWidth, input.tex.y * texHeight);
	float2 XYMosaic =
		float2(int(intXY.x / mosaicSizeCX) * mosaicSizeCX, int(intXY.y / mosaicSizeCY) * mosaicSizeCY);
	float2 UVMosaic = float2(XYMosaic.x / texWidth, XYMosaic.y / texHeight);

	float4 clrMosaic = image0.Sample(sampleType, UVMosaic);
	float4 clrOrigin = image0.Sample(sampleType, input.tex);
	float maskAlpha = image1.Sample(sampleType, input.tex).a;

	float4 clrReturn;
	clrReturn.rgb = clrMosaic.rgb * maskAlpha + clrOrigin.rgb * (1 - maskAlpha);
	clrReturn.a = clrOrigin.a;

	return clrReturn;
}
