uniform sampler2DArray PostTextures;
uniform vec2 RenderRes;

uniform float PostBloomBlur;   // 模糊颜色混合权重.
uniform float PostBloomSource; // 源颜色混合权重.

void main() 
{
	vec2 SampleCoord = vec2(FxCoord.x, 1.0 - FxCoord.y);

	vec4 SceneColor     = texture(PostTextures, vec3(SampleCoord, 1.0));
	vec4 SceneBlurColor = texture(PostTextures, vec3(SampleCoord, 4.0));

	SceneColor += SceneBlurColor;

	FragColor = SceneColor;//(SceneColor * 1.42 * PostBloomSource + SceneBlurColor * 2.4 * PostBloomBlur) * 0.5;
}