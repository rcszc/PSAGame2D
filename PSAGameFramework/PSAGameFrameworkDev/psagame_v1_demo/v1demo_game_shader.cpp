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

const float GlowIntensity = 0.02;
const float Radius = 0.5;
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
    vec4 BlendColor = vec4(1.0, 0.12, 0.0, abs(sin(RenderTime * 0.042)) + 0.2);
    FragColor = TexColor * BlendColor;
}
)";

const char* ActorFragFX1 = R"(
#version 460 core

in vec4 FxColor;
in vec2 FxCoord;

uniform sampler2DArray VirTexture;
uniform int            VirTextureLayer;
uniform vec2           VirTextureCropping;
uniform vec2           VirTextureSize;

uniform vec2  RenderResolution;
uniform float RenderTime;

uniform float Brightness;

out vec4 FragColor;

float Height(in vec2 uv) 
{
    vec3 SampleColor = texture(VirTexture, vec3(uv, float(VirTextureLayer))).rgb;
    return SampleColor.r;
}

const vec2 NE = vec2(0.05, 0.0);
vec3 normal(in vec2 uv)
{
    return normalize(vec3(
        Height(uv + NE.xy) - Height(uv - NE.xy),
        0.0,
        Height(uv + NE.yx) - Height(uv - NE.yx)
    ));
}

const vec4 WaterColor = vec4(0.32, 1.0, 0.0, 1.0);
vec3 lightDir = normalize(vec3(10.0, 15.0, 5.0));

void main() {
    vec2 uv = FxCoord.xy - vec2(0.5);

    float dist = length(uv);
    float angle = atan(uv.y, uv.x);
    
    vec2 ruv = uv;
    uv = vec2(cos(angle + dist * 3.0), dist + RenderTime * 0.012);

    float h = Height(uv);
    vec3 norm = normal(uv);
    
    FragColor = 
        mix(vec4(0.0), 
             mix(mix(WaterColor + WaterColor * max(0.0, dot(lightDir, norm)) * 0.2,
                     vec4(Height(uv), Height(uv), Height(uv), 1.0), 0.2),
                 vec4(Height(norm.xz * 0.5 + 0.5), Height(norm.xz * 0.5 + 0.5), Height(norm.xz * 0.5 + 0.5), 1.0), 0.3),
             min(1.0, (1.0 - length(ruv)) * 10.0)) * vec4(1.0, Brightness * 0.2, Brightness * 1.72, 1.0);

    FragColor.rgb = FragColor.rgb * FragColor.rgb;

    float ColorDist = length(FxCoord.xy - vec2(0.5));
    FragColor.a = 1.0 - smoothstep(0.08, 0.52, ColorDist);
}
)";

// 代码来自: https://www.shadertoy.com/view/4scGWj (改)
const char* ActorFragFX2 = R"(
#version 460 core

in vec4 FxColor;
in vec2 FxCoord;

uniform vec2  RenderResolution;
uniform float RenderTime;

out vec4 FragColor;

vec3 Random3(vec3 c) {
    float j = 4096.0*sin(dot(c, vec3(17.0, 59.4, 15.0)));
    vec3 r;
    r.z = fract(512.0*j);
    j *= .125;
    r.x = fract(512.0*j);
    j *= .125;
    r.y = fract(512.0*j);
    return r - 0.5;
}

const float F3 = 0.3333333;
const float G3 = 0.1666667;

float simplex3d(vec3 p) {
    /* 1. find current tetrahedron T and its four vertices */
    /* s, s+i1, s+i2, s+1.0 - absolute skewed (integer) coordinates of T vertices */
    /* x, x1, x2, x3 - unskewed coordinates of p relative to each of T vertices */

    /* calculate s and x */
    vec3 s = floor(p + dot(p, vec3(F3)));
    vec3 x = p - s + dot(s, vec3(G3));

    /* calculate i1 and i2 */
    vec3 e = step(vec3(0.0), x - x.yzx);
    vec3 i1 = e * (1.0 - e.zxy);
    vec3 i2 = 1.0 - e.zxy * (1.0 - e);

    /* x1, x2, x3 */
    vec3 x1 = x - i1 + G3;
    vec3 x2 = x - i2 + 2.0 * G3;
    vec3 x3 = x - 1.0 + 3.0 * G3;

    /* 2. find four surflets and store them in d */
    vec4 w, d;

    /* calculate surflet weights */
    w.x = dot(x, x);
    w.y = dot(x1, x1);
    w.z = dot(x2, x2);
    w.w = dot(x3, x3);

    /* w fades from 0.6 at the center of the surflet to 0.0 at the margin */
    w = max(0.6 - w, 0.0);

    /* calculate surflet components */
    d.x = dot(Random3(s), x);
    d.y = dot(Random3(s + i1), x1);
    d.z = dot(Random3(s + i2), x2);
    d.w = dot(Random3(s + 1.0), x3);

    /* multiply d by w^4 */
    w *= w;
    w *= w;
    d *= w;

    /* 3. return the sum of the four surflets */
    return dot(d, vec4(52.0));
}

float noise(vec3 m) {
    return 0.5333333 * simplex3d(m)
           + 0.2666667 * simplex3d(2.0 * m)
           + 0.1333333 * simplex3d(4.0 * m)
           + 0.0666667 * simplex3d(8.0 * m);
}

void main() 
{
    vec2 uv = FxCoord.xy;
    uv = uv * 2.0 - 1.0;
    
    vec2 p = FxCoord.xy;
    vec3 p3 = vec3(p, RenderTime * 0.158);

    // Adjust noise parameters to create circular patterns
    float radius = length(uv); // Calculate the distance from the center
    float intensity = noise(vec3(p3 * 1.72 + 12.0));

    // Adjust intensity based on circular radius
    float t = clamp((radius - 0.15) * 5.0, 0.0, 1.0);
    float y = abs(intensity * -t + radius);

    float g = pow(y, 0.072);

    // Apply circular pattern to color
    vec3 col = vec3(1.82, 1.58, 1.92);
    col = col * -g + col;
    col = col * col;
    col = col * col;

    // Set color and alpha
    FragColor.rgb = col * 70.0;
    FragColor.a = 1.0 - g; // Use g for alpha to make the effect more transparent or opaque
}
)";

// 代码来自: https://www.shadertoy.com/view/XtBXW3 (改)
const char* ActorFragFX3 = R"(
#version 460 core

in vec4 FxColor;
in vec2 FxCoord;

uniform sampler2DArray VirTexture;
uniform int            VirTextureLayer;
uniform vec2           VirTextureCropping;
uniform vec2           VirTextureSize;

uniform vec2  RenderResolution;
uniform float RenderTime;

uniform int FireFlag;

out vec4 FragColor;

void main()
{
    float RandomValue = texture(VirTexture, vec3(FxCoord, float(VirTextureLayer))).b;
	vec2 uv = FxCoord;
	uv.y += cos(uv.x * 72.0 - RenderTime * 5.0 + RandomValue * 10) * 0.025;
	
	vec4 Color = texture(VirTexture, vec3(uv, float(VirTextureLayer)));

    FragColor = vec4(0.0f, 0.0, 0.0, 0.0);
    if (FireFlag == 1)
    {
        FragColor = Color;
    }
}
)";