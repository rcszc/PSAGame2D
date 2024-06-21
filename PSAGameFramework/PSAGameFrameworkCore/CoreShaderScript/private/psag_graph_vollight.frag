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

    // ��ǰƬ��λ��Ͷ�����.
    vec2 Delta = FxCoord - LightPos;
    vec2 Step = Delta / float(LightSampleStep);

    // �ۻ���ǿ��.
    float AccumulatedLightIntensity = 0.0;

    for (int i = 0; i < LightSampleStep; i++) {
        vec2 CurrentPos = LightPos + Step * float(i);
        vec4 SampleColor = texture(PostTextures, vec3(CurrentPos, 0.0));

        // ���߾���˥��.
        float Distance = length(CurrentPos - LightPos);
        float CurrentLightIntensity = LightIntensity * exp(-Distance * LightIntensityDecay);

        // Ƭ���赲����.
        if ((SampleColor.r + SampleColor.g + SampleColor.b) / 3.0 > 0.1) {
            CurrentLightIntensity *= LightIntensityDecay * 0.01;
        }
        AccumulatedLightIntensity += CurrentLightIntensity;
    }

    // ����ƽ����ǿ��.
    float AverageLightIntensity = AccumulatedLightIntensity / float(LightSampleStep);

    // ������Ƭ����ɫ.
    vec4 OriginalColor = texture(PostTextures, vec3(FxCoord, 0.0));
    FragColor = OriginalColor + vec4(LightColor, 1.0) * AverageLightIntensity;
}