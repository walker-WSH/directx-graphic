#include "ps-to-rgb-base.hlsli"

float4 main(VertexOut pIn) : SV_Target
{
	float3 rgb = ps_yuy2_to_rgb(pIn);
	float4 res_color = float4(rgb, 1.0);
	return res_color;
}
