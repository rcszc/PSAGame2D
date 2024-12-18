uniform sampler2DArray ProcessTextures;
uniform int FragBlurRadius; // gaussian blur radius.

// vertical blur process.
vec4 BloomEffectV(sampler2DArray TexSample, vec2 TexCoords, float Layer) 
{
	vec2 TexOffset = 1.0 / textureSize(TexSample, 0).xy;
	vec3 Result = texture(TexSample, vec3(TexCoords, Layer)).rgb * Gaussian(0.0, FragBlurRadius);

	for(int i = 1; i < FragBlurRadius; ++i) {
		float BlurColor = Gaussian(float(i), FragBlurRadius);
		
		Result += texture(TexSample, vec3(TexCoords + vec2(0.0, TexOffset.y * i), Layer)).rgb * BlurColor;
		Result += texture(TexSample, vec3(TexCoords - vec2(0.0, TexOffset.y * i), Layer)).rgb * BlurColor;
	}
	return vec4(Result, 1.0);
}

void main() 
{
	FragColor = BloomEffectV(ProcessTextures, FxCoord, 3.0);
}