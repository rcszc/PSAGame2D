uniform sampler2DArray PostTextures;
uniform vec2 RenderRes;

uniform float PostBloomBlur;   // ģ����ɫ���Ȩ��.
uniform float PostBloomSource; // Դ��ɫ���Ȩ��.

void main() 
{
	vec2 SampleCoord = vec2(FxCoord.x, 1.0 - FxCoord.y);

	vec4 SceneColor     = texture(PostTextures, vec3(SampleCoord, 0.0));
	vec4 SceneBlurColor = texture(PostTextures, vec3(SampleCoord, 3.0));

	SceneColor += SceneBlurColor;

	FragColor = (SceneColor * 1.42 * PostBloomSource + SceneBlurColor * 2.4 * PostBloomBlur) * 0.5;
}