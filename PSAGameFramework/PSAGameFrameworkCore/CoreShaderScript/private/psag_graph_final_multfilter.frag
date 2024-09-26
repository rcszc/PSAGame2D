uniform sampler2DArray ProcessTextures;
uniform vec3  FragFilterColor; // [һ��]RGB��������.
uniform float FragFilterAvg;   // [����]RGB��ֵ����.

void main() 
{
	vec4 Lv1_FILTER = FilterColor(texture(ProcessTextures, vec3(FxCoord, 1.0)), FragFilterColor);
	vec4 Lv2_FILTER = FilterColorAvg(Lv1_FILTER, FragFilterAvg);

	FragColor = Lv2_FILTER;
}