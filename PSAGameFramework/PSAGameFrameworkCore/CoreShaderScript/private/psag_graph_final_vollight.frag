uniform sampler2DArray ProcessTextures;

// err: failed load depth_texture. [2024.0701]
// uniform sampler2D PostTextureDep;

uniform vec2  LightPosition;
uniform vec3  LightColor;
uniform float LightIntensity;
uniform float LightIntensityDecay;
uniform float LightAvgCollision;
uniform int   LightSampleStep;

void main()
{
    vec2 Delta = FxCoord - LightPosition;
    vec2 STEP = Delta / float(LightSampleStep);

    float AccLightIntensity = 0.0; // 累积光强度.
    for (int i = 0; i < LightSampleStep; i++) 
    {
        vec2 CurrentPosition = LightPosition + STEP * float(i);
        vec4 SampleColor = texture(ProcessTextures, vec3(CurrentPosition, 0.0));

        float Distance = length(CurrentPosition - LightPosition); // 光线距离衰减.
        float CurrentLightIntensity = LightIntensity * exp(-Distance * LightIntensityDecay);

        if ((SampleColor.r + SampleColor.g + SampleColor.b) / 3.0 > LightAvgCollision) {
            CurrentLightIntensity *= LightIntensityDecay * 0.01; // 片段阻挡过滤.
        }
        AccLightIntensity += CurrentLightIntensity;
    }
    // 计算平均光强度.
    float AverageLightIntensity = AccLightIntensity / float(LightSampleStep);

    // 输出混合片段颜色.
    vec4 OriginalColor = texture(ProcessTextures, vec3(FxCoord, 0.0));
    FragColor = OriginalColor + vec4(LightColor, 1.0) * AverageLightIntensity;
}