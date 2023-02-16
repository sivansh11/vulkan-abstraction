#ifndef GFX_OPENGL_CONTEXT_HPP
#define GFX_OPENGL_CONTEXT_HPP

#include "../context.hpp"

namespace gfx {

namespace opengl {

class OpenglContext : public Context {
public:
    OpenglContext();
    ~OpenglContext() override;

    static constexpr Api api = Api::Opengl;

private:

};

} // namespace opengl

} // namespace gfx

#endif