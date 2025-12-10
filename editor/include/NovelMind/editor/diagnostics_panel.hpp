#pragma once

/**
 * @file diagnostics_panel.hpp
 * @brief DiagnosticsPanel - Error and warning display for the editor
 *
 * Provides a comprehensive diagnostics view:
 * - Errors and warnings from AST Validator
 * - Parse errors from lexer/parser
 * - Compilation errors
 * - Filtering by severity and file
 * - Click-to-navigate to source location
 */

#include "NovelMind/editor/editor_app.hpp"
#include "NovelMind/scripting/script_error.hpp"
#include "NovelMind/scripting/validator.hpp"
#include <string>
#include <vector>
#include <functional>

namespace NovelMind::editor
{

/**
 * @brief Filter options for diagnostics
 */
enum class DiagnosticFilter : u8
{
    All,        // Show all diagnostics
    Errors,     // Only errors
    Warnings,   // Only warnings
    Info,       // Only info/hints
    CurrentFile // Only current file
};

/**
 * @brief Represents a diagnostic entry in the panel
 */
struct DiagnosticEntry
{
    scripting::ScriptError error;
    std::string filePath;
    bool isExpanded = false;  // For showing related info
    bool isSelected = false;
};

/**
 * @brief Callback types
 */
using OnDiagnosticClicked = std::function<void(const std::string& filePath,
                                                u32 line, u32 column)>;
using OnDiagnosticDoubleClicked = std::function<void(const std::string& filePath,
                                                      u32 line, u32 column)>;

/**
 * @brief DiagnosticsPanel - Shows errors and warnings
 *
 * This panel displays all compilation diagnostics in a searchable,
 * filterable list. Users can click entries to navigate to the
 * error location in the script editor or story graph.
 *
 * Features:
 * - Error/warning/info icons and coloring
 * - Severity filtering
 * - File filtering
 * - Search by message text
 * - Expandable entries for related information
 * - Click to navigate to source
 * - Auto-refresh on validation
 */
class DiagnosticsPanel : public EditorPanel
{
public:
    DiagnosticsPanel();
    ~DiagnosticsPanel() override = default;

    void update(f64 deltaTime) override;
    void render() override;
    void onResize(i32 width, i32 height) override;

    // =========================================================================
    // Diagnostic Management
    // =========================================================================

    /**
     * @brief Clear all diagnostics
     */
    void clear();

    /**
     * @brief Set diagnostics from validation result
     */
    void setDiagnostics(const scripting::ValidationResult& result,
                        const std::string& filePath = "");

    /**
     * @brief Add diagnostics from error list
     */
    void addDiagnostics(const scripting::ErrorList& errors,
                        const std::string& filePath = "");

    /**
     * @brief Add a single diagnostic
     */
    void addDiagnostic(const scripting::ScriptError& error,
                       const std::string& filePath = "");

    /**
     * @brief Get current diagnostic count
     */
    [[nodiscard]] size_t getDiagnosticCount() const;

    /**
     * @brief Get error count
     */
    [[nodiscard]] size_t getErrorCount() const;

    /**
     * @brief Get warning count
     */
    [[nodiscard]] size_t getWarningCount() const;

    // =========================================================================
    // Filtering
    // =========================================================================

    /**
     * @brief Set the current filter
     */
    void setFilter(DiagnosticFilter filter);

    /**
     * @brief Get current filter
     */
    [[nodiscard]] DiagnosticFilter getFilter() const;

    /**
     * @brief Set the file filter
     */
    void setFileFilter(const std::string& filePath);

    /**
     * @brief Clear file filter
     */
    void clearFileFilter();

    /**
     * @brief Set search query
     */
    void setSearchQuery(const std::string& query);

    /**
     * @brief Get search query
     */
    [[nodiscard]] const std::string& getSearchQuery() const;

    // =========================================================================
    // Selection and Navigation
    // =========================================================================

    /**
     * @brief Select a diagnostic by index
     */
    void selectDiagnostic(size_t index);

    /**
     * @brief Clear selection
     */
    void clearSelection();

