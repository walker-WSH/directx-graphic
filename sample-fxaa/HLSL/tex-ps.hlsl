//=====================================================================================================
#ifndef FXAA_QUALITY__PRESET
#define FXAA_QUALITY__PRESET 39
#endif

//   FXAA ���� - ���������еȶ���
#if (FXAA_QUALITY__PRESET == 10)
#define FXAA_QUALITY__PS 3 
static const float s_SampleDistances[FXAA_QUALITY__PS] = { 1.5, 3.0, 12.0 };
#endif

#if (FXAA_QUALITY__PRESET == 11)
#define FXAA_QUALITY__PS 4 
static const float s_SampleDistances[FXAA_QUALITY__PS] = { 1.0, 1.5, 3.0, 12.0 };
#endif

#if (FXAA_QUALITY__PRESET == 12)
#define FXAA_QUALITY__PS 5 
static const float s_SampleDistances[FXAA_QUALITY__PS] = { 1.0, 1.5, 2.0, 4.0, 12.0 };
#endif

#if (FXAA_QUALITY__PRESET == 13)
#define FXAA_QUALITY__PS 6 
static const float s_SampleDistances[FXAA_QUALITY__PS] = { 1.0, 1.5, 2.0, 2.0, 4.0, 12.0 };
#endif

#if (FXAA_QUALITY__PRESET == 14)
#define FXAA_QUALITY__PS 7 
static const float s_SampleDistances[FXAA_QUALITY__PS] = { 1.0, 1.5, 2.0, 2.0, 2.0, 4.0, 12.0 };
#endif

#if (FXAA_QUALITY__PRESET == 15)
#define FXAA_QUALITY__PS 8 
static const float s_SampleDistances[FXAA_QUALITY__PS] = { 1.0, 1.5, 2.0, 2.0, 2.0, 2.0, 4.0, 12.0 };
#endif

//   FXAA ���� - �еȣ����ٶ���
#if (FXAA_QUALITY__PRESET == 20)
#define FXAA_QUALITY__PS 3 
static const float s_SampleDistances[FXAA_QUALITY__PS] = { 1.5, 2.0, 8.0 };
#endif

#if (FXAA_QUALITY__PRESET == 21)
#define FXAA_QUALITY__PS 4 
static const float s_SampleDistances[FXAA_QUALITY__PS] = { 1.0, 1.5, 2.0, 8.0 };
#endif

#if (FXAA_QUALITY__PRESET == 22)
#define FXAA_QUALITY__PS 5 
static const float s_SampleDistances[FXAA_QUALITY__PS] = { 1.0, 1.5, 2.0, 2.0, 8.0 };
#endif

#if (FXAA_QUALITY__PRESET == 23)
#define FXAA_QUALITY__PS 6 
static const float s_SampleDistances[FXAA_QUALITY__PS] = { 1.0, 1.5, 2.0, 2.0, 2.0, 8.0 };
#endif

#if (FXAA_QUALITY__PRESET == 24)
#define FXAA_QUALITY__PS 7 
static const float s_SampleDistances[FXAA_QUALITY__PS] = { 1.0, 1.5, 2.0, 2.0, 2.0, 3.0, 8.0 };
#endif

#if (FXAA_QUALITY__PRESET == 25)
#define FXAA_QUALITY__PS 8 
static const float s_SampleDistances[FXAA_QUALITY__PS] = { 1.0, 1.5, 2.0, 2.0, 2.0, 2.0, 4.0, 8.0 };
#endif

#if (FXAA_QUALITY__PRESET == 26)
#define FXAA_QUALITY__PS 9
static const float s_SampleDistances[FXAA_QUALITY__PS] = { 1.0, 1.5, 2.0, 2.0, 2.0, 2.0, 2.0, 4.0, 8.0 };
#endif

#if (FXAA_QUALITY__PRESET == 27)
#define FXAA_QUALITY__PS 10
static const float s_SampleDistances[FXAA_QUALITY__PS] = { 1.0, 1.5, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 4.0, 8.0 };
#endif

#if (FXAA_QUALITY__PRESET == 28)
#define FXAA_QUALITY__PS 11
static const float s_SampleDistances[FXAA_QUALITY__PS] = { 1.0, 1.5, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 4.0, 8.0 };
#endif

#if (FXAA_QUALITY__PRESET == 29)
#define FXAA_QUALITY__PS 12
static const float s_SampleDistances[FXAA_QUALITY__PS] = { 1.0, 1.5, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 4.0, 8.0 };
#endif

