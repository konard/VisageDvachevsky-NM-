#pragma once

/**
 * @file asset_browser_panel.hpp
 * @brief Asset Browser Panel for NovelMind Editor v0.2.0
 *
 * The Asset Browser panel provides:
 * - File system navigation for project assets
 * - Asset preview and metadata display
 * - Drag-drop to scene/timeline/story graph
 * - Asset import/export/reimport
 * - Search and filtering
 * - Context menu operations (rename, delete, duplicate, etc.)
 * - Thumbnail caching with async generation
 */

#include "NovelMind/editor/gui_panel_base.hpp"
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>
#include <filesystem>

// Forward declarations
struct ImVec4;

namespace NovelMind::editor {

/**
 * @brief Asset types
 */
enum class AssetType : u8 {
    Unknown,
    Image,
    Audio,
    Script,
    Scene,
    Character,
    Background,
    Font,
    Video,
    Data,
    Prefab,
    Animation,
    Localization
};

/**
 * @brief Asset metadata
 */
struct AssetMetadata {
    // Common metadata
    std::string uuid;
    std::string importDate;
    std::string lastModified;

    // Image metadata
    i32 imageWidth = 0;
    i32 imageHeight = 0;
    std::string imageFormat;

    // Audio metadata
    f32 audioDuration = 0.0f;
    i32 audioSampleRate = 0;
    i32 audioChannels = 0;
    std::string audioCodec;

    // Video metadata
    f32 videoDuration = 0.0f;
    i32 videoWidth = 0;
    i32 videoHeight = 0;
    f32 videoFrameRate = 0.0f;
};

/**
 * @brief Asset entry in browser
 */
struct AssetBrowserEntry {
    std::string name;
    std::string path;
    std::string extension;
    AssetType type = AssetType::Unknown;
    bool isDirectory = false;
    u64 size = 0;
    std::string modifiedTime;
    AssetMetadata metadata;
    bool hasThumbnail = false;
    u32 thumbnailId = 0;
};

/**
 * @brief Thumbnail cache entry
 */
struct ThumbnailCacheEntry {
    u32 textureId = 0;
    bool isLoading = false;
    bool isReady = false;
    i64 lastAccessed = 0;
};

/**
 * @brief Drag drop payload type for assets
 */
struct AssetDragPayload {
    std::string path;
    AssetType type;
    std::vector<std::string> selectedPaths; // For multi-selection drag
};

/**
 * @brief Callback types
 */
using AssetSelectedCallback = std::function<void(const std::string& path, AssetType type)>;
using AssetDoubleClickCallback = std::function<void(const std::string& path, AssetType type)>;
using AssetDragCallback = std::function<void(const AssetDragPayload& payload)>;

/**
 * @brief Asset Browser Panel implementation
 */
class AssetBrowserPanel : public GUIPanelBase {
public:
    AssetBrowserPanel();
    ~AssetBrowserPanel() override = default;

    // =========================================================================
    // Navigation
    // =========================================================================

    /**
     * @brief Navigate to directory
     */
    void navigateTo(const std::string& path);

    /**
     * @brief Navigate up one level
     */
    void navigateUp();

    /**
     * @brief Navigate back in history
     */
    void navigateBack();

    /**
     * @brief Navigate forward in history
     */
    void navigateForward();

    /**
     * @brief Refresh current directory
     */
    void refresh();

    /**
     * @brief Get current directory path
     */
    [[nodiscard]] const std::string& getCurrentPath() const { return m_currentPath; }

    /**
     * @brief Check if can navigate back
     */
    [[nodiscard]] bool canNavigateBack() const { return m_historyIndex > 0; }

    /**
     * @brief Check if can navigate forward
     */
    [[nodiscard]] bool canNavigateForward() const { return m_historyIndex < m_history.size() - 1; }

    // =========================================================================
    // View Options
    // =========================================================================

    /**
     * @brief Set view mode (grid/list)
     */
    void setGridView(bool gridView) { m_isGridView = gridView; }

    /**
     * @brief Check if using grid view
     */
    [[nodiscard]] bool isGridView() const { return m_isGridView; }

    /**
     * @brief Set thumbnail size
     */
    void setThumbnailSize(f32 size);

    /**
     * @brief Get thumbnail size
     */
    [[nodiscard]] f32 getThumbnailSize() const { return m_thumbnailSize; }

    /**
     * @brief Set search filter
     */
    void setFilter(const std::string& filter) { m_filter = filter; }

    /**
     * @brief Get search filter
     */
    [[nodiscard]] const std::string& getFilter() const { return m_filter; }

    /**
     * @brief Set type filter
     */
    void setTypeFilter(AssetType type) { m_typeFilter = type; }

    /**
     * @brief Clear type filter
     */
    void clearTypeFilter() { m_typeFilter = AssetType::Unknown; }

    /**
     * @brief Toggle preview panel visibility
     */
    void setShowPreview(bool show) { m_showPreview = show; }

    /**
     * @brief Check if preview panel is visible
     */
    [[nodiscard]] bool isShowingPreview() const { return m_showPreview; }

    // =========================================================================
    // Asset Operations
    // =========================================================================

    /**
     * @brief Create new folder
     */
    void createFolder(const std::string& name = "New Folder");

