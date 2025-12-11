#pragma once

/**
 * @file editor_app.hpp
 * @brief NovelMind Editor Application
 *
 * The main editor application providing:
 * - Project management (create, open, save)
 * - Scene editing with visual preview
 * - Story graph node editor
 * - Asset browser and management
 * - Inspector panel for property editing
 * - Build and preview functionality
 */

#include "NovelMind/core/types.hpp"
#include "NovelMind/core/result.hpp"
#include "NovelMind/ui/ui_framework.hpp"
#include "NovelMind/scene/scene_graph.hpp"
#include "NovelMind/scene/scene_inspector.hpp"
#include "NovelMind/scripting/ir.hpp"
#include "NovelMind/editor/sdl_imgui_backend.hpp"
#include <string>
#include <memory>
#include <vector>
#include <functional>

namespace NovelMind::editor
{

// Forward declarations
class ProjectBrowser;
class SceneView;
class StoryGraphView;
class InspectorPanel;
class AssetBrowser;
class PreviewWindow;
class BuildManager;

/**
 * @brief Editor configuration
 */
struct EditorConfig
{
    std::string windowTitle = "NovelMind Editor";
    i32 windowWidth = 1920;
    i32 windowHeight = 1080;
    bool maximized = false;
    std::string lastProjectPath;
    std::string theme = "dark";
    f32 uiScale = 1.0f;
    bool showWelcomeScreen = true;
    bool autoSave = true;
    i32 autoSaveIntervalMinutes = 5;
};

/**
 * @brief Project metadata
 */
struct ProjectInfo
{
    std::string name;
    std::string path;
    std::string version = "1.0.0";
    std::string author;
    std::string description;
    std::vector<std::string> recentFiles;
    u64 createdTimestamp = 0;
    u64 modifiedTimestamp = 0;
};

/**
 * @brief Asset entry for the asset browser
 */
struct AssetEntry
{
    enum class Type : u8
    {
        Folder,
        Image,
        Audio,
        Script,
        Font,
        Video,
        Data,
        Unknown
    };

    std::string name;
    std::string path;
    std::string relativePath;
    Type type;
    u64 size = 0;
    u64 modifiedTimestamp = 0;
    bool selected = false;
};

/**
 * @brief Editor panel base class
 */
class EditorPanel
{
public:
    EditorPanel(const std::string& title);
    virtual ~EditorPanel() = default;

    virtual void update(f64 deltaTime) = 0;
    virtual void render() = 0;
    virtual void onResize(i32 width, i32 height) = 0;

    void setVisible(bool visible) { m_visible = visible; }
    [[nodiscard]] bool isVisible() const { return m_visible; }

    void setFocused(bool focused) { m_focused = focused; }
    [[nodiscard]] bool isFocused() const { return m_focused; }

    [[nodiscard]] const std::string& getTitle() const { return m_title; }

protected:
    std::string m_title;
    bool m_visible = true;
    bool m_focused = false;
    i32 m_x = 0, m_y = 0;
    i32 m_width = 0, m_height = 0;
};

/**
 * @brief Project Browser Panel - File tree view
 */
class ProjectBrowser : public EditorPanel
{
public:
    ProjectBrowser();
    ~ProjectBrowser() override = default;

    void update(f64 deltaTime) override;
    void render() override;
    void onResize(i32 width, i32 height) override;

    void setRootPath(const std::string& path);
    void refresh();

    void setOnFileSelected(std::function<void(const std::string&)> callback);
    void setOnFileDoubleClicked(std::function<void(const std::string&)> callback);

private:
    void scanDirectory(const std::string& path);
    AssetEntry::Type determineAssetType(const std::string& extension);

    std::string m_rootPath;
    std::vector<AssetEntry> m_entries;
    std::string m_selectedPath;

    std::function<void(const std::string&)> m_onFileSelected;
    std::function<void(const std::string&)> m_onFileDoubleClicked;
};

/**
 * @brief Scene View Panel - Visual scene editor with preview
 */
class SceneView : public EditorPanel, public scene::IInspectorListener
{
public:
    SceneView();
    ~SceneView() override;

    void update(f64 deltaTime) override;
    void render() override;
    void onResize(i32 width, i32 height) override;

    void setSceneGraph(scene::SceneGraph* sceneGraph);
    void setInspectorAPI(scene::SceneInspectorAPI* inspector);

    // Viewport control
    void setViewportOffset(f32 x, f32 y);
    void setViewportZoom(f32 zoom);
    void centerViewport();

    // Grid and guides
    void setShowGrid(bool show);
    void setGridSize(f32 size);
    void setSnapToGrid(bool snap);

    // Selection handling
    void selectObjectAt(f32 x, f32 y);
    void boxSelect(f32 x1, f32 y1, f32 x2, f32 y2);

