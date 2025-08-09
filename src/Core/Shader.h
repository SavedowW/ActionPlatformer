#ifndef SHADER_H_
#define SHADER_H_

#include "Utils.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>

// Load file into tar_ string, return true on success
void loadFile(const std::string &filePath_, std::string &tar_);

void dumpErrors();

class Shader
{
public:
    Shader();

    void load(const std::string &vSourcePath_, const std::string &fSourcePath_);

    // sets the current shader as active
    void use();

    // utility functions
    template<Numeric T>
    inline void setFloat(const char *name_, T value_)
    {
        glUniform1f(claimUniformLoc(name_), static_cast<float>(value_));
    }

    
    template<Numeric T>
    inline void setVector2f(const char *name_, T x_, T y_)
    {
        glUniform2f(claimUniformLoc(name_), static_cast<float>(x_), static_cast<float>(y_));
    }

    template<Numeric T>
    inline void setVector3f(const char *name_, T x_, T y_, T z_)
    {
        glUniform3f(claimUniformLoc(name_), static_cast<float>(x_), static_cast<float>(y_), static_cast<float>(z_));
    }

    template<Numeric T>
    inline void setVector4f(const char *name_, T x_, T y_, T z_, T w_)
    {
        glUniform4f(claimUniformLoc(name_), static_cast<float>(x_), static_cast<float>(y_), static_cast<float>(z_), static_cast<float>(w_));
    }

    void setVector2f(const char *name_, const glm::vec2 &value_);
    void setVector3f(const char *name_, const glm::vec3 &value_);
    void setVector4f(const char *name_, const glm::vec4 &value_);
    void setMatrix4(const char *name_, const glm::mat4 &matrix_);
    void setInteger(const char *name_, int value_);
    
    private:
    void compile(const char *vertexSourcePath_, const char *fragmentSourcePath_);

    void validateShader(unsigned int object_); 
    void validateProgram(unsigned int object_);
    GLint claimUniformLoc(const char *name_);

    unsigned int m_id = 0; 
};

#endif
