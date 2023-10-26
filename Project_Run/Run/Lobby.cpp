#include "Lobby.h"
#include "Image.h"
#include <iostream>

CLobby::CLobby()
{
	GLuint shader = CreateShaderProgram("./Shader/LobbyShader.vert", "./Shader/LobbyShader.frag");
	m_shader = shader;

	GLuint vao = InitBuffer();
	m_vao = vao;
}

CLobby::~CLobby()
{
}

void CLobby::Render()
{
	glUseProgram(m_shader);
	glBindVertexArray(m_vao);
	glBindTexture(GL_TEXTURE_2D, m_tex);
	glDrawArrays(GL_QUADS, 0, 4);
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

	// 텍스쳐 로드
	glGenTextures(1, &m_tex);
	glBindTexture(GL_TEXTURE_2D, m_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	int ImageWidth, ImageHeight, numberOfChannel;
	//stbi_set_flip_vertically_on_load(true); //--- 이미지가 거꾸로 읽힌다면 추가
	GLubyte* data = CImage::LoadImg("./Resources/Lobby.png", &ImageWidth, &ImageHeight, &numberOfChannel, 0);
	if (!data)
		std::cerr << "image load Error" << std::endl;
	int texLevel = numberOfChannel == 4 ? GL_RGBA : GL_RGB;
	glTexImage2D(GL_TEXTURE_2D, 0, numberOfChannel, ImageWidth, ImageHeight, 0, texLevel, GL_UNSIGNED_BYTE, data);
	CImage::FreeImg(data);

	return VAO;
}
