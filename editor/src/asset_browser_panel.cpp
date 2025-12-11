/**
 * @file asset_browser_panel.cpp
 * @brief Asset Browser Panel implementation with extended functionality
 */

#include "NovelMind/editor/asset_browser_panel.hpp"
#include "NovelMind/editor/imgui_integration.hpp"
#include <algorithm>
#include <chrono>
#include <cstring>
#include <sstream>
#include <iomanip>

#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
#include <imgui.h>
#endif

namespace NovelMind::editor {

AssetBrowserPanel::AssetBrowserPanel()
    : GUIPanelBase("Asset Browser")
{
}

// =========================================================================
// Navigation
// =========================================================================

void AssetBrowserPanel::navigateTo(const std::string& path)
{
    // Add to history
    if (m_historyIndex < m_history.size())
    {
        m_history.resize(m_historyIndex + 1);
    }
    m_history.push_back(path);
    m_historyIndex = m_history.size() - 1;

    m_currentPath = path;
    loadDirectory(path);

    // Clear selection on navigation
    clearSelection();
}

void AssetBrowserPanel::navigateUp()
{
    std::filesystem::path current(m_currentPath);
    if (current.has_parent_path() && current.parent_path() != current)
    {
        navigateTo(current.parent_path().string());
    }
}

void AssetBrowserPanel::navigateBack()
{
    if (m_historyIndex > 0)
    {
        --m_historyIndex;
        m_currentPath = m_history[m_historyIndex];
        loadDirectory(m_currentPath);
    }
}

void AssetBrowserPanel::navigateForward()
{
    if (m_historyIndex + 1 < m_history.size())
    {
        ++m_historyIndex;
        m_currentPath = m_history[m_historyIndex];
        loadDirectory(m_currentPath);
    }
}

void AssetBrowserPanel::refresh()
{
    loadDirectory(m_currentPath);
}

// =========================================================================
// View Options
// =========================================================================

void AssetBrowserPanel::setThumbnailSize(f32 size)
{
    m_thumbnailSize = std::clamp(size, m_minThumbnailSize, m_maxThumbnailSize);
}

// =========================================================================
// Asset Operations
// =========================================================================

void AssetBrowserPanel::createFolder(const std::string& name)
{
    std::filesystem::path newPath = std::filesystem::path(m_currentPath) / name;

    try
    {
        // Handle duplicate names
        std::string finalName = name;
        int counter = 1;
        while (std::filesystem::exists(newPath))
        {
            finalName = name + " (" + std::to_string(counter++) + ")";
            newPath = std::filesystem::path(m_currentPath) / finalName;
        }

        std::filesystem::create_directory(newPath);
        refresh();

        // Select the new folder
        selectAsset(newPath.string());
    }
    catch (const std::exception& /*e*/)
    {
        // Handle error
    }
}

void AssetBrowserPanel::renameSelected()
{
    if (!m_selectedAssets.empty())
    {
        m_isRenaming = true;
        m_renamingAsset = m_selectedAssets[0];

        // Extract filename
        std::filesystem::path path(m_renamingAsset);
        std::strncpy(m_renameBuffer, path.filename().string().c_str(), sizeof(m_renameBuffer) - 1);
        m_renameBuffer[sizeof(m_renameBuffer) - 1] = '\0';
    }
}

void AssetBrowserPanel::deleteSelected()
{
    for (const auto& path : m_selectedAssets)
    {
        try
        {
            std::filesystem::remove_all(path);

            AssetEvent event(EditorEventType::AssetDeleted);
            event.assetPath = path;
            publishEvent(event);
        }
        catch (const std::exception& /*e*/)
        {
            // Handle error
        }
    }

    clearSelection();
    refresh();
}

void AssetBrowserPanel::duplicateSelected()
{
    std::vector<std::string> duplicatedPaths;

    for (const auto& path : m_selectedAssets)
    {
        try
        {
            std::filesystem::path srcPath(path);
            std::filesystem::path dstPath = srcPath;

            // Generate unique name
            std::string stem = srcPath.stem().string();
            std::string ext = srcPath.extension().string();
            std::string newName = stem + " (Copy)" + ext;
            dstPath = srcPath.parent_path() / newName;

            int counter = 1;
            while (std::filesystem::exists(dstPath))
            {
                newName = stem + " (Copy " + std::to_string(counter++) + ")" + ext;
                dstPath = srcPath.parent_path() / newName;
            }

            std::filesystem::copy(srcPath, dstPath);
            duplicatedPaths.push_back(dstPath.string());

            AssetEvent event(EditorEventType::AssetImported);
            event.assetPath = dstPath.string();
            publishEvent(event);
        }
        catch (const std::exception& /*e*/)
        {
            // Handle error
        }
    }

    refresh();

    // Select duplicated assets
    m_selectedAssets = duplicatedPaths;
}

void AssetBrowserPanel::reimportSelected()
{
    for (const auto& path : m_selectedAssets)
    {
        AssetEvent event(EditorEventType::AssetModified);
        event.assetPath = path;
        publishEvent(event);
    }
}

void AssetBrowserPanel::showInExplorer()
{
    if (m_selectedAssets.empty())
    {
        return;
    }

#ifdef _WIN32
    // Windows: use explorer
    std::string cmd = "explorer /select,\"" + m_selectedAssets[0] + "\"";
    std::system(cmd.c_str());
#elif defined(__APPLE__)
    // macOS: use open -R
    std::string cmd = "open -R \"" + m_selectedAssets[0] + "\"";
    std::system(cmd.c_str());
#else
    // Linux: use xdg-open on parent directory
    std::filesystem::path path(m_selectedAssets[0]);
    std::string cmd = "xdg-open \"" + path.parent_path().string() + "\"";
    std::system(cmd.c_str());
#endif
}

void AssetBrowserPanel::copyPathToClipboard()
{
    if (m_selectedAssets.empty())
    {
        return;
    }

    // Build path string
    std::string paths;
    for (size_t i = 0; i < m_selectedAssets.size(); ++i)
    {
        if (i > 0) paths += "\n";
        paths += m_selectedAssets[i];
    }

    // Would use ImGui::SetClipboardText(paths.c_str());
}

void AssetBrowserPanel::importAssets()
{
    // Would open file dialog
    // Then copy selected files to current directory
}

void AssetBrowserPanel::exportSelected()
{
    // Would open folder dialog
    // Then copy selected assets to destination
}

// =========================================================================
// Selection
// =========================================================================

void AssetBrowserPanel::selectAsset(const std::string& path)
{
    m_selectedAssets.clear();
    m_selectedAssets.push_back(path);

    // Find the entry and notify
    for (const auto& entry : m_entries)
    {
        if (entry.path == path)
        {
            if (m_onAssetSelected)
            {
                m_onAssetSelected(path, entry.type);
            }
            break;
        }
    }
}

void AssetBrowserPanel::addToSelection(const std::string& path)
{
    if (!isAssetSelected(path))
    {
        m_selectedAssets.push_back(path);
    }
}

void AssetBrowserPanel::removeFromSelection(const std::string& path)
{
    m_selectedAssets.erase(
        std::remove(m_selectedAssets.begin(), m_selectedAssets.end(), path),
        m_selectedAssets.end());
}

void AssetBrowserPanel::clearSelection()
{
    m_selectedAssets.clear();
}

bool AssetBrowserPanel::isAssetSelected(const std::string& path) const
{
    return std::find(m_selectedAssets.begin(), m_selectedAssets.end(), path) != m_selectedAssets.end();
}

// =========================================================================
// Metadata
// =========================================================================

const AssetMetadata* AssetBrowserPanel::getAssetMetadata(const std::string& path) const
{
    for (const auto& entry : m_entries)
    {
        if (entry.path == path)
        {
            return &entry.metadata;
        }
    }
    return nullptr;
}

void AssetBrowserPanel::refreshMetadata(const std::string& path)
{
    for (auto& entry : m_entries)
    {
        if (entry.path == path)
        {
            // Would reload metadata from asset pipeline
            break;
        }
    }
}

// =========================================================================
// Menu/Toolbar Items
// =========================================================================

std::vector<MenuItem> AssetBrowserPanel::getMenuItems() const
{
    std::vector<MenuItem> items;

    MenuItem viewMenu;
    viewMenu.label = "View";
    viewMenu.subItems = {
        {"Grid View", "", [this]() { const_cast<AssetBrowserPanel*>(this)->setGridView(true); },
         []() { return true; }, [this]() { return m_isGridView; }},
        {"List View", "", [this]() { const_cast<AssetBrowserPanel*>(this)->setGridView(false); },
         []() { return true; }, [this]() { return !m_isGridView; }},
        MenuItem::separator(),
        {"Show Preview", "", [this]() { const_cast<AssetBrowserPanel*>(this)->setShowPreview(!m_showPreview); },
         []() { return true; }, [this]() { return m_showPreview; }},
        {"Show Hidden Files", "", [this]() { const_cast<AssetBrowserPanel*>(this)->m_showHiddenFiles = !m_showHiddenFiles; },
         []() { return true; }, [this]() { return m_showHiddenFiles; }},
    };
    items.push_back(viewMenu);

    return items;
}

std::vector<ToolbarItem> AssetBrowserPanel::getToolbarItems() const
{
    std::vector<ToolbarItem> items;

    items.push_back({"<", "Back", [this]() { const_cast<AssetBrowserPanel*>(this)->navigateBack(); },
                     [this]() { return canNavigateBack(); }});
    items.push_back({">", "Forward", [this]() { const_cast<AssetBrowserPanel*>(this)->navigateForward(); },
                     [this]() { return canNavigateForward(); }});
    items.push_back({"^", "Up", [this]() { const_cast<AssetBrowserPanel*>(this)->navigateUp(); }});
    items.push_back({"R", "Refresh", [this]() { const_cast<AssetBrowserPanel*>(this)->refresh(); }});

    items.push_back(ToolbarItem::separator());

    items.push_back({"Grid", "Grid View", [this]() { const_cast<AssetBrowserPanel*>(this)->setGridView(true); },
                     []() { return true; }, [this]() { return m_isGridView; }});
    items.push_back({"List", "List View", [this]() { const_cast<AssetBrowserPanel*>(this)->setGridView(false); },
                     []() { return true; }, [this]() { return !m_isGridView; }});

    items.push_back(ToolbarItem::separator());

    items.push_back({"+", "New Folder", [this]() { const_cast<AssetBrowserPanel*>(this)->createFolder(); }});

    return items;
}

std::vector<MenuItem> AssetBrowserPanel::getContextMenuItems() const
{
    std::vector<MenuItem> items;
    bool hasSelection = !m_selectedAssets.empty();

    items.push_back({"Open", "", [this]() {
        if (!m_selectedAssets.empty())
        {
            for (const auto& entry : m_entries)
            {
                if (entry.path == m_selectedAssets[0])
                {
                    const_cast<AssetBrowserPanel*>(this)->handleAssetDoubleClick(entry);
                    break;
                }
            }
        }
    }, [hasSelection]() { return hasSelection; }});

    items.push_back(MenuItem::separator());

    // Create submenu
    MenuItem createMenu;
    createMenu.label = "Create";
    createMenu.subItems = {
        {"Folder", "", [this]() { const_cast<AssetBrowserPanel*>(this)->createFolder(); }},
        {"Scene", "", []() { /* Create scene */ }},
        {"Script", "", []() { /* Create script */ }},
    };
    items.push_back(createMenu);

    items.push_back(MenuItem::separator());

    items.push_back({"Cut", "Ctrl+X", []() { /* Cut */ }, [hasSelection]() { return hasSelection; }});
    items.push_back({"Copy", "Ctrl+C", []() { /* Copy */ }, [hasSelection]() { return hasSelection; }});
    items.push_back({"Paste", "Ctrl+V", []() { /* Paste */ }});

    items.push_back(MenuItem::separator());

    items.push_back({"Duplicate", "Ctrl+D",
        [this]() { const_cast<AssetBrowserPanel*>(this)->duplicateSelected(); },
        [hasSelection]() { return hasSelection; }});
    items.push_back({"Rename", "F2",
        [this]() { const_cast<AssetBrowserPanel*>(this)->renameSelected(); },
        [hasSelection]() { return hasSelection; }});
    items.push_back({"Delete", "Delete",
        [this]() { const_cast<AssetBrowserPanel*>(this)->deleteSelected(); },
        [hasSelection]() { return hasSelection; }});

    items.push_back(MenuItem::separator());

    items.push_back({"Reimport", "",
        [this]() { const_cast<AssetBrowserPanel*>(this)->reimportSelected(); },
        [hasSelection]() { return hasSelection; }});
    items.push_back({"Show in Explorer", "",
        [this]() { const_cast<AssetBrowserPanel*>(this)->showInExplorer(); },
        [hasSelection]() { return hasSelection; }});
    items.push_back({"Copy Path", "",
        [this]() { const_cast<AssetBrowserPanel*>(this)->copyPathToClipboard(); },
        [hasSelection]() { return hasSelection; }});

    return items;
}

void AssetBrowserPanel::onInitialize()
{
    m_rootPath = ".";  // Would be project assets folder
    m_currentPath = m_rootPath;
    m_history.push_back(m_currentPath);
    loadDirectory(m_currentPath);
}

void AssetBrowserPanel::onUpdate(f64 /*deltaTime*/)
{
    updateThumbnailCache();
}

void AssetBrowserPanel::onRender()
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    ImVec2 availSize = ImGui::GetContentRegionAvail();
    m_contentWidth = availSize.x;

