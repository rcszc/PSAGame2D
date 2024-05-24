// psag_graphics_engine_script. RCSZ.
// framework system preset shader_script.
#include "psag_graphics_engine.h"

using namespace std;
using namespace PSAG_LOGGER;

namespace GraphicsShaderScript {
	const char* PsagShaderScriptPublicVS = R"(
#version 460 core
layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec4 VertexColor;
layout (location = 2) in vec2 VertexTexture;
layout (location = 3) in vec3 VertexNormal;

uniform mat4 MvpMatrix;

uniform vec2 RenderMove;
uniform vec2 RenderScale;

out vec4 FxColor; 
out vec2 FxCoord;

void main()
{
	vec2 VerPos = vec2(VertexPosition.xy * RenderScale + RenderMove);
	gl_Position = MvpMatrix * vec4(vec3(VerPos.x, VerPos.y, VertexPosition.z), 1.0);
	FxColor     = VertexColor;
	FxCoord     = VertexTexture;
}
)";

	const char* PsagShaderScriptBloomHFS = R"(
#version 460 core

in vec4 FxColor;
in vec2 FxCoord;

uniform sampler2DArray PostTextures;
uniform int            PostBloomRadius; // gaussian blur radius.

out vec4 FragColor;

float Gaussian(float Gx, float Sigma) {
    return exp(-(Gx * Gx) / (2.0 * Sigma * Sigma)) / (sqrt(2.0 * 3.1415926535) * Sigma);
}

// horizontal blur process.
vec4 BloomEffectH(sampler2DArray TexSample, vec2 TexCoords, float Layer) {
    vec2 TexOffset = 1.0 / textureSize(TexSample, 0).xy;
    vec3 Result = texture(TexSample, vec3(TexCoords, Layer)).rgb * Gaussian(0.0, PostBloomRadius);
    
    for(int i = 1; i < PostBloomRadius; ++i) {
        float BlurColor = Gaussian(float(i), PostBloomRadius);
		
		Result += texture(TexSample, vec3(TexCoords + vec2(TexOffset.x * i, 0.0), Layer)).rgb * BlurColor;
		Result += texture(TexSample, vec3(TexCoords - vec2(TexOffset.x * i, 0.0), Layer)).rgb * BlurColor;
    }
    return vec4(Result, 1.0);
}

void main()
{
	FragColor = BloomEffectH(PostTextures, FxCoord, 0.0);
}
)";
	const char* PsagShaderScriptBloomVFS = R"(
#version 460 core

in vec4 FxColor;
in vec2 FxCoord;

uniform sampler2DArray PostTextures;
uniform int            PostBloomRadius; // 高斯模糊半径.

out vec4 FragColor;

float Gaussian(float Gx, float Sigma) {
    return exp(-(Gx * Gx) / (2.0 * Sigma * Sigma)) / (sqrt(2.0 * 3.1415926535) * Sigma);
}

// vertical blur process.
vec4 BloomEffectV(sampler2DArray TexSample, vec2 TexCoords, float Layer) {
    vec2 TexOffset = 1.0 / textureSize(TexSample, 0).xy;
    vec3 Result = texture(TexSample, vec3(TexCoords, Layer)).rgb * Gaussian(0.0, PostBloomRadius);
    
    for(int i = 1; i < PostBloomRadius; ++i) {
        float BlurColor = Gaussian(float(i), PostBloomRadius);
		
		Result += texture(TexSample, vec3(TexCoords + vec2(0.0, TexOffset.y * i), Layer)).rgb * BlurColor;
		Result += texture(TexSample, vec3(TexCoords - vec2(0.0, TexOffset.y * i), Layer)).rgb * BlurColor;
    }
    return vec4(Result, 1.0);
}

void main() 
{
	FragColor = BloomEffectV(PostTextures, FxCoord, 1.0);
}
)";

	const char* PsagShaderScriptPostFS = R"(
#version 460 core

in vec4 FxColor;
in vec2 FxCoord;

uniform sampler2DArray PostTextures;
uniform vec2 RenderRes;

