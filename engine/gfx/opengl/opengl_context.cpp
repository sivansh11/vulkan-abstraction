#include "opengl_context.hpp"

#include <cassert>

namespace gfx {

namespace opengl {

OpenglContext::OpenglContext() : Context(OpenglContext::api) {

}

OpenglContext::~OpenglContext() {

}

} // namespace opengl

} // namespace gfx