    // Breadcrumb navigation
    renderBreadcrumb();
    ImGui::Separator();

    // Split: directory tree on left, content on right
    f32 treeWidth = 200.0f;
    f32 previewWidth = m_showPreview ? 250.0f : 0.0f;
    f32 contentWidth = availSize.x - treeWidth - previewWidth - 20.0f;

    // Directory tree on left
    ImGui::BeginChild("DirectoryTree", ImVec2(treeWidth, 0), true);
    renderDirectoryTree();
    ImGui::EndChild();

    ImGui::SameLine();

    // Main content area
    ImGui::BeginChild("AssetContent", ImVec2(contentWidth, 0), true);
    if (m_isGridView)
    {
        renderAssetGrid();
    }
    else
    {
        renderAssetList();
    }
    ImGui::EndChild();

    // Preview panel (optional)
    if (m_showPreview && !m_selectedAssets.empty())
    {
        ImGui::SameLine();
        ImGui::BeginChild("PreviewPanel", ImVec2(previewWidth, 0), true);
        renderPreviewPanel();
        ImGui::EndChild();
    }

    // Handle drop target for entire panel
    handleDropTarget();

    // Handle keyboard shortcuts
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
    {
        if (ImGui::IsKeyPressed(ImGuiKey_Delete) && !m_selectedAssets.empty())
        {
            deleteSelected();
        }
        if (ImGui::IsKeyPressed(ImGuiKey_F2) && !m_selectedAssets.empty())
        {
            renameSelected();
        }
        if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_D) && !m_selectedAssets.empty())
        {
            duplicateSelected();
        }
        if (ImGui::IsKeyPressed(ImGuiKey_Backspace))
        {
            navigateUp();
        }
    }