    // IInspectorListener implementation
    void onSelectionChanged(const std::vector<std::string>& selectedIds) override;
    void onSceneModified() override;
    void onUndoStackChanged(bool canUndo, bool canRedo) override;

private:
    void renderGrid();
    void renderObjects();
    void renderSelection();
    void renderGizmos();

    scene::SceneGraph* m_sceneGraph = nullptr;
    scene::SceneInspectorAPI* m_inspector = nullptr;

    f32 m_viewportX = 0.0f;
    f32 m_viewportY = 0.0f;
    f32 m_viewportZoom = 1.0f;

    bool m_showGrid = true;
    f32 m_gridSize = 32.0f;
    bool m_snapToGrid = false;

    bool m_isDragging = false;
    f32 m_dragStartX = 0.0f;
    f32 m_dragStartY = 0.0f;
};

/**
 * @brief Story Graph View Panel - Node-based script editor
 */
class StoryGraphView : public EditorPanel
{
public:
    StoryGraphView();
    ~StoryGraphView() override = default;

    void update(f64 deltaTime) override;
    void render() override;
    void onResize(i32 width, i32 height) override;

    void setVisualGraph(scripting::VisualGraph* graph);

    // Node operations
    void addNode(scripting::IRNodeType type, f32 x, f32 y);
    void deleteSelectedNodes();
    void duplicateSelectedNodes();

    // Connection operations
    void startConnection(scripting::NodeId nodeId, const std::string& portId);
    void completeConnection(scripting::NodeId nodeId, const std::string& portId);
    void cancelConnection();

    // View control
    void setViewOffset(f32 x, f32 y);
    void setViewZoom(f32 zoom);
    void centerView();
    void fitToContent();

    // Selection
    void selectNode(scripting::NodeId nodeId);
    void selectNodes(const std::vector<scripting::NodeId>& nodeIds);
    void clearSelection();
    [[nodiscard]] const std::vector<scripting::NodeId>& getSelectedNodes() const;

    // Callbacks
    void setOnNodeSelected(std::function<void(scripting::NodeId)> callback);
    void setOnGraphModified(std::function<void()> callback);

private:
    void renderNodes();
    void renderConnections();
    void renderPendingConnection();
    void renderMinimap();

    scripting::VisualGraph* m_graph = nullptr;

    f32 m_viewX = 0.0f;
    f32 m_viewY = 0.0f;
    f32 m_viewZoom = 1.0f;

    std::vector<scripting::NodeId> m_selectedNodes;

    bool m_isConnecting = false;
    scripting::NodeId m_connectionStartNode = 0;
    std::string m_connectionStartPort;
    f32 m_connectionEndX = 0.0f;
    f32 m_connectionEndY = 0.0f;

    std::function<void(scripting::NodeId)> m_onNodeSelected;
    std::function<void()> m_onGraphModified;
};

/**
 * @brief Inspector Panel - Property editor
 */
class InspectorPanel : public EditorPanel, public scene::IInspectorListener
{
public:
    InspectorPanel();
    ~InspectorPanel() override;

    void update(f64 deltaTime) override;
    void render() override;
    void onResize(i32 width, i32 height) override;

    void setInspectorAPI(scene::SceneInspectorAPI* inspector);
    void inspectObject(const std::string& objectId);
    void inspectNode(const scripting::VisualGraphNode* node);
    void clearInspection();

    // IInspectorListener implementation
    void onSelectionChanged(const std::vector<std::string>& selectedIds) override;
    void onSceneModified() override;
    void onUndoStackChanged(bool canUndo, bool canRedo) override;

private:
    void renderPropertyGroup(const std::string& groupName,
                             const std::vector<scene::PropertyDescriptor>& properties);
    void renderProperty(const scene::PropertyDescriptor& prop);

    scene::SceneInspectorAPI* m_inspector = nullptr;
    std::string m_currentObjectId;
    const scripting::VisualGraphNode* m_currentNode = nullptr;

    std::unique_ptr<ui::Widget> m_propertyWidgets;
};

/**
 * @brief Asset Browser Panel - Asset management
 */
class AssetBrowser : public EditorPanel
{
public:
    AssetBrowser();
    ~AssetBrowser() override = default;

    void update(f64 deltaTime) override;
    void render() override;
    void onResize(i32 width, i32 height) override;

    void setAssetsPath(const std::string& path);
    void refresh();

    void setViewMode(bool gridView);
    void setThumbnailSize(i32 size);

    void setOnAssetSelected(std::function<void(const AssetEntry&)> callback);
    void setOnAssetDoubleClicked(std::function<void(const AssetEntry&)> callback);

    // Asset operations
    Result<void> importAsset(const std::string& sourcePath);
    Result<void> deleteAsset(const std::string& assetPath);
    Result<void> renameAsset(const std::string& oldPath, const std::string& newPath);
    Result<void> createFolder(const std::string& folderName);

private:
    void scanAssets();
    void generateThumbnail(AssetEntry& entry);

