// v1demo_game_shader.
#include "v1demo_game.h"

using namespace std;
using namespace PSAG_LOGGER;

const char* ShaderFragPawnActor = R"(
#version 460 core

in vec4 FxColor;
in vec2 FxCoord;

uniform vec2  RenderResolution;
uniform float RenderTime;

out vec4 FragColor;

vec4 DrawSquare(vec2 uv, vec2 Center, float SideLength, vec4 Color) {
    vec2 SizeHalf = vec2(SideLength * 0.5);
    vec2 MinBound = Center - SizeHalf;
    vec2 MaxBound = Center + SizeHalf;

    if (uv.x >= MinBound.x && uv.x <= MaxBound.x && uv.y >= MinBound.y && uv.y <= MaxBound.y) 
    {
        return Color;
    }
    return vec4(0.0);
}

bool IsInsideDiamond(vec2 uv) 
{
    vec2 p = uv - vec2(0.5);
    return abs(p.x) + abs(p.y) <= 0.5;
}

void main()
{
    float Gradient = (FxCoord.x + FxCoord.y) * 0.5;

    vec3 Color1 = vec3(0.0, 1.0, 0.25);
    vec3 Color2 = vec3(0.0, 0.25, 1.0);

    vec3 GradientColor = mix(Color1, Color2, Gradient);
    vec4 OutColor = vec4(0.0);

    OutColor += DrawSquare(FxCoord, vec2(0.0, 0.0), 0.16, vec4(1.0, 1.0, 0.0, 1.0));
    OutColor += DrawSquare(FxCoord, vec2(1.0, 0.0), 0.16, vec4(1.0, 0.4, 0.0, 1.0));
    OutColor += DrawSquare(FxCoord, vec2(1.0, 1.0), 0.16, vec4(1.0, 0.4, 0.0, 1.0));
    OutColor += DrawSquare(FxCoord, vec2(0.0, 1.0), 0.16, vec4(1.0, 1.0, 0.0, 1.0));

    if (IsInsideDiamond(FxCoord)) 
    {
        OutColor = vec4(GradientColor, 1.0);
    }
    FragColor = OutColor;
}
)";

const char* ShaderFragBulletActor = R"(
#version 460 core

in vec4 FxColor;
in vec2 FxCoord;

uniform vec2  RenderResolution;
uniform float RenderTime;

out vec4 FragColor;

const float GlowIntensity = 0.01;
const float Radius = 0.58;
const vec2 CircleCenter = vec2(0.5, 0.5);

void main()
{
    float Dist = distance(FxCoord, CircleCenter);
    float Glow = smoothstep(Radius, Radius - GlowIntensity, Dist);

    FragColor = vec4(vec3(FxColor.rgb), Glow);
}
)";

const char* ShaderFragNPCActor = R"(
#version 460 core

in vec4 FxColor;
in vec2 FxCoord;

uniform sampler2DArray VirTexture;
uniform int            VirTextureLayer;
uniform vec2           VirTextureCropping;
uniform vec2           VirTextureSize;

uniform vec2  RenderResolution;
uniform float RenderTime;

out vec4 FragColor;

void main()
{
    vec4 TexColor = texture(VirTexture, vec3(FxCoord, float(VirTextureLayer)));
    vec4 BlendColor = vec4(1.0, abs(sin(RenderTime * 0.05)) * 0.32, 0.0, 0.98);
    FragColor = TexColor * BlendColor;
}
)";