/**
 * @file imgui_integration.cpp
 * @brief ImGui Integration Layer implementation
 */

#include "NovelMind/editor/imgui_integration.hpp"
#include <cmath>
#include <algorithm>

// ImGui is optional at compile time - stub implementation when not available
// This allows the editor to compile without ImGui for testing purposes.
// When ImGui is available (NOVELMIND_HAS_IMGUI), real implementation is used.

#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
#include <imgui.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_opengl3.h>
#include <SDL.h>
#endif

namespace NovelMind::editor {

// ============================================================================
// Static instances
// ============================================================================

std::unique_ptr<ImGuiLayer> ImGuiLayer::s_instance = nullptr;

// ============================================================================
// EditorTheme Implementation
// ============================================================================

EditorTheme EditorTheme::createDarkTheme()
{
    EditorTheme theme;
    theme.name = "Dark";

    // Primary colors
    theme.background = {30, 30, 30, 255};
    theme.backgroundDark = {20, 20, 20, 255};
    theme.backgroundLight = {45, 45, 45, 255};
    theme.foreground = {220, 220, 220, 255};
    theme.foregroundDim = {150, 150, 150, 255};

    // Accent colors (VS Code blue)
    theme.accent = {0, 122, 204, 255};
    theme.accentHover = {0, 150, 230, 255};
    theme.accentActive = {0, 100, 180, 255};

    // Semantic colors
    theme.success = {76, 175, 80, 255};
    theme.warning = {255, 193, 7, 255};
    theme.error = {244, 67, 54, 255};
    theme.info = {33, 150, 243, 255};

    // Border and separators
    theme.border = {60, 60, 60, 255};
    theme.borderHighlight = {80, 80, 80, 255};
    theme.separator = {50, 50, 50, 255};

    // Selection colors
    theme.selection = {0, 122, 204, 128};
    theme.selectionInactive = {100, 100, 100, 128};

    // Text colors
    theme.text = {220, 220, 220, 255};
    theme.textDisabled = {120, 120, 120, 255};
    theme.textHighlight = {255, 255, 255, 255};

    // Graph editor colors
    theme.nodeBackground = {40, 40, 40, 230};
    theme.nodeBorder = {80, 80, 80, 255};
    theme.nodeHeader = {60, 60, 60, 255};
    theme.connectionLine = {150, 150, 150, 255};
    theme.connectionLineActive = {0, 122, 204, 255};

    // Timeline colors
    theme.timelineBackground = {25, 25, 25, 255};
    theme.timelineRuler = {40, 40, 40, 255};
    theme.timelineKeyframe = {0, 122, 204, 255};
    theme.timelinePlayhead = {255, 100, 100, 255};

    // Styling
    theme.windowRounding = 4.0f;
    theme.frameRounding = 2.0f;
    theme.scrollbarRounding = 2.0f;
    theme.grabRounding = 2.0f;
    theme.windowBorderSize = 1.0f;
    theme.frameBorderSize = 0.0f;
    theme.popupBorderSize = 1.0f;

    // Spacing
    theme.windowPadding = 8.0f;
    theme.framePadding = 4.0f;
    theme.itemSpacing = 8.0f;
    theme.itemInnerSpacing = 4.0f;
    theme.indentSpacing = 20.0f;

    theme.fontSize = 14.0f;
    theme.iconSize = 16.0f;

    return theme;
}

EditorTheme EditorTheme::createLightTheme()
{
    EditorTheme theme;
    theme.name = "Light";

    // Primary colors
    theme.background = {245, 245, 245, 255};
    theme.backgroundDark = {230, 230, 230, 255};
    theme.backgroundLight = {255, 255, 255, 255};
    theme.foreground = {40, 40, 40, 255};
    theme.foregroundDim = {120, 120, 120, 255};

    // Accent colors
    theme.accent = {0, 120, 215, 255};
    theme.accentHover = {0, 100, 190, 255};
    theme.accentActive = {0, 80, 170, 255};

    // Semantic colors
    theme.success = {56, 142, 60, 255};
    theme.warning = {245, 127, 23, 255};
    theme.error = {211, 47, 47, 255};
    theme.info = {25, 118, 210, 255};

    // Border and separators
    theme.border = {200, 200, 200, 255};
    theme.borderHighlight = {180, 180, 180, 255};
    theme.separator = {210, 210, 210, 255};

    // Selection colors
    theme.selection = {0, 120, 215, 80};
    theme.selectionInactive = {180, 180, 180, 80};

    // Text colors
    theme.text = {40, 40, 40, 255};
    theme.textDisabled = {160, 160, 160, 255};
    theme.textHighlight = {0, 0, 0, 255};

    // Graph editor colors
    theme.nodeBackground = {250, 250, 250, 230};
    theme.nodeBorder = {180, 180, 180, 255};
    theme.nodeHeader = {235, 235, 235, 255};
    theme.connectionLine = {120, 120, 120, 255};
    theme.connectionLineActive = {0, 120, 215, 255};

    // Timeline colors
    theme.timelineBackground = {240, 240, 240, 255};
    theme.timelineRuler = {220, 220, 220, 255};
    theme.timelineKeyframe = {0, 120, 215, 255};
    theme.timelinePlayhead = {220, 80, 80, 255};

    // Same styling as dark theme
    theme.windowRounding = 4.0f;
    theme.frameRounding = 2.0f;
    theme.scrollbarRounding = 2.0f;
    theme.grabRounding = 2.0f;
    theme.windowBorderSize = 1.0f;
    theme.frameBorderSize = 0.0f;
    theme.popupBorderSize = 1.0f;
    theme.windowPadding = 8.0f;
    theme.framePadding = 4.0f;
    theme.itemSpacing = 8.0f;
    theme.itemInnerSpacing = 4.0f;
    theme.indentSpacing = 20.0f;
    theme.fontSize = 14.0f;
    theme.iconSize = 16.0f;

    return theme;
}

EditorTheme EditorTheme::createHighContrastTheme()
{
    EditorTheme theme;
    theme.name = "High Contrast";

    // High contrast primary colors
    theme.background = {0, 0, 0, 255};
    theme.backgroundDark = {0, 0, 0, 255};
    theme.backgroundLight = {30, 30, 30, 255};
    theme.foreground = {255, 255, 255, 255};
    theme.foregroundDim = {200, 200, 200, 255};

    // Bright accent colors
    theme.accent = {0, 200, 255, 255};
    theme.accentHover = {100, 220, 255, 255};
    theme.accentActive = {0, 160, 220, 255};

    // High contrast semantic colors
    theme.success = {100, 255, 100, 255};
    theme.warning = {255, 255, 0, 255};
    theme.error = {255, 100, 100, 255};
    theme.info = {100, 200, 255, 255};

    // Visible borders
    theme.border = {128, 128, 128, 255};
    theme.borderHighlight = {200, 200, 200, 255};
    theme.separator = {100, 100, 100, 255};

    // High contrast selection
    theme.selection = {0, 200, 255, 180};
    theme.selectionInactive = {150, 150, 150, 180};

    // High contrast text
    theme.text = {255, 255, 255, 255};
    theme.textDisabled = {150, 150, 150, 255};
    theme.textHighlight = {255, 255, 0, 255};

    // Graph editor colors
    theme.nodeBackground = {20, 20, 20, 255};
    theme.nodeBorder = {128, 128, 128, 255};
    theme.nodeHeader = {40, 40, 40, 255};
    theme.connectionLine = {200, 200, 200, 255};
    theme.connectionLineActive = {0, 200, 255, 255};

    // Timeline colors
    theme.timelineBackground = {10, 10, 10, 255};
    theme.timelineRuler = {50, 50, 50, 255};
    theme.timelineKeyframe = {0, 200, 255, 255};
    theme.timelinePlayhead = {255, 100, 100, 255};

    // Sharp corners for high contrast
    theme.windowRounding = 0.0f;
    theme.frameRounding = 0.0f;
    theme.scrollbarRounding = 0.0f;
    theme.grabRounding = 0.0f;
    theme.windowBorderSize = 2.0f;
    theme.frameBorderSize = 1.0f;
    theme.popupBorderSize = 2.0f;
    theme.windowPadding = 8.0f;
    theme.framePadding = 4.0f;
    theme.itemSpacing = 8.0f;
    theme.itemInnerSpacing = 4.0f;
    theme.indentSpacing = 20.0f;
    theme.fontSize = 15.0f;
    theme.iconSize = 18.0f;

    return theme;
}

// ============================================================================
// DockingLayout Implementation
// ============================================================================

DockingLayout DockingLayout::createDefaultLayout()
{
    DockingLayout layout;
    layout.name = "Default";
    layout.showSceneView = true;
    layout.showStoryGraph = true;
    layout.showTimeline = true;
    layout.showInspector = true;
    layout.showAssetBrowser = true;
    layout.showHierarchy = true;
    layout.showConsole = true;
    layout.showProjectBrowser = true;
    layout.leftPanelWidth = 0.2f;
    layout.rightPanelWidth = 0.25f;
    layout.bottomPanelHeight = 0.25f;
    return layout;
}

DockingLayout DockingLayout::createSceneEditLayout()
{
    DockingLayout layout = createDefaultLayout();
    layout.name = "Scene Edit";
    layout.showStoryGraph = false;
    layout.showTimeline = true;
    layout.rightPanelWidth = 0.3f;
    return layout;
}

DockingLayout DockingLayout::createStoryGraphLayout()
{
    DockingLayout layout = createDefaultLayout();
    layout.name = "Story Graph";
    layout.showSceneView = false;
    layout.showTimeline = false;
    layout.showStoryGraph = true;
    return layout;
}

DockingLayout DockingLayout::createTimelineLayout()
{
    DockingLayout layout = createDefaultLayout();
    layout.name = "Timeline";
    layout.showTimeline = true;
    layout.bottomPanelHeight = 0.4f;
    return layout;
}

// ============================================================================
// ImGuiLayer Implementation
// ============================================================================

ImGuiLayer::ImGuiLayer()
{
    m_currentTheme = EditorTheme::createDarkTheme();
    m_currentLayout = DockingLayout::createDefaultLayout();
}

ImGuiLayer::~ImGuiLayer()
{
    if (m_initialized)
    {
        shutdown();
    }
}

ImGuiLayer& ImGuiLayer::instance()
{
    if (!s_instance)
    {
        s_instance = std::make_unique<ImGuiLayer>();
    }
    return *s_instance;
}

Result<void> ImGuiLayer::initialize(void* windowHandle, void* glContext)
{
    if (m_initialized)
    {
        return Result<void>::error("ImGui already initialized");
    }

    m_windowHandle = windowHandle;
    m_glContext = glContext;

#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    // Create ImGui context
    m_context = ImGui::CreateContext();
    if (!m_context)
    {
        return Result<void>::error("Failed to create ImGui context");
    }

    // Setup ImGui IO
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;      // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;    // Enable Multi-Viewport

    // Initialize SDL2 backend
    if (!ImGui_ImplSDL2_InitForOpenGL(static_cast<SDL_Window*>(windowHandle), glContext))
    {
        ImGui::DestroyContext();
        return Result<void>::error("Failed to initialize SDL2 backend");
    }

    // Initialize OpenGL3 backend
    const char* glsl_version = "#version 130";
    if (!ImGui_ImplOpenGL3_Init(glsl_version))
    {
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
        return Result<void>::error("Failed to initialize OpenGL3 backend");
    }
#else
    m_context = nullptr;
#endif

    setupDefaultStyle();
    setupDockspace();

    m_initialized = true;
    return Result<void>::ok();
}

void ImGuiLayer::shutdown()
{
    if (!m_initialized)
    {
        return;
    }

#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    // Cleanup ImGui backends and context
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
#endif

    m_context = nullptr;
    m_windowHandle = nullptr;
    m_glContext = nullptr;
    m_initialized = false;
    m_fonts.clear();
}

void ImGuiLayer::beginFrame()
{
    if (!m_initialized)
    {
        return;
    }

#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    // Begin new ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
#endif

    if (m_dockingEnabled)
    {
        beginDockspace();
    }
}

void ImGuiLayer::endFrame()
{
    if (!m_initialized)
    {
        return;
    }

    if (m_dockingEnabled)
    {
        endDockspace();
    }

#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    // Render ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Handle viewport windows for docking
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
#endif
}

bool ImGuiLayer::processEvent(void* sdlEvent)
{
    if (!m_initialized)
    {
        return false;
    }

#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    // Process SDL event
    return ImGui_ImplSDL2_ProcessEvent(static_cast<SDL_Event*>(sdlEvent));
#else
    (void)sdlEvent;
    return false;
#endif
}

bool ImGuiLayer::wantsKeyboard() const
{
    if (!m_initialized)
    {
        return false;
    }
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    return ImGui::GetIO().WantCaptureKeyboard;
#else
    return false;
#endif
}

bool ImGuiLayer::wantsMouse() const
{
    if (!m_initialized)
    {
        return false;
    }
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    return ImGui::GetIO().WantCaptureMouse;
#else
    return false;
#endif
}

void ImGuiLayer::applyTheme(const EditorTheme& theme)
{
    m_currentTheme = theme;

    if (!m_initialized)
    {
        return;
    }

#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    // Apply theme to ImGui style
    ImGuiStyle& style = ImGui::GetStyle();

    style.WindowRounding = theme.windowRounding;
    style.FrameRounding = theme.frameRounding;
    style.ScrollbarRounding = theme.scrollbarRounding;
    style.GrabRounding = theme.grabRounding;
    style.TabRounding = theme.tabRounding;

    // Colors
    auto toImVec4 = [](const renderer::Color& c) {
        return ImVec4(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, c.a / 255.0f);
    };

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = toImVec4(theme.background);
    colors[ImGuiCol_ChildBg] = toImVec4(theme.backgroundLight);
    colors[ImGuiCol_PopupBg] = toImVec4(theme.backgroundLight);
    colors[ImGuiCol_Border] = toImVec4(theme.border);
    colors[ImGuiCol_Text] = toImVec4(theme.foreground);
    colors[ImGuiCol_TextDisabled] = toImVec4(theme.foregroundDim);
    colors[ImGuiCol_Button] = toImVec4(theme.accent);
    colors[ImGuiCol_ButtonHovered] = toImVec4(theme.accentHover);
    colors[ImGuiCol_ButtonActive] = toImVec4(theme.accentActive);
    // Additional colors can be set as needed
#endif
}

void ImGuiLayer::setUIScale(f32 scale)
{
    m_uiScale = std::clamp(scale, 0.5f, 3.0f);

    if (!m_initialized)
    {
        return;
    }

#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    // Apply scale to ImGui
    ImGuiStyle& style = ImGui::GetStyle();
    ImGuiStyle defaultStyle;  // Get default style
    style = defaultStyle;     // Reset to default
    style.ScaleAllSizes(m_uiScale);
    applyTheme(m_currentTheme);  // Reapply theme after scaling
#endif
}

void ImGuiLayer::setDockingEnabled(bool enabled)
{
    m_dockingEnabled = enabled;

    if (!m_initialized)
    {
        return;
    }

#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    // Update ImGui config flags
    ImGuiIO& io = ImGui::GetIO();
    if (enabled)
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    else
        io.ConfigFlags &= ~ImGuiConfigFlags_DockingEnable;
#endif
}

void ImGuiLayer::applyLayout(const DockingLayout& layout)
{
    m_currentLayout = layout;
    // Layout is applied in beginDockspace()
}

Result<void> ImGuiLayer::saveLayout(const std::string& /*filepath*/)
{
    // Would save ImGui.ini and custom layout data
    return Result<void>::ok();
}

Result<DockingLayout> ImGuiLayer::loadLayout(const std::string& /*filepath*/)
{
    // Would load ImGui.ini and custom layout data
    return Result<DockingLayout>::ok(m_currentLayout);
}

void ImGuiLayer::resetToDefaultLayout()
{
    m_currentLayout = DockingLayout::createDefaultLayout();
    // Would reset ImGui docking state
}

void ImGuiLayer::beginDockspace()
{
    if (!m_initialized || !m_dockingEnabled)
    {
        return;
    }

#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    // Create main dockspace
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("DockSpace", nullptr, flags);
    ImGui::PopStyleVar(3);

    ImGuiID dockspace_id = ImGui::GetID("MainDockspace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
#endif
}

void ImGuiLayer::endDockspace()
{
    if (!m_initialized || !m_dockingEnabled)
    {
        return;
    }

#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    ImGui::End(); // End DockSpace window
#endif
}

Result<void> ImGuiLayer::loadFont(const std::string& name, const std::string& filepath, f32 size)
{
    if (!m_initialized)
    {
        return Result<void>::error("ImGui not initialized");
    }

#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    // Load font using ImGui
    ImGuiIO& io = ImGui::GetIO();
    ImFont* font = io.Fonts->AddFontFromFileTTF(filepath.c_str(), size * m_uiScale);
    if (!font)
    {
        return Result<void>::error("Failed to load font: " + filepath);
    }

    FontEntry entry;
    entry.path = filepath;
    entry.size = size;
    entry.font = font;
    m_fonts[name] = entry;
#else
    FontEntry entry;
    entry.path = filepath;
    entry.size = size;
    entry.font = nullptr;
    m_fonts[name] = entry;
#endif

    return Result<void>::ok();
}

Result<void> ImGuiLayer::loadIconFont(const std::string& name, const std::string& filepath,
                                       f32 size, u16 iconRangeStart, u16 iconRangeEnd)
{
    if (!m_initialized)
    {
        return Result<void>::error("ImGui not initialized");
    }

#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    // Load icon font with glyph range
    static const ImWchar iconRanges[] = { static_cast<ImWchar>(iconRangeStart),
                                          static_cast<ImWchar>(iconRangeEnd), 0 };
    ImFontConfig config;
    config.MergeMode = true;
    config.GlyphMinAdvanceX = size;
    ImGuiIO& io = ImGui::GetIO();
    ImFont* font = io.Fonts->AddFontFromFileTTF(filepath.c_str(), size, &config, iconRanges);

    FontEntry entry;
    entry.path = filepath;
    entry.size = size;
    entry.font = font;
    m_fonts[name] = entry;
#else
    (void)iconRangeStart;
    (void)iconRangeEnd;
    FontEntry entry;
    entry.path = filepath;
    entry.size = size;
    entry.font = nullptr;
    m_fonts[name] = entry;
#endif

    return Result<void>::ok();
}

void ImGuiLayer::pushFont(const std::string& name)
{
    if (!m_initialized)
    {
        return;
    }

    auto it = m_fonts.find(name);
    if (it != m_fonts.end() && it->second.font != nullptr)
    {
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
        ImGui::PushFont(static_cast<ImFont*>(it->second.font));
#endif
    }
}

void ImGuiLayer::popFont()
{
    if (!m_initialized)
    {
        return;
    }
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    ImGui::PopFont();
#endif
}

void ImGuiLayer::setupDefaultStyle()
{
    applyTheme(m_currentTheme);
}

void ImGuiLayer::setupDockspace()
{
    // Configure docking
    if (!m_initialized)
    {
        return;
    }

#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigDockingWithShift = false;
    io.ConfigViewportsNoTaskBarIcon = true;
#endif
}

// ============================================================================
// Custom Widgets Implementation
// ============================================================================

#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
#include <imgui.h>
#include <imgui_internal.h>
#include <cstring>
#endif

namespace widgets {

bool PropertyLabel(const char* label, f32 labelWidth)
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    ImGui::AlignTextToFramePadding();
    ImGui::Text("%s", label);
    ImGui::SameLine();
    ImGui::SetCursorPosX(labelWidth);
    return true;
#else
    (void)label;
    (void)labelWidth;
    return true;
#endif
}

bool PropertyRow(const char* label, const std::function<bool()>& valueWidget, f32 labelWidth)
{
    PropertyLabel(label, labelWidth);
    return valueWidget();
}

bool CollapsingHeader(const char* label, bool* isOpen, bool defaultOpen)
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_AllowItemOverlap;
    if (defaultOpen) flags |= ImGuiTreeNodeFlags_DefaultOpen;
    bool result = ImGui::CollapsingHeader(label, flags);
    if (isOpen) *isOpen = result;
    return result;
#else
    if (isOpen) *isOpen = true;
    (void)label;
    (void)defaultOpen;
    return true;
#endif
}

void SectionHeader(const char* label)
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
    ImGui::Text("%s", label);
    ImGui::PopStyleColor();
    ImGui::Separator();
    ImGui::Spacing();
#else
    (void)label;
#endif
}