    /**
     * @brief Get selected diagnostic index (-1 if none)
     */
    [[nodiscard]] i32 getSelectedIndex() const;

    /**
     * @brief Navigate to next error
     */
    void nextError();

    /**
     * @brief Navigate to previous error
     */
    void prevError();

    /**
     * @brief Navigate to next diagnostic (any severity)
     */
    void nextDiagnostic();

    /**
     * @brief Navigate to previous diagnostic
     */
    void prevDiagnostic();

    // =========================================================================
    // Callbacks
    // =========================================================================

    /**
     * @brief Set callback for single click
     */
    void setOnDiagnosticClicked(OnDiagnosticClicked callback);

    /**
     * @brief Set callback for double click (navigate to source)
     */
    void setOnDiagnosticDoubleClicked(OnDiagnosticDoubleClicked callback);

    // =========================================================================
    // Display Options
    // =========================================================================

    /**
     * @brief Show/hide related information by default
     */
    void setShowRelatedInfo(bool show);

    /**
     * @brief Show/hide error codes
     */
    void setShowErrorCodes(bool show);

    /**
     * @brief Show/hide file paths
     */
    void setShowFilePaths(bool show);

    /**
     * @brief Enable auto-scroll to new errors
     */
    void setAutoScrollToErrors(bool autoScroll);

private:
    // Rendering helpers
    void renderToolbar();
    void renderDiagnosticList();
    void renderDiagnosticEntry(const DiagnosticEntry& entry, size_t index);
    void renderSeverityIcon(scripting::Severity severity);
    void renderSummaryBar();

    // Filtering helpers
    bool matchesFilter(const DiagnosticEntry& entry) const;
    bool matchesSearch(const DiagnosticEntry& entry) const;
    void rebuildFilteredList();

    // All diagnostics
    std::vector<DiagnosticEntry> m_diagnostics;

    // Filtered list (indices into m_diagnostics)
    std::vector<size_t> m_filteredIndices;
    bool m_filterDirty = true;

    // Filter state
    DiagnosticFilter m_filter = DiagnosticFilter::All;
    std::string m_fileFilter;
    std::string m_searchQuery;

    // Selection
    i32 m_selectedIndex = -1;

    // Display options
    bool m_showRelatedInfo = true;
    bool m_showErrorCodes = true;
    bool m_showFilePaths = true;
    bool m_autoScrollToErrors = true;

    // Scroll state
    f32 m_scrollY = 0.0f;
    bool m_needsScroll = false;

    // Callbacks
    OnDiagnosticClicked m_onClicked;
    OnDiagnosticDoubleClicked m_onDoubleClicked;

    // Cached counts
    size_t m_errorCount = 0;
    size_t m_warningCount = 0;
    size_t m_infoCount = 0;
};

/**
 * @brief Helper class to validate scripts and update diagnostics
 *
 * This class provides a convenient interface for live validation
 * that automatically updates the diagnostics panel.
 */
class DiagnosticsManager
{
public:
    DiagnosticsManager();
    ~DiagnosticsManager() = default;

    /**
     * @brief Set the diagnostics panel to update
     */
    void setDiagnosticsPanel(DiagnosticsPanel* panel);

    /**
     * @brief Validate a script and update diagnostics
     */
    scripting::ValidationResult validateScript(const std::string& source,
                                                const std::string& filePath);

    /**
     * @brief Validate an AST and update diagnostics
     */
    scripting::ValidationResult validateAST(const scripting::Program& program,
                                             const std::string& filePath);

    /**
     * @brief Clear all validation diagnostics
     */
    void clearValidation();

    /**
     * @brief Enable live validation (validates on each change)
     */
    void setLiveValidation(bool enabled);

    /**
     * @brief Get whether live validation is enabled
     */
    [[nodiscard]] bool isLiveValidationEnabled() const;

    /**
     * @brief Set the validator configuration
     */
    void setReportUnused(bool report);
    void setReportDeadCode(bool report);

private:
    DiagnosticsPanel* m_panel = nullptr;
    scripting::Validator m_validator;
    bool m_liveValidation = true;
};

} // namespace NovelMind::editor
