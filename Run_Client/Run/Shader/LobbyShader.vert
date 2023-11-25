#version 330 core

in vec3 vPos; //--- 응용 프로그램에서 받아온 도형 좌표값
in vec2 vTexCoord;	// 텍스처 UV 좌표

out vec2 TexCoord;	// Fragment Shader로 넘기는 좌표

uniform mat4 transformMat;

void main()
{
	gl_Position = transformMat * vec4(vPos, 1.0);
	TexCoord = vTexCoord;
}
