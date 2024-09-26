uniform sampler2DArray ProcessTextures;
uniform vec3  FragFilterColor; // [一级]RGB分量过滤.
uniform float FragFilterAvg;   // [二级]RGB均值过滤.

void main() 
{
	vec4 Lv1_FILTER = FilterColor(texture(ProcessTextures, vec3(FxCoord, 1.0)), FragFilterColor);
	vec4 Lv2_FILTER = FilterColorAvg(Lv1_FILTER, FragFilterAvg);

	FragColor = Lv2_FILTER;
}