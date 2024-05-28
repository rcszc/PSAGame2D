uniform sampler2DArray MultipleBackTex;
uniform vec2  RenderRes;

uniform float TextureBackIndex;

uniform float BackVisibility;
uniform vec2  BackStrength;
uniform vec4  BackBlendColor;

void main()
{
	vec4 TexColorFront = texture(MultipleBackTex, vec3(FxCoord, 0.0));

	TexColorFront.a    = step(0.16, length(TexColorFront.rgb));
	TexColorFront.rgb *= BackStrength.y;
	
	vec4 PreviousColor = texture(MultipleBackTex, vec3(FxCoord, TextureBackIndex));
	PreviousColor.rgb *= 0.58 * BackStrength.x;

	vec4 BlendColor = vec4(0.0);
	for (float i = TextureBackIndex - 1.0; i >= 1.0; i -= 1.0) 
	{
		vec4 TexColorMid = texture(MultipleBackTex, vec3(FxCoord, i));

		TexColorMid.rgb *= (TextureBackIndex - i) * 0.25 * BackVisibility + 0.12 * BackVisibility;
		TexColorMid.a = step(0.16, length(TexColorMid.rgb));

		BlendColor = TexColorMid * TexColorMid.w + PreviousColor * (1.0 - TexColorMid.w);
		PreviousColor = BlendColor;
	}
	// multi_layered background blend => out.
	FragColor = (TexColorFront * TexColorFront.w + BlendColor * (1.0 - TexColorFront.w)) * BackBlendColor;
}