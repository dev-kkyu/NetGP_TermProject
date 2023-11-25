#pragma once
#include "stdafx.h"

class CLobby
{
private:
	GLuint m_shader;
	GLuint m_vao;
	GLuint m_tex[4];

public:
	char my_id;
	bool is_lobby;
	bool is_ready[3];

public:
	CLobby();
	~CLobby();

	void Render();

	GLuint InitBuffer();
};

