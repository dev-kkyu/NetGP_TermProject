#include "Lobby.h"
#include "Image.h"
#include <iostream>

#define T_BACKGROUND	0
#define T_MAIN			1
#define T_END			2

#define T_CONN_WAIT		0
#define T_CONN_SUCC		1
#define T_READY_SUCC	2
#define T_READY_OFF		3
#define T_READY_ON		4

CLobby::CLobby() : is_ready{}
{
	GLuint shader = CreateShaderProgram("./Shader/LobbyShader.vert", "./Shader/LobbyShader.frag");
	m_shader = shader;

	GLuint vao = InitBuffer();
	m_vao = vao;

	is_lobby = true;
	my_id = -1;
}

CLobby::~CLobby()
{
}

void CLobby::Render()
{
	glUseProgram(m_shader);
	glBindVertexArray(m_vao);

	GLint transLoc = glGetUniformLocation(m_shader, "transformMat");
	if (transLoc < 0)
		printf("lobby trans 찾지못함\n");
	if (is_lobby) {
		glUniformMatrix4fv(transLoc, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.f)));
		glBindTexture(GL_TEXTURE_2D, m_tex_bground[T_MAIN]);
		glDrawArrays(GL_QUADS, 0, 4);
		glClear(GL_DEPTH_BUFFER_BIT);
		for (int i = 0; i < 3; ++i) {
			auto mat = glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.375f - (i * 0.475), 0.f)) * glm::scale(glm::mat4(1.f), glm::vec3(0.30f, 0.12f, 1.f));
			glUniformMatrix4fv(transLoc, 1, GL_FALSE, glm::value_ptr(mat));
			if (is_accept[i]) {
				if (is_ready[i])
					glBindTexture(GL_TEXTURE_2D, m_tex_button[T_READY_SUCC]);
				else
					glBindTexture(GL_TEXTURE_2D, m_tex_button[T_CONN_SUCC]);
			}
			else {
				glBindTexture(GL_TEXTURE_2D, m_tex_button[T_CONN_WAIT]);
			}
			glDrawArrays(GL_QUADS, 0, 4);
		}
	}
	else {
		glUniformMatrix4fv(transLoc, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.f)));
		glBindTexture(GL_TEXTURE_2D, m_tex_bground[T_BACKGROUND]);
		glDrawArrays(GL_QUADS, 0, 4);
		glClear(GL_DEPTH_BUFFER_BIT);
	}
}

GLuint CLobby::InitBuffer()
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

	// 텍스쳐 로드 - 배경
	glGenTextures(3, m_tex_bground);
	for (int i = 0; i < 3; ++i) {
		glBindTexture(GL_TEXTURE_2D, m_tex_bground[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		int ImageWidth, ImageHeight, numberOfChannel;
		GLubyte* data = nullptr;
		if (T_BACKGROUND == i)
			data = CImage::LoadImg("./Resources/BackGround.png", &ImageWidth, &ImageHeight, &numberOfChannel, 0);
		else if (T_MAIN == i)
			data = CImage::LoadImg("./Resources/Lobby/RUN.png", &ImageWidth, &ImageHeight, &numberOfChannel, 0);
		else if (T_END == i)
			data = CImage::LoadImg("./Resources/Lobby/END.png", &ImageWidth, &ImageHeight, &numberOfChannel, 0);
		if (!data)
			std::cerr << i << ": image load Error" << std::endl;
		int texLevel = numberOfChannel == 4 ? GL_RGBA : GL_RGB;
		glTexImage2D(GL_TEXTURE_2D, 0, numberOfChannel, ImageWidth, ImageHeight, 0, texLevel, GL_UNSIGNED_BYTE, data);
		CImage::FreeImg(data);
	}
	// 텍스쳐 로드 - 버튼
	std::string button_file[5]{ "Connect_Wait", "Connect_Success", "Ready_Success", "Ready_OFF", "Ready_ON" };
	glGenTextures(5, m_tex_button);
	for (int i = 0; i < 5; ++i) {
		glBindTexture(GL_TEXTURE_2D, m_tex_button[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		int ImageWidth, ImageHeight, numberOfChannel;
		GLubyte* data = CImage::LoadImg("./Resources/Lobby/" + button_file[i] + ".png", &ImageWidth, &ImageHeight, &numberOfChannel, 0);
		if (!data)
			std::cerr << i << ": image load Error" << std::endl;
		int texLevel = numberOfChannel == 4 ? GL_RGBA : GL_RGB;
		glTexImage2D(GL_TEXTURE_2D, 0, numberOfChannel, ImageWidth, ImageHeight, 0, texLevel, GL_UNSIGNED_BYTE, data);
		CImage::FreeImg(data);
	}

	return VAO;
}