#else
    renderBreadcrumb();
    renderDirectoryTree();

    if (m_isGridView)
    {
        renderAssetGrid();
    }
    else
    {
        renderAssetList();
    }

    if (m_showPreview && !m_selectedAssets.empty())
    {
        renderPreviewPanel();
    }

    handleDropTarget();
#endif
}

void AssetBrowserPanel::renderToolbar()
{
    widgets::BeginToolbar("AssetBrowserToolbar");

    renderToolbarItems(getToolbarItems());

#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    ImGui::SameLine();
    ImGui::SetNextItemWidth(200);
#endif

    // Search input
    if (widgets::SearchInput("##Search", m_searchBuffer, sizeof(m_searchBuffer), "Search assets..."))
    {
        m_filter = m_searchBuffer;
    }

#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    // Thumbnail size slider (in grid view)
    if (m_isGridView)
    {
        ImGui::SameLine();
        ImGui::SetNextItemWidth(100);
        if (ImGui::SliderFloat("##ThumbnailSize", &m_thumbnailSize, m_minThumbnailSize, m_maxThumbnailSize, "%.0f"))
        {
            // Thumbnail size changed
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Thumbnail Size");
        }
    }

    // Type filter dropdown
    ImGui::SameLine();
    ImGui::SetNextItemWidth(100);
    static const char* typeNames[] = {
        "All Types", "Images", "Audio", "Scripts", "Scenes",
        "Characters", "Backgrounds", "Fonts", "Videos", "Data"
    };
    int currentType = static_cast<int>(m_typeFilter);
    if (ImGui::Combo("##TypeFilter", &currentType, typeNames, IM_ARRAYSIZE(typeNames)))
    {
        m_typeFilter = static_cast<AssetType>(currentType);
    }
#endif

    widgets::EndToolbar();
}

void AssetBrowserPanel::renderBreadcrumb()
{
    std::filesystem::path path(m_currentPath);
    std::vector<std::string> parts;
    std::vector<std::string> paths;

    std::filesystem::path accumulated;
    for (const auto& part : path)
    {
        accumulated /= part;
        parts.push_back(part.string());
        paths.push_back(accumulated.string());
    }

#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    // Navigation buttons
    if (ImGui::Button("<##Back"))
    {
        navigateBack();
    }
    if (ImGui::IsItemHovered() && !ImGui::IsItemActive())
    {
        ImGui::SetTooltip("Back");
    }

    ImGui::SameLine();
    if (ImGui::Button(">##Forward"))
    {
        navigateForward();
    }
    if (ImGui::IsItemHovered() && !ImGui::IsItemActive())
    {
        ImGui::SetTooltip("Forward");
    }

    ImGui::SameLine();
    if (ImGui::Button("^##Up"))
    {
        navigateUp();
    }
    if (ImGui::IsItemHovered() && !ImGui::IsItemActive())
    {
        ImGui::SetTooltip("Up");
    }

    ImGui::SameLine();
    ImGui::Text("|");
    ImGui::SameLine();

    // Render clickable breadcrumb items
    for (size_t i = 0; i < parts.size(); ++i)
    {
        if (i > 0)
        {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), ">");
            ImGui::SameLine();
        }

        // Make each part clickable
        bool isLast = (i == parts.size() - 1);
        if (isLast)
        {
            // Current directory (highlighted)
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", parts[i].c_str());
        }
        else
        {
            // Clickable parent directory
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
            if (ImGui::Button(parts[i].c_str()))
            {
                navigateTo(paths[i]);
            }
            ImGui::PopStyleColor(2);
        }
    }

    // Refresh button on the right
    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 25);
    if (ImGui::Button("R##Refresh"))
    {
        refresh();
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("Refresh");
    }
