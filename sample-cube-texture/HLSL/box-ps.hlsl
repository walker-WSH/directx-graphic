TextureCube g_TexCube : register(t0); // TextureCube 立方体纹理
SamplerState g_Sam : register(s0);

struct VSOutput {
	float4 posH : SV_POSITION;
	float3 posL : POSITION;
};

float4 main(VSOutput pIn) : SV_Target
{
	return float4(1, 0, 0, 1);
	return g_TexCube.Sample(g_Sam, pIn.posL);
}
