#version 460 core

in vec4 FxColor;
in vec2 FxCoord;

uniform sampler2DArray ParticleVirTex;
uniform int   ParticleVirTexLayer;
uniform vec2  ParticleVirTexCropping;
uniform vec2  ParticleVirTexSize;

uniform vec2  RenderUvSize;
uniform vec2  RenderUvOffset;
uniform vec4  RenderColorBlend;
uniform float RenderTime;

out vec4 FragColor;

void main()
{
	vec2 SampleCoord = FxCoord * RenderUvSize + RenderUvOffset;
	vec4 Color = texture(ParticleVirTex, vec3(SampleCoord, float(ParticleVirTexLayer)));

	Color.a = (Color.r + Color.g + Color.b) / 3.0;

    FragColor = Color * RenderColorBlend;
}