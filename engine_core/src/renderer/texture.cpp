#include "nm/renderer/texture.hpp"
#include "nm/core/logger.hpp"

namespace nm::renderer
{

Texture::Texture()
    : m_handle(nullptr)
    , m_width(0)
    , m_height(0)
{
}

Texture::~Texture()
{
    destroy();
}

Texture::Texture(Texture&& other) noexcept
    : m_handle(other.m_handle)
    , m_width(other.m_width)
    , m_height(other.m_height)
{
    other.m_handle = nullptr;
    other.m_width = 0;
    other.m_height = 0;
}

Texture& Texture::operator=(Texture&& other) noexcept
{
    if (this != &other)
    {
        destroy();
        m_handle = other.m_handle;
        m_width = other.m_width;
        m_height = other.m_height;
        other.m_handle = nullptr;
        other.m_width = 0;
        other.m_height = 0;
    }
    return *this;
}

Result<void> Texture::loadFromMemory(const std::vector<u8>& data)
{
    if (data.empty())
    {
        return Result<void>::error("Empty texture data");
    }

    // TODO: Implement actual image loading using stb_image or similar
    // For now, just log and return success as a placeholder
    NM_LOG_DEBUG("Texture::loadFromMemory - placeholder implementation");

    return Result<void>::ok();
}

Result<void> Texture::loadFromRGBA(const u8* pixels, i32 width, i32 height)
{
    if (!pixels || width <= 0 || height <= 0)
    {
        return Result<void>::error("Invalid texture parameters");
    }

    // TODO: Implement actual texture creation from raw pixels
    // For now, just store dimensions as a placeholder
    m_width = width;
    m_height = height;

    NM_LOG_DEBUG("Texture::loadFromRGBA - placeholder implementation");

    return Result<void>::ok();
}

void Texture::destroy()
{
    if (m_handle)
    {
        // TODO: Destroy actual texture resource
        m_handle = nullptr;
    }
    m_width = 0;
    m_height = 0;
}

bool Texture::isValid() const
{
    return m_width > 0 && m_height > 0;
}

i32 Texture::getWidth() const
{
    return m_width;
}

i32 Texture::getHeight() const
{
    return m_height;
}

void* Texture::getNativeHandle() const
{
    return m_handle;
}

} // namespace nm::renderer
