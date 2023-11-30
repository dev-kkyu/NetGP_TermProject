#pragma once
#include "stdafx.h"

class CLobby
{
private:
	GLuint m_shader;
	GLuint m_vao;
	GLuint m_tex_bground[3];
	GLuint m_tex_button[6];

public:
	char my_id;
	bool is_lobby;
	bool is_accept[3];
	bool is_ready[3];

public:
	CLobby();
	~CLobby();

	void Render();

	GLuint InitBuffer();
};

