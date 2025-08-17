#include "Shader.hpp"
#include "Logger.h"
#include <iostream>
#include <fstream>
#include <sstream>

void dumpErrors()
{
    std::string res = "{";
    bool singleError = false;
    bool first = false;
    while (auto err = glGetError())
    {
        singleError = true;
        std::string newField;
        switch (err)
        {
            case GL_INVALID_ENUM:
                newField = " GL_INVALID_ENUM";
                break;
            
            case GL_INVALID_VALUE:
                newField = " GL_INVALID_VALUE";
                break;

            case GL_INVALID_OPERATION:
                newField = " GL_INVALID_OPERATION";
                break;

            case GL_STACK_OVERFLOW:
                newField = " GL_STACK_OVERFLOW";
                break;

            case GL_STACK_UNDERFLOW:
                newField = " GL_STACK_UNDERFLOW";
                break;

            case GL_OUT_OF_MEMORY:
                newField = " GL_OUT_OF_MEMORY";
                break;

            case GL_INVALID_FRAMEBUFFER_OPERATION:
                newField = " GL_INVALID_FRAMEBUFFER_OPERATION";
                break;

            default:
                newField = " Unknown error";
                break;
        }
        
        if (!first)
        {
            res += ",";
        }
        else
            first = false;
            
        res += newField;
    }

    res += " }";
    
    if (singleError)
        std::cout << res << std::endl;
}

Shader::Shader()
{
}

void Shader::load(const std::string &vSourcePath_, const std::string &fSourcePath_)
{
    std::string vCode;
    std::string fCode;

    loadFile(vSourcePath_, vCode);
    loadFile(fSourcePath_, fCode);
    compile(vCode.c_str(), fCode.c_str());
}

void Shader::use()
{
    glUseProgram(m_id);
}

void Shader::compile(const char *vertexSource_, const char *fragmentSource_)
{
    unsigned int sVertex, sFragment;

    // vertex Shader
    sVertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(sVertex, 1, &vertexSource_, NULL);
    glCompileShader(sVertex);
    validateShader(sVertex);

    // fragment Shader
    try
    {
        sFragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(sFragment, 1, &fragmentSource_, NULL);
        glCompileShader(sFragment);
        validateShader(sFragment);
    }
    catch_extend("While creating fragment shader")

    // shader program
    m_id = glCreateProgram();
    glAttachShader(m_id, sVertex);
    glAttachShader(m_id, sFragment);
    glLinkProgram(m_id);
    validateProgram(m_id);

    glDeleteShader(sVertex);
    glDeleteShader(sFragment);
}

void Shader::setInteger(const char *name_, int value_)
{
    glUniform1i(claimUniformLoc(name_), value_);
}

void Shader::setVector2f(const char *name_, const glm::vec2 &value_)
{
    glUniform2f(claimUniformLoc(name_), value_.x, value_.y);
}

void Shader::setVector3f(const char *name_, const glm::vec3 &value_)
{
    glUniform3f(claimUniformLoc(name_), value_.x, value_.y, value_.z);
}

void Shader::setVector4f(const char *name_, const glm::vec4 &value_)
{
    glUniform4f(claimUniformLoc(name_), value_.x, value_.y, value_.z, value_.w);
}

void Shader::setMatrix4(const char *name_, const glm::mat4 &matrix_)
{
    glUniformMatrix4fv(claimUniformLoc(name_), 1, false, glm::value_ptr(matrix_));
}


void Shader::validateShader(unsigned int object_)
{
    int success;
    char infoLog[1024];
    glGetShaderiv(object_, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(object_, 1024, NULL, infoLog);
        throw std::runtime_error(std::string("Failed to compile shader:\n") + infoLog);
    }
}

void Shader::validateProgram(unsigned int object_)
{
    int success;
    char infoLog[1024];
    glGetProgramiv(object_, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(object_, 1024, NULL, infoLog);
        throw std::runtime_error(std::string("Failed to compile shader program:\n") + infoLog);
    }
}

int Shader::claimUniformLoc(const char *name_)
{
    auto res = glGetUniformLocation(m_id, name_);
    if (res == -1)
    {
        std::cout << "Failed to retrieve \"" << name_ << "\" uniform." << std::endl;
        dumpErrors();
    }

    return res;

}

void loadFile(const std::string &filePath_, std::string &tar_)
{
    std::ifstream file(filePath_);

    if (!file.is_open())
        throw std::runtime_error("Failed to open file \"" + filePath_ + "\" for shader");

    std::ostringstream tmp;
    tmp << file.rdbuf();

    tar_ = tmp.str();
}
