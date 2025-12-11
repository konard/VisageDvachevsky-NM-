/**
 * @file gui_panel_base.cpp
 * @brief GUI Panel Base class implementation
 */

#include "NovelMind/editor/gui_panel_base.hpp"
#include "NovelMind/editor/imgui_integration.hpp"
#include <algorithm>

#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
#include <imgui.h>
#endif

namespace NovelMind::editor {

// ============================================================================
// GUIPanelBase Implementation
// ============================================================================

GUIPanelBase::GUIPanelBase(const std::string& name)
    : m_name(name)
{
}

GUIPanelBase::~GUIPanelBase()
{
    if (m_initialized)
    {
        shutdown();
    }
}

void GUIPanelBase::initialize(EditorApp* app)
{
    if (m_initialized)
    {
        return;
    }

    m_app = app;

    // Register as selection listener
    EditorSelectionManager::instance().addListener(this);

    onInitialize();
    m_initialized = true;
}

void GUIPanelBase::shutdown()
{
    if (!m_initialized)
    {
        return;
    }

    onShutdown();

    // Unregister from selection listener
    EditorSelectionManager::instance().removeListener(this);

    // Clear event subscriptions
    m_eventSubscriptions.clear();

    m_initialized = false;
    m_app = nullptr;
}

void GUIPanelBase::update(f64 deltaTime)
{
    if (!m_initialized || !m_isOpen)
    {
        return;
    }

    onUpdate(deltaTime);
}

void GUIPanelBase::render()
{
    if (!m_initialized || !m_isOpen)
    {
        return;
    }

    if (beginPanel())
    {
        // Render menu bar if enabled
        if (hasFlag(m_flags, PanelFlags::MenuBar))
        {
            renderMenuBar();
        }

        // Render toolbar
        renderToolbar();

        // Render main content
        onRender();

        // Render footer/status bar
        renderFooter();
    }
    endPanel();
}

void GUIPanelBase::requestFocus()
{
    // Would set ImGui window focus
    // ImGui::SetWindowFocus(m_name.c_str());
}

void GUIPanelBase::onSelectionChanged(SelectionType /*type*/,
                                       const std::vector<SelectionItem>& /*selection*/)
{
    // Override in derived classes if needed
}

void GUIPanelBase::onSelectionCleared()
{
    // Override in derived classes if needed
}

void GUIPanelBase::onPrimarySelectionChanged(const SelectionItem& /*item*/)
{
    // Override in derived classes if needed
}

bool GUIPanelBase::beginPanel()
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    // Convert panel flags to ImGui flags
    ImGuiWindowFlags imguiFlags = 0;
    if (hasFlag(m_flags, PanelFlags::NoTitleBar)) imguiFlags |= ImGuiWindowFlags_NoTitleBar;
    if (hasFlag(m_flags, PanelFlags::NoResize)) imguiFlags |= ImGuiWindowFlags_NoResize;
    if (hasFlag(m_flags, PanelFlags::NoMove)) imguiFlags |= ImGuiWindowFlags_NoMove;
    if (hasFlag(m_flags, PanelFlags::NoScrollbar)) imguiFlags |= ImGuiWindowFlags_NoScrollbar;
    if (hasFlag(m_flags, PanelFlags::NoScrollWithMouse)) imguiFlags |= ImGuiWindowFlags_NoScrollWithMouse;
    if (hasFlag(m_flags, PanelFlags::NoCollapse)) imguiFlags |= ImGuiWindowFlags_NoCollapse;
    if (hasFlag(m_flags, PanelFlags::AlwaysAutoResize)) imguiFlags |= ImGuiWindowFlags_AlwaysAutoResize;
    if (hasFlag(m_flags, PanelFlags::NoBackground)) imguiFlags |= ImGuiWindowFlags_NoBackground;
    if (hasFlag(m_flags, PanelFlags::NoSavedSettings)) imguiFlags |= ImGuiWindowFlags_NoSavedSettings;
    if (hasFlag(m_flags, PanelFlags::MenuBar)) imguiFlags |= ImGuiWindowFlags_MenuBar;
    if (hasFlag(m_flags, PanelFlags::HorizontalScrollbar)) imguiFlags |= ImGuiWindowFlags_HorizontalScrollbar;
    if (hasFlag(m_flags, PanelFlags::NoFocusOnAppearing)) imguiFlags |= ImGuiWindowFlags_NoFocusOnAppearing;
    if (hasFlag(m_flags, PanelFlags::NoBringToFrontOnFocus)) imguiFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
    if (hasFlag(m_flags, PanelFlags::AlwaysVerticalScrollbar)) imguiFlags |= ImGuiWindowFlags_AlwaysVerticalScrollbar;
    if (hasFlag(m_flags, PanelFlags::AlwaysHorizontalScrollbar)) imguiFlags |= ImGuiWindowFlags_AlwaysHorizontalScrollbar;
    if (hasFlag(m_flags, PanelFlags::UnsavedDocument)) imguiFlags |= ImGuiWindowFlags_UnsavedDocument;
    if (hasFlag(m_flags, PanelFlags::NoDocking)) imguiFlags |= ImGuiWindowFlags_NoDocking;

