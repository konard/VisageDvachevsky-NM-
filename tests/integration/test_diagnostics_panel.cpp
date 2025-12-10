#include <catch2/catch_test_macros.hpp>
#include "NovelMind/editor/diagnostics_panel.hpp"
#include "NovelMind/scripting/script_error.hpp"
#include "NovelMind/scripting/validator.hpp"

using namespace NovelMind;
using namespace NovelMind::editor;
using namespace NovelMind::scripting;

// =============================================================================
// DiagnosticFilter Tests
// =============================================================================

TEST_CASE("DiagnosticFilter - Enum values", "[diagnostic_filter]")
{
    CHECK(static_cast<u8>(DiagnosticFilter::All) == 0);
    CHECK(static_cast<u8>(DiagnosticFilter::Errors) == 1);
    CHECK(static_cast<u8>(DiagnosticFilter::Warnings) == 2);
}

// =============================================================================
// DiagnosticEntry Tests
// =============================================================================

TEST_CASE("DiagnosticEntry - Default values", "[diagnostic_entry]")
{
    DiagnosticEntry entry;

    CHECK(entry.isExpanded == false);
    CHECK(entry.isSelected == false);
    CHECK(entry.filePath.empty());
}

// =============================================================================
// DiagnosticsPanel Tests
// =============================================================================

TEST_CASE("DiagnosticsPanel - Construction", "[diagnostics_panel]")
{
    DiagnosticsPanel panel;
    CHECK(true);  // Just verify construction
}

TEST_CASE("DiagnosticsPanel - Initial counts are zero", "[diagnostics_panel]")
{
    DiagnosticsPanel panel;

    CHECK(panel.getDiagnosticCount() == 0);
    CHECK(panel.getErrorCount() == 0);
    CHECK(panel.getWarningCount() == 0);
}

TEST_CASE("DiagnosticsPanel - Clear does not crash", "[diagnostics_panel]")
{
    DiagnosticsPanel panel;
    panel.clear();
    CHECK(panel.getDiagnosticCount() == 0);
}

TEST_CASE("DiagnosticsPanel - Filter setting", "[diagnostics_panel]")
{
    DiagnosticsPanel panel;

    panel.setFilter(DiagnosticFilter::Errors);
    CHECK(panel.getFilter() == DiagnosticFilter::Errors);

    panel.setFilter(DiagnosticFilter::All);
    CHECK(panel.getFilter() == DiagnosticFilter::All);
}

TEST_CASE("DiagnosticsPanel - Search query", "[diagnostics_panel]")
{
    DiagnosticsPanel panel;

    panel.setSearchQuery("test");
    CHECK(panel.getSearchQuery() == "test");

    panel.setSearchQuery("");
    CHECK(panel.getSearchQuery().empty());
}

TEST_CASE("DiagnosticsPanel - Selection", "[diagnostics_panel]")
{
    DiagnosticsPanel panel;

    // Initially no selection
    CHECK(panel.getSelectedIndex() == -1);

    panel.clearSelection();
    CHECK(panel.getSelectedIndex() == -1);
}

TEST_CASE("DiagnosticsPanel - Display options", "[diagnostics_panel]")
{
    DiagnosticsPanel panel;

    // Just verify these don't crash
    panel.setShowRelatedInfo(true);
    panel.setShowErrorCodes(true);
    panel.setShowFilePaths(true);
    panel.setAutoScrollToErrors(true);

    CHECK(true);  // No crash
}

TEST_CASE("DiagnosticsPanel - Add single diagnostic", "[diagnostics_panel]")
{
    DiagnosticsPanel panel;

    ScriptError error(
        ErrorCode::UndefinedCharacter,
        Severity::Error,
        "Test error",
        SourceLocation(1, 1)
    );

    panel.addDiagnostic(error, "test.nms");

    CHECK(panel.getDiagnosticCount() == 1);
    CHECK(panel.getErrorCount() == 1);
}

TEST_CASE("DiagnosticsPanel - Add diagnostics from ErrorList", "[diagnostics_panel]")
{
    DiagnosticsPanel panel;

    ErrorList errors;
    errors.addError(ErrorCode::UndefinedCharacter, "Error 1", SourceLocation(1, 1));
    errors.addWarning(ErrorCode::UnusedVariable, "Warning 1", SourceLocation(2, 1));

    panel.addDiagnostics(errors, "test.nms");

    CHECK(panel.getDiagnosticCount() == 2);
    CHECK(panel.getErrorCount() == 1);
    CHECK(panel.getWarningCount() == 1);
}

TEST_CASE("DiagnosticsPanel - Set diagnostics from ValidationResult", "[diagnostics_panel]")
{
    DiagnosticsPanel panel;

    // Create a validation result
    ValidationResult result;
    result.isValid = false;
    result.errors.addError(ErrorCode::UndefinedScene, "Scene not found", SourceLocation(5, 1));

    panel.setDiagnostics(result, "script.nms");

    CHECK(panel.getDiagnosticCount() >= 1);
}

// =============================================================================
// DiagnosticsManager Tests
// =============================================================================

TEST_CASE("DiagnosticsManager - Construction", "[diagnostics_manager]")
{
    DiagnosticsManager manager;
    CHECK(true);  // Just verify construction
}

TEST_CASE("DiagnosticsManager - Live validation toggle", "[diagnostics_manager]")
{
    DiagnosticsManager manager;

    // Default should be enabled
    CHECK(manager.isLiveValidationEnabled() == true);

    manager.setLiveValidation(false);
    CHECK(manager.isLiveValidationEnabled() == false);

    manager.setLiveValidation(true);
    CHECK(manager.isLiveValidationEnabled() == true);
}

TEST_CASE("DiagnosticsManager - Validator configuration", "[diagnostics_manager]")
{
    DiagnosticsManager manager;

    // Just verify these don't crash
    manager.setReportUnused(true);
    manager.setReportDeadCode(true);

    CHECK(true);
}
