/**
 * @file localization_panel.cpp
 * @brief Localization Panel implementation
 */

#include "NovelMind/editor/localization_panel.hpp"
#include "NovelMind/editor/imgui_integration.hpp"
#include "NovelMind/editor/style_guide.hpp"
#include <algorithm>
#include <cstring>

namespace NovelMind::editor {

LocalizationPanel::LocalizationPanel()
    : GUIPanelBase("Localization")
{
    setFlags(PanelFlags::MenuBar);

    // Initialize with common locales
    m_primaryLocale = localization::LocaleId::fromString("en");
    m_secondaryLocale = localization::LocaleId::fromString("ja");
}

void LocalizationPanel::onInitialize()
{
    if (m_locManager) {
        m_availableLocales = m_locManager->getAvailableLocales();
    }
}

void LocalizationPanel::onShutdown()
{
}

void LocalizationPanel::onUpdate(f64 /*deltaTime*/)
{
    if (m_entriesDirty) {
        rebuildEntryList();
        m_entriesDirty = false;
    }

    if (m_filterDirty) {
        applyFilters();
        m_filterDirty = false;
    }
}

void LocalizationPanel::onRender()
{
    // Filter bar at top
    renderFilterBar();

    // Locale selector
    renderLocaleSelector();

    // Main table
    renderTable();

    // Coverage stats at bottom
    renderCoverageStats();

    // Dialogs
    if (m_showAddKeyDialog) {
        renderAddKeyDialog();
    }
    if (m_showImportExportDialog) {
        renderImportExportDialog();
    }
    if (m_showLocaleConfigDialog) {
        renderLocaleConfigDialog();
    }
}

void LocalizationPanel::renderToolbar()
{
    widgets::BeginToolbar("LocalizationToolbar", Heights::Toolbar);

    // Refresh
    if (widgets::ToolbarButton(Icons::Refresh, "Refresh")) {
        refresh();
    }

    widgets::ToolbarSeparator();

    // Add key
    if (widgets::ToolbarButton(Icons::Plus, "Add Key")) {
        m_showAddKeyDialog = true;
    }

    widgets::ToolbarSeparator();

    // Export/Import
    if (widgets::ToolbarButton(Icons::Export, "Export")) {
        m_isExporting = true;
        m_showImportExportDialog = true;
    }
    if (widgets::ToolbarButton(Icons::Import, "Import")) {
        m_isExporting = false;
        m_showImportExportDialog = true;
    }

    widgets::ToolbarSeparator();

    // Locale configuration
    if (widgets::ToolbarButton(Icons::Settings, "Configure Locales")) {
        m_showLocaleConfigDialog = true;
    }

    widgets::EndToolbar();
}

void LocalizationPanel::renderMenuBar()
{
}

void LocalizationPanel::renderFilterBar()
{
    // Search input
    if (widgets::SearchInput("##LocalizationSearch", m_searchBuffer, sizeof(m_searchBuffer),
                             "Search keys or values...")) {
        m_filter.searchText = m_searchBuffer;
        m_filterDirty = true;
    }

    // Status filter dropdown
    std::vector<std::string> statusOptions = {
        "All Strings", "Complete", "Missing", "Outdated", "New"
    };
    i32 statusIndex = m_filter.showAllStatuses ? 0 : static_cast<i32>(m_filter.statusFilter) + 1;
    if (widgets::Dropdown("Status", &statusIndex, statusOptions)) {
        if (statusIndex == 0) {
            m_filter.showAllStatuses = true;
        } else {
            m_filter.showAllStatuses = false;
            m_filter.statusFilter = static_cast<LocalizationStatus>(statusIndex - 1);
        }
        m_filterDirty = true;
    }

    // Show modified only
    if (widgets::ToolbarToggle(Icons::Pin, &m_filter.showOnlyModified, "Show Only Modified")) {
        m_filterDirty = true;
    }

    // Clear filters
    if (widgets::ToolbarButton(Icons::Close, "Clear Filters")) {
        clearAllFilters();
    }
}

void LocalizationPanel::renderLocaleSelector()
{
    // Locale selector bar - shows two dropdowns for side-by-side editing

    // Primary locale dropdown
    std::vector<std::string> localeNames;
    for (const auto& locale : m_availableLocales) {
        localeNames.push_back(locale.toString());
    }

    if (!localeNames.empty()) {
        if (widgets::Dropdown("Source", &m_primaryLocaleIndex, localeNames)) {
            if (m_primaryLocaleIndex >= 0 && m_primaryLocaleIndex < static_cast<i32>(m_availableLocales.size())) {
                m_primaryLocale = m_availableLocales[static_cast<size_t>(m_primaryLocaleIndex)];
                m_entriesDirty = true;
            }
        }

        if (widgets::Dropdown("Target", &m_secondaryLocaleIndex, localeNames)) {
            if (m_secondaryLocaleIndex >= 0 && m_secondaryLocaleIndex < static_cast<i32>(m_availableLocales.size())) {
                m_secondaryLocale = m_availableLocales[static_cast<size_t>(m_secondaryLocaleIndex)];
                m_entriesDirty = true;
            }
        }
    }
}

void LocalizationPanel::renderTable()
{
    // Table header
    renderTableHeader();

    // Table body with virtual scrolling
    f32 rowHeight = Heights::TableRow;
    f32 tableHeight = getPanelHeight() - Heights::Toolbar - 150.0f;  // Minus filter bars and stats
    m_visibleRowCount = static_cast<size_t>(tableHeight / rowHeight) + 2;
    m_visibleRowStart = static_cast<size_t>(m_scrollY / rowHeight);

    if (m_visibleRowStart >= m_filteredIndices.size()) {
        m_visibleRowStart = m_filteredIndices.size() > 0 ? m_filteredIndices.size() - 1 : 0;
    }

    size_t visibleEnd = std::min(m_visibleRowStart + m_visibleRowCount, m_filteredIndices.size());

    for (size_t i = m_visibleRowStart; i < visibleEnd; ++i) {
        size_t entryIndex = m_filteredIndices[i];
        if (entryIndex < m_entries.size()) {
            renderTableRow(m_entries[entryIndex], i);
        }
    }
}

void LocalizationPanel::renderTableHeader()
{
    // Key column header (sortable)
    bool isSortedByKey = (m_sortColumn == SortColumn::Key);
    (void)isSortedByKey; // Reserved for sort indicator rendering
    // Render header with sort arrow if sorted

    // Status column header (sortable)
    bool isSortedByStatus = (m_sortColumn == SortColumn::Status);
    (void)isSortedByStatus; // Reserved for sort indicator rendering
    // Render header

    // Primary locale column header
    // Shows locale name/code

    // Secondary locale column header
    // Shows locale name/code

    // Column resize handles
}

void LocalizationPanel::renderTableRow(const LocalizationEntry& entry, size_t rowIndex)
{
    (void)rowIndex; // Reserved for row UI rendering
    bool isSelected = std::find(m_selectedKeys.begin(), m_selectedKeys.end(), entry.key)
                      != m_selectedKeys.end();
    (void)isSelected; // Reserved for row selection highlighting

    bool isEditing = m_isEditing && m_editingKey == entry.key;

    // Row background (alternating, selected highlight)

    // Key column
    // Render entry.key

    // Status column
    renderStatusBadge(entry.status);

    // Primary locale value
    auto primaryIt = entry.translations.find(m_primaryLocale.toString());
    std::string primaryValue = (primaryIt != entry.translations.end()) ? primaryIt->second : "";

    if (isEditing && m_editingLocale == m_primaryLocale) {
        // Render text input
        // On enter/blur, save edit
    } else {
        // Render text (truncated if long)
        // On double-click, start editing
    }

    // Secondary locale value
    auto secondaryIt = entry.translations.find(m_secondaryLocale.toString());
    std::string secondaryValue = (secondaryIt != entry.translations.end()) ? secondaryIt->second : "";
    bool isMissing = secondaryValue.empty() && !primaryValue.empty();

    if (isEditing && m_editingLocale == m_secondaryLocale) {
        // Render text input
    } else {
        // Render text with missing indicator if empty
        if (isMissing) {
            // Show warning icon or highlight
        }
    }

    // Handle row interactions
}

void LocalizationPanel::renderStatusBadge(LocalizationStatus status)
{
    renderer::Color color;
    const char* text = "";

    switch (status) {
        case LocalizationStatus::Complete:
            color = DarkPalette::LocaleComplete;
            text = "OK";
            break;
        case LocalizationStatus::Missing:
            color = DarkPalette::LocaleMissing;
            text = "Missing";
            break;
        case LocalizationStatus::Outdated:
            color = DarkPalette::LocaleOutdated;
            text = "Outdated";
            break;
        case LocalizationStatus::New:
            color = DarkPalette::Info;
            text = "New";
            break;
    }

    // Render colored badge
}

void LocalizationPanel::renderCoverageStats()
{
    if (!m_locManager) return;

    // Stats for secondary locale (target language)
    f32 coverage = getLocaleCoverage(m_secondaryLocale);
    size_t missing = getMissingCount(m_secondaryLocale);
    (void)missing; // Reserved for stats display
    size_t total = getTotalStringCount();
    (void)total; // Reserved for stats display

    // Stats bar
    // Total: X | Translated: Y | Missing: Z | Coverage: XX%

    // Progress bar
    widgets::ProgressBarLabeled(coverage / 100.0f,
                                m_secondaryLocale.toString().c_str(),
                                Heights::StatusBar);
}

void LocalizationPanel::renderAddKeyDialog()
{
    // Modal dialog for adding a new key
    // - Key input
    // - Default value input (for primary locale)
    // - Context/notes input
    // - Add/Cancel buttons

    // Would use ImGui modal
}

void LocalizationPanel::renderImportExportDialog()
{
    // Modal dialog for import/export
    // - Format selection (JSON/CSV/PO/XLIFF)
    // - Target locale selection
    // - File path
    // - Options (export missing only, merge vs replace)
    // - Run/Cancel

    // Would use ImGui modal
}

void LocalizationPanel::renderLocaleConfigDialog()
{
    // Modal dialog for locale configuration
    // - List of available locales
    // - Add new locale
    // - Remove locale (with confirmation)
    // - Set default locale
    // - Locale display names and settings
}

void LocalizationPanel::setLocalizationManager(localization::LocalizationManager* manager)
{
    m_locManager = manager;
    m_entriesDirty = true;

    if (m_locManager) {
        m_availableLocales = m_locManager->getAvailableLocales();

        // Set default locale selection
        if (!m_availableLocales.empty()) {
            m_primaryLocale = m_locManager->getDefaultLocale();
            for (i32 i = 0; i < static_cast<i32>(m_availableLocales.size()); ++i) {
                if (m_availableLocales[static_cast<size_t>(i)] == m_primaryLocale) {
                    m_primaryLocaleIndex = i;
                    break;
                }
            }

            // Find a different locale for secondary
            if (m_availableLocales.size() > 1) {
                m_secondaryLocaleIndex = (m_primaryLocaleIndex == 0) ? 1 : 0;
                m_secondaryLocale = m_availableLocales[static_cast<size_t>(m_secondaryLocaleIndex)];
            }
        }
    }
}

void LocalizationPanel::refresh()
{
    m_entriesDirty = true;

    if (m_locManager) {
        m_availableLocales = m_locManager->getAvailableLocales();
    }
}

void LocalizationPanel::rebuildEntryList()
{
    m_entries.clear();

    if (!m_locManager) return;

    // Get default locale string table for key list
    const auto* defaultTable = m_locManager->getStringTable(m_locManager->getDefaultLocale());
    if (!defaultTable) return;

    // Build entries from all keys
    auto keys = defaultTable->getStringIds();

    for (const auto& key : keys) {
        LocalizationEntry entry;
        entry.key = key;

        // Get translations from all locales
        for (const auto& locale : m_availableLocales) {
            const auto* table = m_locManager->getStringTable(locale);
            if (table) {
                auto str = table->getString(key);
                if (str) {
                    entry.translations[locale.toString()] = *str;
                }
            }
        }

        // Compute status
        entry.status = computeStatus(key);

        m_entries.push_back(entry);
    }

    m_filterDirty = true;
}

void LocalizationPanel::applyFilters()
{
    m_filteredIndices.clear();

    for (size_t i = 0; i < m_entries.size(); ++i) {
        const auto& entry = m_entries[i];

        // Status filter
        if (!m_filter.showAllStatuses && entry.status != m_filter.statusFilter) {
            continue;
        }

        // Search filter
        if (!m_filter.searchText.empty()) {
            bool found = false;

            // Search in key
            if (entry.key.find(m_filter.searchText) != std::string::npos) {
                found = true;
            }

            // Search in translations
            if (!found) {
                for (const auto& [locale, value] : entry.translations) {
                    if (value.find(m_filter.searchText) != std::string::npos) {
                        found = true;
                        break;
                    }
                }
            }

            if (!found) continue;
        }

        // Modified filter
        if (m_filter.showOnlyModified) {
            bool anyModified = false;
            for (const auto& [locale, modified] : entry.isModified) {
                if (modified) {
                    anyModified = true;
                    break;
                }
            }
            if (!anyModified) continue;
        }

        m_filteredIndices.push_back(i);
    }

    sortEntries();
}

void LocalizationPanel::sortEntries()
{
    std::sort(m_filteredIndices.begin(), m_filteredIndices.end(),
        [this](size_t a, size_t b) {
            const auto& entryA = m_entries[a];
            const auto& entryB = m_entries[b];
            i32 cmp = 0;

            switch (m_sortColumn) {
                case SortColumn::Key:
                    cmp = entryA.key.compare(entryB.key);
                    break;
                case SortColumn::Status:
                    cmp = static_cast<i32>(entryA.status) - static_cast<i32>(entryB.status);
                    break;
                case SortColumn::PrimaryValue: {
                    auto aIt = entryA.translations.find(m_primaryLocale.toString());
                    auto bIt = entryB.translations.find(m_primaryLocale.toString());
                    std::string aVal = (aIt != entryA.translations.end()) ? aIt->second : "";
                    std::string bVal = (bIt != entryB.translations.end()) ? bIt->second : "";
                    cmp = aVal.compare(bVal);
                    break;
                }
                case SortColumn::SecondaryValue: {
                    auto aIt = entryA.translations.find(m_secondaryLocale.toString());
                    auto bIt = entryB.translations.find(m_secondaryLocale.toString());
                    std::string aVal = (aIt != entryA.translations.end()) ? aIt->second : "";
                    std::string bVal = (bIt != entryB.translations.end()) ? bIt->second : "";
                    cmp = aVal.compare(bVal);
                    break;
                }
            }

            return m_sortAscending ? (cmp < 0) : (cmp > 0);
        });
}

LocalizationStatus LocalizationPanel::computeStatus(const std::string& key) const
{
    if (!m_locManager) return LocalizationStatus::Missing;

    bool hasDefault = m_locManager->hasString(m_locManager->getDefaultLocale(), key);
    if (!hasDefault) return LocalizationStatus::New;

    // Check if all locales have the string
    for (const auto& locale : m_availableLocales) {
        if (!m_locManager->hasString(locale, key)) {
            return LocalizationStatus::Missing;
        }
    }

    // Could check for outdated based on modification times, but for now:
    return LocalizationStatus::Complete;
}

std::vector<localization::LocaleId> LocalizationPanel::getLocales() const
{
    return m_availableLocales;
}

void LocalizationPanel::setPrimaryLocale(const localization::LocaleId& locale)
{
    m_primaryLocale = locale;
    for (i32 i = 0; i < static_cast<i32>(m_availableLocales.size()); ++i) {
        if (m_availableLocales[static_cast<size_t>(i)] == locale) {
            m_primaryLocaleIndex = i;
            break;
        }
    }
    m_entriesDirty = true;
}

void LocalizationPanel::setSecondaryLocale(const localization::LocaleId& locale)
{
    m_secondaryLocale = locale;
    for (i32 i = 0; i < static_cast<i32>(m_availableLocales.size()); ++i) {
        if (m_availableLocales[static_cast<size_t>(i)] == locale) {
            m_secondaryLocaleIndex = i;
            break;
        }
    }
    m_entriesDirty = true;
}

void LocalizationPanel::addKey(const std::string& key, const std::string& defaultValue)
{
    if (!m_locManager) return;

    m_locManager->setString(m_locManager->getDefaultLocale(), key, defaultValue);
    m_entriesDirty = true;
}

void LocalizationPanel::removeKey(const std::string& key)
{
    if (!m_locManager) return;

    for (const auto& locale : m_availableLocales) {
        m_locManager->removeString(locale, key);
    }
    m_entriesDirty = true;
}

void LocalizationPanel::setTranslation(const std::string& key, const localization::LocaleId& locale,
                                       const std::string& value)
{
    if (!m_locManager) return;

    m_locManager->setString(locale, key, value);

    // Mark as modified in entries
    for (auto& entry : m_entries) {
        if (entry.key == key) {
            entry.translations[locale.toString()] = value;
            entry.isModified[locale.toString()] = true;
            entry.status = computeStatus(key);
            break;
        }
    }
}

void LocalizationPanel::setSearchText(const std::string& text)
{
    m_filter.searchText = text;
    strncpy(m_searchBuffer, text.c_str(), sizeof(m_searchBuffer) - 1);
    m_filterDirty = true;
}

void LocalizationPanel::setStatusFilter(LocalizationStatus status)
{
    m_filter.statusFilter = status;
    m_filter.showAllStatuses = false;
    m_filterDirty = true;
}

void LocalizationPanel::clearStatusFilter()
{
    m_filter.showAllStatuses = true;
    m_filterDirty = true;
}

void LocalizationPanel::setShowOnlyModified(bool show)
{
    m_filter.showOnlyModified = show;
    m_filterDirty = true;
}

void LocalizationPanel::clearAllFilters()
{
    m_filter = LocalizationFilter{};
    m_searchBuffer[0] = '\0';
    m_filterDirty = true;
}

void LocalizationPanel::selectKey(const std::string& key)
{
    m_selectedKeys.clear();
    m_selectedKeys.push_back(key);
    m_lastSelectedKey = key;
}

void LocalizationPanel::clearSelection()
{
    m_selectedKeys.clear();
    m_lastSelectedKey.clear();
}

void LocalizationPanel::exportStrings(const localization::LocaleId& locale,
                                      const std::string& path,
                                      localization::LocalizationFormat format)
{
    if (!m_locManager) return;
    m_locManager->exportStrings(locale, path, format);
}

void LocalizationPanel::exportMissingStrings(const localization::LocaleId& locale,
                                             const std::string& path,
                                             localization::LocalizationFormat format)
{
    if (!m_locManager) return;
    m_locManager->exportMissingStrings(locale, path, format);
}

void LocalizationPanel::importStrings(const localization::LocaleId& locale,
                                      const std::string& path,
                                      localization::LocalizationFormat format)
{
    if (!m_locManager) return;
    m_locManager->loadStrings(locale, path, format);
    m_entriesDirty = true;
}

size_t LocalizationPanel::getTotalStringCount() const
{
    return m_entries.size();
}

f32 LocalizationPanel::getLocaleCoverage(const localization::LocaleId& locale) const
{
    if (m_entries.empty()) return 100.0f;

    size_t translated = 0;
    std::string localeStr = locale.toString();

    for (const auto& entry : m_entries) {
        auto it = entry.translations.find(localeStr);
        if (it != entry.translations.end() && !it->second.empty()) {
            ++translated;
        }
    }

    return 100.0f * static_cast<f32>(translated) / static_cast<f32>(m_entries.size());
}

size_t LocalizationPanel::getMissingCount(const localization::LocaleId& locale) const
{
    size_t missing = 0;
    std::string localeStr = locale.toString();

    for (const auto& entry : m_entries) {
        auto it = entry.translations.find(localeStr);
        if (it == entry.translations.end() || it->second.empty()) {
            ++missing;
        }
    }

    return missing;
}

std::vector<MenuItem> LocalizationPanel::getMenuItems() const
{
    return {
        {"Refresh", "F5", [this]() { const_cast<LocalizationPanel*>(this)->refresh(); }},
        MenuItem::separator(),
        {"Add Key...", "Ctrl+N", [this]() {
            const_cast<LocalizationPanel*>(this)->m_showAddKeyDialog = true; }},
        {"Remove Key", "Delete", [this]() {
            auto* panel = const_cast<LocalizationPanel*>(this);
            for (const auto& key : panel->m_selectedKeys) {
                panel->removeKey(key);
            }
        }, [this]() { return !m_selectedKeys.empty(); }},
        MenuItem::separator(),
        {"Export...", "Ctrl+E", [this]() {
            auto* panel = const_cast<LocalizationPanel*>(this);
            panel->m_isExporting = true;
            panel->m_showImportExportDialog = true;
        }},
        {"Import...", "Ctrl+I", [this]() {
            auto* panel = const_cast<LocalizationPanel*>(this);
            panel->m_isExporting = false;
            panel->m_showImportExportDialog = true;
        }}
    };
}

std::vector<ToolbarItem> LocalizationPanel::getToolbarItems() const
{
    return {
        {Icons::Refresh, "Refresh", [this]() { const_cast<LocalizationPanel*>(this)->refresh(); }},
        ToolbarItem::separator(),
        {Icons::Plus, "Add Key", [this]() {
            const_cast<LocalizationPanel*>(this)->m_showAddKeyDialog = true; }},
        ToolbarItem::separator(),
        {Icons::Export, "Export", [this]() {
            auto* panel = const_cast<LocalizationPanel*>(this);
            panel->m_isExporting = true;
            panel->m_showImportExportDialog = true;
        }},
        {Icons::Import, "Import", [this]() {
            auto* panel = const_cast<LocalizationPanel*>(this);
            panel->m_isExporting = false;
            panel->m_showImportExportDialog = true;
        }}
    };
}

std::vector<MenuItem> LocalizationPanel::getContextMenuItems() const
{
    std::vector<MenuItem> items;

    if (!m_selectedKeys.empty()) {
        items.push_back({"Copy Key", "Ctrl+C", [this]() {
            // Copy key to clipboard
        }, nullptr, {}});
        items.push_back({"Copy Value", "", [this]() {
            // Copy primary locale value to clipboard
        }, nullptr, {}});
        items.push_back(MenuItem::separator());
        items.push_back({"Remove Key", "Delete", [this]() {
            auto* panel = const_cast<LocalizationPanel*>(this);
            for (const auto& key : panel->m_selectedKeys) {
                panel->removeKey(key);
            }
        }, nullptr, {}});
    }

    return items;
}

} // namespace NovelMind::editor
