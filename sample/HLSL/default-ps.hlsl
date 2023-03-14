
Texture2D SHADER_TEXTURE;
SamplerState SampleType;
 
struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};
 
float4 main(PixelInputType input) : SV_TARGET
{ 
    float4 textureColor = SHADER_TEXTURE.Sample(SampleType, input.tex);
    return textureColor;
}