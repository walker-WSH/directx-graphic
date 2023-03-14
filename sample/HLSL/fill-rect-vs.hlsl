
cbuffer MatrixBuffer
{
	matrix wvp;
};

struct VertexOutput {
	float4 position : SV_POSITION;
};

VertexOutput main(float4 position : SV_POSITION)
{
	VertexOutput output;
	output.position = mul(float4(position.xyz, 1.f), wvp);
	return output;
}
