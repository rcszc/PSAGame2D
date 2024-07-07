// pomelostar_game2d graphics_engine.
// public vertex std-vert-shader.

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
	// vertex position trans.
	vec2 Vertex2D = vec2(VertexPosition.xy * RenderScale + RenderMove);
	gl_Position   = MvpMatrix * vec4(vec3(Vertex2D.x, Vertex2D.y, VertexPosition.z), 1.0);

	// vertex out fragment shader.
	FxColor = VertexColor;
	FxCoord = VertexTexture;
}