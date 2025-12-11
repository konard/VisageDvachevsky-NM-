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
        {"name", "Name", "General", "Object name", PropertyType::String, false, {}, ""},
        {"position_x", "Position X", "Transform", "X coordinate", PropertyType::Float, false, {}, "", -10000.0f, 10000.0f, 1.0f},
        {"position_y", "Position Y", "Transform", "Y coordinate", PropertyType::Float, false, {}, "", -10000.0f, 10000.0f, 1.0f},
        {"rotation", "Rotation", "Transform", "Rotation in degrees", PropertyType::Float, false, {}, "", 0.0f, 360.0f, 1.0f},
        {"scale_x", "Scale X", "Transform", "Horizontal scale", PropertyType::Float, false, {}, "", 0.0f, 10.0f, 0.1f},
        {"scale_y", "Scale Y", "Transform", "Vertical scale", PropertyType::Float, false, {}, "", 0.0f, 10.0f, 0.1f},
        {"visible", "Visible", "Rendering", "Object visibility", PropertyType::Bool, false, {}, ""},
        {"alpha", "Alpha", "Rendering", "Opacity (0-1)", PropertyType::Float, false, {}, "", 0.0f, 1.0f, 0.01f},
    };

    m_propertyCache["GraphNode"] = {
        {"type", "Type", "Node", "Node type", PropertyType::String, true, {}, ""},
        {"position", "Position", "Node", "Position in graph", PropertyType::Vector2, true, {}, ""},
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
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    // Header with object name
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.5f, 0.8f, 1.0f));
    ImGui::Text("Scene Object");
    ImGui::PopStyleColor();

    const auto& objectId = selection.getObjectId();
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "ID: %s", objectId.c_str());
    ImGui::Separator();
    ImGui::Spacing();

    // Get property definitions
    auto it = m_propertyCache.find("SceneObject");
    if (it == m_propertyCache.end())
    {
        return;
    }

    // Group properties by category
    std::string currentCategory;
    bool headerOpen = true;

    for (const auto& propDef : it->second)
    {
        if (propDef.category != currentCategory)
        {
            currentCategory = propDef.category;
            headerOpen = ImGui::CollapsingHeader(currentCategory.c_str(), ImGuiTreeNodeFlags_DefaultOpen);
        }

        if (headerOpen)
        {
            std::string value = selection.getPropertyValue(propDef.name);
            renderProperty(propDef.displayName, value, propDef);
        }
    }
#else
    widgets::SectionHeader("Scene Object");
    const auto& objectId = selection.getObjectId();

    auto it = m_propertyCache.find("SceneObject");
    if (it == m_propertyCache.end())
    {
        return;
    }

    std::string currentCategory;

    for (const auto& propDef : it->second)
    {
        if (propDef.category != currentCategory)
        {
            currentCategory = propDef.category;
            widgets::CollapsingHeader(currentCategory.c_str());
        }

        std::string value = selection.getPropertyValue(propDef.name);
        renderProperty(propDef.displayName, value, propDef);
    }

    (void)objectId;
#endif
}

void InspectorPanel::renderGraphNodeInspector(const StoryGraphNodeSelection& selection)
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.5f, 0.0f, 1.0f));
    ImGui::Text("Story Graph Node");
    ImGui::PopStyleColor();

    const auto* node = selection.getNode();
    if (!node)
    {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Node not found");
        return;
    }

    // Node info
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "ID: %zu", selection.getNodeId());
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Type: %s", node->type.c_str());
    ImGui::Separator();
    ImGui::Spacing();

    // Position
    if (ImGui::CollapsingHeader("Position", ImGuiTreeNodeFlags_DefaultOpen))
    {
        f32 pos[2] = {node->x, node->y};
        ImGui::PushItemWidth(-1);
        if (ImGui::DragFloat2("##Position", pos, 1.0f))
        {
            // Would update node position through graph system
        }
        ImGui::PopItemWidth();
    }

    // Node-specific properties
    if (ImGui::CollapsingHeader("Properties", ImGuiTreeNodeFlags_DefaultOpen))
    {
        for (const auto& [key, value] : node->properties)
        {
            PropertyDefinition propDef;
            propDef.name = key;
            propDef.displayName = key;
            propDef.type = PropertyType::String;

            renderProperty(key, value, propDef);
        }

        if (node->properties.empty())
        {
            ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.4f, 1.0f), "No properties");
        }
    }
