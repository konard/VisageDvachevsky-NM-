#pragma once

/**
 * @file nm_diagnostics_panel.hpp
 * @brief Diagnostics panel for errors, warnings, and issues
 */

#include "NovelMind/editor/qt/nm_dock_panel.hpp"

#include <QWidget>
#include <QToolBar>

class QTreeWidget;
class QToolBar;
class QPushButton;

namespace NovelMind::editor::qt {

class NMDiagnosticsPanel : public NMDockPanel
{
    Q_OBJECT

public:
    explicit NMDiagnosticsPanel(QWidget* parent = nullptr);
    ~NMDiagnosticsPanel() override;

    [[nodiscard]] QString panelName() const override { return "Diagnostics"; }
    bool initialize() override;
    void shutdown() override;
    void onUpdate(double deltaTime) override;

private:
    void setupUI();

    QTreeWidget* m_diagnosticsTree = nullptr;
    QToolBar* m_toolbar = nullptr;
};

} // namespace NovelMind::editor::qt
