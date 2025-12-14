#ifndef NOVELMIND_EDITOR_NM_DEBUG_OVERLAY_PANEL_HPP
#define NOVELMIND_EDITOR_NM_DEBUG_OVERLAY_PANEL_HPP

#include <NovelMind/editor/qt/nm_dock_panel.hpp>
#include <QTabWidget>
#include <QTreeWidget>
#include <QListWidget>
#include <QVBoxLayout>
#include <QVariantMap>

namespace NovelMind::editor::qt {

/**
 * @brief Debug overlay panel for runtime inspection
 *
 * Provides tabs for:
 * - Variables (editable during pause)
 * - Call Stack
 * - Active Animations
 * - Audio Channels
 * - Performance Metrics
 */
class NMDebugOverlayPanel : public NMDockPanel {
    Q_OBJECT

public:
    explicit NMDebugOverlayPanel(QWidget* parent = nullptr);
    ~NMDebugOverlayPanel() override = default;

    void initialize() override;
    void shutdown() override;
    void onUpdate(float deltaTime) override;

private slots:
    void onVariablesChanged(const QVariantMap& variables);
    void onCallStackChanged(const QStringList& stack);
    void onPlayModeChanged(int mode);
    void onVariableItemDoubleClicked(QTreeWidgetItem* item, int column);

private:
    void setupUI();
    void updateVariablesTab(const QVariantMap& variables);
    void updateCallStackTab(const QStringList& stack);
    void editVariable(const QString& name, const QVariant& currentValue);

    // UI Elements
    QTabWidget* m_tabWidget = nullptr;

    // Variables Tab
    QTreeWidget* m_variablesTree = nullptr;

    // Call Stack Tab
    QListWidget* m_callStackList = nullptr;

    // Animations Tab
    QTreeWidget* m_animationsTree = nullptr;

    // Audio Tab
    QTreeWidget* m_audioTree = nullptr;

    // Performance Tab
    QTreeWidget* m_performanceTree = nullptr;

    // State
    QVariantMap m_currentVariables;
    QStringList m_currentCallStack;
};

} // namespace NovelMind::editor::qt

#endif // NOVELMIND_EDITOR_NM_DEBUG_OVERLAY_PANEL_HPP
