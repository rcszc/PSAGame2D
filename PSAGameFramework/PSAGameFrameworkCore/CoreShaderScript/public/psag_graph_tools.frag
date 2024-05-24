// pomelostar_game2d graphics_engine.
// public fragment std-tools-func.

#define SHADER_FRAG_MS_PI 3.1415926536

// midpoint value: 0.0
float Gaussian(float Gx, float Sigma)
{
    return exp(-(Gx * Gx) / (2.0 * Sigma * Sigma)) / (sqrt(2.0 * SHADER_FRAG_MS_PI) * Sigma);
}

// average_filter color, RGB.
vec4 FilterColorAvg(vec4 InColor, float Value)
{
    float ColorAverage = (InColor.x + InColor.y + InColor.z) / 3.0;
    if (ColorAverage > Value) {
        return InColor;
    } 
    else {
        return vec4(0.0, 0.0, 0.0, 0.0);
    }
}