#ifndef GFX_OPENGL_CONTEXT_HPP
#define GFX_OPENGL_CONTEXT_HPP

#include "../context.hpp"

#include <glad/glad.h>

namespace gfx {

namespace opengl {

class OpenglContext : public Context {
public:
    OpenglContext(core::Window& window, bool enableValidation);
    ~OpenglContext() override;

    static constexpr Api api = Api::Opengl;

private:

};

} // namespace opengl

} // namespace gfx

#endif