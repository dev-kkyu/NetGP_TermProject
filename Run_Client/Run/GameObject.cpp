#include "GameObject.h"

CGameObject::CGameObject()
{
	Initialize();
}

CGameObject::~CGameObject()
{
	Release();
}

void CGameObject::Initialize()
{
	isInitialized = false;
}

void CGameObject::Update(float ElapsedTime)
{
}

void CGameObject::FixedUpdate()
{
}

void CGameObject::Render()
{
}

void CGameObject::Release()
{
}

void CGameObject::SetShader(GLuint shader)
{
	m_shader = shader;
}

void CGameObject::SetVao(GLuint vao)
{
	m_vao = vao;
	isInitialized = true;
}
