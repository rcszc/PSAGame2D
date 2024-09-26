uniform sampler2DArray ProcessTextures;
uniform vec2 RenderRes;

uniform float OutFragmentBlur;   // ģ����ɫ���Ȩ��.
uniform float OutFragmentSource; // Դ��ɫ���Ȩ��.

const float STD_BLEND_W_SRC  = 1.2;
const float STD_BLEND_W_BLUR = 1.6;

void main() 
{
	vec2 SampleCoord = vec2(FxCoord.x, 1.0 - FxCoord.y);

	vec4 SceneColor     = texture(ProcessTextures, vec3(SampleCoord, 1.0));
	vec4 SceneBlurColor = texture(ProcessTextures, vec3(SampleCoord, 4.0));

	FragColor = vec4(0.0);

	FragColor += SceneColor     * STD_BLEND_W_SRC  * OutFragmentSource;
	FragColor += SceneBlurColor * STD_BLEND_W_BLUR * OutFragmentBlur;
}