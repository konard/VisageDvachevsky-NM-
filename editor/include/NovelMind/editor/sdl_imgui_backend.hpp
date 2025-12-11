#pragma once

/**
 * @file sdl_imgui_backend.hpp
 * @brief SDL2 + ImGui Backend for NovelMind Editor
 *
 * Provides the actual window creation and ImGui integration:
 * - SDL2 window initialization with OpenGL context
 * - ImGui context setup with SDL2 + OpenGL3 backends
 * - Event processing and rendering loop support
 */

#include "NovelMind/core/types.hpp"
#include "NovelMind/core/result.hpp"
#include <string>
#include <functional>

// Forward declarations to avoid including SDL and ImGui headers
struct SDL_Window;
typedef void* SDL_GLContext;
union SDL_Event;

namespace NovelMind::editor {

/**
 * @brief Configuration for the editor window
 */
struct EditorWindowConfig {
    std::string title = "NovelMind Editor";
    i32 width = 1920;
    i32 height = 1080;
    bool maximized = false;
    bool vsync = true;
    f32 uiScale = 1.0f;
    bool darkTheme = true;
};

/**
 * @brief SDL2 + ImGui Backend
 *
 * Manages the actual window creation and ImGui integration.
 * This class provides the infrastructure for the editor UI.
 */
class SDLImGuiBackend {
public:
    SDLImGuiBackend();
    ~SDLImGuiBackend();

    // Prevent copying
    SDLImGuiBackend(const SDLImGuiBackend&) = delete;
    SDLImGuiBackend& operator=(const SDLImGuiBackend&) = delete;

    /**
     * @brief Initialize the backend
     * @param config Window configuration
     * @return Success or error
     */
    Result<void> initialize(const EditorWindowConfig& config);

    /**
     * @brief Shutdown the backend
     */
    void shutdown();

    /**
     * @brief Check if backend is initialized
     */
    [[nodiscard]] bool isInitialized() const { return m_initialized; }

    /**
     * @brief Check if window should close
     */
    [[nodiscard]] bool shouldClose() const { return m_shouldClose; }

    /**
     * @brief Request window close
     */
    void requestClose() { m_shouldClose = true; }

    /**
     * @brief Process events and begin new frame
     * @return false if window should close
     */
    bool beginFrame();

    /**
     * @brief End frame and render
     */
    void endFrame();

    /**
     * @brief Get window width
     */
    [[nodiscard]] i32 getWidth() const { return m_width; }

    /**
     * @brief Get window height
     */
    [[nodiscard]] i32 getHeight() const { return m_height; }

    /**
     * @brief Set window title
     */
    void setTitle(const std::string& title);

    /**
     * @brief Apply dark theme to ImGui
     */
    void applyDarkTheme();

    /**
     * @brief Apply light theme to ImGui
     */
    void applyLightTheme();

    /**
     * @brief Set UI scale factor
     */
    void setUIScale(f32 scale);

    /**
     * @brief Begin main dockspace
     * Call this at the beginning of the frame after beginFrame()
     */
    void beginDockspace();

    /**
     * @brief End main dockspace
     * Call this before endFrame()
     */
    void endDockspace();

    /**
     * @brief Get the SDL window handle
     */
    [[nodiscard]] SDL_Window* getWindow() const { return m_window; }

    /**
     * @brief Get the OpenGL context
     */
    [[nodiscard]] SDL_GLContext getGLContext() const { return m_glContext; }

    /**
     * @brief Set callback for file drop events
     */
    void setFileDropCallback(std::function<void(const std::string&)> callback) {
        m_fileDropCallback = std::move(callback);
    }

private:
    bool processEvents();
    void setupImGuiStyle();

    SDL_Window* m_window = nullptr;
    SDL_GLContext m_glContext = nullptr;

    bool m_initialized = false;
    bool m_shouldClose = false;
    i32 m_width = 1920;
    i32 m_height = 1080;
    f32 m_uiScale = 1.0f;

    std::function<void(const std::string&)> m_fileDropCallback;
};

} // namespace NovelMind::editor