void Separator(const char* label)
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    if (label)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
        ImGui::Text("%s", label);
        ImGui::PopStyleColor();
    }
    ImGui::Separator();
#else
    (void)label;
#endif
}

bool ToolbarButton(const char* icon, const char* tooltip, bool selected)
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    ImGui::PushID(icon);

    if (selected)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.4f, 0.7f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.5f, 0.8f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.35f, 0.6f, 1.0f));
    }

    bool clicked = ImGui::Button(icon, ImVec2(24, 24));

    if (selected)
    {
        ImGui::PopStyleColor(3);
    }

    if (tooltip && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
    {
        ImGui::SetTooltip("%s", tooltip);
    }

    ImGui::SameLine();
    ImGui::PopID();
    return clicked;
#else
    (void)icon;
    (void)tooltip;
    (void)selected;
    return false;
#endif
}

bool ToolbarToggle(const char* icon, bool* value, const char* tooltip)
{
    bool clicked = ToolbarButton(icon, tooltip, *value);
    if (clicked)
    {
        *value = !*value;
    }
    return clicked;
}

void ToolbarSeparator()
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    ImGui::SameLine();
    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
    ImGui::SameLine();
#endif
}

bool SearchInput(const char* label, char* buffer, size_t bufferSize, const char* hint)
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    ImGui::PushItemWidth(-1);
    bool changed = ImGui::InputTextWithHint(label, hint, buffer, bufferSize);
    ImGui::PopItemWidth();
    return changed;
