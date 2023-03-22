SamplerState sampleType : register(s0);
Texture2D image0 : register(t0);

cbuffer ConstBuffer
{
	float intensity; // default 1.f
	int toGrey;
	float2 reserve;
};

struct PixelInputType {
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};

static const float4 to_y = float4(0.256788194, 0.504129171, 0.0979057774, 0.0627449304);
static const float4 to_u = float4(-0.148222953, -0.290992796, 0.439215750, 0.501961052);
static const float4 to_v = float4(0.439215571, -0.367788255, -0.0714273080, 0.501960695);

static const float4 to_r = float4(1.16438401, 0, 1.59602702, -0.874202013);
static const float4 to_g = float4(1.16438401, -0.391761988, -0.812968016, 0.531668007);
static const float4 to_b = float4(1.16438401, 2.01723194, 0, -1.08563101);
static const float3 color_range_min = float3(0.0627451017, 0.0627451017, 0.0627451017);
static const float3 color_range_max = float3(0.921568632, 0.941176474, 0.941176474);

float4 main(PixelInputType input) : SV_TARGET
{
	float4 rgba = image0.Sample(sampleType, input.tex);

	float y = dot(to_y.xyz, rgba.xyz) + to_y.w;
	y *= intensity;
	y = clamp(y, 0.f, 1.f);

	if (toGrey != 0) {
		return float4(y, y, y, rgba.a);
	}

	float u = dot(to_u.xyz, rgba.xyz) + to_u.w;
	float v = dot(to_v.xyz, rgba.xyz) + to_v.w;

	float3 yuv = float3(y, u, v);
	yuv = clamp(yuv, color_range_min, color_range_max);

	float r = dot(to_r.xyz, yuv) + to_r.w;
	float g = dot(to_g.xyz, yuv) + to_g.w;
	float b = dot(to_b.xyz, yuv) + to_b.w;

	return float4(r, g, b, rgba.a);
}
