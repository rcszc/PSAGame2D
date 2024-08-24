uniform sampler2DArray PostTextures;
uniform vec2 RenderRes;

uniform float PostBloomBlur;   // ģ����ɫ���Ȩ��.
uniform float PostBloomSource; // Դ��ɫ���Ȩ��.

void main() 
{
	vec2 SampleCoord = vec2(FxCoord.x, 1.0 - FxCoord.y);

	vec4 SceneColor     = texture(PostTextures, vec3(SampleCoord, 1.0));
	vec4 SceneBlurColor = texture(PostTextures, vec3(SampleCoord, 4.0));

	FragColor = SceneColor * 1.2 * PostBloomSource + SceneBlurColor * 1.6 * PostBloomBlur;
}