/**
 * @file console_panel.cpp
 * @brief Console Panel implementation
 */

#include "NovelMind/editor/console_panel.hpp"
#include "NovelMind/editor/imgui_integration.hpp"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <algorithm>

#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
#include <imgui.h>
#endif

namespace NovelMind::editor {

ConsolePanel::ConsolePanel()
    : GUIPanelBase("Console")
{
}

void ConsolePanel::log(const std::string& message, LogSeverity severity,
                       const std::string& category)
{
    // Get timestamp
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%H:%M:%S");

    // Check for collapse
    if (m_collapse && !m_messages.empty())
    {
        auto& last = m_messages.back();
        if (last.text == message && last.severity == severity && last.category == category)
        {
            last.count++;
            last.timestamp = ss.str();
            return;
        }
    }

    // Add new message
    LogMessage msg;
    msg.text = message;
    msg.severity = severity;
    msg.category = category;
    msg.timestamp = ss.str();

    m_messages.push_back(msg);

    // Update counts
    switch (severity)
    {
        case LogSeverity::Info:
        case LogSeverity::Debug:
            m_infoCount++;
            break;
        case LogSeverity::Warning:
            m_warningCount++;
            break;
        case LogSeverity::Error:
            m_errorCount++;
            break;
    }

    // Trim if too many messages
    while (m_messages.size() > MAX_MESSAGES)
    {
        const auto& front = m_messages.front();
        switch (front.severity)
        {
            case LogSeverity::Info:
            case LogSeverity::Debug:
                m_infoCount -= front.count;
                break;
            case LogSeverity::Warning:
                m_warningCount -= front.count;
                break;
            case LogSeverity::Error:
                m_errorCount -= front.count;
                break;
        }
        m_messages.pop_front();
    }

    if (m_autoScroll)
    {
        m_scrollToBottom = true;
    }
}

void ConsolePanel::log(const std::string& message, const std::string& file, u32 line,
                       LogSeverity severity)
{
    LogMessage msg;
    msg.text = message;
    msg.severity = severity;
    msg.file = file;
    msg.line = line;

    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%H:%M:%S");
    msg.timestamp = ss.str();

    m_messages.push_back(msg);

    switch (severity)
    {
        case LogSeverity::Info:
        case LogSeverity::Debug:
            m_infoCount++;
            break;
        case LogSeverity::Warning:
            m_warningCount++;
            break;
        case LogSeverity::Error:
            m_errorCount++;
            break;
    }

    if (m_autoScroll)
    {
        m_scrollToBottom = true;
    }
}

void ConsolePanel::clear()
{
    m_messages.clear();
    m_infoCount = 0;
    m_warningCount = 0;
    m_errorCount = 0;
}

std::vector<ToolbarItem> ConsolePanel::getToolbarItems() const
{
    std::vector<ToolbarItem> items;

    items.push_back({"Clear", "Clear Console", [this]() { const_cast<ConsolePanel*>(this)->clear(); }});

    items.push_back(ToolbarItem::separator());

    // Filter toggles with counts
    char infoLabel[32];
    snprintf(infoLabel, sizeof(infoLabel), "Info (%d)", m_infoCount);
    items.push_back({infoLabel, "Show Info Messages",
                     [this]() { const_cast<ConsolePanel*>(this)->m_showInfo = !m_showInfo; },
                     []() { return true; }, [this]() { return m_showInfo; }});

    char warnLabel[32];
    snprintf(warnLabel, sizeof(warnLabel), "Warn (%d)", m_warningCount);
    items.push_back({warnLabel, "Show Warnings",
                     [this]() { const_cast<ConsolePanel*>(this)->m_showWarnings = !m_showWarnings; },
                     []() { return true; }, [this]() { return m_showWarnings; }});

    char errLabel[32];
    snprintf(errLabel, sizeof(errLabel), "Error (%d)", m_errorCount);
    items.push_back({errLabel, "Show Errors",
                     [this]() { const_cast<ConsolePanel*>(this)->m_showErrors = !m_showErrors; },
                     []() { return true; }, [this]() { return m_showErrors; }});

    items.push_back(ToolbarItem::separator());

    items.push_back({"Collapse", "Collapse Identical Messages",
                     [this]() { const_cast<ConsolePanel*>(this)->m_collapse = !m_collapse; },
                     []() { return true; }, [this]() { return m_collapse; }});

    return items;
}

void ConsolePanel::onInitialize()
{
    // Subscribe to error events
    subscribeEvent<ErrorEvent>(
        [this](const ErrorEvent& event) {
            LogSeverity severity = LogSeverity::Info;
            switch (event.severity)
            {
                case ErrorEvent::Severity::Info:
                    severity = LogSeverity::Info;
                    break;
                case ErrorEvent::Severity::Warning:
                    severity = LogSeverity::Warning;
                    break;
                case ErrorEvent::Severity::Error:
                case ErrorEvent::Severity::Fatal:
                    severity = LogSeverity::Error;
                    break;
            }

            if (!event.location.empty())
            {
                // Parse "file:line" format
                auto colonPos = event.location.find(':');
                if (colonPos != std::string::npos)
                {
                    std::string file = event.location.substr(0, colonPos);
                    u32 line = static_cast<u32>(std::stoul(event.location.substr(colonPos + 1)));
                    log(event.message, file, line, severity);
                }
                else
                {
                    log(event.message, severity, event.location);
                }
            }
            else
            {
                log(event.message, severity);
            }
        });

    // Add initial message
    log("Console initialized", LogSeverity::Info);
}

void ConsolePanel::onRender()
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    // Search bar
    ImGui::SetNextItemWidth(200);
    widgets::SearchInput("##ConsoleSearch", m_searchBuffer, sizeof(m_searchBuffer), "Filter...");
    m_searchFilter = m_searchBuffer;

