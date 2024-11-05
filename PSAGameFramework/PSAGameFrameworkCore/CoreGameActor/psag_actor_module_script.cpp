// psag_actor_module_script. RCSZ.
// framework system preset shader_script.
#include "psag_actor_module.hpp"

using namespace std;
using namespace PSAG_LOGGER;

namespace GameActorScript {
	const char* PsagShaderPublicFrag_Header = R"(
#version 460 core

in vec4 FxColor;
in vec2 FxCoord;

uniform vec2  RenderResolution;
uniform float RenderTime;

out vec4 FragColor;
)";

	const char* PsagShaderPublicFrag_Tools = R"(
uniform sampler2DArray VirTexture;
uniform sampler2DArray VirHDRTexture;

uniform int VirTextureLayer;
uniform int VirHDRTextureLayer;

uniform vec2 VirTextureCropping;
uniform vec2 VirHDRTextureCropping;

uniform vec2 VirTextureSize;
uniform vec2 VirHDRTextureSize;

vec4 SampleTexture(vec2 SmpCoord) {
	vec2 SamplerCoord = VirTextureCropping * SmpCoord;
	return texture(VirTexture, vec3(SamplerCoord, float(VirTextureLayer)));
}

vec4 SampleTextureHDR(vec2 SmpCoord) {
	vec2 SamplerCoord = VirHDRTextureCropping * SmpCoord;
	return texture(VirHDRTexture, vec3(SamplerCoord, float(VirHDRTextureLayer)));
}
)";

	const char* PsagShaderPrivateFrag_Brick = R"(
#version 460 core

in vec4 FxColor;
in vec2 FxCoord;

uniform vec2  RenderResolution;
uniform float RenderTime;

out vec4 FragColor;

uniform sampler2DArray VirTexture;
uniform sampler2DArray VirHDRTexture;

uniform int VirTextureLayer;
uniform int VirHDRTextureLayer;

uniform vec2 VirTextureCropping;
uniform vec2 VirHDRTextureCropping;

uniform vec2 VirTextureSize;
uniform vec2 VirHDRTextureSize;

vec4 SampleTexture(vec2 SmpCoord) {
	vec2 SamplerCoord = VirTextureCropping * SmpCoord;
	return texture(VirTexture, vec3(SamplerCoord, float(VirTextureLayer)));
}

vec4 SampleTextureHDR(vec2 SmpCoord) {
	vec2 SamplerCoord = VirHDRTextureCropping * SmpCoord;
	return texture(VirHDRTexture, vec3(SamplerCoord, float(VirHDRTextureLayer)));
}

void main()
{
	vec4 HDRCOLOR = SampleTextureHDR(FxCoord);
	vec4 BLEND = SampleTexture(FxCoord);
	if (HDRCOLOR.a > 0.005) {
		BLEND += HDRCOLOR;
	}
    FragColor = BLEND * FxColor;
}
)";
}