#else
    (void)parts;
    (void)paths;
#endif
}

void AssetBrowserPanel::renderDirectoryTree()
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Folders");
    ImGui::Separator();

    // Root folder
    ImGuiTreeNodeFlags rootFlags = ImGuiTreeNodeFlags_OpenOnArrow |
                                   ImGuiTreeNodeFlags_SpanAvailWidth |
                                   ImGuiTreeNodeFlags_DefaultOpen;

    if (m_currentPath == m_rootPath)
    {
        rootFlags |= ImGuiTreeNodeFlags_Selected;
    }

    bool rootOpen = ImGui::TreeNodeEx("Assets", rootFlags);
    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
    {
        navigateTo(m_rootPath);
    }

    if (rootOpen)
    {
        // Render directory entries
        for (const auto& entry : m_entries)
        {
            if (!entry.isDirectory)
            {
                continue;
            }

            ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow |
                                           ImGuiTreeNodeFlags_SpanAvailWidth |
                                           ImGuiTreeNodeFlags_Leaf; // Simplified: treat all as leaf

            bool isSelected = (entry.path == m_currentPath);
            if (isSelected)
            {
                nodeFlags |= ImGuiTreeNodeFlags_Selected;
            }

            // Folder icon
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.8f, 0.4f, 1.0f));
            bool nodeOpen = ImGui::TreeNodeEx(entry.name.c_str(), nodeFlags);
            ImGui::PopStyleColor();

            if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
            {
                navigateTo(entry.path);
            }

            // Context menu for folders
            if (ImGui::BeginPopupContextItem())
            {
                if (ImGui::MenuItem("Open"))
                {
                    navigateTo(entry.path);
                }
                if (ImGui::MenuItem("New Folder"))
                {
                    // Would create subfolder
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Delete"))
                {
                    selectAsset(entry.path);
                    deleteSelected();
                }
                if (ImGui::MenuItem("Rename"))
                {
                    selectAsset(entry.path);
                    renameSelected();
                }
                ImGui::EndPopup();
            }

            if (nodeOpen)
            {
                ImGui::TreePop();
            }
        }

        ImGui::TreePop();
    }

    // Quick access section
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Quick Access");
    ImGui::Separator();

    if (ImGui::Selectable("Images", m_typeFilter == AssetType::Image))
    {
        m_typeFilter = (m_typeFilter == AssetType::Image) ? AssetType::Unknown : AssetType::Image;
    }
    if (ImGui::Selectable("Audio", m_typeFilter == AssetType::Audio))
    {
        m_typeFilter = (m_typeFilter == AssetType::Audio) ? AssetType::Unknown : AssetType::Audio;
    }
    if (ImGui::Selectable("Scripts", m_typeFilter == AssetType::Script))
    {
        m_typeFilter = (m_typeFilter == AssetType::Script) ? AssetType::Unknown : AssetType::Script;
    }
    if (ImGui::Selectable("Scenes", m_typeFilter == AssetType::Scene))
    {
        m_typeFilter = (m_typeFilter == AssetType::Scene) ? AssetType::Unknown : AssetType::Scene;
    }
#else
    for (const auto& entry : m_entries)
    {
        if (entry.isDirectory)
        {
            bool isExpanded = false;
            if (widgets::TreeNode(entry.name.c_str(), false, false))
            {
                isExpanded = true;
            }
            (void)isExpanded;
        }
    }
#endif
}

