/**
 * @file inspector_panel.cpp
 * @brief Inspector Panel implementation
 */

#include "NovelMind/editor/inspector_panel.hpp"
#include "NovelMind/editor/imgui_integration.hpp"
#include <cstring>

#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
#include <imgui.h>
#endif

namespace NovelMind::editor {

InspectorPanel::InspectorPanel()
    : GUIPanelBase("Inspector")
{
}

void InspectorPanel::refresh()
{
    // Force refresh of property display
}

void InspectorPanel::clear()
{
    if (!m_isLocked)
    {
        m_lockedItem = SelectionItem();
    }
}

std::vector<ToolbarItem> InspectorPanel::getToolbarItems() const
{
    std::vector<ToolbarItem> items;

    items.push_back({"Lock", "Lock Inspector",
                     [this]() { const_cast<InspectorPanel*>(this)->setLocked(!m_isLocked); },
                     []() { return true; }, [this]() { return m_isLocked; }});

    items.push_back({"Refresh", "Refresh Properties",
                     [this]() { const_cast<InspectorPanel*>(this)->refresh(); }});

    return items;
}

void InspectorPanel::onInitialize()
{
    // Initialize property definitions for common types
    m_propertyCache["SceneObject"] = {
        {"name", "Name", "General", "Object name", PropertyType::String},
        {"position_x", "Position X", "Transform", "X coordinate", PropertyType::Float, false, {}, "", -10000.0f, 10000.0f, 1.0f},
        {"position_y", "Position Y", "Transform", "Y coordinate", PropertyType::Float, false, {}, "", -10000.0f, 10000.0f, 1.0f},
        {"rotation", "Rotation", "Transform", "Rotation in degrees", PropertyType::Float, false, {}, "", 0.0f, 360.0f, 1.0f},
        {"scale_x", "Scale X", "Transform", "Horizontal scale", PropertyType::Float, false, {}, "", 0.0f, 10.0f, 0.1f},
        {"scale_y", "Scale Y", "Transform", "Vertical scale", PropertyType::Float, false, {}, "", 0.0f, 10.0f, 0.1f},
        {"visible", "Visible", "Rendering", "Object visibility", PropertyType::Bool},
        {"alpha", "Alpha", "Rendering", "Opacity (0-1)", PropertyType::Float, false, {}, "", 0.0f, 1.0f, 0.01f},
    };

    m_propertyCache["GraphNode"] = {
        {"type", "Type", "Node", "Node type", PropertyType::String, true},
        {"position", "Position", "Node", "Position in graph", PropertyType::Vector2, true},
    };
}

void InspectorPanel::onRender()
{
    // Get selection to inspect
    SelectionItem itemToInspect;
    if (m_isLocked && m_lockedItem.isValid())
    {
        itemToInspect = m_lockedItem;
    }
    else if (auto primary = getSelection().getPrimarySelection())
    {
        itemToInspect = *primary;
    }

    if (!itemToInspect.isValid())
    {
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "No selection");
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.4f, 1.0f), "Select an object in the Scene View");
        ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.4f, 1.0f), "or Story Graph to inspect it.");
#endif
        return;
    }

    // Render appropriate inspector based on selection type
    switch (itemToInspect.type)
    {
        case SelectionType::SceneObject:
            if (auto selection = getSelection().getSceneObjectSelection())
            {
                renderSceneObjectInspector(*selection);
            }
            break;

        case SelectionType::StoryGraphNode:
            if (auto selection = getSelection().getStoryGraphNodeSelection())
            {
                renderGraphNodeInspector(*selection);
            }
            break;

        case SelectionType::TimelineItem:
            if (auto selection = getSelection().getTimelineItemSelection())
            {
                renderTimelineItemInspector(*selection);
            }
            break;

        default:
            // ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Unsupported selection type");
            break;
    }
}

void InspectorPanel::renderToolbar()
{
    widgets::BeginToolbar("InspectorToolbar");
    renderToolbarItems(getToolbarItems());
    widgets::EndToolbar();
}

void InspectorPanel::onSelectionChanged(SelectionType /*type*/,
                                        const std::vector<SelectionItem>& /*selection*/)
{
    if (!m_isLocked)
    {
        refresh();
    }
}

void InspectorPanel::onPrimarySelectionChanged(const SelectionItem& item)
{
    if (!m_isLocked)
    {
        m_lockedItem = item;
        refresh();
    }
}

void InspectorPanel::renderSceneObjectInspector(const SceneObjectSelection& selection)
{
    // Header with object name
    widgets::SectionHeader("Scene Object");

    const auto& objectId = selection.getObjectId();
    // ImGui::Text("ID: %s", objectId.c_str());
    // ImGui::Separator();

    // Get property definitions
    auto it = m_propertyCache.find("SceneObject");
    if (it == m_propertyCache.end())
    {
        return;
    }

    // Group properties by category
    std::string currentCategory;

    for (const auto& propDef : it->second)
    {
        if (propDef.category != currentCategory)
        {
            currentCategory = propDef.category;
            if (widgets::CollapsingHeader(currentCategory.c_str()))
            {
                // Properties in this category will be rendered below
            }
        }

        std::string value = selection.getPropertyValue(propDef.name);
        renderProperty(propDef.displayName, value, propDef);
    }

    (void)objectId;
}

