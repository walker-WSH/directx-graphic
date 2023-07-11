SamplerState sampleType : register(s0);
Texture2D image0 : register(t0);

cbuffer ConstBuffer
{
	int texWidth;     // ������
	int texHeight;    // ����߶�
	int mosaicSizeCX; // �����˷���ˮƽ��С
	int mosaicSizeCY; // �����˷��鴹ֱ��С
};

struct PixelInputType {
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};

float4 main(PixelInputType input) : SV_TARGET
{
	//�õ���ǰ�����������ͼ���С����ֵ��
	float2 intXY = float2(input.tex.x * texWidth, input.tex.y * texHeight);

	//���������˿��С����ȡ����
	float2 XYMosaic = float2(int(intXY.x / mosaicSizeCX) * mosaicSizeCX,
				 int(intXY.y / mosaicSizeCY) * mosaicSizeCY);

	//����������ת���������������
	float2 UVMosaic = float2(XYMosaic.x / texWidth, XYMosaic.y / texHeight);

	float4 textureColor = image0.Sample(sampleType, UVMosaic);
	return textureColor;
}
