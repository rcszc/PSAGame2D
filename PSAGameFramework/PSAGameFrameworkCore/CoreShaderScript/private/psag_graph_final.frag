uniform sampler2DArray ProcessTextures;
uniform vec2 RenderRes;

uniform float OutFragmentBlur;   // ģ����ɫ���Ȩ��.
uniform float OutFragmentSource; // Դ��ɫ���Ȩ��.

uniform float OutFragmentContrast; // ���նԱȶ�.
uniform vec3  OutFragmentBlend;    // ���ջ����ɫ.
uniform vec2  OutFragmentVignette; // ���հ��Ǵ���.

const float STD_BLEND_W_SRC  = 1.0;
const float STD_BLEND_W_BLUR = 1.6;

const vec2 VignetteCenter = vec2(0.5, 0.5);

void main() 
{
	vec2 SampleCoord = vec2(FxCoord.x, 1.0 - FxCoord.y);

	vec4 SceneColor     = texture(ProcessTextures, vec3(SampleCoord, 1.0));
	vec4 SceneBlurColor = texture(ProcessTextures, vec3(SampleCoord, 4.0));

	vec4 FragmentProcess = vec4(0.0);

	FragmentProcess += SceneColor     * STD_BLEND_W_SRC  * OutFragmentSource;
	FragmentProcess += SceneBlurColor * STD_BLEND_W_BLUR * OutFragmentBlur;

	FragmentProcess.rgb = (FragmentProcess.rgb - 0.5) * OutFragmentContrast + 0.5;
	FragmentProcess.rgb *= OutFragmentBlend;

    float Dist = length(FxCoord - VignetteCenter);
    float VignetteValue = smoothstep(OutFragmentVignette.x, OutFragmentVignette.x - OutFragmentVignette.y, Dist);

	FragColor = FragmentProcess * vec4(vec3(VignetteValue), 1.0);
}