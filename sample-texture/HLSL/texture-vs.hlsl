
struct VertexInput {
	float4 position : POSITION;
	float2 tex : TEXCOORD0;
};

struct VertexOutput {
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};

VertexOutput main(VertexInput input)
{
	VertexOutput output;
	output.position = float4(input.position.xyz, 1.f);
	output.tex = input.tex;
	return output;
}