    ImGui::SameLine();

    // Auto-scroll toggle
    ImGui::Checkbox("Auto-scroll", &m_autoScroll);

    ImGui::Separator();

    // Messages area
    ImGui::BeginChild("ConsoleMessages", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

    std::string filterLower = m_searchFilter;
    std::transform(filterLower.begin(), filterLower.end(), filterLower.begin(), ::tolower);

    for (const auto& msg : m_messages)
    {
        // Apply severity filter
        if (msg.severity == LogSeverity::Info && !m_showInfo) continue;
        if (msg.severity == LogSeverity::Debug && !m_showDebug) continue;
        if (msg.severity == LogSeverity::Warning && !m_showWarnings) continue;
        if (msg.severity == LogSeverity::Error && !m_showErrors) continue;

        // Apply text filter
        if (!filterLower.empty())
        {
            std::string textLower = msg.text;
            std::transform(textLower.begin(), textLower.end(), textLower.begin(), ::tolower);
            if (textLower.find(filterLower) == std::string::npos)
            {
                continue;
            }
        }

        renderMessage(msg);
    }

    // Auto-scroll
    if (m_scrollToBottom)
    {
        scrollToBottom();
        m_scrollToBottom = false;
    }

    ImGui::EndChild();
#else
    widgets::SearchInput("##ConsoleSearch", m_searchBuffer, sizeof(m_searchBuffer), "Filter...");
    m_searchFilter = m_searchBuffer;

    std::string filterLower = m_searchFilter;
    std::transform(filterLower.begin(), filterLower.end(), filterLower.begin(), ::tolower);

    for (const auto& msg : m_messages)
    {
        if (msg.severity == LogSeverity::Info && !m_showInfo) continue;
        if (msg.severity == LogSeverity::Debug && !m_showDebug) continue;
        if (msg.severity == LogSeverity::Warning && !m_showWarnings) continue;
        if (msg.severity == LogSeverity::Error && !m_showErrors) continue;

        if (!filterLower.empty())
        {
            std::string textLower = msg.text;
            std::transform(textLower.begin(), textLower.end(), textLower.begin(), ::tolower);
            if (textLower.find(filterLower) == std::string::npos)
            {
                continue;
            }
        }

        renderMessage(msg);
    }

    if (m_scrollToBottom)
    {
        scrollToBottom();
        m_scrollToBottom = false;
    }
#endif
}

void ConsolePanel::renderToolbar()
{
    widgets::BeginToolbar("ConsoleToolbar");
    renderToolbarItems(getToolbarItems());
    widgets::EndToolbar();
}

void ConsolePanel::renderMessage(const LogMessage& msg)
{
    renderer::Color color = getSeverityColor(msg.severity);
    const char* icon = getSeverityIcon(msg.severity);

#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    // Format: [time] [icon] message (count) [file:line]
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, 1.0f));

    ImGui::Text("[%s] %s %s", msg.timestamp.c_str(), icon, msg.text.c_str());

    if (msg.count > 1)
    {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "(%d)", msg.count);
    }

    if (!msg.file.empty())
    {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.8f, 1.0f), "[%s:%u]", msg.file.c_str(), msg.line);

        // Double-click to open file
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
        {
            // Would open file in editor - publish event
            // FileOpenRequestEvent event;
            // event.filePath = msg.file;
            // event.line = msg.line;
            // publishEvent(event);
        }
    }

    ImGui::PopStyleColor();
#else
    (void)color;
    (void)icon;
#endif
}

renderer::Color ConsolePanel::getSeverityColor(LogSeverity severity) const
{
    switch (severity)
    {
        case LogSeverity::Info:
            return {200, 200, 200, 255};
        case LogSeverity::Debug:
            return {150, 150, 150, 255};
        case LogSeverity::Warning:
            return {255, 193, 7, 255};
        case LogSeverity::Error:
            return {244, 67, 54, 255};
        default:
            return {200, 200, 200, 255};
    }
}

const char* ConsolePanel::getSeverityIcon(LogSeverity severity) const
{
    switch (severity)
    {
        case LogSeverity::Info:
            return "[I]";
        case LogSeverity::Debug:
            return "[D]";
        case LogSeverity::Warning:
            return "[W]";
        case LogSeverity::Error:
            return "[E]";
        default:
            return "[ ]";
    }
}

void ConsolePanel::scrollToBottom()
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    ImGui::SetScrollHereY(1.0f);
#endif
}

} // namespace NovelMind::editor