#else
    (void)label;
    (void)buffer;
    (void)bufferSize;
    (void)hint;
    return false;
#endif
}

bool ColorPickerButton(const char* label, f32* color4, bool showAlpha)
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    ImGuiColorEditFlags flags = ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel;
    if (!showAlpha) flags |= ImGuiColorEditFlags_NoAlpha;

    bool changed = false;
    ImGui::PushID(label);

    // Color preview button
    if (ImGui::ColorButton("##preview", ImVec4(color4[0], color4[1], color4[2], color4[3]), flags, ImVec2(20, 20)))
    {
        ImGui::OpenPopup("##colorpicker");
    }

    // Color picker popup
    if (ImGui::BeginPopup("##colorpicker"))
    {
        if (showAlpha)
        {
            changed = ImGui::ColorPicker4("##picker", color4, ImGuiColorEditFlags_AlphaBar);
        }
        else
        {
            changed = ImGui::ColorPicker3("##picker", color4);
        }
        ImGui::EndPopup();
    }

    ImGui::SameLine();
    ImGui::Text("%s", label);
    ImGui::PopID();
    return changed;
#else
    (void)label;
    (void)color4;
    (void)showAlpha;
    return false;
#endif
}

bool Vector2Input(const char* label, f32* values, f32 speed)
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    ImGui::PushID(label);
    ImGui::PushMultiItemsWidths(2, ImGui::CalcItemWidth());

    bool changed = false;

    // X component
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.5f, 0.2f, 0.2f, 1.0f));
    changed |= ImGui::DragFloat("##X", &values[0], speed, 0.0f, 0.0f, "X: %.2f");
    ImGui::PopStyleColor();
    ImGui::PopItemWidth();
    ImGui::SameLine();

    // Y component
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2f, 0.5f, 0.2f, 1.0f));
    changed |= ImGui::DragFloat("##Y", &values[1], speed, 0.0f, 0.0f, "Y: %.2f");
    ImGui::PopStyleColor();
    ImGui::PopItemWidth();

    ImGui::SameLine();
    ImGui::Text("%s", label);
    ImGui::PopID();
    return changed;
