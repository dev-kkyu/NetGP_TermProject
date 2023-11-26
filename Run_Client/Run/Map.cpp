#include "Map.h"
#include "Player.h"
#include "NetModule.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

const int CMap::MAX_Layer = 30;
CMap::CMap(std::string filename, int& winWidth, int& winHeight, std::shared_ptr<CNetModule> NetModule)
	: w_width{ winWidth }, w_height{ winHeight }, m_NetModule{ NetModule }
{
	std::ifstream in{ filename };
	if (!in) {
		std::cerr << filename << " ���� ����" << std::endl;
	}
	std::string str;
	while (std::getline(in, str)) {
		std::stringstream ss{ str };
		glm::mat4 mati;
		for (int i = 0; i < 4; ++i) {
			glm::vec4 line;
			ss >> line[0] >> line[1] >> line[2] >> line[3];
			mati[i] = line;
		}
		map_data.push_back(mati);
	}
	if (not map_data.empty()) {
		std::cout << filename << " �� �ε� �Ϸ�" << std::endl;
	}

	Initialize();
}

CMap::~CMap()
{
	Release();
}

void CMap::Initialize()
{
	GLuint shader = CreateShaderProgram("./Shader/MapShader.vert", "./Shader/MapShader.frag");
	SetShader(shader);

	GLuint vao = InitBuffer();
	SetVao(vao);

	for (int i = 0; i < 3; ++i)
		m_pplayers[i] = std::make_unique<CPlayer>(i);
}

void CMap::Update(float ElapsedTime)
{
	// �ʴ� 5��
	if (isInitialized) {
		glUseProgram(m_shader);

		GLint cameraLoc = glGetUniformLocation(m_shader, "cameraMat");
		if (cameraLoc < 0) {
			std::cerr << "cameraLoc ã�� ����" << std::endl;
		}
		GLint projLoc = glGetUniformLocation(m_shader, "projMat");
		if (projLoc < 0) {
			std::cerr << "projLoc ã�� ����" << std::endl;
		}

		// ������ ī�޶� ã��...
		m_NetModule->m_mutex.lock();
		int id = m_NetModule->my_id;
		glm::vec3 eye(m_NetModule->m_player[id].info.x, -0.9f + m_NetModule->m_player[id].info.y, 0.9f);
		m_NetModule->m_mutex.unlock();
		glm::mat4 camera = glm::lookAt(glm::vec3(eye.x, eye.y, eye.z), glm::vec3(eye.x, eye.y, -50.f), glm::vec3(0.f, 1.f, 0.f));
		glUniformMatrix4fv(cameraLoc, 1, GL_FALSE, glm::value_ptr(camera));

		glm::mat4 projection = glm::perspective(glm::radians(90.f), (float)w_width / (float)w_height, 0.1f, 100.f);
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		for (int i = 0; i < 3; ++i) {
			if (m_pplayers[i]) {
				m_pplayers[i]->SetCamera(camera);
				m_pplayers[i]->SetProjection(projection);
				m_NetModule->m_mutex.lock();
				m_pplayers[i]->SetMoveX(m_NetModule->m_player[i].info.x);
				m_pplayers[i]->SetMoveY(m_NetModule->m_player[i].info.y);
				if (i != id) {
					float move_z = m_NetModule->m_player[id].info.map_index + m_NetModule->m_player[id].info.z - m_NetModule->m_player[i].info.map_index - m_NetModule->m_player[i].info.z;
					m_pplayers[i]->SetMoveZ(move_z);	// �÷��̾� ����, Ÿ �÷��̾� �Ÿ� ���
					float angle = -m_NetModule->m_player[i].info.now_angle + m_NetModule->m_player[id].info.now_angle;
					angle += -180.f + ((m_NetModule->m_player[i].info.bottom_index + (2 - m_NetModule->m_player[id].info.bottom_index)) % 4) * 90.f;
					m_pplayers[i]->SetRotate(angle);	// �÷��̾� ���� ���� ���
				}
				m_pplayers[i]->SetWalk(m_NetModule->m_player[i].info.is_walk);
				m_NetModule->m_mutex.unlock();
				m_pplayers[i]->Update(ElapsedTime);
			}
		}
	}
}

void CMap::FixedUpdate()
{
}

