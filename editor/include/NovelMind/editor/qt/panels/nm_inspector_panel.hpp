#pragma once

/**
 * @file nm_inspector_panel.hpp
 * @brief Inspector panel for viewing and editing object properties
 *
 * Displays properties of the currently selected object:
 * - Property groups (collapsible)
 * - Various property types (text, number, color, etc.)
 * - Read-only view (Phase 1)
 * - Editable properties (Phase 2+)
 */

#include "NovelMind/editor/qt/nm_dock_panel.hpp"
#include <QScrollArea>
#include <QLabel>
#include <QVBoxLayout>

namespace NovelMind::editor::qt {

/**
 * @brief A collapsible group box for property categories
 */
class NMPropertyGroup : public QWidget
{
    Q_OBJECT

public:
    explicit NMPropertyGroup(const QString& title, QWidget* parent = nullptr);

    void setExpanded(bool expanded);
    [[nodiscard]] bool isExpanded() const { return m_expanded; }

    void addProperty(const QString& name, const QString& value);
    void addProperty(const QString& name, QWidget* widget);
    void clearProperties();

private slots:
    void onHeaderClicked();

private:
    QWidget* m_header = nullptr;
    QWidget* m_content = nullptr;
    QVBoxLayout* m_contentLayout = nullptr;
    QLabel* m_expandIcon = nullptr;
    bool m_expanded = true;
};

/**
 * @brief Inspector panel for property editing
 */
class NMInspectorPanel : public NMDockPanel
{
    Q_OBJECT

public:
    explicit NMInspectorPanel(QWidget* parent = nullptr);
    ~NMInspectorPanel() override;

    void onInitialize() override;
    void onUpdate(double deltaTime) override;

    /**
     * @brief Clear all properties
     */
    void clear();

    /**
     * @brief Show properties for an object
     * @param objectType Type of the object (for header display)
     * @param objectId ID of the object
     */
    void inspectObject(const QString& objectType, const QString& objectId);

    /**
     * @brief Add a property group
     */
    NMPropertyGroup* addGroup(const QString& title);

    /**
     * @brief Show "nothing selected" message
     */
    void showNoSelection();

signals:
    void propertyChanged(const QString& objectId, const QString& propertyName, const QString& newValue);

private:
    void setupContent();

    QScrollArea* m_scrollArea = nullptr;
    QWidget* m_scrollContent = nullptr;
    QVBoxLayout* m_mainLayout = nullptr;
    QLabel* m_headerLabel = nullptr;
    QLabel* m_noSelectionLabel = nullptr;
    QList<NMPropertyGroup*> m_groups;
};

} // namespace NovelMind::editor::qt
