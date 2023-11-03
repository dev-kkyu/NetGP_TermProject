#pragma once
#include "stdafx.h"

class CGameObject
{
protected:
	GLuint m_shader;
	GLuint m_vao;

	bool isInitialized;

public:
	CGameObject();
	virtual ~CGameObject();

	virtual void Initialize() = 0;				//생성될 때 할 일
	virtual void Update(float ElapsedTime) = 0;	//타이머에서 할 일
	virtual void FixedUpdate() = 0;					//충돌처리 등
	virtual void Render() = 0;					//드로우
	virtual void Release() = 0;					//소멸될 때 할 일

	void SetShader(GLuint shader);
	void SetVao(GLuint vao);
};