#else
    (void)label;
    (void)values;
    (void)speed;
    return false;
#endif
}

bool Vector3Input(const char* label, f32* values, f32 speed)
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    ImGui::PushID(label);
    ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());

    bool changed = false;

    // X component
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.5f, 0.2f, 0.2f, 1.0f));
    changed |= ImGui::DragFloat("##X", &values[0], speed, 0.0f, 0.0f, "X: %.2f");
    ImGui::PopStyleColor();
    ImGui::PopItemWidth();
    ImGui::SameLine();

    // Y component
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2f, 0.5f, 0.2f, 1.0f));
    changed |= ImGui::DragFloat("##Y", &values[1], speed, 0.0f, 0.0f, "Y: %.2f");
    ImGui::PopStyleColor();
    ImGui::PopItemWidth();
    ImGui::SameLine();

    // Z component
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2f, 0.2f, 0.5f, 1.0f));
    changed |= ImGui::DragFloat("##Z", &values[2], speed, 0.0f, 0.0f, "Z: %.2f");
    ImGui::PopStyleColor();
    ImGui::PopItemWidth();

    ImGui::SameLine();
    ImGui::Text("%s", label);
    ImGui::PopID();
    return changed;
#else
    (void)label;
    (void)values;
    (void)speed;
    return false;
