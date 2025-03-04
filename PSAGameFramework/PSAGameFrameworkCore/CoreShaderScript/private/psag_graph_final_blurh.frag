uniform sampler2DArray ProcessTextures;
uniform int FragBlurRadius; // gaussian blur radius.

// horizontal blur process.
vec4 BloomEffectH(sampler2DArray TexSample, vec2 TexCoords, float Layer) 
{
	vec2 TexOffset = 1.0 / textureSize(TexSample, 0).xy;
	vec3 Result = texture(TexSample, vec3(TexCoords, Layer)).rgb * Gaussian(0.0, FragBlurRadius);
	
	for(int i = 1; i < FragBlurRadius; ++i) {
		float BlurColor = Gaussian(float(i), FragBlurRadius);
		
		Result += texture(TexSample, vec3(TexCoords + vec2(TexOffset.x * i, 0.0), Layer)).rgb * BlurColor;
		Result += texture(TexSample, vec3(TexCoords - vec2(TexOffset.x * i, 0.0), Layer)).rgb * BlurColor;
    }
	return vec4(Result, 1.0);
}

void main()
{
	FragColor = BloomEffectH(ProcessTextures, FxCoord, 2.0);
}