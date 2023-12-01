#version 330 core

out vec4 FragColor;

in float zVal;
in float alpha_val;

uniform float Index;		// 0~29 값이 넘어옴. 색상 변화를 위해 넘겨줌(깊이)

void main()
{
	vec4 newColor = vec4(0.f, 0.f, 0.f, alpha_val);

	float idx = Index - zVal;
	if (idx < 15) {
		newColor.g = (19.f - idx + 1.f) / 20.f;
		newColor.b = (19.f - idx + 1.f) / 20.f;
	}
	else {
		newColor.g = ((19.f - 15.f + 1.f) / 20.f) * (30.f - idx) / 15.f;
		newColor.b = ((19.f - 15.f + 1.f) / 20.f) * (30.f - idx) / 15.f;
	}
//	if(outIdx == 0.f){
//		newColor.r = 1.f;
//		newColor.gb = vec2(0.f);
//	}
//	else if(outIdx > 0.99f && outIdx < 1.01f){
//		newColor.rb = vec2(0.f);
//	}
	if (alpha_val > 1.f)
		newColor = vec4(1.f, 0.f, 0.f, 1.f);
	FragColor = newColor;
}
