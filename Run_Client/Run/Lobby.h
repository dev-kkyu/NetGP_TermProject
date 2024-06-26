#pragma once
#include "stdafx.h"

class CLobby
{
private:
	GLuint m_shader;
	GLuint m_vao;
	GLuint m_tex_bground[3];	// 0: ���� �޹��, 1: �κ� �޹��, 2: �������� �޹��
	GLuint m_tex_button[6];
	GLuint m_tex_numbers[11];	// 10: �ݷ�

public:
	char my_id;
	bool is_lobby;
	bool is_end;
	bool is_accept[3];
	bool is_ready[3];
	float end_time[3];

public:
	CLobby();
	~CLobby();

	void Render();
	void SetGameEnd(float* end_time);

	GLuint InitBuffer();
};

