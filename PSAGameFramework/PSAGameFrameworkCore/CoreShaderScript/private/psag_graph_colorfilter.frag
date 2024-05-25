uniform sampler2DArray PostTextures;
uniform vec3  PostFilterColor; // [一级]RGB分量过滤.
uniform float PostFilterAvg;   // [二级]RGB均值过滤.

void main() 
{
	vec4 Lv1_FILTER = FilterColor(texture(PostTextures, vec3(FxCoord, 0.0)), PostFilterColor);
	vec4 Lv2_FILTER = FilterColorAvg(Lv1_FILTER, PostFilterAvg);

	FragColor = Lv2_FILTER;
}