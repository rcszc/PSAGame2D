uniform sampler2DArray PostTextures;
uniform sampler2D PostDepTexture;

uniform vec2  LightPosition;
uniform vec3  LightColor;
uniform float LightIntensity;
uniform float LightIntensityDecay;
uniform int   LightSampleStep;

void main()
{
    vec2 LightPos = LightPosition;

    // 当前片段位置投射光线.
    vec2 Delta = FxCoord - LightPos;
    vec2 Step = Delta / float(LightSampleStep);

    // 累积光强度.
    float AccumulatedLightIntensity = 0.0;

    for (int i = 0; i < LightSampleStep; i++) {
        vec2 CurrentPos = LightPos + Step * float(i);
        vec4 SampleColor = texture(PostTextures, vec3(CurrentPos, 0.0));

        // 光线距离衰减.
        float Distance = length(CurrentPos - LightPos);
        float CurrentLightIntensity = LightIntensity * exp(-Distance * LightIntensityDecay);

        // 片段阻挡过滤.
        if ((SampleColor.r + SampleColor.g + SampleColor.b) / 3.0 > 0.1) {
            CurrentLightIntensity *= LightIntensityDecay * 0.01;
        }
        AccumulatedLightIntensity += CurrentLightIntensity;
    }

    // 计算平均光强度.
    float AverageLightIntensity = AccumulatedLightIntensity / float(LightSampleStep);

    // 输出混合片段颜色.
    vec4 OriginalColor = texture(PostTextures, vec3(FxCoord, 0.0));
    FragColor = OriginalColor + vec4(LightColor, 1.0) * AverageLightIntensity;
}