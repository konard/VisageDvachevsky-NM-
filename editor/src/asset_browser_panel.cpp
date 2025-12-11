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
#elif __APPLE__
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
    // Breadcrumb navigation
    renderBreadcrumb();

    // Split: directory tree on left, content on right
    renderDirectoryTree();

    if (m_isGridView)
    {
        renderAssetGrid();
    }
    else
    {
        renderAssetList();
    }

    // Preview panel (optional)
    if (m_showPreview && !m_selectedAssets.empty())
    {
        renderPreviewPanel();
    }

    // Handle drop target for entire panel
    handleDropTarget();
}

void AssetBrowserPanel::renderToolbar()
{
    widgets::BeginToolbar("AssetBrowserToolbar");

    renderToolbarItems(getToolbarItems());

    // Search input
    if (widgets::SearchInput("##Search", m_searchBuffer, sizeof(m_searchBuffer), "Search assets..."))
    {
        m_filter = m_searchBuffer;
    }

    // Thumbnail size slider (in grid view)
    if (m_isGridView)
    {
        // Would add slider for thumbnail size
    }

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

    // Would render clickable breadcrumb items
    (void)parts;
    (void)paths;
}

void AssetBrowserPanel::renderDirectoryTree()
{
    // Recursive tree of directories
    for (const auto& entry : m_entries)
    {
        if (entry.isDirectory)
        {
            bool isExpanded = false;
            if (widgets::TreeNode(entry.name.c_str(), false, false))
            {
                isExpanded = true;
            }

            if (isExpanded)
            {
                // Would recursively render subdirectories
            }
        }
    }
}

void AssetBrowserPanel::renderAssetGrid()
{
    f32 padding = 10.0f;
    f32 cellSize = m_thumbnailSize + padding;
    i32 columns = std::max(1, static_cast<i32>(m_contentWidth / cellSize));

    std::string filterLower = m_filter;
    std::transform(filterLower.begin(), filterLower.end(), filterLower.begin(), ::tolower);

    for (const auto& entry : m_entries)
    {
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

        // Apply type filter
        if (m_typeFilter != AssetType::Unknown && entry.type != m_typeFilter)
        {
            continue;
        }

        renderAssetEntry(entry);
    }

    (void)columns;
    (void)cellSize;
}

void AssetBrowserPanel::renderAssetList()
{
    std::string filterLower = m_filter;
    std::transform(filterLower.begin(), filterLower.end(), filterLower.begin(), ::tolower);

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

        bool isSelected = isAssetSelected(entry.path);
        // Would render selectable list item
        (void)isSelected;
    }
}

void AssetBrowserPanel::renderAssetEntry(const AssetBrowserEntry& entry)
{
    bool isSelected = isAssetSelected(entry.path);

    // Icon based on type
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

    // Render rename input if this asset is being renamed
    if (m_isRenaming && m_renamingAsset == entry.path)
    {
        // Would render text input
    }

    // Would render asset with icon, selection highlight, etc.
    (void)icon;
    (void)isSelected;
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

    renderMetadataSection(*selectedEntry);
}

void AssetBrowserPanel::renderMetadataSection(const AssetBrowserEntry& entry)
{
    // Common metadata
    // Would render: Name, Path, Type, Size, Modified date

    // Type-specific metadata
    switch (entry.type)
    {
        case AssetType::Image:
            // Dimensions, format
            // Would show image preview
            break;
        case AssetType::Audio:
            // Duration, sample rate, channels
            // Would show waveform and playback controls
            break;
        case AssetType::Video:
            // Duration, dimensions, frame rate
            // Would show video thumbnail
            break;
        default:
            break;
    }
}

void AssetBrowserPanel::renderAssetContextMenu(const AssetBrowserEntry& /*entry*/)
{
    // Would render popup context menu
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

} // namespace NovelMind::editor