//   FXAA ���� - ��
#if (FXAA_QUALITY__PRESET == 39)
#define FXAA_QUALITY__PS 12
static const float s_SampleDistances[FXAA_QUALITY__PS] = { 1.0, 1.0, 1.0, 1.0, 1.0, 1.5, 2.0, 2.0, 2.0, 2.0, 4.0, 8.0 };
#endif


//----------------------------------------------------------------------
SamplerState g_SamplerLinearClamp : register(s0);
Texture2D<float4> g_TextureInput : register(t0);


struct PixelInputType {
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};


//
// ���ȼ���
//
float LinearRGBToLuminance(float3 LinearRGB)
{
    // return dot(LinearRGB, float3(0.299f, 0.587f, 0.114f));
    return dot(LinearRGB, float3(0.212671f, 0.715160, 0.072169));
    // return sqrt(dot(LinearRGB, float3(0.212671f, 0.715160, 0.072169)));
    // return log2(1 + dot(LinearRGB, float3(0.212671, 0.715160, 0.072169)) * 15) / 4;
}


cbuffer CB : register(b0)
{
    // Ӱ�������̶�
    // 1.00 - ���
    // 0.75 - Ĭ���˲�ֵ
    // 0.50 - ���������Ƴ����ٵ�����������
    // 0.25 - �����ص�
    // 0.00 - ��ȫ�ص�
   // float g_QualitySubPix;
    
    // ����ֲ��Աȶȵ���ֵ����
    // 0.333 - �ǳ��ͣ����죩
    // 0.250 - ������
    // 0.166 - Ĭ��
    // 0.125 - ������
    // 0.063 - �ǳ��ߣ�������
  //  float g_QualityEdgeThreshold;
    
    // �԰������򲻽��д������ֵ
    // 0.0833 - Ĭ��
    // 0.0625 - �Կ�
    // 0.0312 - ����
   // float g_QualityEdgeThresholdMin;
}


