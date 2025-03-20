
cbuffer MatrixBuffer
{
    matrix wvp;
};

struct VertexInput
{
    float2 position : POSITION;
};

struct VertexOutput
{
    float4 position : SV_POSITION;
};

VertexOutput main(VertexInput input)
{
    VertexOutput output;
    output.position = mul(float4(input.position.xy, 0.f, 1.f), wvp);
    return output;
}
