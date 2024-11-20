// demo_origin_game_script.
#include "demo_origin_game.h"

using namespace std;
using namespace PSAG_LOGGER;

SHADER_CODE ActorPawnShaderFrag = R"(
uniform float BarWidth;
uniform float BarPosition;
uniform float Figure;

const vec4 TargetColor = vec4(0.32, 0.0, 1.0, 1.0);

void main()
{
    float DistToCenter = abs(FxCoord.x - BarPosition);
    float BarIntensity = 1.0 - smoothstep(0.0, BarWidth / 2.0, DistToCenter);

    vec4 FragNOR = SampleTextureNOR(FxCoord);
    if (length(TargetColor.rgb - FragNOR.rbg) > 1.0 / Figure) {
        FragNOR *= 2.0;
    }
    vec4 FragHDR = SampleTextureHDR(FxCoord);
    if (FragHDR.a > 0.005) {
        FragColor = FragNOR + FragHDR * BarIntensity * 1.42;
        return;
    }
    FragColor = FragNOR;
}
)";

SHADER_CODE ActorPawnFxShaderFrag = R"(
uniform float BarWidth;
uniform float BarPosition;

void main()
{
    float DistToCenter = abs(FxCoord.x - BarPosition);
    float BarIntensity = 1.0 - smoothstep(0.0, BarWidth / 2.0, DistToCenter);

    vec4 FragNOR = SampleTextureNOR(FxCoord);
    FragNOR.rgb += 0.1;
    FragColor = FragNOR * BarIntensity * 2.5;
}
)";

SHADER_CODE ActorNpcShaderFrag = R"(
void main()
{
    vec4 FragNOR = SampleTextureNOR(FxCoord);
    vec4 FragHDR = SampleTextureHDR(FxCoord);
    if (FragHDR.a > 0.005) {
        FragColor = FragNOR * 0.7 + abs(sin(RenderTime * 0.05)) * FragHDR * 2.5;
        return;
    }
    FragColor = FragNOR * 0.7;
}
)";