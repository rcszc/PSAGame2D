// psag_actor_module_script. RCSZ.
// framework system preset shader_script.
#include "psag_actor_module.hpp"

using namespace std;
using namespace PSAG_LOGGER;

namespace GameActorScript {
	const char* PsagShaderPublicVert = R"(
#version 460 core
layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec4 VertexColor;
layout (location = 2) in vec2 VertexTexture;
layout (location = 3) in vec3 VertexNormal;

uniform vec2  ActorPos;
uniform float ActorRotate;
uniform vec2  ActorSize;
uniform float ActorZ;
uniform vec4  ActorColor;

out vec4 FxColor;
out vec2 FxCoord;

layout(std140, binding = 0) uniform PsagUniformActorMatrix {
    mat4 Matrix;
};

void main()
{
	// rotate & scale matrix2x2.
    mat2 Rotation2DMatrix = mat2(cos(ActorRotate), -sin(ActorRotate), sin(ActorRotate),  cos(ActorRotate));
	mat2 Scale2DMatrix    = mat2(ActorSize.x, 0.0, 0.0, ActorSize.y);

	// scale => rotate => move.
	vec2 VerPos = vec2(VertexPosition.xy * Scale2DMatrix * Rotation2DMatrix + ActorPos);
	gl_Position = Matrix * vec4(vec3(VerPos.x, VerPos.y, ActorZ), 1.0);

    // non: vertex_color, v_color = uniform. [20240825]
	FxColor = ActorColor;
	FxCoord = VertexTexture;
}
)";
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