void CMap::Render()
{
	if (isInitialized) {
		glUseProgram(m_shader);
		glBindVertexArray(m_vao);


		GLint modelLoc = glGetUniformLocation(m_shader, "modelMat");
		if (modelLoc < 0) {
			std::cerr << "modelLoc ã�� ����" << std::endl;
		}
		GLint idxLoc = glGetUniformLocation(m_shader, "Index");
		if (idxLoc < 0) {
			std::cerr << "idxLoc ã�� ����" << std::endl;
		}
		GLint zLoc = glGetUniformLocation(m_shader, "move_z");
		if (zLoc < 0) {
			std::cerr << "zLoc ã�� ����" << std::endl;
		}
		GLint alphaLoc = glGetUniformLocation(m_shader, "alpha_mat");
		if (alphaLoc < 0) {
			std::cerr << "alphaLoc ã�� ����" << std::endl;
		}

		m_NetModule->m_mutex.lock();
		int id = m_NetModule->my_id;
		glUniform1f(zLoc, m_NetModule->m_player[id].info.z);

		float angle = 180.f - (m_NetModule->m_player[id].info.bottom_index * 90.f);
		glm::mat4 rotateMat = glm::rotate(glm::mat4(1.f), glm::radians(angle + m_NetModule->m_player[id].info.now_angle), glm::vec3(0.f, 0.f, 1.f));
		int base_idx = m_NetModule->m_player[id].info.map_index;
		m_NetModule->m_mutex.unlock();
		for (int i = 0; i < MAX_Layer; ++i) {
			glm::mat4 alpha(0.f);
			int index = base_idx + i;
			if (index >= 0 and index < 100) {
				alpha = map_data[index];
			}
			else {
				for (int a = 0; a < 4; ++a) {
					for (int b = 0; b < 4; ++b) {
						alpha[a][b] = 1.f;
					}
				}
			}
			glUniformMatrix4fv(alphaLoc, 1, GL_FALSE, glm::value_ptr(alpha));

			glm::mat4 model = glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, -1.f * i)) * rotateMat;
			glUniform1f(idxLoc, (float)i);
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			glDrawArrays(GL_QUADS, 0, 64);
		}

		int idxs[3]{ 0, 1, 2 };
		m_NetModule->m_mutex.lock();
		// ������ ���� �׸��Ƿ� �����ؼ� �׸��� (�������� �� �÷��̾��̹Ƿ� ��������)
		std::sort(idxs, idxs + 3, [&](const int& a, const int& b) {return m_NetModule->m_player[a].info.map_index + m_NetModule->m_player[a].info.z
					> m_NetModule->m_player[b].info.map_index + m_NetModule->m_player[b].info.z; });
		m_NetModule->m_mutex.unlock();
		glDisable(GL_DEPTH_TEST);
		for (int i = 0; i < 3; ++i) {
			if (m_pplayers[idxs[i]])
				m_pplayers[idxs[i]]->Render();
		}
		glEnable(GL_DEPTH_TEST);
	}
}

void CMap::Release()
{
}

