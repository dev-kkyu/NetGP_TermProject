#pragma once
#include "stdafx.h"

class CLobby
{
private:
	GLuint m_shader;
	GLuint m_vao;
	GLuint m_tex[2];

	bool isLobby;

public:
	CLobby();
	~CLobby();

	void SetIsLobby(bool isLobby);
	void Render();

	GLuint InitBuffer();
};

