#pragma once

#ifdef NDEBUG	// Release	일때
#	pragma comment (lib, "libglew32.lib")
#else			// Debug	일때
#   pragma comment (lib, "libglew32d.lib")
#endif

#define GLEW_STATIC
#include "glew.h"

#define FREEGLUT_STATIC
#include "freeglut.h"

#include "glm/ext.hpp"

#include <string>

#include "../../Run_Server/Run_Server/protocol.h"

std::string ReadFile(std::string fileName);
GLuint CreateShaderProgram(std::string vertexFile, std::string fragmentFile);
GLuint CompileShader(std::string fileName, GLenum shaderType);