#endif
}

bool AssetReference(const char* label, std::string& assetPath, const char* assetType)
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    bool changed = false;
    ImGui::PushID(label);

    char buffer[256];
    std::strncpy(buffer, assetPath.c_str(), sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    float buttonWidth = 30.0f;
    ImGui::SetNextItemWidth(ImGui::CalcItemWidth() - buttonWidth - ImGui::GetStyle().ItemSpacing.x);

    if (ImGui::InputText("##path", buffer, sizeof(buffer)))
    {
        assetPath = buffer;
        changed = true;
    }

    ImGui::SameLine();
    if (ImGui::Button("...", ImVec2(buttonWidth, 0)))
    {
        // Would open asset picker dialog
        // For now, just a placeholder
    }

    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("Browse for %s", assetType);
    }

    // Drag-drop target
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PATH"))
        {
            const char* droppedPath = static_cast<const char*>(payload->Data);
            assetPath = droppedPath;
            changed = true;
        }
        ImGui::EndDragDropTarget();
    }

    ImGui::SameLine();
    ImGui::Text("%s", label);
    ImGui::PopID();
    return changed;
#else
    (void)label;
    (void)assetPath;
    (void)assetType;
    return false;
#endif
}

bool Dropdown(const char* label, i32* currentIndex, const std::vector<std::string>& items)
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    bool changed = false;
    const char* preview = (*currentIndex >= 0 && static_cast<size_t>(*currentIndex) < items.size()) ?
        items[static_cast<size_t>(*currentIndex)].c_str() : "";

    if (ImGui::BeginCombo(label, preview))
    {
        for (size_t i = 0; i < items.size(); i++)
        {
            bool selected = (static_cast<size_t>(*currentIndex) == i);
            if (ImGui::Selectable(items[i].c_str(), selected))
            {
                *currentIndex = static_cast<i32>(i);
                changed = true;
            }
            if (selected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    return changed;
#else
    (void)label;
    (void)currentIndex;
    (void)items;
    return false;
#endif
}

bool TreeNode(const char* label, bool isLeaf, bool isSelected,
              const char* dragDropType, void* dragDropData)
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow |
                                ImGuiTreeNodeFlags_OpenOnDoubleClick |
                                ImGuiTreeNodeFlags_SpanAvailWidth;
    if (isLeaf) flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    if (isSelected) flags |= ImGuiTreeNodeFlags_Selected;

    bool isOpen = ImGui::TreeNodeEx(label, flags);

    // Handle drag source
    if (dragDropType && dragDropData && ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
    {
        ImGui::SetDragDropPayload(dragDropType, &dragDropData, sizeof(void*));
        ImGui::Text("%s", label);
        ImGui::EndDragDropSource();
    }

    // Handle drag target
    if (dragDropType && ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(dragDropType))
        {
            // Payload accepted - caller would handle via callback
        }
        ImGui::EndDragDropTarget();
    }

    // Handle click selection
    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
    {
        // Selection would be handled by caller
    }

    return isOpen;
#else
    (void)label;
    (void)isLeaf;
    (void)isSelected;
    (void)dragDropType;
    (void)dragDropData;
    return false;
#endif
}

void TimelineRuler(f32 startTime, f32 endTime, f32 currentTime,
                   f32& viewStart, f32& viewEnd)
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    ImVec2 canvasSize = ImGui::GetContentRegionAvail();
    canvasSize.y = 30.0f; // Fixed ruler height

    // Draw ruler background
    drawList->AddRectFilled(canvasPos,
                            ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y),
                            IM_COL32(40, 40, 40, 255));

    // Calculate time range
    f32 timeRange = viewEnd - viewStart;
    if (timeRange <= 0) timeRange = endTime - startTime;

    // Draw time markers
    f32 majorInterval = 1.0f; // 1 second major ticks
    if (timeRange > 10.0f) majorInterval = 5.0f;
    if (timeRange > 60.0f) majorInterval = 10.0f;

    for (f32 t = std::floor(viewStart / majorInterval) * majorInterval; t <= viewEnd; t += majorInterval)
    {
        if (t < viewStart) continue;

        f32 x = canvasPos.x + ((t - viewStart) / timeRange) * canvasSize.x;

        // Major tick
        drawList->AddLine(ImVec2(x, canvasPos.y + canvasSize.y - 10),
                          ImVec2(x, canvasPos.y + canvasSize.y),
                          IM_COL32(150, 150, 150, 255), 1.0f);

        // Time label
        char timeLabel[16];
        int minutes = static_cast<int>(t) / 60;
        int seconds = static_cast<int>(t) % 60;
        std::snprintf(timeLabel, sizeof(timeLabel), "%d:%02d", minutes, seconds);
        drawList->AddText(ImVec2(x + 2, canvasPos.y + 2), IM_COL32(200, 200, 200, 255), timeLabel);
    }

    // Draw current time playhead
    f32 playheadX = canvasPos.x + ((currentTime - viewStart) / timeRange) * canvasSize.x;
    if (playheadX >= canvasPos.x && playheadX <= canvasPos.x + canvasSize.x)
    {
        drawList->AddLine(ImVec2(playheadX, canvasPos.y),
                          ImVec2(playheadX, canvasPos.y + canvasSize.y),
                          IM_COL32(255, 100, 100, 255), 2.0f);
        // Playhead triangle
        drawList->AddTriangleFilled(
            ImVec2(playheadX - 5, canvasPos.y),
            ImVec2(playheadX + 5, canvasPos.y),
            ImVec2(playheadX, canvasPos.y + 8),
            IM_COL32(255, 100, 100, 255));
    }

    // Handle invisible button for scrubbing
    ImGui::SetCursorScreenPos(canvasPos);
    ImGui::InvisibleButton("##ruler", canvasSize);
    if (ImGui::IsItemActive())
    {
        f32 mouseX = ImGui::GetMousePos().x - canvasPos.x;
        // currentTime = viewStart + (mouseX / canvasSize.x) * timeRange;
        // Caller would handle updating current time
    }

    (void)startTime;
    (void)endTime;
