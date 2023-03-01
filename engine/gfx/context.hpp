#ifndef GFX_CONTEXT_HPP
#define GFX_CONTEXT_HPP

#include "apis.hpp"

#include "../core/window.hpp"

#include <memory>

#ifndef NDEBUG
#define CAST(TYPE, VALUE) \
    reinterpret_cast<TYPE *>(VALUE); \
    if (VALUE->m_selectedApi != TYPE::api) \
        throw std::runtime_error("Bad Cast! Casting between different types")
#else
#define CAST(TYPE, VALUE) \
reinterpret_cast<TYPE *>(VALUE)
#endif

namespace gfx {

class Context {
public:
    Context() : m_selectedApi(Api::None) {}
    virtual ~Context() {}

    static Context *createContext(bool enableValidation, core::Window& window);
    static void destroyContext(Context *ctx);

    const Api m_selectedApi;

protected:
    Context(Api selectedApi);

};

} // namespace gfx

#endif