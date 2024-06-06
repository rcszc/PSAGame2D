uniform sampler2DArray SequeVirTex;
uniform int   SequeVirTexLayer;
uniform vec2  SequeVirTexCropping;
uniform vec2  SequeVirTexSize;

uniform vec2  RenderUvSize;
uniform vec2  RenderUvOffset;
uniform vec4  RenderColorBlend;
uniform float RenderTime;

void main()
{
	vec2 SampleCoord = FxCoord * RenderUvSize + RenderUvOffset;
	vec4 Color = texture(SequeVirTex, vec3(SampleCoord, float(SequeVirTexLayer)));

	Color.a = (Color.r + Color.g + Color.b) / 3.0;

	FragColor = Color * RenderColorBlend;
}