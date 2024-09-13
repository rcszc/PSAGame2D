uniform sampler2DArray PostTextures;

// err: failed load depth_texture. [2024.0701]
// uniform sampler2D PostTextureDep;

uniform vec2  LightPosition;
uniform vec3  LightColor;
uniform float LightIntensity;
uniform float LightIntensityDecay;
uniform int   LightSampleStep;

void main()
{
    vec2 Delta = FxCoord - LightPosition;
    vec2 STEP = Delta / float(LightSampleStep);

    // 累积光强度.
    float AccLightIntensity = 0.0;
    for (int i = 0; i < LightSampleStep; i++) 
    {
        vec2 CurrentPosition = LightPosition + STEP * float(i);
        vec4 SampleColor = texture(PostTextures, vec3(CurrentPosition, 0.0));

        // 光线距离衰减.
        float Distance = length(CurrentPosition - LightPosition);
        float CurrentLightIntensity = LightIntensity * exp(-Distance * LightIntensityDecay);

        // 片段阻挡过滤.
        if ((SampleColor.r + SampleColor.g + SampleColor.b) / 3.0 > 0.1) {
            CurrentLightIntensity *= LightIntensityDecay * 0.01;
        }
        AccLightIntensity += CurrentLightIntensity;
    }
    // 计算平均光强度.
    float AverageLightIntensity = AccLightIntensity / float(LightSampleStep);

    // 输出混合片段颜色.
    vec4 OriginalColor = texture(PostTextures, vec3(FxCoord, 0.0));
    FragColor = OriginalColor + vec4(LightColor, 1.0) * AverageLightIntensity;
}