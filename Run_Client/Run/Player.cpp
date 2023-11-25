#include "Player.h"
#include "Image.h"
#include <iostream>

CPlayer::CPlayer(int id) : my_id{ id }
{
	Initialize();
}

CPlayer::~CPlayer()
{
	Release();
}

void CPlayer::Initialize()
{
	GLuint shader = CreateShaderProgram("./Shader/PlayerShader.vert", "./Shader/PlayerShader.frag");
	SetShader(shader);

	GLuint vao = InitBuffer();
	SetVao(vao);

	time_sec = 0.f;

	TexFrame = 0;

	isWalk = true;

	move_x = 0.f;
	move_y = 0.f;
}

void CPlayer::Update(float ElapsedTime)
{
	if (isInitialized) {
		if (isWalk)
			time_sec += 12.5f * ElapsedTime;		// 초당 텍스쳐 몇장 지나갈건지
		if (time_sec >= 6.f)
			time_sec -= 6.f;
		TexFrame = int(time_sec);
	}
}

void CPlayer::FixedUpdate()
{
}

void CPlayer::Render()
{
	if (isInitialized) {
		glUseProgram(m_shader);

		GLint cameraLoc = glGetUniformLocation(m_shader, "cameraMat");
		if (cameraLoc < 0) {
			std::cerr << "cameraLoc 찾지 못함" << std::endl;
		}
		GLint projLoc = glGetUniformLocation(m_shader, "projMat");
		if (projLoc < 0) {
			std::cerr << "projLoc 찾지 못함" << std::endl;
		}
		GLint modelLoc = glGetUniformLocation(m_shader, "modelMat");
		if (modelLoc < 0) {
			std::cerr << "modelLoc 찾지 못함" << std::endl;
		}

		glUniformMatrix4fv(cameraLoc, 1, GL_FALSE, glm::value_ptr(cameraMat));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projMat));

		glm::mat4 scale = glm::scale(glm::mat4(1.f), glm::vec3(0.3f, 0.3f, 1.f));
		modelMat = glm::translate(glm::mat4(1.f), glm::vec3(move_x, -1.7f + move_y, -1.f)) * scale;
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMat));

		glBindVertexArray(m_vao);
		if (TexFrame > 3)
			glBindTexture(GL_TEXTURE_2D, Textures[6 - TexFrame]);
		else
			glBindTexture(GL_TEXTURE_2D, Textures[TexFrame]);
		glDrawArrays(GL_QUADS, 0, 4);
	}
}

void CPlayer::Release()
{
}

void CPlayer::SetCamera(glm::mat4 cameraMat)
{
	this->cameraMat = cameraMat;
}

void CPlayer::SetProjection(glm::mat4 projMat)
{
	this->projMat = projMat;
}

GLuint CPlayer::InitBuffer()
{
	glUseProgram(m_shader);
	GLuint VAO, VBO, textureVBO;					// 정점 데이터를 GPU에 넘겨줄 VAO, VBO 생성
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);	// VBO를 정점버퍼로 설정 및 바인딩

	float vertex[]{
		-1.f, 1.f, 0.f,
		-1.f, -1.f, 0.f,
		1.f, -1.f, 0.f,
		1.f, 1.f, 0.f
	};

	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), vertex, GL_STATIC_DRAW);	// VBO(GPU)로 정점 데이터 복사

	GLint AttribPosLoc = glGetAttribLocation(m_shader, "vPos");						// 셰이더에서 vPos의 위치 가져온다.
	if (AttribPosLoc < 0) {
		std::cerr << "AttribLoc 찾기 실패" << std::endl;
	}
	glVertexAttribPointer(AttribPosLoc, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), reinterpret_cast<void*>(0));	// 현재 VBO에 있는 데이터 속성 정의
	// glVertexAttribPointer(attrib 위치, vertex 몇개씩 읽을건지, gl_float, gl_false, stride(간격), 시작 위치(포인터));
	glEnableVertexAttribArray(AttribPosLoc);										// Attribute 활성화

	glGenBuffers(1, &textureVBO);
	glBindBuffer(GL_ARRAY_BUFFER, textureVBO);
	float texCoord[] = {
		0.f, 0.f,
		0.f, 1.f,
		1.f, 1.f,
		1.f, 0.f
	};
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), texCoord, GL_STATIC_DRAW);
	GLint TexLoc = glGetAttribLocation(m_shader, "vTexCoord");						// 셰이더에서 vPos의 위치 가져온다.
	if (TexLoc < 0) {
		std::cerr << "TexLoc 찾기 실패" << std::endl;
	}
	glVertexAttribPointer(TexLoc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), reinterpret_cast<void*>(0)); //--- 텍스처 좌표 속성
	glEnableVertexAttribArray(TexLoc);

	// 텍스쳐 로드
	std::string folder_name[3]{ "Blue", "Yellow", "Pink" };
	glGenTextures(4, Textures);
	for (int i = 0; i < 4; ++i) {
		glBindTexture(GL_TEXTURE_2D, Textures[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		int ImageWidth, ImageHeight, numberOfChannel;
		//stbi_set_flip_vertically_on_load(true); //--- 이미지가 거꾸로 읽힌다면 추가
		std::string name = "./Resources/Character/" + folder_name[my_id] + "/" + std::to_string(i + 1) + ".png";
		GLubyte* data = CImage::LoadImg(name, &ImageWidth, &ImageHeight, &numberOfChannel, 0);
		if (!data)
			std::cerr << "image load Error" << std::endl;
		int texLevel = numberOfChannel == 4 ? GL_RGBA : GL_RGB;
		glTexImage2D(GL_TEXTURE_2D, 0, numberOfChannel, ImageWidth, ImageHeight, 0, texLevel, GL_UNSIGNED_BYTE, data);
		CImage::FreeImg(data);
	}

	return VAO;
}

void CPlayer::SetWalk(bool isWalk)
{
	this->isWalk = isWalk;
}

void CPlayer::SetMoveX(float move_x)
{
	this->move_x = move_x;
}

void CPlayer::SetMoveY(float move_y)
{
	this->move_y = move_y;
}
