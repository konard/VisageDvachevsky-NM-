/**
 * @file sdl_imgui_backend.cpp
 * @brief SDL2 + ImGui Backend Implementation
 */

#include "NovelMind/editor/sdl_imgui_backend.hpp"
#include "NovelMind/core/logger.hpp"

// Conditional compilation - only include SDL2 and ImGui if available
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)

#include <SDL.h>
#include <SDL_opengl.h>

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>

namespace NovelMind::editor {

SDLImGuiBackend::SDLImGuiBackend() = default;

SDLImGuiBackend::~SDLImGuiBackend()
{
    if (m_initialized)
    {
        shutdown();
    }
}

Result<void> SDLImGuiBackend::initialize(const EditorWindowConfig& config)
{
    if (m_initialized)
    {
        return Result<void>::error("Backend already initialized");
    }

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
        return Result<void>::error(std::string("SDL_Init Error: ") + SDL_GetError());
    }

    // Setup OpenGL attributes
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    // Create window with OpenGL context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    u32 windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
    if (config.maximized)
    {
        windowFlags |= SDL_WINDOW_MAXIMIZED;
    }

    m_window = SDL_CreateWindow(
        config.title.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        config.width,
        config.height,
        windowFlags
    );

    if (!m_window)
    {
        SDL_Quit();
        return Result<void>::error(std::string("SDL_CreateWindow Error: ") + SDL_GetError());
    }

    // Create OpenGL context
    m_glContext = SDL_GL_CreateContext(m_window);
    if (!m_glContext)
    {
        SDL_DestroyWindow(m_window);
        SDL_Quit();
        return Result<void>::error(std::string("SDL_GL_CreateContext Error: ") + SDL_GetError());
    }

    SDL_GL_MakeCurrent(m_window, m_glContext);
    SDL_GL_SetSwapInterval(config.vsync ? 1 : 0);

    // Get actual window size
    SDL_GetWindowSize(m_window, &m_width, &m_height);

    // Setup ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    // Enable docking and viewports
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Configure docking
    io.ConfigDockingWithShift = false;
    io.ConfigViewportsNoTaskBarIcon = true;

    // When viewports are enabled, adjust style
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(m_window, m_glContext);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Apply theme
    m_uiScale = config.uiScale;
    if (config.darkTheme)
    {
        applyDarkTheme();
    }
    else
    {
        applyLightTheme();
    }

    // Scale UI
    setUIScale(config.uiScale);

    m_initialized = true;
    m_shouldClose = false;

    NOVELMIND_LOG_INFO("SDL2 + ImGui backend initialized successfully");
    return Result<void>::ok();
}

void SDLImGuiBackend::shutdown()
{
    if (!m_initialized)
    {
        return;
    }

    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    // Cleanup SDL
    if (m_glContext)
    {
        SDL_GL_DeleteContext(m_glContext);
        m_glContext = nullptr;
    }

    if (m_window)
    {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }

    SDL_Quit();

    m_initialized = false;
    NOVELMIND_LOG_INFO("SDL2 + ImGui backend shutdown complete");
}

bool SDLImGuiBackend::beginFrame()
{
    if (!m_initialized)
    {
        return false;
    }

    // Process events
    if (!processEvents())
    {
        return false;
    }

    // Start ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    return true;
}

void SDLImGuiBackend::endFrame()
{
    if (!m_initialized)
    {
        return;
    }

    // Rendering
    ImGui::Render();

    ImGuiIO& io = ImGui::GetIO();
    glViewport(0, 0, static_cast<int>(io.DisplaySize.x), static_cast<int>(io.DisplaySize.y));
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Handle viewports
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
        SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
    }

    SDL_GL_SwapWindow(m_window);
}

bool SDLImGuiBackend::processEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSDL2_ProcessEvent(&event);

        switch (event.type)
        {
            case SDL_QUIT:
                m_shouldClose = true;
                return false;

            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_CLOSE &&
                    event.window.windowID == SDL_GetWindowID(m_window))
                {
                    m_shouldClose = true;
                    return false;
                }
                if (event.window.event == SDL_WINDOWEVENT_RESIZED)
                {
                    m_width = event.window.data1;
                    m_height = event.window.data2;
                }
                break;

            case SDL_DROPFILE:
                if (m_fileDropCallback && event.drop.file)
                {
                    m_fileDropCallback(event.drop.file);
                    SDL_free(event.drop.file);
                }
                break;

            default:
                break;
        }
    }

    return true;
}

