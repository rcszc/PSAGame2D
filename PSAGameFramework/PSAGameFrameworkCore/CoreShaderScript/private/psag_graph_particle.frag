uniform sampler2DArray ParticleVirTex;
uniform int   ParticleVirTexLayer;
uniform vec2  ParticleVirTexCropping;
uniform vec2  ParticleVirTexSize;

uniform float RenderTime;
uniform float RenderRotate;
uniform float RenderTwist;

const vec2 Pivot = vec2(0.5, 0.5);

float ComputeAlpha(float Life, float DecayPoint) {
    float LogLife = log(Life + 1.0);
    float AdjustedLife = LogLife / (LogLife + pow(DecayPoint, 2.0));
    
    return AdjustedLife;
}

void main()
{
	float RandomValue = RandomFloat(FxColor.r + FxColor.g + FxColor.b + FxColor.a);

	// rotate_stop => "RenderRotate" = 0.0f.
	// rotate angle * time_step.
	float Angle = (RenderTime * 0.2 + RandomValue * 5.0) * RenderRotate;
	mat2 Rotation = mat2(cos(Angle), -sin(Angle), sin(Angle), cos(Angle));

	vec2 RotatedTexCoord = Rotation * (FxCoord - Pivot) + Pivot;

	vec2 uv = RotatedTexCoord;
	
	uv.x += sin(uv.y * 10.0 + RenderTime + RandomValue * 10) / 25.0 * RenderTwist;
	uv.y += cos(uv.x * 10.0 + RenderTime + RandomValue * 10) / 25.0 * RenderTwist;
	
	vec4 Color = texture(ParticleVirTex, vec3(uv, float(ParticleVirTexLayer)));
	// particle_system: FxNvec3.x = life.
	vec3 OutRGB = Color.rgb * ComputeAlpha(FxNvec3.x, 1.2) * 1.6;

	FragColor = vec4(vec3(OutRGB), Color.a) * FxColor;
}