#else
    (void)startTime;
    (void)endTime;
    (void)currentTime;
    (void)viewStart;
    (void)viewEnd;
#endif
}

bool KeyframeMarker(f32 time, bool selected, renderer::Color color)
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();

    f32 size = selected ? 8.0f : 6.0f;
    ImU32 fillColor = IM_COL32(color.r, color.g, color.b, color.a);
    ImU32 borderColor = selected ? IM_COL32(255, 255, 255, 255) : IM_COL32(80, 80, 80, 255);

    // Draw diamond shape
    ImVec2 center(pos.x + size, pos.y + size);
    drawList->AddQuadFilled(
        ImVec2(center.x, center.y - size),
        ImVec2(center.x + size, center.y),
        ImVec2(center.x, center.y + size),
        ImVec2(center.x - size, center.y),
        fillColor);
    drawList->AddQuad(
        ImVec2(center.x, center.y - size),
        ImVec2(center.x + size, center.y),
        ImVec2(center.x, center.y + size),
        ImVec2(center.x - size, center.y),
        borderColor, 1.5f);

    // Handle click
    ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y));
    ImGui::InvisibleButton("##keyframe", ImVec2(size * 2, size * 2));
    bool clicked = ImGui::IsItemClicked();

    (void)time;
    return clicked;