    /**
     * @brief Rename selected asset
     */
    void renameSelected();

    /**
     * @brief Delete selected assets
     */
    void deleteSelected();

    /**
     * @brief Duplicate selected assets
     */
    void duplicateSelected();

    /**
     * @brief Reimport selected assets
     */
    void reimportSelected();

    /**
     * @brief Show selected asset in system explorer
     */
    void showInExplorer();

    /**
     * @brief Copy selected asset paths to clipboard
     */
    void copyPathToClipboard();

    /**
     * @brief Import assets from external location
     */
    void importAssets();

    /**
     * @brief Export selected assets
     */
    void exportSelected();

    // =========================================================================
    // Selection
    // =========================================================================

    /**
     * @brief Select asset by path
     */
    void selectAsset(const std::string& path);

    /**
     * @brief Add asset to selection
     */
    void addToSelection(const std::string& path);

    /**
     * @brief Remove asset from selection
     */
    void removeFromSelection(const std::string& path);

    /**
     * @brief Clear selection
     */
    void clearSelection();

    /**
     * @brief Get selected asset paths
     */
    [[nodiscard]] const std::vector<std::string>& getSelectedAssets() const { return m_selectedAssets; }

    /**
     * @brief Check if asset is selected
     */
    [[nodiscard]] bool isAssetSelected(const std::string& path) const;

    // =========================================================================
    // Metadata
    // =========================================================================

    /**
     * @brief Get metadata for asset
     */
    [[nodiscard]] const AssetMetadata* getAssetMetadata(const std::string& path) const;

    /**
     * @brief Refresh metadata for asset
     */
    void refreshMetadata(const std::string& path);

    // =========================================================================
    // Callbacks
    // =========================================================================

    /**
     * @brief Set callback for asset selection
     */
    void setOnAssetSelected(AssetSelectedCallback callback) { m_onAssetSelected = std::move(callback); }

    /**
     * @brief Set callback for asset double-click
     */
    void setOnAssetDoubleClick(AssetDoubleClickCallback callback) { m_onAssetDoubleClick = std::move(callback); }

    /**
     * @brief Set callback for drag start
     */
    void setOnDragStart(AssetDragCallback callback) { m_onDragStart = std::move(callback); }

    [[nodiscard]] std::vector<MenuItem> getMenuItems() const override;
    [[nodiscard]] std::vector<ToolbarItem> getToolbarItems() const override;
    [[nodiscard]] std::vector<MenuItem> getContextMenuItems() const override;

protected:
    void onInitialize() override;
    void onUpdate(f64 deltaTime) override;
    void onRender() override;
    void renderToolbar() override;

private:
    // Rendering helpers
    void renderBreadcrumb();
    void renderDirectoryTree();
    void renderAssetGrid();
    void renderAssetList();
    void renderAssetEntry(const AssetBrowserEntry& entry);
    void renderPreviewPanel();
    void renderMetadataSection(const AssetBrowserEntry& entry);
    void renderAssetContextMenu(const AssetBrowserEntry& entry);

    // Asset operations
    void loadDirectory(const std::string& path);
    AssetType getAssetType(const std::string& extension) const;
    void handleAssetDoubleClick(const AssetBrowserEntry& entry);
    void handleAssetDragDrop(const AssetBrowserEntry& entry);
    void handleDropTarget();

    // Thumbnail management
    void requestThumbnail(const std::string& path);
    void updateThumbnailCache();
    void clearThumbnailCache();

    // Utility
    std::string formatFileSize(u64 bytes) const;
    std::string getAssetTypeName(AssetType type) const;
    const char* getAssetTypeIcon(AssetType type) const;
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    ImVec4 getAssetTypeColor(AssetType type) const;
#endif

    std::string m_currentPath;
    std::string m_rootPath;
    std::string m_filter;
    AssetType m_typeFilter = AssetType::Unknown;
    std::vector<AssetBrowserEntry> m_entries;
    std::vector<std::string> m_history;
    size_t m_historyIndex = 0;

    bool m_isGridView = true;
    f32 m_thumbnailSize = 80.0f;
    f32 m_minThumbnailSize = 40.0f;
    f32 m_maxThumbnailSize = 200.0f;
    bool m_showPreview = true;
    bool m_showHiddenFiles = false;
    bool m_showDirectoriesInGrid = true;
    f32 m_contentWidth = 800.0f;

    // Selection
    std::vector<std::string> m_selectedAssets;

    // Rename state
    bool m_isRenaming = false;
    std::string m_renamingAsset;
    char m_renameBuffer[256] = {0};

    // Search
    char m_searchBuffer[256] = {0};

    // Thumbnail cache
    std::unordered_map<std::string, ThumbnailCacheEntry> m_thumbnailCache;
    std::vector<std::string> m_pendingThumbnails;
    size_t m_maxCacheSize = 1000;

    // Drag state
    bool m_isDragging = false;
    std::string m_draggedAsset;

    // Callbacks
    AssetSelectedCallback m_onAssetSelected;
    AssetDoubleClickCallback m_onAssetDoubleClick;
    AssetDragCallback m_onDragStart;
};

} // namespace NovelMind::editor
