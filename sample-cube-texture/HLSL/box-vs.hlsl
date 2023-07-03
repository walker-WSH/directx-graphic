
TextureCube g_TexCube : register(t0);
SamplerState g_Sam : register(s0);

cbuffer CBChangesEveryFrame : register(b0)
{
	matrix g_WorldViewProj;
}

struct VertexInput {
	float3 posL : POSITION;
};

struct VSOutput {
	float4 posH : SV_POSITION;
	float3 posL : POSITION;
};

VSOutput main(VertexInput vIn)
{
	// ����z = w��ʹ��z/w = 1(��պб�����Զƽ��)
	float4 temp = mul(float4(vIn.posL, 1.0f), g_WorldViewProj);

	VSOutput vOut;
	vOut.posH = temp.xyww;
	vOut.posL = vIn.posL;
	return vOut;
}
