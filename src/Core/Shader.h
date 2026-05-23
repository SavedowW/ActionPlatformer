#pragma once
#include "Utils.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>

// Load file into tar_ string, return true on success
void loadFile(const std::string &filePath_, std::string &tar_);

void dumpErrors();

class Shader
{
public:
    Shader() = default;

    void load(const std::string &vSourcePath_, const std::string &fSourcePath_);

    // sets the current shader as active
    void use();

    // utility functions
    template<Numeric T>
    void setFloat(const char *name_, T value_);
    
    template<Numeric T>
    void setVector2f(const char *name_, T x_, T y_);

    template<Numeric T>
    void setVector3f(const char *name_, T x_, T y_, T z_);

    template<Numeric T>
    void setVector4f(const char *name_, T x_, T y_, T z_, T w_);

    void setVector2f(const char *name_, const glm::vec2 &value_);
    void setVector3f(const char *name_, const glm::vec3 &value_);
    void setVector4f(const char *name_, const glm::vec4 &value_);
    void setMatrix4(const char *name_, const glm::mat4 &matrix_);
    void setInteger(const char *name_, int value_);
    
private:
    void compile(const char *vertexSourcePath_, const char *fragmentSourcePath_);

    static void validateShader(unsigned int object_); 
    void validateProgram() const;
    int claimUniformLoc(const char *name_) const;

    unsigned int m_id = 0; 
};
