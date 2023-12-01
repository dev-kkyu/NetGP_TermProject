#pragma once
#include "stdafx.h"

class CLobby
{
private:
	GLuint m_shader;
	GLuint m_vao;
	GLuint m_tex_bground[3];	// 0: 게임 뒷배경, 1: 로비 뒷배경, 2: 게임종료 뒷배경
	GLuint m_tex_button[6];
	GLuint m_tex_numbers[11];	// 10: 콜론

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

