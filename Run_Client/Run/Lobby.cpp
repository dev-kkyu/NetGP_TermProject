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
#define T_BORDER		5

CLobby::CLobby() : is_ready{}
{
	GLuint shader = CreateShaderProgram("./Shader/LobbyShader.vert", "./Shader/LobbyShader.frag");
	m_shader = shader;

	GLuint vao = InitBuffer();
	m_vao = vao;

	is_lobby = true;
	is_end = false;
	end_time[0] = end_time[1] = end_time[2] = 0.f;

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
		glDrawArrays(GL_QUADS, 0, 4);		// 로비 뒷배경
		glClear(GL_DEPTH_BUFFER_BIT);
		for (int i = 0; i < 3; ++i) {		// 플레이어 전원의 상태 나타내기
			auto mat = glm::translate(glm::mat4(1.f), glm::vec3(0.05f, 0.375f - (i * 0.475), 0.f)) * glm::scale(glm::mat4(1.f), glm::vec3(0.30f, 0.12f, 1.f));
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
		// 테두리
		auto mat = glm::translate(glm::mat4(1.f), glm::vec3(0.05f, 0.375f - (my_id * 0.475), 0.f)) * glm::scale(glm::mat4(1.f), glm::vec3(1.2f, 0.24f, 1.f));
		glUniformMatrix4fv(transLoc, 1, GL_FALSE, glm::value_ptr(mat));
		glBindTexture(GL_TEXTURE_2D, m_tex_button[T_BORDER]);
		glDrawArrays(GL_QUADS, 0, 4);

		// 버튼
		mat = glm::translate(glm::mat4(1.f), glm::vec3(0.75f, -0.9f, 0.f)) * glm::scale(glm::mat4(1.f), glm::vec3(0.25f, 0.10f, 1.f));
		glUniformMatrix4fv(transLoc, 1, GL_FALSE, glm::value_ptr(mat));
		if (is_ready[my_id])
			glBindTexture(GL_TEXTURE_2D, m_tex_button[T_READY_ON]);
		else
			glBindTexture(GL_TEXTURE_2D, m_tex_button[T_READY_OFF]);
		glDrawArrays(GL_QUADS, 0, 4);
	}
	else if (is_end) {
		glUniformMatrix4fv(transLoc, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.f)));
		glBindTexture(GL_TEXTURE_2D, m_tex_bground[T_END]);
		glDrawArrays(GL_QUADS, 0, 4);		// 게임종료 뒷배경
		glClear(GL_DEPTH_BUFFER_BIT);
		for (int i = 0; i < 3; ++i) {
			// 00:00:00 총 8개 위치에 해당하는 값 정해주기
			int minute = int(end_time[i]) / 60;
			int second = int(end_time[i]) % 60;
			int decimal = int(end_time[i] * 100) % 100;
			int t_val[8]{ minute / 10, minute % 10, 10, second / 10, second % 10, 10, decimal / 10, decimal % 10 };

			for (int j = 0; j < 8; ++j) {
				auto mat = glm::translate(glm::mat4(1.f), glm::vec3(-0.25f + j * 0.1f, 0.375f - (i * 0.475), 0.f))
					* glm::scale(glm::mat4(1.f), glm::vec3(0.05f, 0.1f, 1.f));
				glUniformMatrix4fv(transLoc, 1, GL_FALSE, glm::value_ptr(mat));
				glBindTexture(GL_TEXTURE_2D, m_tex_numbers[t_val[j]]);
				glDrawArrays(GL_QUADS, 0, 4);
			}
		}
		//테두리
		auto mat = glm::translate(glm::mat4(1.f), glm::vec3(0.05f, 0.375f - (my_id * 0.475), 0.f)) * glm::scale(glm::mat4(1.f), glm::vec3(1.2f, 0.24f, 1.f));
		glUniformMatrix4fv(transLoc, 1, GL_FALSE, glm::value_ptr(mat));
		glBindTexture(GL_TEXTURE_2D, m_tex_button[T_BORDER]);
		glDrawArrays(GL_QUADS, 0, 4);
	}
	else {
		glUniformMatrix4fv(transLoc, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.f)));
		glBindTexture(GL_TEXTURE_2D, m_tex_bground[T_BACKGROUND]);
		glDrawArrays(GL_QUADS, 0, 4);	// 인게임 뒷배경
		glClear(GL_DEPTH_BUFFER_BIT);
	}
}

void CLobby::SetGameEnd(float* end_time)
{
	is_end = true;
	memcpy(this->end_time, end_time, sizeof(float) * 3);

	for (int i = 0; i < 3; ++i) {
		std::cout << "P" << i + 1 << ": " << this->end_time[i] << std::endl;
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
	std::string button_file[6]{ "Connect_Wait", "Connect_Success", "Ready_Success", "Ready_OFF", "Ready_ON", "Border"};
	glGenTextures(6, m_tex_button);
	for (int i = 0; i < 6; ++i) {
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
	// 텍스쳐 로드 - 숫자
	glGenTextures(11, m_tex_numbers);
	for (int i = 0; i < 11; ++i) {
		glBindTexture(GL_TEXTURE_2D, m_tex_numbers[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		int ImageWidth, ImageHeight, numberOfChannel;
		GLubyte* data = CImage::LoadImg("./Resources/Numbers/" + std::to_string(i) + ".png", &ImageWidth, &ImageHeight, &numberOfChannel, 0);
		if (!data)
			std::cerr << i << ": image load Error" << std::endl;
		int texLevel = numberOfChannel == 4 ? GL_RGBA : GL_RGB;
		glTexImage2D(GL_TEXTURE_2D, 0, numberOfChannel, ImageWidth, ImageHeight, 0, texLevel, GL_UNSIGNED_BYTE, data);
		CImage::FreeImg(data);
	}

	return VAO;
}