uniform int   PostBloomRadius; // 高斯模糊半径.
uniform float PostBloomBlur;   // 模糊颜色混合权重.
uniform float PostBloomSource; // 源颜色混合权重.
uniform float PostBloomFilter; // 辉光颜色过滤(RGBA均值).

out vec4 FragColor;

vec4 ColorFilter(vec4 InColor, float Value)
{
    float ColorSum = InColor.x + InColor.y + InColor.z;
    float ColorAverage = ColorSum / 3.0;

    if (ColorAverage > Value) {
        return InColor;
    } 
    else {
        return vec4(0.0, 0.0, 0.0, 0.0);
    }
}

void main() 
{
	vec2 SampleCoord = vec2(FxCoord.x, 1.0 - FxCoord.y);

	vec4 SceneColor     = texture(PostTextures, vec3(SampleCoord, 0.0));
	vec4 SceneBlurColor = texture(PostTextures, vec3(SampleCoord, 2.0));

	SceneColor = SceneColor + SceneBlurColor;

	FragColor = (SceneColor * 1.42 + SceneBlurColor * 2.4) * 0.5;
}
)";

	const char* PsagShaderScriptBackFS = R"(
#version 460 core

in vec4 FxColor;
in vec2 FxCoord;

uniform sampler2DArray MultipleBackTex;
uniform vec2  RenderRes;
uniform float RenderTime;

uniform float TextureIndex;
uniform float TextureBackIndex;

uniform float BackVisibility;
uniform vec2  BackStrength;
uniform vec4  BackBlendColor;

out vec4 FragColor;

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

	FragColor = (TexColorFront * TexColorFront.w + BlendColor * (1.0 - TexColorFront.w)) * BackBlendColor;
}
)";

	const char* PsagShaderScriptParticleFS = R"(
#version 460 core

in vec4 FxColor;
in vec2 FxCoord;

uniform sampler2DArray ParticleVirTex;
uniform int   ParticleVirTexLayer;
uniform vec2  ParticleVirTexCropping;
uniform vec2  ParticleVirTexSize;

uniform float RenderTime;

out vec4 FragColor;

const vec2 Pivot = vec2(0.5, 0.5);

float Random(float seed) {
    return fract(sin(seed) * 43758.5453123);
}

void main()
{
	float RandomValue = Random(FxColor.r + FxColor.g + FxColor.b + FxColor.a);
	float Angle = RenderTime * 0.2 + RandomValue * 5.0;
    mat2 Rotation = mat2(cos(Angle), -sin(Angle), sin(Angle), cos(Angle));

    vec2 RotatedTexCoord = Rotation * (FxCoord - Pivot) + Pivot;

	vec2 uv = RotatedTexCoord;
	
	uv.x += sin(uv.y * 10.0 + RenderTime + RandomValue * 10) / 25.0;
	uv.y += cos(uv.x * 10.0 + RenderTime + RandomValue * 10) / 25.0;
	
	vec4 Color = texture(ParticleVirTex, vec3(uv, float(ParticleVirTexLayer)));

    FragColor = Color * FxColor;
}
)";

	const char* PsagShaderScriptFxSequenceFS = R"(
#version 460 core

in vec4 FxColor;
in vec2 FxCoord;

uniform sampler2DArray ParticleVirTex;
uniform int   ParticleVirTexLayer;
uniform vec2  ParticleVirTexCropping;
uniform vec2  ParticleVirTexSize;

uniform vec2  RenderSizeUV;
uniform vec2  RenderOffsetUV;
uniform vec4  RenderColorBlend;
uniform float RenderTime;

out vec4 FragColor;

void main()
{
	vec2 SampleCoord = FxCoord * RenderSizeUV + RenderOffsetUV;
	vec4 Color = texture(ParticleVirTex, vec3(SampleCoord, float(ParticleVirTexLayer)));
	Color.a = (Color.r + Color.g + Color.b) / 3.0;
    FragColor = Color * RenderColorBlend;
}
)";
}