    bool visible = ImGui::Begin(getTitle().c_str(), &m_isOpen, imguiFlags);

    // Track focus and hover state
    m_isFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
    m_isHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows);

    // Track size
    ImVec2 size = ImGui::GetWindowSize();
    ImVec2 contentSize = ImGui::GetContentRegionAvail();
    if (size.x != m_width || size.y != m_height)
    {
        m_width = size.x;
        m_height = size.y;
        m_contentWidth = contentSize.x;
        m_contentHeight = contentSize.y;
        onResize(m_width, m_height);
    }

    return visible;
#else
    // Stub returns true to allow content rendering
    return true;
#endif
}

void GUIPanelBase::endPanel()
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    ImGui::End();
#endif
}

void GUIPanelBase::renderMenuItems(const std::vector<MenuItem>& items)
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    for (const auto& item : items)
    {
        if (item.isSeparator)
        {
            ImGui::Separator();
            continue;
        }

        if (!item.subItems.empty())
        {
            if (ImGui::BeginMenu(item.label.c_str()))
            {
                renderMenuItems(item.subItems);
                ImGui::EndMenu();
            }
        }
        else if (item.isChecked)
        {
            bool checked = item.isChecked();
            if (ImGui::MenuItem(item.label.c_str(), item.shortcut.c_str(),
                                checked, item.isEnabled()))
            {
                if (item.action) item.action();
            }
        }
        else
        {
            if (ImGui::MenuItem(item.label.c_str(), item.shortcut.c_str(),
                                false, item.isEnabled()))
            {
                if (item.action) item.action();
            }
        }
    }
#else
    (void)items;
#endif
}

void GUIPanelBase::renderToolbarItems(const std::vector<ToolbarItem>& items)
{
    for (const auto& item : items)
    {
        if (item.isSeparator)
        {
            widgets::ToolbarSeparator();
            continue;
        }

        bool toggled = item.isToggled ? item.isToggled() : false;
        bool enabled = item.isEnabled();

        // if (!enabled)
        //     ImGui::BeginDisabled();

        if (item.isToggled)
        {
            bool value = toggled;
            if (widgets::ToolbarToggle(item.icon.c_str(), &value, item.tooltip.c_str()))
            {
                if (item.action)
                {
                    item.action();
                }
            }
        }
        else
        {
            if (widgets::ToolbarButton(item.icon.c_str(), item.tooltip.c_str(), toggled))
            {
                if (item.action && enabled)
                {
                    item.action();
                }
            }
        }

        // if (!enabled)
        //     ImGui::EndDisabled();

        // ImGui::SameLine();
        (void)toggled;
        (void)enabled;
    }
}

// ============================================================================
// PanelManager Implementation
// ============================================================================

PanelManager::PanelManager() = default;
PanelManager::~PanelManager() = default;

void PanelManager::registerPanel(std::unique_ptr<GUIPanelBase> panel)
{
    m_panels.push_back(std::move(panel));
}

GUIPanelBase* PanelManager::getPanel(const std::string& name)
{
    auto it = std::find_if(m_panels.begin(), m_panels.end(),
        [&name](const auto& panel) { return panel->getName() == name; });
    return it != m_panels.end() ? it->get() : nullptr;
}

void PanelManager::initializeAll(EditorApp* app)
{
    for (auto& panel : m_panels)
    {
        panel->initialize(app);
    }
}

void PanelManager::shutdownAll()
{
    for (auto& panel : m_panels)
    {
        panel->shutdown();
    }
}

void PanelManager::updateAll(f64 deltaTime)
{
    for (auto& panel : m_panels)
    {
        panel->update(deltaTime);
    }
}

void PanelManager::renderAll()
{
    for (auto& panel : m_panels)
    {
        panel->render();
    }

    // Track focused panel
    for (auto& panel : m_panels)
    {
        if (panel->isFocused() && panel.get() != m_focusedPanel)
        {
            setFocusedPanel(panel.get());
            break;
        }
    }
}

void PanelManager::setFocusedPanel(GUIPanelBase* panel)
{
    if (m_focusedPanel != panel)
    {
        // Notify old panel of focus loss
        if (m_focusedPanel)
        {
            PanelFocusChangedEvent event;
            event.panelName = m_focusedPanel->getName();
            event.hasFocus = false;
            EventBus::instance().publish(event);
        }

        m_focusedPanel = panel;

        // Notify new panel of focus gain
        if (m_focusedPanel)
        {
            PanelFocusChangedEvent event;
            event.panelName = m_focusedPanel->getName();
            event.hasFocus = true;
            EventBus::instance().publish(event);
        }
    }
}

} // namespace NovelMind::editor
