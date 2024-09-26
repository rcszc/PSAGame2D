// psag_actor_module_script. RCSZ.
// framework system preset shader_script.
#include "psag_actor_module.hpp"

using namespace std;
using namespace PSAG_LOGGER;

namespace GameActorScript {
	const char* PsagShaderPrivateFrag_Brick = R"(
#version 460 core

in vec4 FxColor;
in vec2 FxCoord;

uniform vec2  RenderResolution;
uniform float RenderTime;

uniform sampler2DArray VirTexture;
uniform int            VirTextureLayer;
uniform vec2           VirTextureCropping;
uniform vec2           VirTextureSize;

out vec4 FragColor;

void main()
{
	vec2 SamplerCoord = VirTextureCropping * FxCoord;
    FragColor = texture(VirTexture, vec3(SamplerCoord, float(VirTextureLayer))) * FxColor;
}
)";
}