void AssetBrowserPanel::renderAssetGrid()
{
    f32 padding = 10.0f;
    f32 cellSize = m_thumbnailSize + padding;

#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    f32 contentWidth = ImGui::GetContentRegionAvail().x;
    i32 columns = std::max(1, static_cast<i32>(contentWidth / cellSize));

    std::string filterLower = m_filter;
    std::transform(filterLower.begin(), filterLower.end(), filterLower.begin(), ::tolower);

    // Context menu for empty area
    if (ImGui::BeginPopupContextWindow("AssetGridContext", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
    {
        if (ImGui::MenuItem("New Folder"))
        {
            createFolder();
        }
        if (ImGui::MenuItem("Import Assets..."))
        {
            importAssets();
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Paste", "Ctrl+V"))
        {
            // Would paste from clipboard
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Refresh"))
        {
            refresh();
        }
        ImGui::EndPopup();
    }

    // Grid layout
    i32 columnIndex = 0;
    for (const auto& entry : m_entries)
    {
        // Skip directories in grid view (they're shown in tree)
        if (entry.isDirectory && !m_showDirectoriesInGrid)
        {
            continue;
        }

        // Apply text filter
        if (!filterLower.empty())
        {
            std::string nameLower = entry.name;
            std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
            if (nameLower.find(filterLower) == std::string::npos)
            {
                continue;
            }
        }

        // Apply type filter (for files only)
        if (!entry.isDirectory && m_typeFilter != AssetType::Unknown && entry.type != m_typeFilter)
        {
            continue;
        }

        // Begin cell
        ImGui::PushID(entry.path.c_str());

        ImGui::BeginGroup();

        renderAssetEntry(entry);

        ImGui::EndGroup();

        // Handle selection
        if (ImGui::IsItemClicked())
        {
            if (ImGui::GetIO().KeyCtrl)
            {
                if (isAssetSelected(entry.path))
                {
                    removeFromSelection(entry.path);
                }
                else
                {
                    addToSelection(entry.path);
                }
            }
            else
            {
                selectAsset(entry.path);
            }
        }

        // Handle double-click
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
        {
            handleAssetDoubleClick(entry);
        }

        // Context menu
        renderAssetContextMenu(entry);

        // Drag source
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
        {
            ImGui::SetDragDropPayload("ASSET_BROWSER_ITEM", entry.path.c_str(), entry.path.size() + 1);
            ImGui::Text("%s", entry.name.c_str());
            ImGui::EndDragDropSource();
        }

        ImGui::PopID();

        // Grid layout: move to next column or new row
        ++columnIndex;
        if (columnIndex < columns)
        {
            ImGui::SameLine();
        }
        else
        {
            columnIndex = 0;
        }
    }

    // Empty state
    if (m_entries.empty() || (filterLower.length() > 0 && columnIndex == 0))
    {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "No assets found");
        if (!filterLower.empty())
        {
            ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.4f, 1.0f), "Try adjusting your search filter");
        }
    }
#else
    i32 columns = std::max(1, static_cast<i32>(m_contentWidth / cellSize));

    std::string filterLower = m_filter;
    std::transform(filterLower.begin(), filterLower.end(), filterLower.begin(), ::tolower);

    for (const auto& entry : m_entries)
    {
        if (!filterLower.empty())
        {
            std::string nameLower = entry.name;
            std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
            if (nameLower.find(filterLower) == std::string::npos)
            {
                continue;
            }
        }

        if (m_typeFilter != AssetType::Unknown && entry.type != m_typeFilter)
        {
            continue;
        }

        renderAssetEntry(entry);
    }

    (void)columns;
    (void)cellSize;
#endif
}

void AssetBrowserPanel::renderAssetList()
{
    std::string filterLower = m_filter;
    std::transform(filterLower.begin(), filterLower.end(), filterLower.begin(), ::tolower);

#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    // Column headers
    ImGui::Columns(4, "AssetListColumns", true);
    ImGui::SetColumnWidth(0, 250);
    ImGui::SetColumnWidth(1, 80);
    ImGui::SetColumnWidth(2, 100);
    ImGui::SetColumnWidth(3, 150);

    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Name");
    ImGui::NextColumn();
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Type");
    ImGui::NextColumn();
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Size");
    ImGui::NextColumn();
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Modified");
    ImGui::NextColumn();

    ImGui::Separator();

    for (const auto& entry : m_entries)
    {
        // Apply filter
        if (!filterLower.empty())
        {
            std::string nameLower = entry.name;
            std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
            if (nameLower.find(filterLower) == std::string::npos)
            {
                continue;
            }
        }

        // Apply type filter
        if (!entry.isDirectory && m_typeFilter != AssetType::Unknown && entry.type != m_typeFilter)
        {
            continue;
        }

        bool isSelected = isAssetSelected(entry.path);

        ImGui::PushID(entry.path.c_str());

        // Name column with icon
        const char* icon = entry.isDirectory ? "[DIR]" : getAssetTypeIcon(entry.type);
        ImVec4 iconColor = entry.isDirectory ? ImVec4(0.9f, 0.8f, 0.4f, 1.0f) : getAssetTypeColor(entry.type);

        if (ImGui::Selectable("##row", isSelected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick))
        {
            if (ImGui::GetIO().KeyCtrl)
            {
                if (isSelected)
                {
                    removeFromSelection(entry.path);
                }
                else
                {
                    addToSelection(entry.path);
                }
            }
            else
            {
                selectAsset(entry.path);
            }

            if (ImGui::IsMouseDoubleClicked(0))
            {
                handleAssetDoubleClick(entry);
            }
        }

        // Context menu
        renderAssetContextMenu(entry);

        // Drag source
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
        {
            ImGui::SetDragDropPayload("ASSET_BROWSER_ITEM", entry.path.c_str(), entry.path.size() + 1);
            ImGui::Text("%s", entry.name.c_str());
            ImGui::EndDragDropSource();
        }

        ImGui::SameLine();
        ImGui::TextColored(iconColor, "%s", icon);
        ImGui::SameLine();
        ImGui::Text("%s", entry.name.c_str());

        ImGui::NextColumn();

        // Type column
        if (entry.isDirectory)
        {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Folder");
        }
        else
        {
            ImGui::Text("%s", getAssetTypeName(entry.type).c_str());
        }
        ImGui::NextColumn();

        // Size column
        if (!entry.isDirectory)
        {
            ImGui::Text("%s", formatFileSize(entry.size).c_str());
        }
        ImGui::NextColumn();

        // Modified column
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "%s", entry.modifiedTime.c_str());
        ImGui::NextColumn();

        ImGui::PopID();
    }

    ImGui::Columns(1);

    // Empty state
    if (m_entries.empty())
    {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "No assets in this folder");
    }
#else
    for (const auto& entry : m_entries)
    {
        if (!filterLower.empty())
        {
            std::string nameLower = entry.name;
            std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
            if (nameLower.find(filterLower) == std::string::npos)
            {
                continue;
            }
        }

        bool isSelected = isAssetSelected(entry.path);
        (void)isSelected;
    }
#endif
}

