#include "OpenGL/glew.h"
#include "OpenGL/freeglut.h"
#include "OpenGL/glm/ext.hpp"
#include <string>

#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "freeglut.lib")

#include <iostream>
#include <mutex>
#include <thread>
#include "NetModule.h"

// 콜백함수
GLvoid Display(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid KeyboardUp(unsigned char key, int x, int y);
GLvoid SpecialKeyboard(int key, int x, int y);
GLvoid SpecialKeyboardUp(int key, int x, int y);
GLvoid Mouse(int button, int state, int x, int y);
GLvoid Motion(int x, int y);
GLvoid PassiveMotion(int x, int y);
GLvoid IdleScene(GLvoid);
GLvoid TimerFunction(int value);


// --- 전역 변수
int winWidth = 800;
int winHeight = 800;

std::thread g_t;
std::mutex g_m;
std::unique_ptr<CNetModule> g_NetModule;

void main(int argc, char** argv)								//--- 윈도우 출력하고 콜백함수 설정 
{
	printf("소켓을 연결합니다. ");
	system("pause");
	g_NetModule = std::make_unique<CNetModule>();
	g_t = std::thread{ CNetModule::RecvThread, g_NetModule->sock, std::ref(g_m), std::ref(g_NetModule) };

	//--- 윈도우 생성하기
	glutInit(&argc, argv);										// glut 초기화
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);	// 디스플레이 모드 설정
	glutInitWindowPosition(100, 100);							// 윈도우의 위치 지정
	glutInitWindowSize(winWidth, winHeight);								// 윈도우의 크기 지정
	glutCreateWindow("Dummy_Client");									// 윈도우 생성(윈도우 이름)

	//--- GLEW 초기화하기
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {								// glew 초기화
		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	else {
		std::cout << "GLEW Initialized\n";
	}

	if (glewIsSupported("GL_VERSION_3_0")) {
		std::cout << "GLEW Version is 3.0\n";
	}
	else {
		std::cout << "GLEW 3.0 not supported\n";
	}

	//	콜백함수 설정
	glutDisplayFunc(Display);									// 출력 함수의 지정
	glutReshapeFunc(Reshape);									// 다시 그리기 함수 지정
	glutKeyboardFunc(Keyboard);									// 키보드 입력(눌렀을 때) 콜백함수 지정
	glutKeyboardUpFunc(KeyboardUp);								// 키보드 입력(뗐을 때) 콜백함수 지정
	glutSpecialFunc(SpecialKeyboard);							// F1~F9 등등
	glutSpecialUpFunc(SpecialKeyboardUp);
	glutMouseFunc(Mouse);										// 클릭했을 때
	glutMotionFunc(Motion);										// 누르고 이동할 때
	glutPassiveMotionFunc(PassiveMotion);						// 누르지 않고 이동할 때
	glutIdleFunc(IdleScene);									// 아무 이벤트가 없을 때
	//glutTimerFunc(0, TimerFunction, 0);						// 특정 시간마다 할 일 설정


	glEnable(GL_DEPTH_TEST);	// 깊이검사 활성화
	glEnable(GL_CULL_FACE);		// 컬링	(뒷면 제거)
	//glFrontFace(GL_CCW);		// 컬링의 앞면 설정 (GL_CW - 시계, GL_CCW - 반시계)
	//glCullFace(GL_BACK);		// 어떤 면을 제거할지 설정
	//glPolygonMode(GL_FRONT_AND_BACK, GL_POLYGON);	// 폴리곤을 어떤 모드로 그릴것인지(GL_POINT, GL_LINE, GL_POLYGON)

	// 아래는 안쓰는 옵션
	////glEnable(GL_DITHER);		// 표면을 매끄럽게
	////glEnable(GL_LINE_SMOOTH);	// 안티 앨리어싱
	////glEnable(GL_POLYGON_SMOOTH);// 안티 앨리어싱
	////glShadeModel(GL_SMOOTH);	// 부드러운 음영을 수행합니다.

	glEnable(GL_BLEND);			// 블렌딩 기능을 활성화한다.	// Alpha 활성화
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	//지수를 원본 컬러 벡터 Csource의 알파값으로 설정

	glutMainLoop();												// 이벤트 처리 시작
	glutLeaveMainLoop();										// 프로그램 종료
	g_t.join();
}

GLvoid Display(GLvoid)
{
	glClearColor(0.f, 0.f, 0.f, 1.0f);						// 바탕색 지정
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);			// 설정된 색으로 전체를 칠하기

	// 그리기 부분 구현: 그리기 관련 부분이 여기에 포함된다.

	glutSwapBuffers();											// 화면에 출력하기
}

GLvoid Reshape(int w, int h)
{
	winWidth = w;
	winHeight = h;
	glViewport(0, 0, w, h);
}

GLvoid Keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 27:			// ESC Key
		glutLeaveMainLoop();
		break;
	case ' ':
		break;
	}
}

GLvoid KeyboardUp(unsigned char key, int x, int y)
{
}

GLvoid SpecialKeyboard(int key, int x, int y)
{
	switch (key) {
	case GLUT_KEY_LEFT:
		break;
	case GLUT_KEY_RIGHT:
		break;
	}
}

GLvoid SpecialKeyboardUp(int key, int x, int y)
{
	switch (key) {
	case GLUT_KEY_F9:			// F9 누르면 전체화면
		glutFullScreenToggle();
		break;
	}
}

GLvoid Mouse(int button, int state, int x, int y)
{
	switch (state) {
	case GLUT_DOWN:
		switch (button) {
		case GLUT_LEFT_BUTTON:
			if (g_NetModule)
				g_NetModule->send_cs_ready_packet();
			break;
		}
		break;
	}
}

GLvoid Motion(int x, int y)
{
	return GLvoid();
}

GLvoid PassiveMotion(int x, int y)
{
	return GLvoid();
}

GLvoid IdleScene(GLvoid)
{
	glutPostRedisplay();						// glutDisplayFunc 콜백 호출
}

GLvoid TimerFunction(int value)
{
	glutPostRedisplay();						// 화면 재 출력
	glutTimerFunc(0, TimerFunction, 0);			// 타이머함수 재 설정
}
