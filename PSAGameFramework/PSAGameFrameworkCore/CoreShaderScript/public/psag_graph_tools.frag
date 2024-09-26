// pomelostar_game2d graphics_engine.
// public fragment std-tools-func.

#define SHADER_FRAG_MS_PI 3.1415926536

// midpoint value: 0.0
float Gaussian(float Gx, float Sigma)
{
    return exp(-(Gx * Gx) / (2.0 * Sigma * Sigma)) / (sqrt(2.0 * SHADER_FRAG_MS_PI) * Sigma);
}

// component filter color, RGB.
vec4 FilterColor(vec4 InColor, vec3 Value)
{
    if ((InColor.r >= Value.r) && (InColor.g >= Value.g) && (InColor.b >= Value.b)) {
        return InColor;
    }
    return vec4(0.0, 0.0, 0.0, 0.0);
}

// average filter color, RGB.
vec4 FilterColorAvg(vec4 InColor, float Value)
{
    float ColorAverage = (InColor.r + InColor.g + InColor.b) / 3.0;
    if (ColorAverage > Value) {
        return InColor;
    }
    return vec4(0.0, 0.0, 0.0, 0.0);
}

// float32 random, value limit[0.0,1.0].
float RandomFloat(float seed)
{
    return fract(sin(seed) * 1949.100112 + cos(seed) * 194508.1512);
}