void SDLImGuiBackend::setTitle(const std::string& title)
{
    if (m_window)
    {
        SDL_SetWindowTitle(m_window, title.c_str());
    }
}

void SDLImGuiBackend::applyDarkTheme()
{
    ImGui::StyleColorsDark();
    setupImGuiStyle();
}

void SDLImGuiBackend::applyLightTheme()
{
    ImGui::StyleColorsLight();
    setupImGuiStyle();
}

void SDLImGuiBackend::setupImGuiStyle()
{
    ImGuiStyle& style = ImGui::GetStyle();

    // Rounding
    style.WindowRounding = 4.0f;
    style.FrameRounding = 2.0f;
    style.ScrollbarRounding = 2.0f;
    style.GrabRounding = 2.0f;
    style.TabRounding = 4.0f;

    // Borders
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.PopupBorderSize = 1.0f;

    // Padding
    style.WindowPadding = ImVec2(8, 8);
    style.FramePadding = ImVec2(4, 3);
    style.ItemSpacing = ImVec2(8, 4);
    style.ItemInnerSpacing = ImVec2(4, 4);
    style.IndentSpacing = 20.0f;

    // Custom dark theme colors
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.14f, 0.14f, 0.14f, 0.94f);
    colors[ImGuiCol_Border] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.10f, 0.10f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.00f, 0.48f, 0.80f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.00f, 0.48f, 0.80f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.00f, 0.58f, 0.90f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.00f, 0.48f, 0.80f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.00f, 0.38f, 0.70f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.00f, 0.48f, 0.80f, 0.80f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.00f, 0.48f, 0.80f, 1.00f);
    colors[ImGuiCol_Separator] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.00f, 0.48f, 0.80f, 0.78f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.00f, 0.48f, 0.80f, 1.00f);
    colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.00f, 0.48f, 0.80f, 0.80f);
    colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_DockingPreview] = ImVec4(0.00f, 0.48f, 0.80f, 0.70f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
}

void SDLImGuiBackend::setUIScale(f32 scale)
{
    m_uiScale = scale;

    ImGuiIO& io = ImGui::GetIO();
    io.FontGlobalScale = scale;

    // Note: For proper high-DPI support, fonts should be rebuilt with new size
}

void SDLImGuiBackend::beginDockspace()
{
    if (!m_initialized)
    {
        return;
    }

    // Create fullscreen dockspace
    ImGuiWindowFlags windowFlags =
        ImGuiWindowFlags_MenuBar |
        ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus;

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("DockSpace", nullptr, windowFlags);
    ImGui::PopStyleVar(3);

    // Create dockspace
    ImGuiID dockspaceId = ImGui::GetID("MainDockspace");
    ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
}

void SDLImGuiBackend::endDockspace()
{
    ImGui::End();
}

} // namespace NovelMind::editor

#else // No SDL2 or ImGui available

// Stub implementation for systems without SDL2/ImGui
namespace NovelMind::editor {

SDLImGuiBackend::SDLImGuiBackend() = default;

SDLImGuiBackend::~SDLImGuiBackend()
{
    if (m_initialized)
    {
        shutdown();
    }
}

Result<void> SDLImGuiBackend::initialize(const EditorWindowConfig& /*config*/)
{
    NOVELMIND_LOG_WARN("SDL2/ImGui not available - editor running in stub mode");
    m_initialized = true;
    return Result<void>::ok();
}

void SDLImGuiBackend::shutdown()
{
    m_initialized = false;
}

bool SDLImGuiBackend::beginFrame()
{
    return !m_shouldClose;
}

void SDLImGuiBackend::endFrame()
{
    // Stub implementation
}

void SDLImGuiBackend::setTitle(const std::string& /*title*/)
{
    // Stub implementation
}

void SDLImGuiBackend::applyDarkTheme()
{
    // Stub implementation
}

void SDLImGuiBackend::applyLightTheme()
{
    // Stub implementation
}

void SDLImGuiBackend::setUIScale(f32 scale)
{
    m_uiScale = scale;
}

void SDLImGuiBackend::beginDockspace()
{
    // Stub implementation
}

void SDLImGuiBackend::endDockspace()
{
    // Stub implementation
}

} // namespace NovelMind::editor

#endif // NOVELMIND_HAS_SDL2 && NOVELMIND_HAS_IMGUI
