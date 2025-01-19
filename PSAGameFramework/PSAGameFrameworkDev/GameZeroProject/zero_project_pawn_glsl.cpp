// zero_project_pawn_glsl.
#include "zero_project_pawn.h"

using namespace std;

ShaderCode PawnActorNormal = R"(
#define WAVE_FREQUENCY 48.0
#define WAVE_AMPLITUDE 0.005

void main()
{
    float Wave = sin(WAVE_FREQUENCY * FxCoord.y + RenderTime) * WAVE_AMPLITUDE;
    vec2 DistTexCoord = vec2(FxCoord.x + Wave, FxCoord.y);

    FragColor = SampleTextureNOR(DistTexCoord);
}
)";