/*--------------------------------------------------------------------------*/
float4 main(PixelInputType input) : SV_TARGET
{
    uint Width = 0;
    uint Height= 0;        
    uint NumberOfLevels;
    g_TextureInput.GetDimensions(0,  Width, Height, NumberOfLevels);
    
    float2 g_TexelSize = float2(1.f / (float)Width, 1.f / (float)Height);
    float g_QualitySubPix = 1.f;
    float g_QualityEdgeThreshold = 0.063;
    float g_QualityEdgeThresholdMin = 0.0833;
    

    float2 posM = input.tex;
    float4 color = g_TextureInput.SampleLevel(g_SamplerLinearClamp, input.tex, 0);
    
    //   N
    // W M E
    //   S
    float lumaM = LinearRGBToLuminance(color.rgb);
    float lumaS = LinearRGBToLuminance(g_TextureInput.SampleLevel(g_SamplerLinearClamp, posM, 0, int2(0, 1)).rgb);
    float lumaE = LinearRGBToLuminance(g_TextureInput.SampleLevel(g_SamplerLinearClamp, posM, 0, int2(1, 0)).rgb);
    float lumaN = LinearRGBToLuminance(g_TextureInput.SampleLevel(g_SamplerLinearClamp, posM, 0, int2(0, -1)).rgb);
    float lumaW = LinearRGBToLuminance(g_TextureInput.SampleLevel(g_SamplerLinearClamp, posM, 0, int2(-1, 0)).rgb);

    //
    // ����Աȶȣ�ȷ���Ƿ�Ӧ�ÿ����
    //
    
    // ���5�������е����/��С������ȣ��õ��Աȶ�
    float lumaRangeMax = max(lumaM, max(max(lumaW, lumaE), max(lumaN, lumaS)));
    float lumaRangeMin = min(lumaM, min(min(lumaW, lumaE), min(lumaN, lumaS)));
    float lumaRange = lumaRangeMax - lumaRangeMin;
    // ������ȱ仯����һ����������ȳ�����ص���ֵ�����ߵ���һ��������ֵ��˵�����Ǵ��ڱ�Ե���򣬲������κο���ݲ���
    bool earlyExit = lumaRange < max(g_QualityEdgeThresholdMin, lumaRangeMax * g_QualityEdgeThreshold);

    // δ�ﵽ��ֵ����ǰ����
    if (earlyExit)
        return color;

    //
    // ȷ���߽��Ǿֲ�ˮƽ�Ļ�����ֱ��
    //
    
    //           
    //  NW N NE          
    //  W  M  E
    //  WS S SE     
    //  edgeHorz = |(NW - W) - (W - WS)| + 2|(N - M) - (M - S)| + |(NE - E) - (E - SE)|
    //  edgeVert = |(NE - N) - (N - NW)| + 2|(E - M) - (M - W)| + |(SE - S) - (S - WS)|
    float lumaNW = LinearRGBToLuminance(g_TextureInput.SampleLevel(g_SamplerLinearClamp, posM, 0, int2(-1, -1)).rgb);
    float lumaSE = LinearRGBToLuminance(g_TextureInput.SampleLevel(g_SamplerLinearClamp, posM, 0, int2(1, 1)).rgb);
    float lumaNE = LinearRGBToLuminance(g_TextureInput.SampleLevel(g_SamplerLinearClamp, posM, 0, int2(1, -1)).rgb);
    float lumaSW = LinearRGBToLuminance(g_TextureInput.SampleLevel(g_SamplerLinearClamp, posM, 0, int2(-1, 1)).rgb);

    float lumaNS = lumaN + lumaS;
    float lumaWE = lumaW + lumaE;
    float lumaNESE = lumaNE + lumaSE;
    float lumaNWNE = lumaNW + lumaNE;
    float lumaNWSW = lumaNW + lumaSW;
    float lumaSWSE = lumaSW + lumaSE;

    // ����ˮƽ�ʹ�ֱ�Աȶ�
    float edgeHorz = abs(lumaNWSW - 2.0 * lumaW) + abs(lumaNS - 2.0 * lumaM) * 2.0 + abs(lumaNESE - 2.0 * lumaE);
    float edgeVert = abs(lumaSWSE - 2.0 * lumaS) + abs(lumaWE - 2.0 * lumaM) * 2.0 + abs(lumaNWNE - 2.0 * lumaN);

    // �ж��� �ֲ�ˮƽ�߽� ���� �ֲ���ֱ�߽�
    bool horzSpan = edgeHorz >= edgeVert;
    
    //
    // �����ݶȡ�ȷ���߽緽��
    //
    float luma1 = horzSpan ? lumaN : lumaW;
    float luma2 = horzSpan ? lumaS : lumaE;
    
    float gradient1 = luma1 - lumaM;
    float gradient2 = luma2 - lumaM;
    // �����Ӧ������ݶȣ�Ȼ������������ں����Ƚ�
    float gradientScaled = max(abs(gradient1), abs(gradient2)) * 0.25f;
    // �ĸ��������
    bool is1Steepest = abs(gradient1) >= abs(gradient2);
    
    //
    // ��ǰ�������ݶȷ����ƶ����texel
    //
    float lengthSign = horzSpan ? g_TexelSize.y : g_TexelSize.x;
    lengthSign = is1Steepest ? -lengthSign : lengthSign;
    
    float2 posB = posM.xy;
    // ��texelƫ��
    if (!horzSpan)
        posB.x += lengthSign * 0.5;
    if (horzSpan)
        posB.y += lengthSign * 0.5;
    
    //
    // ������posB���ڵ��������ص�luma��ƽ��ֵ
    //
    float luma3 = luma1 + lumaM;
    float luma4 = luma2 + lumaM;
    float lumaLocalAvg = luma3;
    if (!is1Steepest)
        lumaLocalAvg = luma4;
    lumaLocalAvg *= 0.5f;
    
    

    //
    // �����߽��б�����ֱ������������ﵽ�Ǳ�Ե��
    //
    
    // �ر߽�������ƫ��
    // 0    0    0
    // <-  posB ->
    // 1    1    1
    float2 offset;
    offset.x = (!horzSpan) ? 0.0 : g_TexelSize.x;
    offset.y = (horzSpan) ? 0.0 : g_TexelSize.y;
    // ������ƫ��
    float2 posN = posB - offset * s_SampleDistances[0];
    // ������ƫ��
    float2 posP = posB + offset * s_SampleDistances[0];
    
    // ��ƫ�ƺ�ĵ��ȡlumaֵ��Ȼ��������м��luma�Ĳ���
    float lumaEndN = LinearRGBToLuminance(g_TextureInput.SampleLevel(g_SamplerLinearClamp, posN, 0).rgb);
    float lumaEndP = LinearRGBToLuminance(g_TextureInput.SampleLevel(g_SamplerLinearClamp, posP, 0).rgb);
    lumaEndN -= lumaLocalAvg;
    lumaEndP -= lumaLocalAvg;
    
    // ����˵㴦��luma������ھֲ��ݶȣ�˵�������Ե��һ��
    bool doneN = abs(lumaEndN) >= gradientScaled;
    bool doneP = abs(lumaEndP) >= gradientScaled;
    bool doneNP = doneN && doneP;
    
    // ���û�е���Ǳ�Ե�㣬�������Ÿ÷�������
    if (!doneN)
        posN -= offset * s_SampleDistances[1];
    if (!doneP)
        posP += offset * s_SampleDistances[1];

    // ��������ֱ�����߶������Ե��һ�࣬���ߴﵽ��������
    if (!doneNP)
    {
        [unroll]
        for (int i = 2; i < FXAA_QUALITY__PS; ++i)
        {
            if (!doneN)
                lumaEndN = LinearRGBToLuminance(g_TextureInput.SampleLevel(g_SamplerLinearClamp, posN.xy, 0).rgb) - lumaLocalAvg;
            if (!doneP)
                lumaEndP = LinearRGBToLuminance(g_TextureInput.SampleLevel(g_SamplerLinearClamp, posP.xy, 0).rgb) - lumaLocalAvg;
            
            doneN = abs(lumaEndN) >= gradientScaled;
            doneP = abs(lumaEndP) >= gradientScaled;
            doneNP = doneN && doneP;
        
            if (!doneN)
                posN -= offset * s_SampleDistances[i];
            if (!doneP)
                posP += offset * s_SampleDistances[i];
            // ���߶������Ե��һ���ͣ��
            if (doneNP)
                break;
        }
    }
    
    // �ֱ���㵽�����˵�ľ���
    float distN = horzSpan ? (posM.x - posN.x) : (posM.y - posN.y);
    float distP = horzSpan ? (posP.x - posM.x) : (posP.y - posM.y);

    // ����ǰ�㵽��һ���˵������ȡ�����
    bool directionN = distN < distP;
    float dist = min(distN, distP);
    
    // ���˵��ľ���
    float spanLength = (distP + distN);
    
    // ��������˵��ƶ�������ƫ����
    float pixelOffset = -dist / spanLength + 0.5f;
/*--------------------------------------------------------------------------*/
    
    // ��ǰ���ص�luma�Ƿ�С��posB���ڵ��������ص�luma��ƽ��ֵ
    bool isLumaMSmaller = lumaM < lumaLocalAvg;
    
    // �ж����Ƿ�Ϊһ���õı߽�
    bool goodSpanN = (lumaEndN < 0.0) != isLumaMSmaller;
    bool goodSpanP = (lumaEndP < 0.0) != isLumaMSmaller;
    bool goodSpan = directionN ? goodSpanN : goodSpanP;
    
    // ������ǵĻ���������ƫ��
    float pixelOffsetGood = goodSpan ? pixelOffset : 0.0;
/*--------------------------------------------------------------------------*/
    
    // ��3x3��Χ���ص����ȱ仯
    //      [1  2  1]
    // 1/12 [2 -12 2]
    //      [1  2  1]
    float subpixNSWE = lumaNS + lumaWE;
    float subpixNWSWNESE = lumaNWSW + lumaNESE;
    float subpixA = (2.0 * subpixNSWE + subpixNWSWNESE) * (1.0 / 12.0) - lumaM;
    // ����������ȱ仯����������ƫ����
    float subpixB = saturate(abs(subpixA) * (1.0 / lumaRange));
    float subpixC = (-2.0 * subpixB + 3.0) * subpixB * subpixB;
    float subpix = subpixC * subpixC * g_QualitySubPix;
    
    // ѡ������ƫ��
    float pixelOffsetSubpix = max(pixelOffsetGood, subpix);
/*--------------------------------------------------------------------------*/
    
    if (!horzSpan)
        posM.x += pixelOffsetSubpix * lengthSign;
    if (horzSpan)
        posM.y += pixelOffsetSubpix * lengthSign;
        
#ifdef DEBUG_OUTPUT
    return float4(1.0f - 2.0f * pixelOffsetSubpix, 2.0f * pixelOffsetSubpix, 0.0f, 1.0f);
#else
    return float4(g_TextureInput.SampleLevel(g_SamplerLinearClamp, posM, 0).xyz, lumaM);
#endif
}
