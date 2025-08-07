#ifndef SHADER_H_
#define SHADER_H_

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
    void setFloat(const char *name_, float value_);
    void setInteger(const char *name_, int value_);
    void setVector2f(const char *name_, float x_, float y_);
    void setVector2f(const char *name_, const glm::vec2 &value_);
    void setVector3f(const char *name_, float x_, float y_, float z_);
    void setVector3f(const char *name_, const glm::vec3 &value_);
    void setVector4f(const char *name_, float x_, float y_, float z_, float w_);
    void setVector4f(const char *name_, const glm::vec4 &value_);
    void setMatrix4(const char *name_, const glm::mat4 &matrix_);

private:
    void compile(const char *vertexSourcePath_, const char *fragmentSourcePath_);

    bool validateShader(unsigned int object_, const std::string &type_); 
    bool validateProgram(unsigned int object_);
    GLint claimUniformLoc(const char *name_);

    unsigned int m_id = 0; 
};

#endif