void AssetBrowserPanel::renderAssetEntry(const AssetBrowserEntry& entry)
{
    bool isSelected = isAssetSelected(entry.path);

#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();

    // Draw thumbnail background
    ImU32 bgColor = isSelected ? IM_COL32(70, 100, 150, 255) : IM_COL32(50, 50, 50, 255);
    drawList->AddRectFilled(pos, ImVec2(pos.x + m_thumbnailSize, pos.y + m_thumbnailSize + 20), bgColor, 4.0f);

    // Draw selection highlight
    if (isSelected)
    {
        drawList->AddRect(pos, ImVec2(pos.x + m_thumbnailSize, pos.y + m_thumbnailSize + 20),
                          IM_COL32(100, 150, 200, 255), 4.0f, 0, 2.0f);
    }

    // Draw icon/thumbnail
    const char* icon = getAssetTypeIcon(entry.type);
    if (entry.isDirectory)
    {
        icon = "[DIR]";
    }

    ImVec4 iconColor = entry.isDirectory ? ImVec4(0.9f, 0.8f, 0.4f, 1.0f) : getAssetTypeColor(entry.type);

    // Center the icon in the thumbnail area
    ImVec2 iconSize = ImGui::CalcTextSize(icon);
    ImVec2 iconPos(pos.x + (m_thumbnailSize - iconSize.x) / 2, pos.y + (m_thumbnailSize - iconSize.y) / 2);
    drawList->AddText(iconPos, ImGui::GetColorU32(iconColor), icon);

    // Draw name below thumbnail
    ImVec2 namePos(pos.x, pos.y + m_thumbnailSize + 2);

    // Truncate name if too long
    std::string displayName = entry.name;
    f32 maxWidth = m_thumbnailSize - 4;
    ImVec2 nameSize = ImGui::CalcTextSize(displayName.c_str());
    if (nameSize.x > maxWidth)
    {
        while (displayName.length() > 3 && ImGui::CalcTextSize((displayName + "...").c_str()).x > maxWidth)
        {
            displayName.pop_back();
        }
        displayName += "...";
    }

    // Handle rename mode
    if (m_isRenaming && m_renamingAsset == entry.path)
    {
        ImGui::SetCursorScreenPos(namePos);
        ImGui::SetNextItemWidth(m_thumbnailSize - 4);
        ImGui::SetKeyboardFocusHere();
        if (ImGui::InputText("##rename", m_renameBuffer, sizeof(m_renameBuffer),
                             ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
        {
            // Commit rename
            std::filesystem::path oldPath(entry.path);
            std::filesystem::path newPath = oldPath.parent_path() / m_renameBuffer;
            try
            {
                std::filesystem::rename(oldPath, newPath);
                refresh();
            }
            catch (...)
            {
                // Handle error
            }
            m_isRenaming = false;
        }
        if (ImGui::IsKeyPressed(ImGuiKey_Escape))
        {
            m_isRenaming = false;
        }
    }
    else
    {
        // Center the name
        nameSize = ImGui::CalcTextSize(displayName.c_str());
        namePos.x = pos.x + (m_thumbnailSize - nameSize.x) / 2;
        drawList->AddText(namePos, IM_COL32(220, 220, 220, 255), displayName.c_str());
    }

    // Reserve space for the entry
    ImGui::Dummy(ImVec2(m_thumbnailSize, m_thumbnailSize + 20));

    // Tooltip with full name and info
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("%s", entry.name.c_str());
        if (!entry.isDirectory)
        {
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Type: %s", getAssetTypeName(entry.type).c_str());
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Size: %s", formatFileSize(entry.size).c_str());
        }
        ImGui::EndTooltip();
    }
#else
    const char* icon = entry.isDirectory ? "[DIR]" : "[FILE]";
    switch (entry.type)
    {
        case AssetType::Image: icon = "[IMG]"; break;
        case AssetType::Audio: icon = "[SND]"; break;
        case AssetType::Script: icon = "[SCR]"; break;
        case AssetType::Scene: icon = "[SCN]"; break;
        case AssetType::Character: icon = "[CHR]"; break;
        case AssetType::Background: icon = "[BG]"; break;
        case AssetType::Font: icon = "[FNT]"; break;
        case AssetType::Video: icon = "[VID]"; break;
        case AssetType::Data: icon = "[DAT]"; break;
        case AssetType::Prefab: icon = "[PFB]"; break;
        case AssetType::Animation: icon = "[ANM]"; break;
        case AssetType::Localization: icon = "[LOC]"; break;
        default: break;
    }
    (void)icon;
    (void)isSelected;
#endif
}

void AssetBrowserPanel::renderPreviewPanel()
{
    if (m_selectedAssets.empty())
    {
        return;
    }

    // Find selected entry
    const AssetBrowserEntry* selectedEntry = nullptr;
    for (const auto& entry : m_entries)
    {
        if (entry.path == m_selectedAssets[0])
        {
            selectedEntry = &entry;
            break;
        }
    }

    if (!selectedEntry)
    {
        return;
    }

#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Preview");
    ImGui::Separator();

    // Preview thumbnail (larger)
    f32 previewSize = ImGui::GetContentRegionAvail().x - 10;
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // Draw preview background
    drawList->AddRectFilled(pos, ImVec2(pos.x + previewSize, pos.y + previewSize),
                            IM_COL32(40, 40, 40, 255), 4.0f);

    // Draw icon centered
    const char* icon = selectedEntry->isDirectory ? "[DIR]" : getAssetTypeIcon(selectedEntry->type);
    ImVec4 iconColor = selectedEntry->isDirectory ?
                       ImVec4(0.9f, 0.8f, 0.4f, 1.0f) : getAssetTypeColor(selectedEntry->type);

    // Large icon text
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts.Size > 0 ? ImGui::GetIO().Fonts->Fonts[0] : nullptr);
    ImVec2 iconSize = ImGui::CalcTextSize(icon);
    ImVec2 iconPos(pos.x + (previewSize - iconSize.x) / 2, pos.y + (previewSize - iconSize.y) / 2);
    drawList->AddText(iconPos, ImGui::GetColorU32(iconColor), icon);
    ImGui::PopFont();

    ImGui::Dummy(ImVec2(previewSize, previewSize));
    ImGui::Spacing();
#endif

    renderMetadataSection(*selectedEntry);
}