#else
    widgets::SectionHeader("Story Graph Node");

    const auto* node = selection.getNode();
    if (!node)
    {
        return;
    }

    if (widgets::CollapsingHeader("Position"))
    {
        f32 pos[2] = {node->x, node->y};
        widgets::Vector2Input("Position", pos);
    }

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
#endif
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
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    ImGui::PushID(name.c_str());

    // Label on the left
    ImGui::AlignTextToFramePadding();
    ImGui::Text("%s", name.c_str());
    ImGui::SameLine(120.0f);
    ImGui::SetNextItemWidth(-1);

    bool changed = false;

    switch (def.type)
    {
        case PropertyType::Bool:
        {
            bool boolValue = (value == "true" || value == "1");
            if (ImGui::Checkbox("##value", &boolValue))
            {
                changed = true;
                // Would emit PropertyChangedEvent
            }
            break;
        }

        case PropertyType::Int:
        {
            i32 intValue = value.empty() ? 0 : std::stoi(value);
            if (ImGui::DragInt("##value", &intValue, 1.0f,
                               static_cast<int>(def.minValue), static_cast<int>(def.maxValue)))
            {
                changed = true;
            }
            break;
        }

        case PropertyType::Float:
        {
            f32 floatValue = value.empty() ? 0.0f : std::stof(value);
            if (ImGui::DragFloat("##value", &floatValue, def.step, def.minValue, def.maxValue, "%.2f"))
            {
                changed = true;
            }
            break;
        }

        case PropertyType::String:
        {
            char buffer[256];
            std::strncpy(buffer, value.c_str(), sizeof(buffer) - 1);
            buffer[sizeof(buffer) - 1] = '\0';

            if (def.readOnly)
            {
                ImGui::TextDisabled("%s", buffer);
            }
            else if (ImGui::InputText("##value", buffer, sizeof(buffer)))
            {
                changed = true;
            }
            break;
        }

        case PropertyType::Vector2:
        {
            f32 vec[2] = {0.0f, 0.0f};
            auto commaPos = value.find(',');
            if (commaPos != std::string::npos)
            {
                try {
                    vec[0] = std::stof(value.substr(0, commaPos));
                    vec[1] = std::stof(value.substr(commaPos + 1));
                } catch (...) {}
            }
            if (ImGui::DragFloat2("##value", vec, 1.0f))
            {
                changed = true;
            }
            break;
        }

        case PropertyType::Color:
        {
            f32 color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
            if (ImGui::ColorEdit4("##value", color, ImGuiColorEditFlags_NoInputs))
            {
                changed = true;
            }
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

            const char* preview = currentIndex >= 0 && static_cast<size_t>(currentIndex) < def.enumValues.size() ?
                def.enumValues[static_cast<size_t>(currentIndex)].c_str() : "";

            if (ImGui::BeginCombo("##value", preview))
            {
                for (size_t i = 0; i < def.enumValues.size(); i++)
                {
                    bool isSelected = (static_cast<size_t>(currentIndex) == i);
                    if (ImGui::Selectable(def.enumValues[i].c_str(), isSelected))
                    {
                        currentIndex = static_cast<i32>(i);
                        changed = true;
                    }
                    if (isSelected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
            break;
        }

        case PropertyType::Asset:
        {
            char buffer[256];
            std::strncpy(buffer, value.c_str(), sizeof(buffer) - 1);
            buffer[sizeof(buffer) - 1] = '\0';

            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 30);
            if (ImGui::InputText("##assetpath", buffer, sizeof(buffer)))
            {
                changed = true;
            }
            ImGui::SameLine();
            if (ImGui::Button("...", ImVec2(25, 0)))
            {
                // Would open asset browser/picker
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("Browse for %s", def.assetType.c_str());
            }
            break;
        }

        default:
        {
            ImGui::TextDisabled("%s", value.c_str());
            break;
        }
    }

    // Tooltip
    if (!def.tooltip.empty() && ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("%s", def.tooltip.c_str());
    }

    ImGui::PopID();

    (void)changed;
#else
    // Stub implementation uses widgets
    switch (def.type)
    {
        case PropertyType::Vector2:
        {
            f32 vec[2] = {0.0f, 0.0f};
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
            break;
    }
#endif
}

} // namespace NovelMind::editor
