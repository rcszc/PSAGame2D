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

	const char* psag_shader_public_frag_texnor = R"(
uniform sampler2DArray VirTextureNOR;

uniform int  VirTextureNORLayer;
uniform vec2 VirTextureNORCropping;
uniform vec2 VirTextureNORSize;

vec4 SampleTextureNOR(vec2 smp_coord) {
	vec2 SamplerCoord = VirTextureNORCropping * smp_coord;
	return texture(VirTextureNOR, vec3(SamplerCoord, float(VirTextureNORLayer)));
}
)";

	const char* psag_shader_public_frag_texhdr = R"(
uniform sampler2DArray VirTextureHDR;

uniform int  VirTextureHDRLayer;
uniform vec2 VirTextureHDRCropping;
uniform vec2 VirTextureHDRSize;

vec4 SampleTextureHDR(vec2 smp_coord) {
	vec2 SamplerCoord = VirTextureHDRCropping * smp_coord;
	return texture(VirTextureHDR, vec3(SamplerCoord, float(VirTextureHDRLayer)));
}
)";

	const char* psag_shader_private_frag_brick_nor = R"(
void main()
{
    FragColor = SampleTextureNOR(FxCoord) * FxColor;
}
)";

	const char* psag_shader_private_frag_brick_hdr = R"(
void main()
{
	vec4 HDR_FRAG = SampleTextureHDR(FxCoord);
	vec4 NOR_FRAG = SampleTextureNOR(FxCoord);
	if (HDR_FRAG.a > 0.005) {
		NOR_FRAG += HDRCOLOR;
	}
    FragColor = NOR_FRAG * FxColor;
}
)";
}