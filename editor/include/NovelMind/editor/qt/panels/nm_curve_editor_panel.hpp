#pragma once

/**
 * @file nm_curve_editor_panel.hpp
 * @brief Curve editor for animation curves and interpolation
 */

#include "NovelMind/editor/qt/nm_dock_panel.hpp"

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>

class QToolBar;
class QPushButton;
class QComboBox;

namespace NovelMind::editor::qt {

/**
 * @brief Curve editor panel for editing animation curves
 */
class NMCurveEditorPanel : public NMDockPanel
{
    Q_OBJECT

public:
    explicit NMCurveEditorPanel(QWidget* parent = nullptr);
    ~NMCurveEditorPanel() override;

    [[nodiscard]] QString panelName() const override { return "Curve Editor"; }
    bool initialize() override;
    void shutdown() override;
    void onUpdate(double deltaTime) override;

private:
    void setupUI();

    QGraphicsView* m_curveView = nullptr;
    QGraphicsScene* m_curveScene = nullptr;
    QToolBar* m_toolbar = nullptr;
};

} // namespace NovelMind::editor::qt
