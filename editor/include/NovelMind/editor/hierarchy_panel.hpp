#pragma once

/**
 * @file hierarchy_panel.hpp
 * @brief Hierarchy Panel for NovelMind Editor v0.2.0
 *
 * The Hierarchy panel displays the scene object tree:
 * - Parent-child relationships
 * - Object visibility toggles
 * - Drag-drop reparenting
 * - Multi-selection support
 * - Search filtering
 * - Full context menu operations
 * - Sync with SceneView selection
 */

#include "NovelMind/editor/gui_panel_base.hpp"
#include <functional>
#include <string>
#include <vector>

namespace NovelMind::scene {
class SceneGraph;
}

namespace NovelMind::editor {

/**
 * @brief Scene hierarchy node
 */
struct HierarchyNode {
    std::string id;
    std::string name;
    std::string type;
    std::vector<HierarchyNode> children;
    bool visible = true;
    bool locked = false;
    bool expanded = true;
    i32 sortOrder = 0;  // Render order within parent
    i32 layer = 0;      // Layer assignment
};

/**
 * @brief Drag drop mode for hierarchy reordering
 */
enum class HierarchyDropPosition : u8 {
    Before,     // Drop before target (sibling above)
    After,      // Drop after target (sibling below)
    Inside      // Drop as child of target
};

/**
 * @brief Callback for hierarchy operations
 */
using HierarchyOperationCallback = std::function<void(const std::string& nodeId)>;
using HierarchyReparentCallback = std::function<void(const std::string& nodeId,
                                                       const std::string& newParentId,
                                                       i32 index)>;

/**
 * @brief Hierarchy Panel implementation
 */
class HierarchyPanel : public GUIPanelBase {
public:
    HierarchyPanel();
    ~HierarchyPanel() override = default;

    /**
     * @brief Refresh hierarchy from scene
     */
    void refresh();

    /**
     * @brief Set search filter
     */
    void setFilter(const std::string& filter) { m_filter = filter; }

    /**
     * @brief Get current filter
     */
    [[nodiscard]] const std::string& getFilter() const { return m_filter; }

    // =========================================================================
    // Object Operations
    // =========================================================================

    /**
     * @brief Create a new empty object
     */
    void createEmpty(const std::string& parentId = "");

    /**
     * @brief Create a child object
     */
    void createChild(const std::string& parentId);

    /**
     * @brief Duplicate selected objects
     */
    void duplicateSelected();

    /**
     * @brief Delete selected objects
     */
    void deleteSelected();

    /**
     * @brief Rename object
     */
    void startRename(const std::string& nodeId);

    /**
     * @brief Set object visibility
     */
    void setObjectVisible(const std::string& nodeId, bool visible);

    /**
     * @brief Toggle object lock
     */
    void setObjectLocked(const std::string& nodeId, bool locked);

    /**
     * @brief Isolate object (hide all others)
     */
    void isolateObject(const std::string& nodeId);

    /**
     * @brief Show only specified layer
     */
    void showOnlyLayer(i32 layer);

    /**
     * @brief Show all objects
     */
    void showAll();

    // =========================================================================
    // Drag-Drop & Reordering
    // =========================================================================

    /**
     * @brief Reparent node
     */
    void reparentNode(const std::string& nodeId, const std::string& newParentId, i32 index = -1);

    /**
     * @brief Move node up in sibling order
     */
    void moveNodeUp(const std::string& nodeId);

    /**
     * @brief Move node down in sibling order
     */
    void moveNodeDown(const std::string& nodeId);

    /**
     * @brief Move to front (highest render order)
     */
    void moveToFront(const std::string& nodeId);

    /**
     * @brief Move to back (lowest render order)
     */
    void moveToBack(const std::string& nodeId);

    // =========================================================================
    // Selection Sync
    // =========================================================================

    /**
     * @brief Scroll to show selected node
     */
    void scrollToSelected();

    /**
     * @brief Expand parents of selected node
     */
    void revealSelected();

    /**
     * @brief Check if node is being renamed
     */
    [[nodiscard]] bool isRenaming() const { return m_isRenaming; }

    // =========================================================================
    // Callbacks
    // =========================================================================

    /**
     * @brief Set callback for double-click (navigate in SceneView)
     */
    void setOnDoubleClick(HierarchyOperationCallback callback) { m_onDoubleClick = std::move(callback); }

    /**
     * @brief Set callback for reparent operations
     */
    void setOnReparent(HierarchyReparentCallback callback) { m_onReparent = std::move(callback); }

    [[nodiscard]] std::vector<ToolbarItem> getToolbarItems() const override;
    [[nodiscard]] std::vector<MenuItem> getContextMenuItems() const override;
    [[nodiscard]] std::vector<MenuItem> getMenuItems() const override;

protected:
    void onInitialize() override;
    void onRender() override;
    void renderToolbar() override;

    void onSelectionChanged(SelectionType type,
                           const std::vector<SelectionItem>& selection) override;

private:
    void renderNode(const HierarchyNode& node, int depth = 0);
    void renderNodeContextMenu(const HierarchyNode& node);
    void handleNodeSelection(const std::string& nodeId, bool ctrlHeld, bool shiftHeld);
    void handleDragDrop(const std::string& draggedId, const std::string& targetId,
                        HierarchyDropPosition position);

    HierarchyNode* findNode(const std::string& id);
    HierarchyNode* findParentNode(const std::string& childId);
    bool isDescendantOf(const std::string& nodeId, const std::string& potentialAncestorId) const;

    std::vector<HierarchyNode> m_rootNodes;
    std::string m_filter;
    char m_searchBuffer[256] = {0};

    // For drag-drop reparenting
    std::string m_draggedNodeId;
    HierarchyDropPosition m_dropPosition = HierarchyDropPosition::Inside;
    std::string m_dropTargetId;

    // Rename state
    bool m_isRenaming = false;
    std::string m_renamingNodeId;
    char m_renameBuffer[256] = {0};

    // Scroll state
    bool m_needsScroll = false;
    std::string m_scrollTargetId;

    // Callbacks
    HierarchyOperationCallback m_onDoubleClick;
    HierarchyReparentCallback m_onReparent;

    // Scene integration (populated during onInitialize())
    scene::SceneGraph* m_sceneGraph = nullptr;
};

} // namespace NovelMind::editor
