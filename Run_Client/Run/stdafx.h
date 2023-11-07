#pragma once

#include "OpenGL/glew.h"
#include "OpenGL/freeglut.h"
#include "OpenGL/glm/ext.hpp"
#include <string>

#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "freeglut.lib")

#include "../../Run_Server/Run_Server/protocol.h"

std::string ReadFile(std::string fileName);
GLuint CreateShaderProgram(std::string vertexFile, std::string fragmentFile);
GLuint CompileShader(std::string fileName, GLenum shaderType);