void AssetBrowserPanel::renderMetadataSection(const AssetBrowserEntry& entry)
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Info");
    ImGui::Separator();

    // Name
    ImGui::Text("Name:");
    ImGui::SameLine(80);
    ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "%s", entry.name.c_str());

    // Type
    ImGui::Text("Type:");
    ImGui::SameLine(80);
    if (entry.isDirectory)
    {
        ImGui::TextColored(ImVec4(0.9f, 0.8f, 0.4f, 1.0f), "Folder");
    }
    else
    {
        ImGui::TextColored(getAssetTypeColor(entry.type), "%s", getAssetTypeName(entry.type).c_str());
    }

    // Size (for files)
    if (!entry.isDirectory)
    {
        ImGui::Text("Size:");
        ImGui::SameLine(80);
        ImGui::Text("%s", formatFileSize(entry.size).c_str());
    }

    // Path
    ImGui::Text("Path:");
    ImGui::SameLine(80);
    ImGui::TextWrapped("%s", entry.path.c_str());

    // Modified date
    if (!entry.modifiedTime.empty())
    {
        ImGui::Text("Modified:");
        ImGui::SameLine(80);
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "%s", entry.modifiedTime.c_str());
    }

    ImGui::Spacing();
    ImGui::Separator();

    // Type-specific metadata and actions
    switch (entry.type)
    {
        case AssetType::Image:
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Image Properties");
            ImGui::Separator();
            // Would show actual dimensions from metadata
            ImGui::Text("Dimensions: (Load to view)");
            ImGui::Text("Format: %s", entry.extension.c_str());
            break;

        case AssetType::Audio:
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Audio Properties");
            ImGui::Separator();
            ImGui::Text("Duration: (Load to view)");
            ImGui::Text("Format: %s", entry.extension.c_str());
            // Play button
            if (ImGui::Button("Play"))
            {
                // Would play audio preview
            }
            ImGui::SameLine();
            if (ImGui::Button("Stop"))
            {
                // Would stop audio preview
            }
            break;

        case AssetType::Video:
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Video Properties");
            ImGui::Separator();
            ImGui::Text("Duration: (Load to view)");
            ImGui::Text("Format: %s", entry.extension.c_str());
            break;

        case AssetType::Script:
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Script");
            ImGui::Separator();
            if (ImGui::Button("Open in Editor"))
            {
                // Would open script editor
            }
            break;

        case AssetType::Scene:
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Scene");
            ImGui::Separator();
            if (ImGui::Button("Load Scene"))
            {
                // Would load scene
            }
            break;

        default:
            break;
    }

    ImGui::Spacing();

    // Action buttons
    ImGui::Separator();
    if (ImGui::Button("Open"))
    {
        handleAssetDoubleClick(entry);
    }
    ImGui::SameLine();
    if (ImGui::Button("Show in Explorer"))
    {
        showInExplorer();
    }
#else
    switch (entry.type)
    {
        case AssetType::Image:
        case AssetType::Audio:
        case AssetType::Video:
        default:
            break;
    }
#endif
}