#else
    (void)time;
    (void)selected;
    (void)color;
    return false;
#endif
}

void NodeGraphMinimap(f32 x, f32 y, f32 width, f32 height,
                      const std::vector<std::pair<f32, f32>>& nodePositions)
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 canvasPos(x, y);
    ImVec2 canvasSize(width, height);

    // Draw minimap background
    drawList->AddRectFilled(canvasPos,
                            ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y),
                            IM_COL32(30, 30, 30, 200));
    drawList->AddRect(canvasPos,
                      ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y),
                      IM_COL32(80, 80, 80, 255));

    if (nodePositions.empty()) return;

    // Calculate bounds
    f32 minX = nodePositions[0].first, maxX = minX;
    f32 minY = nodePositions[0].second, maxY = minY;
    for (const auto& [nx, ny] : nodePositions)
    {
        minX = std::min(minX, nx);
        maxX = std::max(maxX, nx);
        minY = std::min(minY, ny);
        maxY = std::max(maxY, ny);
    }

    f32 rangeX = maxX - minX;
    f32 rangeY = maxY - minY;
    if (rangeX < 1.0f) rangeX = 1.0f;
    if (rangeY < 1.0f) rangeY = 1.0f;

    // Draw node markers
    for (const auto& [nx, ny] : nodePositions)
    {
        f32 mx = canvasPos.x + ((nx - minX) / rangeX) * (canvasSize.x - 10) + 5;
        f32 my = canvasPos.y + ((ny - minY) / rangeY) * (canvasSize.y - 10) + 5;
        drawList->AddRectFilled(ImVec2(mx - 2, my - 2), ImVec2(mx + 2, my + 2),
                                IM_COL32(0, 122, 204, 255));
    }
