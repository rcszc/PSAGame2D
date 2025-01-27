uniform sampler2DArray SequeVirTex;
uniform int   SequeVirTexLayer;
uniform vec2  SequeVirTexCropping;
uniform vec2  SequeVirTexSize;

uniform float RenderTime;
uniform vec4  RenderColorBlend;
uniform vec2  RenderUvSize;
uniform vec2  RenderUvOffset;

void main()
{
	vec2 SampleCoord = FxCoord * RenderUvSize + RenderUvOffset;
	vec3 Color = texture(SequeVirTex, vec3(SampleCoord, float(SequeVirTexLayer))).rgb;

	vec4 FragOut = vec4(vec3(Color), (Color.r + Color.g + Color.b) / 3.0);
	FragColor = FragOut * RenderColorBlend;
}