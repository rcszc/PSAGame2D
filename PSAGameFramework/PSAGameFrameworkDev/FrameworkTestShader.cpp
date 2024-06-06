// FrameworkTestShader.
#include "FrameworkTest.h"

const char* ActorFragPawn = R"(
#version 460 core

in vec4 FxColor;
in vec2 FxCoord;

uniform vec2  RenderResolution;
uniform float RenderTime;

out vec4 FragColor;

vec4 DrawSquare(vec2 uv, vec2 Center, float SideLength, vec4 Color) {
    vec2 HalfSize = vec2(SideLength * 0.5);
    vec2 MinBound = Center - HalfSize;
    vec2 MaxBound = Center + HalfSize;

    if (uv.x >= MinBound.x && uv.x <= MaxBound.x &&
        uv.y >= MinBound.y && uv.y <= MaxBound.y) 
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

    vec3 Color1 = vec3(1.0, 0.0, 0.0);
    vec3 Color2 = vec3(0.4, 0.0, 1.0);

    vec3 GradientColor = mix(Color1, Color2, Gradient);
    vec4 OutColor = vec4(0.0);

    OutColor += DrawSquare(FxCoord, vec2(0.0, 0.0), 0.16, vec4(0.0, 1.0, 1.0, 1.0));
    OutColor += DrawSquare(FxCoord, vec2(1.0, 0.0), 0.16, vec4(0.0, 1.0, 1.0, 1.0));
    OutColor += DrawSquare(FxCoord, vec2(1.0, 1.0), 0.16, vec4(0.0, 1.0, 1.0, 1.0));
    OutColor += DrawSquare(FxCoord, vec2(0.0, 1.0), 0.16, vec4(0.0, 1.0, 1.0, 1.0));

    if (IsInsideDiamond(FxCoord)) 
    {
        OutColor = vec4(GradientColor, 1.0);
    }
    FragColor = OutColor;
}
)";

const char* ActorFragBullet = R"(
#version 460 core

in vec4 FxColor;
in vec2 FxCoord;

uniform vec2  RenderResolution;
uniform float RenderTime;

out vec4 FragColor;

void main()
{
    float Gradient = (FxCoord.x + FxCoord.y) * 0.5;

    vec3 Color1 = vec3(0.0, 1.0, 0.7);
    vec3 Color2 = vec3(0.4, 0.9, 1.0);

    FragColor = vec4(mix(Color1, Color2, Gradient), 1.0);
}
)";

const char* ActorFragNPC = R"(
#version 460 core

in vec4 FxColor;
in vec2 FxCoord;

uniform vec2  RenderResolution;
uniform float RenderTime;

out vec4 FragColor;

vec4 DrawSquare(vec2 uv, vec2 Center, float SideLength, vec4 Color) {
    vec2 HalfSize = vec2(SideLength * 0.5);
    vec2 MinBound = Center - HalfSize;
    vec2 MaxBound = Center + HalfSize;

    if (uv.x >= MinBound.x && uv.x <= MaxBound.x &&
        uv.y >= MinBound.y && uv.y <= MaxBound.y) 
    {
        return Color;
    }
    return vec4(0.0);
}

void main()
{
    float Gradient = (FxCoord.x + FxCoord.y) * 0.5;

    vec3 Color1 = vec3(1.0, 0.0, 0.0);
    vec3 Color2 = vec3(0.4, 0.0, 1.0);

    vec3 GradientColor = mix(Color1, Color2, Gradient);
    vec4 OutColor = vec4(0.0);

    OutColor += DrawSquare(FxCoord, vec2(0.5, 0.5), 1.0, vec4(0.0, 0.0, 0.0, 0.92));
    OutColor += DrawSquare(FxCoord, vec2(0.5, 0.5), 0.8, vec4(0.0, 0.0, 0.0, 0.08));
    
    if (OutColor.a == 1.0) {
        FragColor = vec4(0.0);
        return;
    }
    FragColor = vec4(GradientColor, 0.92);
}
)";

const char* ActorFragWall = R"(
#version 460 core

in vec4 FxColor;
in vec2 FxCoord;

uniform vec2  RenderResolution;
uniform float RenderTime;

out vec4 FragColor;

void main()
{
    FragColor = vec4(0.2, 0.2, 0.2, 0.72);
}
)";