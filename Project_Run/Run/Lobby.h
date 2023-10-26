#pragma once
#include "stdafx.h"

class CLobby
{
private:
	GLuint m_shader;
	GLuint m_vao;
	GLuint m_tex;

public:
	CLobby();
	~CLobby();

	void Render();

	GLuint InitBuffer();
};