    std::string m_assetsPath;
    std::string m_currentFolder;
    std::vector<AssetEntry> m_assets;
    std::vector<std::string> m_breadcrumbs;

    bool m_gridView = true;
    i32 m_thumbnailSize = 64;

    std::function<void(const AssetEntry&)> m_onAssetSelected;
    std::function<void(const AssetEntry&)> m_onAssetDoubleClicked;
};

/**
 * @brief Preview Window - Runtime preview
 */
class PreviewWindow : public EditorPanel
{
public:
    PreviewWindow();
    ~PreviewWindow() override = default;

    void update(f64 deltaTime) override;
    void render() override;
    void onResize(i32 width, i32 height) override;

    void startPreview();
    void stopPreview();
    void pausePreview();
    void resumePreview();
    void stepFrame();

    [[nodiscard]] bool isPreviewRunning() const { return m_running; }
    [[nodiscard]] bool isPreviewPaused() const { return m_paused; }

    void setSceneGraph(scene::SceneGraph* sceneGraph);

private:
    scene::SceneGraph* m_sceneGraph = nullptr;
    scene::SceneState m_savedState;

    bool m_running = false;
    bool m_paused = false;
    f64 m_previewTime = 0.0;
};

/**
 * @brief Build Manager - Compilation and export
 */
class BuildManager
{
public:
    struct BuildConfig
    {
        std::string targetPlatform = "windows";
        std::string outputPath;
        bool debug = false;
        bool includeDevTools = false;
        bool compress = true;
        std::vector<std::string> excludePatterns;
    };

    struct BuildProgress
    {
        std::string currentTask;
        f32 progress = 0.0f;  // 0.0 - 1.0
        std::vector<std::string> warnings;
        std::vector<std::string> errors;
        bool completed = false;
        bool success = false;
    };

    BuildManager();
    ~BuildManager() = default;

    void setProjectPath(const std::string& projectPath);

    Result<void> startBuild(const BuildConfig& config);
    void cancelBuild();

    [[nodiscard]] bool isBuildInProgress() const { return m_building; }
    [[nodiscard]] const BuildProgress& getProgress() const { return m_progress; }

    void setOnBuildComplete(std::function<void(bool success)> callback);
    void setOnBuildProgress(std::function<void(const BuildProgress&)> callback);

    // Build steps
    Result<void> validateProject();
    Result<void> compileScripts();
    Result<void> packAssets();
    Result<void> generateExecutable();

private:
    std::string m_projectPath;
    BuildConfig m_currentConfig;
    BuildProgress m_progress;
    bool m_building = false;
    bool m_cancelRequested = false;

    std::function<void(bool)> m_onBuildComplete;
    std::function<void(const BuildProgress&)> m_onBuildProgress;
};

/**
 * @brief Main Editor Application
 */
class EditorApp
{
public:
    EditorApp();
    ~EditorApp();

    /**
     * @brief Initialize the editor
     */
    Result<void> initialize(const EditorConfig& config = {});

    /**
     * @brief Run the editor main loop
     */
    void run();

    /**
     * @brief Shutdown the editor
     */
    void shutdown();

    // Project management
    Result<void> newProject(const std::string& path, const std::string& name);
    Result<void> openProject(const std::string& path);
    Result<void> saveProject();
    Result<void> closeProject();
    [[nodiscard]] bool hasProject() const { return m_projectLoaded; }
    [[nodiscard]] const ProjectInfo& getProjectInfo() const { return m_projectInfo; }

    // File operations
    Result<void> newScene();
    Result<void> openScene(const std::string& path);
    Result<void> saveScene();
    Result<void> saveSceneAs(const std::string& path);

    Result<void> newScript();
    Result<void> openScript(const std::string& path);
    Result<void> saveScript();
    Result<void> saveScriptAs(const std::string& path);

    // Edit operations
    void undo();
    void redo();
    void cut();
    void copy();
    void paste();
    void deleteSelection();
    void selectAll();

    // View operations
    void setActivePanel(const std::string& panelName);
    void togglePanel(const std::string& panelName);
    void resetLayout();

    // Build operations
    Result<void> build(const BuildManager::BuildConfig& config);
    Result<void> quickBuild();
    void startPreview();
    void stopPreview();

    // Access to panels
    [[nodiscard]] ProjectBrowser* getProjectBrowser() { return m_projectBrowser.get(); }
    [[nodiscard]] SceneView* getSceneView() { return m_sceneView.get(); }
    [[nodiscard]] StoryGraphView* getStoryGraphView() { return m_storyGraphView.get(); }
    [[nodiscard]] InspectorPanel* getInspectorPanel() { return m_inspectorPanel.get(); }
    [[nodiscard]] AssetBrowser* getAssetBrowser() { return m_assetBrowser.get(); }
    [[nodiscard]] PreviewWindow* getPreviewWindow() { return m_previewWindow.get(); }

