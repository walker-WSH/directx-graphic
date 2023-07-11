SamplerState sampleType : register(s0);
Texture2D image0 : register(t0);

cbuffer ConstBuffer
{
	int texWidth;     // 纹理宽度
	int texHeight;    // 纹理高度
	int mosaicSizeCX; // 马赛克方块水平大小
	int mosaicSizeCY; // 马赛克方块垂直大小
};

struct PixelInputType {
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};

float4 main(PixelInputType input) : SV_TARGET
{
	//得到当前纹理坐标相对图像大小整数值。
	float2 intXY = float2(input.tex.x * texWidth, input.tex.y * texHeight);

	//根据马赛克块大小进行取整。
	float2 XYMosaic = float2(int(intXY.x / mosaicSizeCX) * mosaicSizeCX,
				 int(intXY.y / mosaicSizeCY) * mosaicSizeCY);

	//把整数坐标转换回纹理采样坐标
	float2 UVMosaic = float2(XYMosaic.x / texWidth, XYMosaic.y / texHeight);

	float4 textureColor = image0.Sample(sampleType, UVMosaic);
	return textureColor;
}