GLuint CMap::InitBuffer()
{
	glUseProgram(m_shader);
	GLuint VAO, VBO;					// ���� �����͸� GPU�� �Ѱ��� VAO, VBO ����
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);	// VBO�� �������۷� ���� �� ���ε�

	// y -2.f ~ 2.f
	// x -2.f ~ 2.f
	// z -0.5f, 0.5f
	float zSize = 0.5f;
	float vertexRects[]{	// �� �� �� ��	// �ݽð�
		// ���� �װ� �簢��
		1.f, 2.f, zSize,
		1.f, 2.f, -zSize,
		2.f, 2.f, -zSize,
		2.f, 2.f, zSize,

		0.f, 2.f, zSize,
		0.f, 2.f, -zSize,
		1.f, 2.f, -zSize,
		1.f, 2.f, zSize,

		-1.f, 2.f, zSize,
		-1.f, 2.f, -zSize,
		0.f, 2.f, -zSize,
		0.f, 2.f, zSize,

		-2.f, 2.f, zSize,
		-2.f, 2.f, -zSize,
		-1.f, 2.f, -zSize,
		-1.f, 2.f, zSize,

		// ���� �װ� �簢��
		-2.f, 2.f, zSize,
		-2.f, 1.f, zSize,
		-2.f, 1.f, -zSize,
		-2.f, 2.f, -zSize,

		-2.f, 1.f, zSize,
		-2.f, 0.f, zSize,
		-2.f, 0.f, -zSize,
		-2.f, 1.f, -zSize,

		-2.f, 0.f, zSize,
		-2.f, -1.f, zSize,
		-2.f, -1.f, -zSize,
		-2.f, 0.f, -zSize,

		-2.f, -1.f, zSize,
		-2.f, -2.f, zSize,
		-2.f, -2.f, -zSize,
		-2.f, -1.f, -zSize,

		// �Ʒ��� �װ� �簢��
		-2.f, -2.f, -zSize,
		-2.f, -2.f, zSize,
		-1.f, -2.f, zSize,
		-1.f, -2.f, -zSize,

		-1.f, -2.f, -zSize,
		-1.f, -2.f, zSize,
		0.f, -2.f, zSize,
		0.f, -2.f, -zSize,

		0.f, -2.f, -zSize,
		0.f, -2.f, zSize,
		1.f, -2.f, zSize,
		1.f, -2.f, -zSize,

		1.f, -2.f, -zSize,
		1.f, -2.f, zSize,
		2.f, -2.f, zSize,
		2.f, -2.f, -zSize,

		// ������ �װ� �簢��
		2.f, -1.f, -zSize,
		2.f, -2.f, -zSize,
		2.f, -2.f, zSize,
		2.f, -1.f, zSize,

		2.f, 0.f, -zSize,
		2.f, -1.f, -zSize,
		2.f, -1.f, zSize,
		2.f, 0.f, zSize,

		2.f, 1.f, -zSize,
		2.f, 0.f, -zSize,
		2.f, 0.f, zSize,
		2.f, 1.f, zSize,

		2.f, 2.f, -zSize,
		2.f, 1.f, -zSize,
		2.f, 1.f, zSize,
		2.f, 2.f, zSize

	};

	glBufferData(GL_ARRAY_BUFFER, 192 * sizeof(float), vertexRects, GL_STATIC_DRAW);	// VBO(GPU)�� ���� ������ ����

	GLint AttribPosLoc = glGetAttribLocation(m_shader, "vPos");						// ���̴����� vPos�� ��ġ �����´�.
	if (AttribPosLoc < 0) {
		std::cerr << "AttribLoc ã�� ����" << std::endl;
	}
	glVertexAttribPointer(AttribPosLoc, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), reinterpret_cast<void*>(0));	// ���� VBO�� �ִ� ������ �Ӽ� ����
	// glVertexAttribPointer(attrib ��ġ, vertex ��� ��������, gl_float, gl_false, stride(����), ���� ��ġ(������));
	glEnableVertexAttribArray(AttribPosLoc);										// Attribute Ȱ��ȭ

	return VAO;
}

void CMap::KeyboardEvent(int state, unsigned char key)
{
	switch (state) {
	case GLUT_DOWN:
		switch (key) {
		case ' ':
			m_NetModule->send_cs_key_event_packet(MY_KEY_EVENT::KEY_SPACE, true);
			break;
		}
		break;
	case GLUT_UP:
		switch (key)
		{
		case ' ':
			m_NetModule->send_cs_key_event_packet(MY_KEY_EVENT::KEY_SPACE, false);
			break;
		}
		break;
	}
}

void CMap::SpecialKeyEvent(int state, int key)
{
	switch (state) {
	case GLUT_DOWN:
		switch (key) {
		case GLUT_KEY_LEFT:
			m_NetModule->send_cs_key_event_packet(MY_KEY_EVENT::KEY_LEFT, true);
			break;
		case GLUT_KEY_RIGHT:
			m_NetModule->send_cs_key_event_packet(MY_KEY_EVENT::KEY_RIGHT, true);
			break;
		}
		break;
	case GLUT_UP:
		switch (key) {
		case GLUT_KEY_LEFT:
			m_NetModule->send_cs_key_event_packet(MY_KEY_EVENT::KEY_LEFT, false);
			break;
		case GLUT_KEY_RIGHT:
			m_NetModule->send_cs_key_event_packet(MY_KEY_EVENT::KEY_RIGHT, false);
			break;
		}
		break;
	}
}