void InspectorPanel::renderGraphNodeInspector(const StoryGraphNodeSelection& selection)
{
    widgets::SectionHeader("Story Graph Node");

    const auto* node = selection.getNode();
    if (!node)
    {
        // ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Node not found");
        return;
    }

    // Node info
    // ImGui::Text("ID: %zu", selection.getNodeId());
    // ImGui::Text("Type: %s", node->type.c_str());
    // ImGui::Separator();

    // Position
    if (widgets::CollapsingHeader("Position"))
    {
        f32 pos[2] = {node->x, node->y};
        if (widgets::Vector2Input("Position", pos))
        {
            // Would update node position
        }
    }

    // Node-specific properties
    if (widgets::CollapsingHeader("Properties"))
    {
        for (const auto& [key, value] : node->properties)
        {
            PropertyDefinition propDef;
            propDef.name = key;
            propDef.displayName = key;
            propDef.type = PropertyType::String;

            renderProperty(key, value, propDef);
        }
    }
}

void InspectorPanel::renderTimelineItemInspector(const TimelineItemSelection& selection)
{
    widgets::SectionHeader("Timeline Item");

    const auto& itemId = selection.getItemId();
    // ImGui::Text("Track: %s", itemId.trackId.c_str());
    // ImGui::Text("Keyframe: %llu", itemId.keyframeIndex);
    // ImGui::Separator();

    // Keyframe properties
    if (widgets::CollapsingHeader("Keyframe"))
    {
        for (const auto& propName : selection.getPropertyNames())
        {
            std::string value = selection.getPropertyValue(propName);
            PropertyDefinition propDef;
            propDef.name = propName;
            propDef.displayName = propName;
            propDef.type = PropertyType::String;

            renderProperty(propName, value, propDef);
        }
    }

    (void)itemId;
}

void InspectorPanel::renderProperty(const std::string& name, const std::string& value,
                                    const PropertyDefinition& def)
{
    // ImGui::PushID(name.c_str());

    switch (def.type)
    {
        case PropertyType::Bool:
        {
            bool boolValue = (value == "true" || value == "1");
            // if (ImGui::Checkbox(name.c_str(), &boolValue))
            // {
            //     // Handle change
            // }
            (void)boolValue;
            break;
        }

        case PropertyType::Int:
        {
            i32 intValue = value.empty() ? 0 : std::stoi(value);
            // if (ImGui::DragInt(name.c_str(), &intValue))
            // {
            //     // Handle change
            // }
            (void)intValue;
            break;
        }

        case PropertyType::Float:
        {
            f32 floatValue = value.empty() ? 0.0f : std::stof(value);
            // if (ImGui::DragFloat(name.c_str(), &floatValue, def.step, def.minValue, def.maxValue))
            // {
            //     // Handle change
            // }
            (void)floatValue;
            break;
        }

        case PropertyType::String:
        {
            char buffer[256];
            std::strncpy(buffer, value.c_str(), sizeof(buffer) - 1);
            buffer[sizeof(buffer) - 1] = '\0';
            // if (ImGui::InputText(name.c_str(), buffer, sizeof(buffer)))
            // {
            //     // Handle change
            // }
            (void)buffer;
            break;
        }

        case PropertyType::Vector2:
        {
            f32 vec[2] = {0.0f, 0.0f};
            // Parse "x,y" format
            auto commaPos = value.find(',');
            if (commaPos != std::string::npos)
            {
                vec[0] = std::stof(value.substr(0, commaPos));
                vec[1] = std::stof(value.substr(commaPos + 1));
            }
            widgets::Vector2Input(name.c_str(), vec);
            break;
        }

        case PropertyType::Color:
        {
            f32 color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
            widgets::ColorPickerButton(name.c_str(), color);
            break;
        }

        case PropertyType::Enum:
        {
            i32 currentIndex = 0;
            for (size_t i = 0; i < def.enumValues.size(); ++i)
            {
                if (def.enumValues[i] == value)
                {
                    currentIndex = static_cast<i32>(i);
                    break;
                }
            }
            widgets::Dropdown(name.c_str(), &currentIndex, def.enumValues);
            break;
        }

        case PropertyType::Asset:
        {
            std::string assetPath = value;
            widgets::AssetReference(name.c_str(), assetPath, def.assetType.c_str());
            break;
        }

        default:
        {
            // ImGui::Text("%s: %s", name.c_str(), value.c_str());
            break;
        }
    }

    // Tooltip
    if (!def.tooltip.empty())
    {
        // if (ImGui::IsItemHovered())
        // {
        //     ImGui::SetTooltip("%s", def.tooltip.c_str());
        // }
    }

    // ImGui::PopID();
}

} // namespace NovelMind::editor
