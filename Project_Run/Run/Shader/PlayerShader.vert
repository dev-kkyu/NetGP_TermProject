#version 330 core

in vec3 vPos; //--- 응용 프로그램에서 받아온 도형 좌표값
in vec2 vTexCoord;	// 텍스처 UV 좌표

uniform mat4 modelMat;
uniform mat4 cameraMat;
uniform mat4 projMat;

out vec2 TexCoord;	// Fragment Shader로 넘기는 좌표

void main()
{
	gl_Position = projMat * cameraMat * modelMat * vec4(vPos, 1.0);
	gl_Position.y += 0.2f;		// 자체적으로 카메라 낮춰주기
	TexCoord = vTexCoord;
}
