#ifndef GFX_OPENGL_WINDOW_HPP
#define GFX_OPENGL_WINDOW_HPP

#include "../apis.hpp"
#include "../../core/types.hpp"

#include <GLFW/glfw3.h>

#include <vector>
#include <array>

namespace gfx {

namespace opengl {

class OpenglWindow {
public:
    OpenglWindow(GLFWwindow *window) : m_windowPtr(window) {}

    static constexpr Api api = Api::Opengl;

private:
    GLFWwindow *m_windowPtr;
};

} // namespace opengl

} // namespace gfx

#endif