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