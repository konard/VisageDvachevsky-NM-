/**
 * @file hierarchy_panel.cpp
 * @brief Hierarchy Panel implementation
 */

#include "NovelMind/editor/hierarchy_panel.hpp"
#include "NovelMind/editor/imgui_integration.hpp"
#include <algorithm>
#include <cstring>

namespace NovelMind::editor {

HierarchyPanel::HierarchyPanel()
    : GUIPanelBase("Hierarchy")
{
}

void HierarchyPanel::refresh()
{
    // Would reload from scene manager
    m_rootNodes.clear();

    // Placeholder data for demonstration
    HierarchyNode background;
    background.id = "bg_001";
    background.name = "Background";
    background.type = "Background";
    background.layer = 0;
    background.sortOrder = 0;

    HierarchyNode character1;
    character1.id = "char_001";
    character1.name = "Character: Alice";
    character1.type = "CharacterSprite";
    character1.layer = 1;
    character1.sortOrder = 0;

    HierarchyNode character2;
    character2.id = "char_002";
    character2.name = "Character: Bob";
    character2.type = "CharacterSprite";
    character2.layer = 1;
    character2.sortOrder = 1;

    HierarchyNode ui;
    ui.id = "ui_001";
    ui.name = "UI";
    ui.type = "Container";
    ui.layer = 2;
    ui.sortOrder = 0;

    HierarchyNode dialogueBox;
    dialogueBox.id = "dlg_001";
    dialogueBox.name = "Dialogue Box";
    dialogueBox.type = "DialogueBox";
    dialogueBox.layer = 2;
    dialogueBox.sortOrder = 0;
    ui.children.push_back(dialogueBox);

    HierarchyNode choiceMenu;
    choiceMenu.id = "choice_001";
    choiceMenu.name = "Choice Menu";
    choiceMenu.type = "ChoiceMenu";
    choiceMenu.visible = false;
    choiceMenu.layer = 2;
    choiceMenu.sortOrder = 1;
    ui.children.push_back(choiceMenu);

    m_rootNodes.push_back(background);
    m_rootNodes.push_back(character1);
    m_rootNodes.push_back(character2);
    m_rootNodes.push_back(ui);
}

// =========================================================================
// Object Operations
// =========================================================================

void HierarchyPanel::createEmpty(const std::string& parentId)
{
    HierarchyNode newNode;
    newNode.id = "obj_" + std::to_string(std::rand());
    newNode.name = "New Object";
    newNode.type = "Empty";

    if (parentId.empty())
    {
        m_rootNodes.push_back(newNode);
    }
    else
    {
        if (HierarchyNode* parent = findNode(parentId))
        {
            parent->children.push_back(newNode);
        }
    }

    SceneObjectEvent event(EditorEventType::SceneObjectCreated);
    event.objectId = newNode.id;
    publishEvent(event);

    // Auto-select the new object
    getSelection().selectObject(newNode.id);
}

void HierarchyPanel::createChild(const std::string& parentId)
{
    createEmpty(parentId);
}

void HierarchyPanel::duplicateSelected()
{
    auto selectedIds = getSelection().getSelectedObjectIds();
    std::vector<std::string> duplicatedIds;

    for (const auto& id : selectedIds)
    {
        if (HierarchyNode* node = findNode(id))
        {
            HierarchyNode duplicate = *node;
            duplicate.id = node->id + "_copy_" + std::to_string(std::rand());
            duplicate.name = node->name + " (Copy)";

            // Add as sibling
            HierarchyNode* parent = findParentNode(id);
            if (parent)
            {
                parent->children.push_back(duplicate);
            }
            else
            {
                m_rootNodes.push_back(duplicate);
            }

            duplicatedIds.push_back(duplicate.id);

            SceneObjectEvent event(EditorEventType::SceneObjectCreated);
            event.objectId = duplicate.id;
            publishEvent(event);
        }
    }

    // Select duplicated objects
    getSelection().clearSelection();
    for (const auto& id : duplicatedIds)
    {
        getSelection().addToSelection(SelectionItem(id));
    }
}

void HierarchyPanel::deleteSelected()
{
    auto selectedIds = getSelection().getSelectedObjectIds();

    for (const auto& id : selectedIds)
    {
        // Remove from parent's children
        HierarchyNode* parent = findParentNode(id);
        if (parent)
        {
            auto& children = parent->children;
            children.erase(
                std::remove_if(children.begin(), children.end(),
                    [&id](const HierarchyNode& n) { return n.id == id; }),
                children.end());
        }
        else
        {
            // Remove from root
            m_rootNodes.erase(
                std::remove_if(m_rootNodes.begin(), m_rootNodes.end(),
                    [&id](const HierarchyNode& n) { return n.id == id; }),
                m_rootNodes.end());
        }

        SceneObjectEvent event(EditorEventType::SceneObjectDeleted);
        event.objectId = id;
        publishEvent(event);
    }

    getSelection().clearSelection();
}

void HierarchyPanel::startRename(const std::string& nodeId)
{
    if (HierarchyNode* node = findNode(nodeId))
    {
        m_isRenaming = true;
        m_renamingNodeId = nodeId;
        std::strncpy(m_renameBuffer, node->name.c_str(), sizeof(m_renameBuffer) - 1);
        m_renameBuffer[sizeof(m_renameBuffer) - 1] = '\0';
    }
}

void HierarchyPanel::setObjectVisible(const std::string& nodeId, bool visible)
{
    if (HierarchyNode* node = findNode(nodeId))
    {
        node->visible = visible;

        SceneObjectEvent event(EditorEventType::SceneLayerChanged);
        event.objectId = nodeId;
        publishEvent(event);
    }
}

void HierarchyPanel::setObjectLocked(const std::string& nodeId, bool locked)
{
    if (HierarchyNode* node = findNode(nodeId))
    {
        node->locked = locked;
    }
}

void HierarchyPanel::isolateObject(const std::string& nodeId)
{
    // Hide all objects except the specified one
    std::function<void(HierarchyNode&)> hideAll = [&hideAll, &nodeId](HierarchyNode& node) {
        node.visible = (node.id == nodeId);
        for (auto& child : node.children)
        {
            hideAll(child);
        }
    };

    for (auto& root : m_rootNodes)
    {
        hideAll(root);
    }

    // Make sure the isolated object and its parents are visible
    if (HierarchyNode* node = findNode(nodeId))
    {
        node->visible = true;
    }

    SceneObjectEvent event(EditorEventType::SceneLayerChanged);
    publishEvent(event);
}

void HierarchyPanel::showOnlyLayer(i32 layer)
{
    std::function<void(HierarchyNode&)> setVisibility = [&setVisibility, layer](HierarchyNode& node) {
        node.visible = (node.layer == layer);
        for (auto& child : node.children)
        {
            setVisibility(child);
        }
    };

    for (auto& root : m_rootNodes)
    {
        setVisibility(root);
    }

    SceneObjectEvent event(EditorEventType::SceneLayerChanged);
    publishEvent(event);
}

void HierarchyPanel::showAll()
{
    std::function<void(HierarchyNode&)> showAllNodes = [&showAllNodes](HierarchyNode& node) {
        node.visible = true;
        for (auto& child : node.children)
        {
            showAllNodes(child);
        }
    };

    for (auto& root : m_rootNodes)
    {
        showAllNodes(root);
    }

    SceneObjectEvent event(EditorEventType::SceneLayerChanged);
    publishEvent(event);
}

// =========================================================================
// Drag-Drop & Reordering
// =========================================================================

void HierarchyPanel::reparentNode(const std::string& nodeId, const std::string& newParentId, i32 index)
{
    // Prevent reparenting to self or descendant
    if (nodeId == newParentId || isDescendantOf(newParentId, nodeId))
    {
        return;
    }

    HierarchyNode* node = findNode(nodeId);
    if (!node)
    {
        return;
    }

    HierarchyNode nodeCopy = *node;

    // Remove from current parent
    HierarchyNode* oldParent = findParentNode(nodeId);
    if (oldParent)
    {
        auto& children = oldParent->children;
        children.erase(
            std::remove_if(children.begin(), children.end(),
                [&nodeId](const HierarchyNode& n) { return n.id == nodeId; }),
            children.end());
    }
    else
    {
        m_rootNodes.erase(
            std::remove_if(m_rootNodes.begin(), m_rootNodes.end(),
                [&nodeId](const HierarchyNode& n) { return n.id == nodeId; }),
            m_rootNodes.end());
    }

    // Add to new parent
    if (newParentId.empty())
    {
        if (index < 0 || index >= static_cast<i32>(m_rootNodes.size()))
        {
            m_rootNodes.push_back(nodeCopy);
        }
        else
        {
            m_rootNodes.insert(m_rootNodes.begin() + index, nodeCopy);
        }
    }
    else
    {
        HierarchyNode* newParent = findNode(newParentId);
        if (newParent)
        {
            if (index < 0 || index >= static_cast<i32>(newParent->children.size()))
            {
                newParent->children.push_back(nodeCopy);
            }
            else
            {
                newParent->children.insert(newParent->children.begin() + index, nodeCopy);
            }
        }
    }

    // Notify
    if (m_onReparent)
    {
        m_onReparent(nodeId, newParentId, index);
    }

    SceneObjectEvent event(EditorEventType::SceneLayerChanged);
    event.objectId = nodeId;
    publishEvent(event);
}

void HierarchyPanel::moveNodeUp(const std::string& nodeId)
{
    HierarchyNode* parent = findParentNode(nodeId);
    std::vector<HierarchyNode>& siblings = parent ? parent->children : m_rootNodes;

    for (size_t i = 1; i < siblings.size(); ++i)
    {
        if (siblings[i].id == nodeId)
        {
            std::swap(siblings[i], siblings[i - 1]);
            break;
        }
    }

    SceneObjectEvent event(EditorEventType::SceneLayerChanged);
    event.objectId = nodeId;
    publishEvent(event);
}

void HierarchyPanel::moveNodeDown(const std::string& nodeId)
{
    HierarchyNode* parent = findParentNode(nodeId);
    std::vector<HierarchyNode>& siblings = parent ? parent->children : m_rootNodes;

    for (size_t i = 0; i + 1 < siblings.size(); ++i)
    {
        if (siblings[i].id == nodeId)
        {
            std::swap(siblings[i], siblings[i + 1]);
            break;
        }
    }

    SceneObjectEvent event(EditorEventType::SceneLayerChanged);
    event.objectId = nodeId;
    publishEvent(event);
}

void HierarchyPanel::moveToFront(const std::string& nodeId)
{
    HierarchyNode* parent = findParentNode(nodeId);
    std::vector<HierarchyNode>& siblings = parent ? parent->children : m_rootNodes;

    for (size_t i = 0; i < siblings.size(); ++i)
    {
        if (siblings[i].id == nodeId)
        {
            HierarchyNode node = siblings[i];
            siblings.erase(siblings.begin() + static_cast<std::ptrdiff_t>(i));
            siblings.push_back(node);
            break;
        }
    }

    SceneObjectEvent event(EditorEventType::SceneLayerChanged);
    event.objectId = nodeId;
    publishEvent(event);
}

void HierarchyPanel::moveToBack(const std::string& nodeId)
{
    HierarchyNode* parent = findParentNode(nodeId);
    std::vector<HierarchyNode>& siblings = parent ? parent->children : m_rootNodes;

    for (size_t i = 0; i < siblings.size(); ++i)
    {
        if (siblings[i].id == nodeId)
        {
            HierarchyNode node = siblings[i];
            siblings.erase(siblings.begin() + static_cast<std::ptrdiff_t>(i));
            siblings.insert(siblings.begin(), node);
            break;
        }
    }

    SceneObjectEvent event(EditorEventType::SceneLayerChanged);
    event.objectId = nodeId;
    publishEvent(event);
}

// =========================================================================
// Selection Sync
// =========================================================================

void HierarchyPanel::scrollToSelected()
{
    auto selectedIds = getSelection().getSelectedObjectIds();
    if (!selectedIds.empty())
    {
        m_needsScroll = true;
        m_scrollTargetId = selectedIds[0];
    }
}

void HierarchyPanel::revealSelected()
{
    auto selectedIds = getSelection().getSelectedObjectIds();

    for (const auto& id : selectedIds)
    {
        // Expand all parents
        std::function<bool(HierarchyNode&)> expandParents = [&expandParents, &id](HierarchyNode& node) -> bool {
            if (node.id == id)
            {
                return true;
            }
            for (auto& child : node.children)
            {
                if (expandParents(child))
                {
                    node.expanded = true;
                    return true;
                }
            }
            return false;
        };

        for (auto& root : m_rootNodes)
        {
            expandParents(root);
        }
    }
}

// =========================================================================
// Menu/Toolbar Items
// =========================================================================

std::vector<ToolbarItem> HierarchyPanel::getToolbarItems() const
{
    std::vector<ToolbarItem> items;

    items.push_back({"+", "Create Object", [this]() { const_cast<HierarchyPanel*>(this)->createEmpty(); }});
    items.push_back({"Refresh", "Refresh Hierarchy", [this]() { const_cast<HierarchyPanel*>(this)->refresh(); }});

    return items;
}

std::vector<MenuItem> HierarchyPanel::getMenuItems() const
{
    std::vector<MenuItem> items;

    MenuItem viewMenu;
    viewMenu.label = "View";
    viewMenu.subItems = {
        {"Reveal Selected", "", [this]() { const_cast<HierarchyPanel*>(this)->revealSelected(); }},
        {"Show All", "", [this]() { const_cast<HierarchyPanel*>(this)->showAll(); }},
    };
    items.push_back(viewMenu);

    return items;
}

std::vector<MenuItem> HierarchyPanel::getContextMenuItems() const
{
    std::vector<MenuItem> items;
    bool hasSelection = getSelection().hasSelection();
    std::string selectedId;
    if (hasSelection)
    {
        auto ids = getSelection().getSelectedObjectIds();
        if (!ids.empty()) selectedId = ids[0];
    }

    // Create submenu
    MenuItem createMenu;
    createMenu.label = "Create";
    createMenu.subItems = {
        {"Empty Object", "", [this]() { const_cast<HierarchyPanel*>(this)->createEmpty(); }},
        {"Character", "", []() { /* Create character */ }},
        {"Background", "", []() { /* Create background */ }},
        {"UI Element", "", []() { /* Create UI */ }},
    };
    items.push_back(createMenu);

    items.push_back({"Create Child", "",
        [this, selectedId]() { const_cast<HierarchyPanel*>(this)->createChild(selectedId); },
        [hasSelection]() { return hasSelection; }});

    items.push_back(MenuItem::separator());

    items.push_back({"Cut", "Ctrl+X", []() { /* Cut action */ }, [hasSelection]() { return hasSelection; }});
    items.push_back({"Copy", "Ctrl+C", []() { /* Copy action */ }, [hasSelection]() { return hasSelection; }});
    items.push_back({"Paste", "Ctrl+V", []() { /* Paste action */ }});

    items.push_back(MenuItem::separator());

    items.push_back({"Duplicate", "Ctrl+D",
        [this]() { const_cast<HierarchyPanel*>(this)->duplicateSelected(); },
        [hasSelection]() { return hasSelection; }});
    items.push_back({"Delete", "Delete",
        [this]() { const_cast<HierarchyPanel*>(this)->deleteSelected(); },
        [hasSelection]() { return hasSelection; }});
    items.push_back({"Rename", "F2",
        [this, selectedId]() { const_cast<HierarchyPanel*>(this)->startRename(selectedId); },
        [hasSelection]() { return hasSelection; }});

    items.push_back(MenuItem::separator());

    // Order submenu
    MenuItem orderMenu;
    orderMenu.label = "Order";
    orderMenu.isEnabled = [hasSelection]() { return hasSelection; };
    orderMenu.subItems = {
        {"Bring to Front", "Ctrl+Shift+]",
            [this, selectedId]() { const_cast<HierarchyPanel*>(this)->moveToFront(selectedId); }},
        {"Send to Back", "Ctrl+Shift+[",
            [this, selectedId]() { const_cast<HierarchyPanel*>(this)->moveToBack(selectedId); }},
        {"Move Up", "Ctrl+]",
            [this, selectedId]() { const_cast<HierarchyPanel*>(this)->moveNodeUp(selectedId); }},
        {"Move Down", "Ctrl+[",
            [this, selectedId]() { const_cast<HierarchyPanel*>(this)->moveNodeDown(selectedId); }},
    };
    items.push_back(orderMenu);

    items.push_back(MenuItem::separator());

    items.push_back({"Isolate", "",
        [this, selectedId]() { const_cast<HierarchyPanel*>(this)->isolateObject(selectedId); },
        [hasSelection]() { return hasSelection; }});
    items.push_back({"Show All", "",
        [this]() { const_cast<HierarchyPanel*>(this)->showAll(); }});

    return items;
}

void HierarchyPanel::onInitialize()
{
    refresh();

    subscribeEvent<SceneObjectEvent>(
        [this](const SceneObjectEvent& /*event*/) {
            refresh();
        });
}

void HierarchyPanel::onRender()
{
    // Search bar
    if (widgets::SearchInput("##HierarchySearch", m_searchBuffer, sizeof(m_searchBuffer), "Search..."))
    {
        m_filter = m_searchBuffer;
    }

    // Render tree
    for (const auto& node : m_rootNodes)
    {
        renderNode(node);
    }

    // Handle scroll to selected
    if (m_needsScroll)
    {
        // Would scroll to m_scrollTargetId
        m_needsScroll = false;
    }

    // Handle rename completion
    if (m_isRenaming)
    {
        // Check for Enter/Escape keys would happen here
        // For now, assume rename completes on next focus change
    }
}

void HierarchyPanel::renderToolbar()
{
    widgets::BeginToolbar("HierarchyToolbar");
    renderToolbarItems(getToolbarItems());
    widgets::EndToolbar();
}

void HierarchyPanel::onSelectionChanged(SelectionType type,
                                        const std::vector<SelectionItem>& /*selection*/)
{
    if (type == SelectionType::SceneObject)
    {
        // Reveal and scroll to selected item
        revealSelected();
        scrollToSelected();
    }
}

void HierarchyPanel::renderNode(const HierarchyNode& node, int depth)
{
    // Apply filter
    if (!m_filter.empty())
    {
        std::string nameLower = node.name;
        std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
        std::string filterLower = m_filter;
        std::transform(filterLower.begin(), filterLower.end(), filterLower.begin(), ::tolower);

        bool matchesFilter = nameLower.find(filterLower) != std::string::npos;
        bool childMatches = false;

        std::function<bool(const HierarchyNode&)> checkChildren = [&](const HierarchyNode& n) -> bool {
            std::string childNameLower = n.name;
            std::transform(childNameLower.begin(), childNameLower.end(), childNameLower.begin(), ::tolower);
            if (childNameLower.find(filterLower) != std::string::npos)
            {
                return true;
            }
            for (const auto& child : n.children)
            {
                if (checkChildren(child))
                {
                    return true;
                }
            }
            return false;
        };

        for (const auto& child : node.children)
        {
            if (checkChildren(child))
            {
                childMatches = true;
                break;
            }
        }

        if (!matchesFilter && !childMatches)
        {
            return;
        }
    }

    bool isSelected = getSelection().isObjectSelected(node.id);
    bool isLeaf = node.children.empty();

    // Display visibility indicator
    std::string displayName = node.name;
    if (!node.visible)
    {
        displayName = "[H] " + displayName;
    }
    if (node.locked)
    {
        displayName = "[L] " + displayName;
    }

    // Render rename input if this node is being renamed
    if (m_isRenaming && m_renamingNodeId == node.id)
    {
        // Would render text input here
        displayName = std::string(m_renameBuffer);
    }

    // Tree node
    bool isExpanded = widgets::TreeNode(displayName.c_str(), isLeaf, isSelected,
                                        "HIERARCHY_NODE", (void*)node.id.c_str());

    // Handle double-click
    // Would trigger m_onDoubleClick(node.id) to focus in SceneView

    // Render context menu
    renderNodeContextMenu(node);

    // Render children
    if (isExpanded && !isLeaf)
    {
        for (const auto& child : node.children)
        {
            renderNode(child, depth + 1);
        }
    }

    (void)depth;
}

void HierarchyPanel::renderNodeContextMenu(const HierarchyNode& node)
{
    // This would be called when right-clicking on the node
    // The actual popup rendering happens in the ImGui context
    (void)node;
}

void HierarchyPanel::handleNodeSelection(const std::string& nodeId, bool ctrlHeld, bool shiftHeld)
{
    if (ctrlHeld)
    {
        // Toggle selection
        if (getSelection().isObjectSelected(nodeId))
        {
            getSelection().removeFromSelection(SelectionItem(nodeId));
        }
        else
        {
            getSelection().addToSelection(SelectionItem(nodeId));
        }
    }
    else if (shiftHeld)
    {
        // Range selection (would need to implement)
        getSelection().addToSelection(SelectionItem(nodeId));
    }
    else
    {
        // Single selection
        getSelection().selectObject(nodeId);
    }
}

void HierarchyPanel::handleDragDrop(const std::string& draggedId, const std::string& targetId,
                                    HierarchyDropPosition position)
{
    if (draggedId == targetId)
    {
        return;
    }

    // Prevent dragging into own descendant
    if (isDescendantOf(targetId, draggedId))
    {
        return;
    }

    switch (position)
    {
        case HierarchyDropPosition::Inside:
            reparentNode(draggedId, targetId, -1);
            break;
        case HierarchyDropPosition::Before:
        case HierarchyDropPosition::After:
        {
            // Find target's parent and index
            HierarchyNode* parent = findParentNode(targetId);
            std::vector<HierarchyNode>& siblings = parent ? parent->children : m_rootNodes;
            i32 targetIndex = 0;
            for (size_t i = 0; i < siblings.size(); ++i)
            {
                if (siblings[i].id == targetId)
                {
                    targetIndex = static_cast<i32>(i);
                    if (position == HierarchyDropPosition::After)
                    {
                        targetIndex++;
                    }
                    break;
                }
            }
            reparentNode(draggedId, parent ? parent->id : "", targetIndex);
            break;
        }
    }
}

// =========================================================================
// Helper Methods
// =========================================================================

HierarchyNode* HierarchyPanel::findNode(const std::string& id)
{
    std::function<HierarchyNode*(std::vector<HierarchyNode>&)> search =
        [&search, &id](std::vector<HierarchyNode>& nodes) -> HierarchyNode* {
        for (auto& node : nodes)
        {
            if (node.id == id)
            {
                return &node;
            }
            if (HierarchyNode* found = search(node.children))
            {
                return found;
            }
        }
        return nullptr;
    };

    return search(m_rootNodes);
}

HierarchyNode* HierarchyPanel::findParentNode(const std::string& childId)
{
    std::function<HierarchyNode*(std::vector<HierarchyNode>&, HierarchyNode*)> search =
        [&search, &childId](std::vector<HierarchyNode>& nodes, HierarchyNode* parent) -> HierarchyNode* {
        for (auto& node : nodes)
        {
            if (node.id == childId)
            {
                return parent;
            }
            if (HierarchyNode* found = search(node.children, &node))
            {
                return found;
            }
        }
        return nullptr;
    };

    return search(m_rootNodes, nullptr);
}

bool HierarchyPanel::isDescendantOf(const std::string& nodeId, const std::string& potentialAncestorId) const
{
    std::function<bool(const std::vector<HierarchyNode>&)> search =
        [&search, &nodeId](const std::vector<HierarchyNode>& nodes) -> bool {
        for (const auto& node : nodes)
        {
            if (node.id == nodeId)
            {
                return true;
            }
            if (search(node.children))
            {
                return true;
            }
        }
        return false;
    };

    // Find the ancestor node and search its descendants
    std::function<const HierarchyNode*(const std::vector<HierarchyNode>&)> findAncestor =
        [&findAncestor, &potentialAncestorId](const std::vector<HierarchyNode>& nodes) -> const HierarchyNode* {
        for (const auto& node : nodes)
        {
            if (node.id == potentialAncestorId)
            {
                return &node;
            }
            if (const HierarchyNode* found = findAncestor(node.children))
            {
                return found;
            }
        }
        return nullptr;
    };

    const HierarchyNode* ancestor = findAncestor(m_rootNodes);
    if (!ancestor)
    {
        return false;
    }

    return search(ancestor->children);
}

} // namespace NovelMind::editor
