
cbuffer MatrixBuffer
{
	matrix wvp;
};

struct VertexInput{
	float4 position : POSITION;
	float4 color : COLOR0;
};

struct VertexOutput {
	float4 position : SV_POSITION;
	float4 color : COLOR0;
};

VertexOutput main(VertexInput input)
{
	VertexOutput output;
	output.position = mul(float4(input.position.xyz, 1.f), wvp);
	output.color = input.color;
	return output;
}