void AssetBrowserPanel::renderAssetContextMenu(const AssetBrowserEntry& entry)
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    if (ImGui::BeginPopupContextItem())
    {
        if (ImGui::MenuItem("Open"))
        {
            handleAssetDoubleClick(entry);
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Cut", "Ctrl+X"))
        {
            // Would cut to clipboard
        }
        if (ImGui::MenuItem("Copy", "Ctrl+C"))
        {
            // Would copy to clipboard
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Duplicate", "Ctrl+D"))
        {
            selectAsset(entry.path);
            duplicateSelected();
        }
        if (ImGui::MenuItem("Rename", "F2"))
        {
            selectAsset(entry.path);
            renameSelected();
        }
        if (ImGui::MenuItem("Delete", "Delete"))
        {
            selectAsset(entry.path);
            deleteSelected();
        }

        ImGui::Separator();

        if (!entry.isDirectory)
        {
            if (ImGui::MenuItem("Reimport"))
            {
                selectAsset(entry.path);
                reimportSelected();
            }
        }

        if (ImGui::MenuItem("Show in Explorer"))
        {
            selectAsset(entry.path);
            showInExplorer();
        }
        if (ImGui::MenuItem("Copy Path"))
        {
            selectAsset(entry.path);
            copyPathToClipboard();
        }

        ImGui::EndPopup();
    }
#else
    (void)entry;
#endif
}

// =========================================================================
// Asset Operations (Internal)
// =========================================================================

void AssetBrowserPanel::loadDirectory(const std::string& path)
{
    m_entries.clear();

    try
    {
        for (const auto& dirEntry : std::filesystem::directory_iterator(path))
        {
            AssetBrowserEntry entry;
            entry.name = dirEntry.path().filename().string();
            entry.path = dirEntry.path().string();
            entry.isDirectory = dirEntry.is_directory();

            // Skip hidden files unless enabled
            if (!m_showHiddenFiles && !entry.name.empty() && entry.name[0] == '.')
            {
                continue;
            }

            if (!entry.isDirectory)
            {
                entry.extension = dirEntry.path().extension().string();
                entry.type = getAssetType(entry.extension);

                try
                {
                    entry.size = std::filesystem::file_size(dirEntry);
                    auto ftime = std::filesystem::last_write_time(dirEntry);
                    // Convert to readable string (simplified)
                    entry.modifiedTime = ""; // Would format properly
                }
                catch (...)
                {
                    entry.size = 0;
                }

                // Request thumbnail for supported types
                if (entry.type == AssetType::Image)
                {
                    requestThumbnail(entry.path);
                }
            }

            m_entries.push_back(entry);
        }

        // Sort: directories first, then by name
        std::sort(m_entries.begin(), m_entries.end(),
            [](const AssetBrowserEntry& a, const AssetBrowserEntry& b) {
                if (a.isDirectory != b.isDirectory)
                {
                    return a.isDirectory > b.isDirectory;
                }
                return a.name < b.name;
            });
    }
    catch (const std::exception& /*e*/)
    {
        // Handle filesystem errors
    }
}

AssetType AssetBrowserPanel::getAssetType(const std::string& extension) const
{
    std::string ext = extension;
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tga" || ext == ".gif")
    {
        return AssetType::Image;
    }
    if (ext == ".wav" || ext == ".mp3" || ext == ".ogg" || ext == ".flac" || ext == ".aac")
    {
        return AssetType::Audio;
    }
    if (ext == ".nms" || ext == ".ns" || ext == ".txt")
    {
        return AssetType::Script;
    }
    if (ext == ".nmscene" || ext == ".scene")
    {
        return AssetType::Scene;
    }
    if (ext == ".ttf" || ext == ".otf" || ext == ".woff" || ext == ".woff2")
    {
        return AssetType::Font;
    }
    if (ext == ".mp4" || ext == ".webm" || ext == ".avi" || ext == ".mkv")
    {
        return AssetType::Video;
    }
    if (ext == ".json" || ext == ".xml" || ext == ".yaml" || ext == ".yml")
    {
        return AssetType::Data;
    }
    if (ext == ".nmchar" || ext == ".character")
    {
        return AssetType::Character;
    }
    if (ext == ".nmbg" || ext == ".background")
    {
        return AssetType::Background;
    }
    if (ext == ".prefab")
    {
        return AssetType::Prefab;
    }
    if (ext == ".anim" || ext == ".nmanim")
    {
        return AssetType::Animation;
    }
    if (ext == ".po" || ext == ".pot" || ext == ".xliff" || ext == ".csv")
    {
        return AssetType::Localization;
    }

    return AssetType::Unknown;
}

void AssetBrowserPanel::handleAssetDoubleClick(const AssetBrowserEntry& entry)
{
    if (entry.isDirectory)
    {
        navigateTo(entry.path);
    }
    else
    {
        if (m_onAssetDoubleClick)
        {
            m_onAssetDoubleClick(entry.path, entry.type);
        }

        AssetEvent event(EditorEventType::AssetModified);
        event.assetPath = entry.path;
        event.assetType = entry.extension;
        publishEvent(event);
    }
}

void AssetBrowserPanel::handleAssetDragDrop(const AssetBrowserEntry& entry)
{
    if (m_onDragStart)
    {
        AssetDragPayload payload;
        payload.path = entry.path;
        payload.type = entry.type;
        payload.selectedPaths = m_selectedAssets;
        m_onDragStart(payload);
    }
}

void AssetBrowserPanel::handleDropTarget()
{
    // Would handle files dropped from outside the application
    // or from other panels (e.g., external file import)
}

// =========================================================================
// Thumbnail Management
// =========================================================================

void AssetBrowserPanel::requestThumbnail(const std::string& path)
{
    if (m_thumbnailCache.find(path) == m_thumbnailCache.end())
    {
        m_pendingThumbnails.push_back(path);
    }
}

void AssetBrowserPanel::updateThumbnailCache()
{
    // Process a few pending thumbnails per frame
    const size_t maxPerFrame = 3;
    size_t processed = 0;

    while (!m_pendingThumbnails.empty() && processed < maxPerFrame)
    {
        std::string path = m_pendingThumbnails.back();
        m_pendingThumbnails.pop_back();

        // Would actually load and generate thumbnail here
        ThumbnailCacheEntry cacheEntry;
        cacheEntry.isReady = true;
        cacheEntry.lastAccessed = std::chrono::system_clock::now().time_since_epoch().count();
        m_thumbnailCache[path] = cacheEntry;

        ++processed;
    }

    // Evict old entries if cache is too large
    if (m_thumbnailCache.size() > m_maxCacheSize)
    {
        // Would find and remove least recently used entries
    }
}

void AssetBrowserPanel::clearThumbnailCache()
{
    m_thumbnailCache.clear();
    m_pendingThumbnails.clear();
}

// =========================================================================
// Utility
// =========================================================================

std::string AssetBrowserPanel::formatFileSize(u64 bytes) const
{
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int unitIndex = 0;
    double size = static_cast<double>(bytes);

    while (size >= 1024.0 && unitIndex < 4)
    {
        size /= 1024.0;
        ++unitIndex;
    }

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << size << " " << units[unitIndex];
    return oss.str();
}

std::string AssetBrowserPanel::getAssetTypeName(AssetType type) const
{
    switch (type)
    {
        case AssetType::Image: return "Image";
        case AssetType::Audio: return "Audio";
        case AssetType::Script: return "Script";
        case AssetType::Scene: return "Scene";
        case AssetType::Character: return "Character";
        case AssetType::Background: return "Background";
        case AssetType::Font: return "Font";
        case AssetType::Video: return "Video";
        case AssetType::Data: return "Data";
        case AssetType::Prefab: return "Prefab";
        case AssetType::Animation: return "Animation";
        case AssetType::Localization: return "Localization";
        default: return "Unknown";
    }
}

const char* AssetBrowserPanel::getAssetTypeIcon(AssetType type) const
{
    switch (type)
    {
        case AssetType::Image: return "[IMG]";
        case AssetType::Audio: return "[SND]";
        case AssetType::Script: return "[SCR]";
        case AssetType::Scene: return "[SCN]";
        case AssetType::Character: return "[CHR]";
        case AssetType::Background: return "[BG]";
        case AssetType::Font: return "[FNT]";
        case AssetType::Video: return "[VID]";
        case AssetType::Data: return "[DAT]";
        case AssetType::Prefab: return "[PFB]";
        case AssetType::Animation: return "[ANM]";
        case AssetType::Localization: return "[LOC]";
        default: return "[FILE]";
    }
}

#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
ImVec4 AssetBrowserPanel::getAssetTypeColor(AssetType type) const
{
    switch (type)
    {
        case AssetType::Image:
            return ImVec4(0.4f, 0.8f, 0.4f, 1.0f);  // Green
        case AssetType::Audio:
            return ImVec4(0.4f, 0.6f, 0.9f, 1.0f);  // Blue
        case AssetType::Script:
            return ImVec4(0.9f, 0.7f, 0.4f, 1.0f);  // Orange
        case AssetType::Scene:
            return ImVec4(0.8f, 0.4f, 0.8f, 1.0f);  // Purple
        case AssetType::Character:
            return ImVec4(0.9f, 0.5f, 0.5f, 1.0f);  // Red
        case AssetType::Background:
            return ImVec4(0.5f, 0.7f, 0.9f, 1.0f);  // Light blue
        case AssetType::Font:
            return ImVec4(0.7f, 0.7f, 0.7f, 1.0f);  // Gray
        case AssetType::Video:
            return ImVec4(0.9f, 0.4f, 0.6f, 1.0f);  // Pink
        case AssetType::Data:
            return ImVec4(0.6f, 0.8f, 0.6f, 1.0f);  // Light green
        case AssetType::Prefab:
            return ImVec4(0.4f, 0.9f, 0.9f, 1.0f);  // Cyan
        case AssetType::Animation:
            return ImVec4(0.9f, 0.9f, 0.4f, 1.0f);  // Yellow
        case AssetType::Localization:
            return ImVec4(0.7f, 0.5f, 0.9f, 1.0f);  // Violet
        default:
            return ImVec4(0.6f, 0.6f, 0.6f, 1.0f);  // Default gray
    }
}
#endif

} // namespace NovelMind::editor
