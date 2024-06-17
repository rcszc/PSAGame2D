uniform sampler2DArray PostTextures;

uniform vec2  LightPosition;
uniform vec3  LightColor;
uniform float LightIntensityDecay;
uniform int   LightSampleStep;

void main()
{
    vec2 LightPos = LightPosition;

    // 初始光强度.
    float LightIntensity = 100.0;

    // 从光源位置到当前片段位置投射光线.
    vec2 Delta = FxCoord - LightPos;
    vec2 Step = Delta / float(LightSampleStep);

    float IntensityBlend = 0.0;
	for (int i = 0; i < LightSampleStep; i++) {
        vec2 CurrentPos = LightPos + Step * float(i);
        vec4 SampleColor = texture(PostTextures, vec3(CurrentPos, 0.0));

        // 如果遇到不透明片段，减少光强度.
        if ((SampleColor.r + SampleColor.g + SampleColor.b) / 3.0 < 0.1) {
            LightIntensity *= LightIntensityDecay;
            IntensityBlend = LightIntensity;
        }
        else {
            IntensityBlend = 1.0;
        }
    }
    FragColor = texture(PostTextures, vec3(FxCoord, 0.0)) * IntensityBlend * vec4(LightColor, 1.0);
}