    // Access to systems
    [[nodiscard]] scene::SceneGraph* getSceneGraph() { return m_sceneGraph.get(); }
    [[nodiscard]] scene::SceneInspectorAPI* getInspectorAPI() { return m_inspectorAPI.get(); }
    [[nodiscard]] scripting::RoundTripConverter* getConverter() { return m_converter.get(); }
    [[nodiscard]] BuildManager* getBuildManager() { return m_buildManager.get(); }
    [[nodiscard]] ui::UIManager* getUIManager() { return m_uiManager.get(); }

    // Configuration
    void setConfig(const EditorConfig& config);
    [[nodiscard]] const EditorConfig& getConfig() const { return m_config; }
    void saveConfig();
    void loadConfig();

private:
    void setupMenuBar();
    void setupToolbar();
    void setupPanels();
    void setupShortcuts();

    void processInput();
    void update(f64 deltaTime);
    void render();

    void handleFileDropped(const std::string& path);
    void showWelcomeScreen();
    void showAboutDialog();
    void renderMainMenuBar();

    // Dialog rendering
    void renderNewProjectDialog();
    void renderOpenProjectDialog();
    void renderOpenSceneDialog();
    void renderAboutDialogContent();
    void renderErrorDialog();
    void renderConfirmationDialog();
    void renderProgressDialog();
    void renderFileBrowserDialog();
    void showError(const std::string& message);
    void showConfirmation(const std::string& message, std::function<void()> onConfirm, std::function<void()> onCancel = nullptr);
    void showProgress(const std::string& title, const std::string& message, float progress);
    void hideProgress();

    // Helper methods
    void loadRecentProjects();
    void saveRecentProjects();
    void addToRecentProjects(const std::string& path);
    std::vector<std::string> getProjectTemplates() const;

    EditorConfig m_config;
    bool m_initialized = false;
    bool m_running = false;

    // Project state
    ProjectInfo m_projectInfo;
    bool m_projectLoaded = false;
    bool m_projectModified = false;
    std::string m_currentScenePath;
    std::string m_currentScriptPath;

    // Core systems
    std::unique_ptr<SDLImGuiBackend> m_backend;
    std::unique_ptr<scene::SceneGraph> m_sceneGraph;
    std::unique_ptr<scene::SceneInspectorAPI> m_inspectorAPI;
    std::unique_ptr<scripting::RoundTripConverter> m_converter;
    std::unique_ptr<scripting::VisualGraph> m_visualGraph;
    std::unique_ptr<BuildManager> m_buildManager;
    std::unique_ptr<ui::UIManager> m_uiManager;

    // Panels
    std::unique_ptr<ProjectBrowser> m_projectBrowser;
    std::unique_ptr<SceneView> m_sceneView;
    std::unique_ptr<StoryGraphView> m_storyGraphView;
    std::unique_ptr<InspectorPanel> m_inspectorPanel;
    std::unique_ptr<AssetBrowser> m_assetBrowser;
    std::unique_ptr<PreviewWindow> m_previewWindow;

    // Auto-save timer
    f64 m_autoSaveTimer = 0.0;

    // Dialog state
    bool m_showNewProjectDialog = false;
    bool m_showOpenProjectDialog = false;
    bool m_showOpenSceneDialog = false;
    bool m_showAboutDialog = false;
    bool m_showErrorDialog = false;
    bool m_showConfirmationDialog = false;
    bool m_showProgressDialog = false;
    bool m_showFileBrowser = false;
    std::string m_errorDialogMessage;
    std::string m_confirmationDialogMessage;
    std::string m_progressDialogTitle;
    std::string m_progressDialogMessage;
    float m_progressDialogValue = 0.0f;
    std::function<void()> m_confirmationCallback;
    std::function<void()> m_confirmationCancelCallback;

    // Dialog input buffers
    char m_newProjectNameBuffer[256] = "";
    char m_newProjectPathBuffer[512] = "";
    char m_openFilePathBuffer[512] = "";

    // File browser state
    enum class FileBrowserMode { OpenFile, OpenFolder, SaveFile };
    FileBrowserMode m_fileBrowserMode = FileBrowserMode::OpenFolder;
    std::string m_fileBrowserCurrentPath;
    std::string m_fileBrowserSelectedPath;
    std::function<void(const std::string&)> m_fileBrowserCallback;
    std::vector<std::string> m_fileBrowserEntries;
    bool m_fileBrowserNeedsRefresh = true;

    // Recent projects
    std::vector<std::string> m_recentProjects;

    // Project templates
    int m_selectedTemplate = 0;
};

} // namespace NovelMind::editor
