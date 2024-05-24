// psag_actor_module_script. RCSZ.
// framework system preset shader_script.
#include "psag_actor_module.hpp"

using namespace std;
using namespace PSAG_LOGGER;

namespace ActorShaderScript {
	const char* PsagShaderScriptPublicVS = R"(
#version 460 core
layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec4 VertexColor;
layout (location = 2) in vec2 VertexTexture;
layout (location = 3) in vec3 VertexNormal;

uniform mat4 MvpMatrix;

uniform vec2  ActorMove;
uniform float ActorRotate;
uniform vec2  ActorScale;

out vec4 FxColor;
out vec2 FxCoord;

void main()
{
    mat2 Rotation2DMatrix = mat2(
        cos(ActorRotate), -sin(ActorRotate),
        sin(ActorRotate),  cos(ActorRotate)
    );

	mat2 Scale2DMatrix = mat2(
		ActorScale.x, 0.0,
		0.0, ActorScale.y
	);

	// scale => rotate => move.
	vec2 VerPos = vec2(VertexPosition.xy * Scale2DMatrix * Rotation2DMatrix + ActorMove);
	gl_Position = MvpMatrix * vec4(vec3(VerPos.x, VerPos.y, VertexPosition.z), 1.0);
	FxColor     = VertexColor;
	FxCoord     = VertexTexture;
}
)";
}