#else
    (void)x; (void)y; (void)width; (void)height;
    (void)nodePositions;
#endif
}

bool Splitter(bool splitVertically, f32 thickness, f32* size1, f32* size2,
              f32 minSize1, f32 minSize2)
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    ImGuiID id = window->GetID("##Splitter");

    ImRect bb;
    bb.Min = window->DC.CursorPos + (splitVertically ? ImVec2(*size1, 0.0f) : ImVec2(0.0f, *size1));
    bb.Max = bb.Min + (splitVertically ? ImVec2(thickness, window->Size.y) : ImVec2(window->Size.x, thickness));

    return ImGui::SplitterBehavior(bb, id, splitVertically ? ImGuiAxis_X : ImGuiAxis_Y, size1, size2, minSize1, minSize2, 0.0f);
#else
    (void)splitVertically;
    (void)thickness;
    (void)size1;
    (void)size2;
    (void)minSize1;
    (void)minSize2;
    return false;
#endif
}

void BeginToolbar(const char* id, f32 height)
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    ImGui::PushID(id);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 4));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 4));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));

    // Draw toolbar background
    ImVec2 pos = ImGui::GetCursorPos();
    ImVec2 size = ImVec2(ImGui::GetContentRegionAvail().x, height);
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 screenPos = ImGui::GetCursorScreenPos();
    drawList->AddRectFilled(screenPos, ImVec2(screenPos.x + size.x, screenPos.y + size.y),
                            IM_COL32(45, 45, 45, 255));

    ImGui::SetCursorPosY(pos.y + 3);
#else
    (void)id;
    (void)height;
#endif
}

void EndToolbar()
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    ImGui::PopStyleVar(3);
    ImGui::PopID();
    ImGui::Spacing();
#endif
}

bool BeginPanel(const char* name, bool* open, i32 flags)
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    return ImGui::Begin(name, open, static_cast<ImGuiWindowFlags>(flags));
#else
    (void)name;
    (void)open;
    (void)flags;
    return true;
#endif
}

void EndPanel()
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    ImGui::End();
#endif
}

void RichTooltip(const std::function<void()>& content)
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(350.0f);
        content();
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
#else
    (void)content;
#endif
}

void LoadingSpinner(const char* label, f32 radius, f32 thickness)
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImVec2 center(pos.x + radius, pos.y + radius);

    // Animated rotation
    f32 time = static_cast<f32>(ImGui::GetTime());
    f32 numSegments = 30;
    f32 startAngle = time * 3.0f;
    f32 arcLength = 3.14159f * 1.5f;

    // Draw spinning arc
    for (int i = 0; i < static_cast<int>(numSegments * 0.75f); i++)
    {
        f32 a1 = startAngle + (static_cast<f32>(i) / numSegments) * 2.0f * 3.14159f;
        f32 a2 = startAngle + (static_cast<f32>(i + 1) / numSegments) * 2.0f * 3.14159f;

        if (a2 - startAngle > arcLength) break;

        drawList->AddLine(
            ImVec2(center.x + std::cos(a1) * radius, center.y + std::sin(a1) * radius),
            ImVec2(center.x + std::cos(a2) * radius, center.y + std::sin(a2) * radius),
            IM_COL32(0, 122, 204, 255), thickness);
    }

    ImGui::Dummy(ImVec2(radius * 2, radius * 2));
    if (label && label[0])
    {
        ImGui::SameLine();
        ImGui::Text("%s", label);
    }
#else
    (void)label;
    (void)radius;
    (void)thickness;
#endif
}

void ProgressBarLabeled(f32 progress, const char* label, f32 height)
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    ImGui::ProgressBar(progress, ImVec2(-1, height));
    if (label && label[0])
    {
        ImGui::SameLine();
        ImGui::Text("%s", label);
    }
#else
    (void)progress;
    (void)label;
    (void)height;
#endif
}

void NotificationBadge(i32 count, renderer::Color color)
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    if (count <= 0) return;

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();

    char countStr[16];
    std::snprintf(countStr, sizeof(countStr), "%d", count > 99 ? 99 : count);
    ImVec2 textSize = ImGui::CalcTextSize(countStr);

    f32 badgeRadius = std::max(textSize.x, textSize.y) / 2.0f + 3.0f;
    ImVec2 center(pos.x + badgeRadius, pos.y + badgeRadius);

    // Draw badge circle
    drawList->AddCircleFilled(center, badgeRadius, IM_COL32(color.r, color.g, color.b, color.a));

    // Draw count text
    drawList->AddText(ImVec2(center.x - textSize.x / 2.0f, center.y - textSize.y / 2.0f),
                      IM_COL32(255, 255, 255, 255), countStr);

    ImGui::Dummy(ImVec2(badgeRadius * 2, badgeRadius * 2));
#else
    (void)count;
    (void)color;
#endif
}

} // namespace widgets

} // namespace NovelMind::editor
