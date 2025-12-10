/**
 * @file diagnostics_panel.cpp
 * @brief DiagnosticsPanel implementation
 */

#include "NovelMind/editor/diagnostics_panel.hpp"
#include "NovelMind/scripting/lexer.hpp"
#include "NovelMind/scripting/parser.hpp"
#include <algorithm>

namespace NovelMind::editor
{

// ============================================================================
// DiagnosticsPanel
// ============================================================================

DiagnosticsPanel::DiagnosticsPanel()
    : EditorPanel("Diagnostics")
{
}

void DiagnosticsPanel::update(f64 /*deltaTime*/)
{
    // Rebuild filtered list if needed
    if (m_filterDirty)
    {
        rebuildFilteredList();
        m_filterDirty = false;
    }
}

void DiagnosticsPanel::render()
{
    if (!m_visible)
    {
        return;
    }

    renderToolbar();
    renderSummaryBar();
    renderDiagnosticList();
}

void DiagnosticsPanel::onResize(i32 width, i32 height)
{
    m_width = width;
    m_height = height;
}

// ============================================================================
// Diagnostic Management
// ============================================================================

void DiagnosticsPanel::clear()
{
    m_diagnostics.clear();
    m_filteredIndices.clear();
    m_selectedIndex = -1;
    m_errorCount = 0;
    m_warningCount = 0;
    m_infoCount = 0;
    m_filterDirty = true;
}

void DiagnosticsPanel::setDiagnostics(const scripting::ValidationResult& result,
                                       const std::string& filePath)
{
    clear();
    addDiagnostics(result.errors, filePath);
}

void DiagnosticsPanel::addDiagnostics(const scripting::ErrorList& errors,
                                       const std::string& filePath)
{
    for (const auto& error : errors.all())
    {
        addDiagnostic(error, filePath);
    }
}

void DiagnosticsPanel::addDiagnostic(const scripting::ScriptError& error,
                                      const std::string& filePath)
{
    DiagnosticEntry entry;
    entry.error = error;
    entry.filePath = filePath;
    entry.isExpanded = false;
    entry.isSelected = false;

    m_diagnostics.push_back(std::move(entry));

    // Update counts
    switch (error.severity)
    {
        case scripting::Severity::Error:
            ++m_errorCount;
            break;
        case scripting::Severity::Warning:
            ++m_warningCount;
            break;
        case scripting::Severity::Info:
        case scripting::Severity::Hint:
            ++m_infoCount;
            break;
    }

    m_filterDirty = true;

    // Auto-scroll to new errors
    if (m_autoScrollToErrors && error.severity == scripting::Severity::Error)
    {
        m_needsScroll = true;
    }
}

size_t DiagnosticsPanel::getDiagnosticCount() const
{
    return m_diagnostics.size();
}

size_t DiagnosticsPanel::getErrorCount() const
{
    return m_errorCount;
}

size_t DiagnosticsPanel::getWarningCount() const
{
    return m_warningCount;
}

// ============================================================================
// Filtering
// ============================================================================

void DiagnosticsPanel::setFilter(DiagnosticFilter filter)
{
    if (m_filter != filter)
    {
        m_filter = filter;
        m_filterDirty = true;
    }
}

DiagnosticFilter DiagnosticsPanel::getFilter() const
{
    return m_filter;
}

void DiagnosticsPanel::setFileFilter(const std::string& filePath)
{
    if (m_fileFilter != filePath)
    {
        m_fileFilter = filePath;
        m_filterDirty = true;
    }
}

void DiagnosticsPanel::clearFileFilter()
{
    if (!m_fileFilter.empty())
    {
        m_fileFilter.clear();
        m_filterDirty = true;
    }
}

void DiagnosticsPanel::setSearchQuery(const std::string& query)
{
    if (m_searchQuery != query)
    {
        m_searchQuery = query;
        m_filterDirty = true;
    }
}

const std::string& DiagnosticsPanel::getSearchQuery() const
{
    return m_searchQuery;
}

// ============================================================================
// Selection and Navigation
// ============================================================================

void DiagnosticsPanel::selectDiagnostic(size_t index)
{
    if (index < m_filteredIndices.size())
    {
        // Clear previous selection
        if (m_selectedIndex >= 0 &&
            m_selectedIndex < static_cast<i32>(m_filteredIndices.size()))
        {
            m_diagnostics[m_filteredIndices[m_selectedIndex]].isSelected = false;
        }

        m_selectedIndex = static_cast<i32>(index);
        m_diagnostics[m_filteredIndices[index]].isSelected = true;

        // Fire click callback
        if (m_onClicked)
        {
            const auto& entry = m_diagnostics[m_filteredIndices[index]];
            m_onClicked(entry.filePath,
                       entry.error.span.start.line,
                       entry.error.span.start.column);
        }
    }
}

void DiagnosticsPanel::clearSelection()
{
    if (m_selectedIndex >= 0 &&
        m_selectedIndex < static_cast<i32>(m_filteredIndices.size()))
    {
        m_diagnostics[m_filteredIndices[m_selectedIndex]].isSelected = false;
    }
    m_selectedIndex = -1;
}

i32 DiagnosticsPanel::getSelectedIndex() const
{
    return m_selectedIndex;
}

void DiagnosticsPanel::nextError()
{
    // Find next error starting from current selection
    size_t start = (m_selectedIndex >= 0)
                   ? static_cast<size_t>(m_selectedIndex + 1)
                   : 0;

    for (size_t i = start; i < m_filteredIndices.size(); ++i)
    {
        const auto& entry = m_diagnostics[m_filteredIndices[i]];
        if (entry.error.severity == scripting::Severity::Error)
        {
            selectDiagnostic(i);
            return;
        }
    }

    // Wrap around
    for (size_t i = 0; i < start && i < m_filteredIndices.size(); ++i)
    {
        const auto& entry = m_diagnostics[m_filteredIndices[i]];
        if (entry.error.severity == scripting::Severity::Error)
        {
            selectDiagnostic(i);
            return;
        }
    }
}

void DiagnosticsPanel::prevError()
{
    // Find previous error starting from current selection
    i32 start = (m_selectedIndex > 0)
                ? m_selectedIndex - 1
                : static_cast<i32>(m_filteredIndices.size()) - 1;

    for (i32 i = start; i >= 0; --i)
    {
        const auto& entry = m_diagnostics[m_filteredIndices[i]];
        if (entry.error.severity == scripting::Severity::Error)
        {
            selectDiagnostic(static_cast<size_t>(i));
            return;
        }
    }

    // Wrap around
    for (i32 i = static_cast<i32>(m_filteredIndices.size()) - 1; i > start; --i)
    {
        const auto& entry = m_diagnostics[m_filteredIndices[i]];
        if (entry.error.severity == scripting::Severity::Error)
        {
            selectDiagnostic(static_cast<size_t>(i));
            return;
        }
    }
}

void DiagnosticsPanel::nextDiagnostic()
{
    if (m_filteredIndices.empty())
    {
        return;
    }

    i32 nextIndex = m_selectedIndex + 1;
    if (nextIndex >= static_cast<i32>(m_filteredIndices.size()))
    {
        nextIndex = 0;
    }
    selectDiagnostic(static_cast<size_t>(nextIndex));
}

void DiagnosticsPanel::prevDiagnostic()
{
    if (m_filteredIndices.empty())
    {
        return;
    }

    i32 prevIndex = m_selectedIndex - 1;
    if (prevIndex < 0)
    {
        prevIndex = static_cast<i32>(m_filteredIndices.size()) - 1;
    }
    selectDiagnostic(static_cast<size_t>(prevIndex));
}

// ============================================================================
// Callbacks
// ============================================================================

void DiagnosticsPanel::setOnDiagnosticClicked(OnDiagnosticClicked callback)
{
    m_onClicked = std::move(callback);
}

void DiagnosticsPanel::setOnDiagnosticDoubleClicked(OnDiagnosticDoubleClicked callback)
{
    m_onDoubleClicked = std::move(callback);
}

// ============================================================================
// Display Options
// ============================================================================

void DiagnosticsPanel::setShowRelatedInfo(bool show)
{
    m_showRelatedInfo = show;
}

void DiagnosticsPanel::setShowErrorCodes(bool show)
{
    m_showErrorCodes = show;
}

void DiagnosticsPanel::setShowFilePaths(bool show)
{
    m_showFilePaths = show;
}

void DiagnosticsPanel::setAutoScrollToErrors(bool autoScroll)
{
    m_autoScrollToErrors = autoScroll;
}

// ============================================================================
// Rendering Helpers
// ============================================================================

void DiagnosticsPanel::renderToolbar()
{
    // Render filter buttons
    // [All] [Errors] [Warnings] [Info] | [Search: ________]
    // This would use the UI framework to render buttons
}

void DiagnosticsPanel::renderDiagnosticList()
{
    // Render scrollable list of diagnostics
    for (size_t i = 0; i < m_filteredIndices.size(); ++i)
    {
        renderDiagnosticEntry(m_diagnostics[m_filteredIndices[i]], i);
    }
}

void DiagnosticsPanel::renderDiagnosticEntry(const DiagnosticEntry& entry, size_t index)
{
    // Render a single diagnostic entry
    // [Icon] [Line:Col] Message [E####]
    //   └─ Related: ...

    renderSeverityIcon(entry.error.severity);

    // Location
    std::string location = std::to_string(entry.error.span.start.line)
                         + ":" + std::to_string(entry.error.span.start.column);

    // File path (if showing)
    if (m_showFilePaths && !entry.filePath.empty())
    {
        // Show abbreviated path
    }

    // Message
    // Would render entry.error.message

    // Error code (if showing)
    if (m_showErrorCodes)
    {
        // Would render [E####] badge
    }

    // Related information (if expanded and has related info)
    if (entry.isExpanded && m_showRelatedInfo && !entry.error.relatedInfo.empty())
    {
        for (const auto& related : entry.error.relatedInfo)
        {
            // Render related info with indentation
            (void)related;
        }
    }

    // Handle selection state
    if (entry.isSelected)
    {
        // Highlight the entry
    }

    (void)index;
    (void)location;
}

void DiagnosticsPanel::renderSeverityIcon(scripting::Severity severity)
{
    // Render appropriate icon based on severity
    // Error: red X
    // Warning: yellow triangle
    // Info: blue i
    // Hint: lightbulb
    (void)severity;
}

void DiagnosticsPanel::renderSummaryBar()
{
    // Render summary at bottom
    // "5 errors, 3 warnings, 2 hints"
}

// ============================================================================
// Filtering Helpers
// ============================================================================

bool DiagnosticsPanel::matchesFilter(const DiagnosticEntry& entry) const
{
    // Check severity filter
    switch (m_filter)
    {
        case DiagnosticFilter::All:
            break;

        case DiagnosticFilter::Errors:
            if (entry.error.severity != scripting::Severity::Error)
            {
                return false;
            }
            break;

        case DiagnosticFilter::Warnings:
            if (entry.error.severity != scripting::Severity::Warning)
            {
                return false;
            }
            break;

        case DiagnosticFilter::Info:
            if (entry.error.severity != scripting::Severity::Info &&
                entry.error.severity != scripting::Severity::Hint)
            {
                return false;
            }
            break;

        case DiagnosticFilter::CurrentFile:
            if (!m_fileFilter.empty() && entry.filePath != m_fileFilter)
            {
                return false;
            }
            break;
    }

    // Check file filter (if not using CurrentFile mode)
    if (m_filter != DiagnosticFilter::CurrentFile &&
        !m_fileFilter.empty() &&
        entry.filePath != m_fileFilter)
    {
        return false;
    }

    return true;
}

bool DiagnosticsPanel::matchesSearch(const DiagnosticEntry& entry) const
{
    if (m_searchQuery.empty())
    {
        return true;
    }

    // Case-insensitive search in message
    std::string message = entry.error.message;
    std::string query = m_searchQuery;

    // Convert to lowercase for comparison
    std::transform(message.begin(), message.end(), message.begin(), ::tolower);
    std::transform(query.begin(), query.end(), query.begin(), ::tolower);

    return message.find(query) != std::string::npos;
}

void DiagnosticsPanel::rebuildFilteredList()
{
    m_filteredIndices.clear();

    for (size_t i = 0; i < m_diagnostics.size(); ++i)
    {
        if (matchesFilter(m_diagnostics[i]) && matchesSearch(m_diagnostics[i]))
        {
            m_filteredIndices.push_back(i);
        }
    }

    // Sort by severity (errors first), then by line number
    std::sort(m_filteredIndices.begin(), m_filteredIndices.end(),
        [this](size_t a, size_t b)
        {
            const auto& entryA = m_diagnostics[a];
            const auto& entryB = m_diagnostics[b];

            // Errors first
            if (entryA.error.severity != entryB.error.severity)
            {
                return static_cast<u8>(entryA.error.severity) >
                       static_cast<u8>(entryB.error.severity);
            }

            // Then by file
            if (entryA.filePath != entryB.filePath)
            {
                return entryA.filePath < entryB.filePath;
            }

            // Then by line
            return entryA.error.span.start.line < entryB.error.span.start.line;
        });

    // Reset selection if it's now out of bounds
    if (m_selectedIndex >= static_cast<i32>(m_filteredIndices.size()))
    {
        m_selectedIndex = -1;
    }
}

// ============================================================================
// DiagnosticsManager
// ============================================================================

DiagnosticsManager::DiagnosticsManager()
{
}

void DiagnosticsManager::setDiagnosticsPanel(DiagnosticsPanel* panel)
{
    m_panel = panel;
}

scripting::ValidationResult DiagnosticsManager::validateScript(
    const std::string& source,
    const std::string& filePath)
{
    (void)filePath;  // Reserved for future use

    // Parse the script first
    scripting::Lexer lexer;
    auto tokensResult = lexer.tokenize(source);

    scripting::ValidationResult result;
    result.isValid = false;

    if (!tokensResult.isOk())
    {
        // Add lexer error
        scripting::ScriptError error(
            scripting::ErrorCode::InvalidSyntax,
            scripting::Severity::Error,
            tokensResult.error(),
            scripting::SourceLocation{1, 1}
        );

        result.errors.add(error);
        return result;
    }

    scripting::Parser parser;
    auto parseResult = parser.parse(tokensResult.value());

    if (!parseResult.isOk())
    {
        // Add parse error
        scripting::ScriptError error(
            scripting::ErrorCode::InvalidSyntax,
            scripting::Severity::Error,
            parseResult.error(),
            scripting::SourceLocation{1, 1}
        );

        result.errors.add(error);

        if (m_panel)
        {
            m_panel->addDiagnostic(error, filePath);
        }

        return result;
    }

    // Validate the AST
    return validateAST(parseResult.value(), filePath);
}

scripting::ValidationResult DiagnosticsManager::validateAST(
    const scripting::Program& program,
    const std::string& filePath)
{
    auto result = m_validator.validate(program);

    if (m_panel)
    {
        m_panel->setDiagnostics(result, filePath);
    }

    return result;
}

void DiagnosticsManager::clearValidation()
{
    if (m_panel)
    {
        m_panel->clear();
    }
}

void DiagnosticsManager::setLiveValidation(bool enabled)
{
    m_liveValidation = enabled;
}

bool DiagnosticsManager::isLiveValidationEnabled() const
{
    return m_liveValidation;
}

void DiagnosticsManager::setReportUnused(bool report)
{
    m_validator.setReportUnused(report);
}

void DiagnosticsManager::setReportDeadCode(bool report)
{
    m_validator.setReportDeadCode(report);
}

} // namespace NovelMind::editor
