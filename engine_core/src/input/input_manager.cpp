#include "nm/input/input_manager.hpp"

namespace nm::input
{

InputManager::InputManager()
    : m_mouseX(0)
    , m_mouseY(0)
{
}

InputManager::~InputManager() = default;

void InputManager::update()
{
    // TODO: Implement actual input polling
}

bool InputManager::isKeyDown(Key /*key*/) const
{
    // TODO: Implement
    return false;
}

bool InputManager::isKeyPressed(Key /*key*/) const
{
    // TODO: Implement
    return false;
}

bool InputManager::isKeyReleased(Key /*key*/) const
{
    // TODO: Implement
    return false;
}

bool InputManager::isMouseButtonDown(MouseButton /*button*/) const
{
    // TODO: Implement
    return false;
}

bool InputManager::isMouseButtonPressed(MouseButton /*button*/) const
{
    // TODO: Implement
    return false;
}

bool InputManager::isMouseButtonReleased(MouseButton /*button*/) const
{
    // TODO: Implement
    return false;
}

i32 InputManager::getMouseX() const
{
    return m_mouseX;
}

i32 InputManager::getMouseY() const
{
    return m_mouseY;
}

} // namespace nm::input
