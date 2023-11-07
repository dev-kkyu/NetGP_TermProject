#version 330 core

in vec3 vPos; //--- 응용 프로그램에서 받아온 도형 좌표값

out float zVal;
out float alpha_val;

uniform mat4 modelMat;
uniform mat4 cameraMat;
uniform mat4 projMat;

uniform float move_z;
uniform mat4 alpha_mat;

int getquo(float a, float b) {
    return int(floor(float(a) / float(b)));
}

int getrem(float a, float b){
	return int(mod(int(a), int(b)));
}

void main()
{
	vec4 newPos = vec4(vPos, 1.0);
	newPos.z += move_z;
	gl_Position = projMat * cameraMat * modelMat * newPos;
	gl_Position.y += 0.2f;		// 자체적으로 카메라 낮춰주기
	zVal = move_z;				// move_z는 0~1을 반복, 색상 변화를 위해 넘겨줌(애니메이션)

	float val = gl_VertexID / 4.f;
	val = floor(val);	// 0~15
	
	alpha_val = alpha_mat[getquo(val, 4)][getrem(val, 4)];
}
