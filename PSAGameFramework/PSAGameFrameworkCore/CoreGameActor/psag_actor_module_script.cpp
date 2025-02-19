// psag_actor_module_script. RCSZ.
// framework system preset shader_script.
#include "psag_actor_module.hpp"

using namespace std;
using namespace PSAG_LOGGER;

namespace GameActorScript {
	const char* psag_shader_public_frag_header = R"(
#version 460 core

in vec4 FxColor;
in vec2 FxCoord;

uniform vec2  RenderResolution;
uniform float RenderTime;

out vec4 FragColor;
)";

	const char* psag_shader_public_frag_texture = R"(
uniform sampler2DArray VirTexture%s;

uniform int  VirTexture%sLayer;
uniform vec2 VirTexture%sCropping;
uniform vec2 VirTexture%sSize;

vec4 SampleTexture%s(vec2 smp_coord) {
	vec2 SamplerCoord = VirTexture%sCropping * smp_coord;
	return texture(VirTexture%s, vec3(SamplerCoord, float(VirTexture%sLayer)));
}
)";

	const char* psag_shader_private_frag_dfirst = R"(
void main()
{
    FragColor = SampleTexture)";
	const char* psag_shader_private_frag_dlast = R"((FxCoord) * FxColor;
}
)";
}