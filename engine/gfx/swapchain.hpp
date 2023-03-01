#ifndef GFX_SWAPCHAIN_HPP
#define GFX_SWAPCHAIN_HPP

#include "apis.hpp"

#include "../core/types.hpp"

#include "context.hpp"

namespace gfx {

class SwapChain {
public:
    SwapChain() : m_selectedApi(Api::None) {}

    static SwapChain *createSwapChain(Context *ctx, const core::Extent2D windowExtent);
    static void destroySwapChain(SwapChain *swapChain);

    const Api m_selectedApi; 

protected:
    SwapChain(Api selectedApi);
};

} // namespace gfx

#endif