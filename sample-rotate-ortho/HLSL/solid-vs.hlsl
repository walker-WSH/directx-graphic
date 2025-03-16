
cbuffer MatrixBuffer
{
    matrix wvp;
};

struct VertexInput
{
    float3 position : POSITION;
    float3 clr : COLOR0;
};

struct VertexOutput
{
    float4 position : SV_POSITION;
    float3 clr : COLOR0;
};

VertexOutput main(VertexInput input)
{
    VertexOutput output;
    output.position = mul(float4(input.position.xyz, 1.f), wvp);
    output.clr = input.clr;
    return output;
}
