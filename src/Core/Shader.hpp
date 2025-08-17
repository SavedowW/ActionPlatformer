#ifndef SHADER_HPP_
#define SHADER_HPP_

#include "Shader.h"
#include "glad/glad.h"

template<Numeric T>
inline void Shader::setFloat(const char *name_, T value_)
{
    glUniform1f(claimUniformLoc(name_), static_cast<float>(value_));
}


template<Numeric T>
inline void Shader::setVector2f(const char *name_, T x_, T y_)
{
    glUniform2f(claimUniformLoc(name_), static_cast<float>(x_), static_cast<float>(y_));
}

template<Numeric T>
inline void Shader::setVector3f(const char *name_, T x_, T y_, T z_)
{
    glUniform3f(claimUniformLoc(name_), static_cast<float>(x_), static_cast<float>(y_), static_cast<float>(z_));
}

template<Numeric T>
inline void Shader::setVector4f(const char *name_, T x_, T y_, T z_, T w_)
{
    glUniform4f(claimUniformLoc(name_), static_cast<float>(x_), static_cast<float>(y_), static_cast<float>(z_), static_cast<float>(w_));
}

#endif
