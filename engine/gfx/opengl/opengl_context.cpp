#include "opengl_context.hpp"

#include "../../core/log.hpp"

#include <cassert>

namespace gfx {

namespace opengl {

OpenglContext::OpenglContext() : Context(OpenglContext::api) {
    if (!gladLoadGL()) {
        throw std::runtime_error("Opengl: Failed to load gl");
    }

    INFO("LOADED GLAD");
}

OpenglContext::~OpenglContext() {

}

} // namespace opengl

} // namespace gfx
