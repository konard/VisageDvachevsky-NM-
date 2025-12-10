#include "nm/renderer/font.hpp"
#include "nm/core/logger.hpp"

namespace nm::renderer
{

Font::Font()
    : m_handle(nullptr)
    , m_size(0)
{
}

Font::~Font()
{
    destroy();
}

Font::Font(Font&& other) noexcept
    : m_handle(other.m_handle)
    , m_size(other.m_size)
{
    other.m_handle = nullptr;
    other.m_size = 0;
}

Font& Font::operator=(Font&& other) noexcept
{
    if (this != &other)
    {
        destroy();
        m_handle = other.m_handle;
        m_size = other.m_size;
        other.m_handle = nullptr;
        other.m_size = 0;
    }
    return *this;
}

Result<void> Font::loadFromMemory(const std::vector<u8>& data, i32 size)
{
    if (data.empty() || size <= 0)
    {
        return Result<void>::error("Invalid font data or size");
    }

    // TODO: Implement actual font loading using FreeType
    m_size = size;
    NM_LOG_DEBUG("Font::loadFromMemory - placeholder implementation");

    return Result<void>::ok();
}

void Font::destroy()
{
    if (m_handle)
    {
        // TODO: Destroy actual font resource
        m_handle = nullptr;
    }
    m_size = 0;
}

bool Font::isValid() const
{
    return m_size > 0;
}

i32 Font::getSize() const
{
    return m_size;
}

void* Font::getNativeHandle() const
{
    return m_handle;
}

} // namespace nm::renderer
