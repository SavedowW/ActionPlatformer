#include "Shader.h"

#include <iostream>
#include <fstream>
#include <sstream>

void dumpErrors()
{
    std::string res = "{";
    bool singleError = false;
    GLenum err = 0;
    bool first = false;
    while (err = glGetError())
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

    if (!loadFile(vSourcePath_, vCode))
    {
        std::cout << "Failed to create a shader program" << std::endl;
        return;
    }

    if (!loadFile(fSourcePath_, fCode))
    {
        std::cout << "Failed to create a shader program" << std::endl;
        return;
    }

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
    if (!validateShader(sVertex, "VERTEX"))
        return;

    // fragment Shader
    sFragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(sFragment, 1, &fragmentSource_, NULL);
    glCompileShader(sFragment);
    if (!validateShader(sFragment, "FRAGMENT"))
        return;

    // shader program
    m_id = glCreateProgram();
    glAttachShader(m_id, sVertex);
    glAttachShader(m_id, sFragment);
    glLinkProgram(m_id);
    if (!validateProgram(m_id))
        return;

    glDeleteShader(sVertex);
    glDeleteShader(sFragment);
}

void Shader::setFloat(const char *name_, float value_)
{
    glUniform1f(claimUniformLoc(name_), value_);
}
void Shader::setInteger(const char *name_, int value_)
{
    glUniform1i(claimUniformLoc(name_), value_);
}
void Shader::setVector2f(const char *name_, float x_, float y_)
{
    glUniform2f(claimUniformLoc(name_), x_, y_);
}
void Shader::setVector2f(const char *name_, const glm::vec2 &value_)
{
    glUniform2f(claimUniformLoc(name_), value_.x, value_.y);
}
void Shader::setVector3f(const char *name_, float x_, float y_, float z_)
{
    glUniform3f(claimUniformLoc(name_), x_, y_, z_);
}
void Shader::setVector3f(const char *name_, const glm::vec3 &value_)
{
    glUniform3f(claimUniformLoc(name_), value_.x, value_.y, value_.z);
}
void Shader::setVector4f(const char *name_, float x_, float y_, float z_, float w_)
{
    glUniform4f(claimUniformLoc(name_), x_, y_, z_, w_);
}
void Shader::setVector4f(const char *name_, const glm::vec4 &value_)
{
    glUniform4f(claimUniformLoc(name_), value_.x, value_.y, value_.z, value_.w);
}
void Shader::setMatrix4(const char *name_, const glm::mat4 &matrix_)
{
    auto uniLoc = claimUniformLoc(name_);
            dumpErrors();
    glUniformMatrix4fv(uniLoc, 1, false, glm::value_ptr(matrix_));
            dumpErrors();

}


bool Shader::validateShader(unsigned int object_, const std::string &type_)
{
    int success;
    char infoLog[1024];
    glGetShaderiv(object_, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(object_, 1024, NULL, infoLog);
        std::cout << "Failed to compile " << type_  << " shader" << std::endl
            << infoLog << std::endl;

        return false;
    }

    return true;
}

bool Shader::validateProgram(unsigned int object_)
{
    int success;
    char infoLog[1024];
    glGetProgramiv(object_, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(object_, 1024, NULL, infoLog);
        std::cout << "Failed to compile shader program: " << std::endl
            << infoLog << std::endl;

        return false;
    }

    return true;
}

GLint Shader::claimUniformLoc(const char *name_)
{
    auto res = glGetUniformLocation(m_id, name_);
    if (res == -1)
    {
        std::cout << "Failed to retrieve \"" << name_ << "\" uniform." << std::endl;
        dumpErrors();
    }

    return res;

}

bool loadFile(const std::string &filePath_, std::string &tar_)
{
    std::ifstream file(filePath_);

    if (!file.is_open())
    {
        std::cout << "Failed to open file \"" << filePath_ << "\"" << std::endl;
        return false;
    }

    std::ostringstream tmp;
    tmp << file.rdbuf();

    tar_ = tmp.str();

    return true;
}
