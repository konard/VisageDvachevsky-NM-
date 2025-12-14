#include <NovelMind/editor/qt/panels/nm_debug_overlay_panel.hpp>
#include <NovelMind/editor/qt/nm_play_mode_controller.hpp>
#include <NovelMind/editor/qt/nm_icon_manager.hpp>
#include <QInputDialog>
#include <QHeaderView>
#include <QPushButton>
#include <QHBoxLayout>

namespace NovelMind::editor::qt {

NMDebugOverlayPanel::NMDebugOverlayPanel(QWidget* parent)
    : NMDockPanel("Debug Overlay", parent)
{
    setupUI();
}

void NMDebugOverlayPanel::initialize() {
    NMDockPanel::initialize();

    auto& controller = NMPlayModeController::instance();

    // Connect to controller signals
    connect(&controller, &NMPlayModeController::variablesChanged,
            this, &NMDebugOverlayPanel::onVariablesChanged);
    connect(&controller, &NMPlayModeController::callStackChanged,
            this, &NMDebugOverlayPanel::onCallStackChanged);

    // Initial update
    updateVariablesTab(controller.currentVariables());
    updateCallStackTab(controller.callStack());
}

void NMDebugOverlayPanel::shutdown() {
    NMDockPanel::shutdown();
}

void NMDebugOverlayPanel::onUpdate(float deltaTime) {
    NMDockPanel::onUpdate(deltaTime);
}

void NMDebugOverlayPanel::setupUI() {
    auto* layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_tabWidget = new QTabWidget;

    auto& iconMgr = NMIconManager::instance();

    // === Variables Tab ===
    {
        auto* varWidget = new QWidget;
        auto* varLayout = new QVBoxLayout(varWidget);
        varLayout->setContentsMargins(4, 4, 4, 4);

        m_variablesTree = new QTreeWidget;
        m_variablesTree->setHeaderLabels({"Name", "Value", "Type"});
        m_variablesTree->setAlternatingRowColors(true);
        m_variablesTree->setEditTriggers(QAbstractItemView::NoEditTriggers);
        m_variablesTree->header()->setStretchLastSection(false);
        m_variablesTree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
        m_variablesTree->header()->setSectionResizeMode(1, QHeaderView::Stretch);
        m_variablesTree->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);

        connect(m_variablesTree, &QTreeWidget::itemDoubleClicked,
                this, &NMDebugOverlayPanel::onVariableItemDoubleClicked);

        auto* helpLabel = new QLabel("💡 Double-click a variable to edit (only when paused)");
        helpLabel->setStyleSheet("QLabel { color: #a0a0a0; font-size: 9pt; padding: 4px; }");

        varLayout->addWidget(m_variablesTree);
        varLayout->addWidget(helpLabel);

        m_tabWidget->addTab(varWidget, iconMgr.getIcon("info", 16), "Variables");
    }

    // === Call Stack Tab ===
    {
        auto* stackWidget = new QWidget;
        auto* stackLayout = new QVBoxLayout(stackWidget);
        stackLayout->setContentsMargins(4, 4, 4, 4);

        m_callStackList = new QListWidget;
        m_callStackList->setAlternatingRowColors(true);

        stackLayout->addWidget(m_callStackList);

        m_tabWidget->addTab(stackWidget, "Call Stack");
    }

    // === Animations Tab ===
    {
        auto* animWidget = new QWidget;
        auto* animLayout = new QVBoxLayout(animWidget);
        animLayout->setContentsMargins(4, 4, 4, 4);

        m_animationsTree = new QTreeWidget;
        m_animationsTree->setHeaderLabels({"Animation", "Progress", "Target"});
        m_animationsTree->setAlternatingRowColors(true);

        // Add placeholder items
        auto* item1 = new QTreeWidgetItem(m_animationsTree, {"character_fade", "65%", "Character_Alice"});
        auto* item2 = new QTreeWidgetItem(m_animationsTree, {"bg_transition", "95%", "Background_School"});
        item1->setForeground(1, QBrush(QColor("#4caf50")));
        item2->setForeground(1, QBrush(QColor("#4caf50")));

        animLayout->addWidget(m_animationsTree);

        m_tabWidget->addTab(animWidget, "Animations");
    }

    // === Audio Tab ===
    {
        auto* audioWidget = new QWidget;
        auto* audioLayout = new QVBoxLayout(audioWidget);
        audioLayout->setContentsMargins(4, 4, 4, 4);

        m_audioTree = new QTreeWidget;
        m_audioTree->setHeaderLabels({"Channel", "File", "Volume", "State"});
        m_audioTree->setAlternatingRowColors(true);

        // Add placeholder items
        auto* item1 = new QTreeWidgetItem(m_audioTree, {"BGM", "music_menu.ogg", "80%", "Playing"});
        auto* item2 = new QTreeWidgetItem(m_audioTree, {"SFX_1", "door_open.wav", "100%", "Stopped"});
        item1->setForeground(3, QBrush(QColor("#4caf50")));
        item2->setForeground(3, QBrush(QColor("#a0a0a0")));

        audioLayout->addWidget(m_audioTree);

        m_tabWidget->addTab(audioWidget, "Audio");
    }

    // === Performance Tab ===
    {
        auto* perfWidget = new QWidget;
        auto* perfLayout = new QVBoxLayout(perfWidget);
        perfLayout->setContentsMargins(4, 4, 4, 4);

        m_performanceTree = new QTreeWidget;
        m_performanceTree->setHeaderLabels({"Metric", "Value"});
        m_performanceTree->setAlternatingRowColors(true);
        m_performanceTree->header()->setStretchLastSection(true);

        // Add placeholder metrics
        new QTreeWidgetItem(m_performanceTree, {"Frame Time", "16.7 ms"});
        new QTreeWidgetItem(m_performanceTree, {"FPS", "60"});
        new QTreeWidgetItem(m_performanceTree, {"Memory Usage", "45 MB"});
        new QTreeWidgetItem(m_performanceTree, {"Active Objects", "12"});
        new QTreeWidgetItem(m_performanceTree, {"Script Instructions/sec", "1,250"});

        perfLayout->addWidget(m_performanceTree);

        m_tabWidget->addTab(perfWidget, "Performance");
    }

    layout->addWidget(m_tabWidget);
    setLayout(layout);
}

void NMDebugOverlayPanel::updateVariablesTab(const QVariantMap& variables) {
    m_currentVariables = variables;

    m_variablesTree->clear();

    // Create top-level groups
    auto* globalGroup = new QTreeWidgetItem(m_variablesTree, {"📁 Global Variables", "", ""});
    globalGroup->setExpanded(true);
    globalGroup->setForeground(0, QBrush(QColor("#0078d4")));

    auto* localGroup = new QTreeWidgetItem(m_variablesTree, {"📁 Local Variables", "", ""});
    localGroup->setExpanded(true);
    localGroup->setForeground(0, QBrush(QColor("#0078d4")));

    // Populate variables (all go to global for now)
    for (auto it = variables.constBegin(); it != variables.constEnd(); ++it) {
        const QString& name = it.key();
        const QVariant& value = it.value();

        QString valueStr = value.toString();
        QString typeStr = value.typeName();

        // Add quotes for strings
        if (value.type() == QVariant::String) {
            valueStr = QString("\"%1\"").arg(valueStr);
        }

        auto* item = new QTreeWidgetItem(globalGroup, {name, valueStr, typeStr});

        // Color-code by type
        QColor valueColor;
        if (value.type() == QVariant::String) {
            valueColor = QColor("#ce9178");  // String color
        } else if (value.type() == QVariant::Int || value.type() == QVariant::Double) {
            valueColor = QColor("#b5cea8");  // Number color
        } else {
            valueColor = QColor("#e0e0e0");  // Default
        }
        item->setForeground(1, QBrush(valueColor));
        item->setForeground(2, QBrush(QColor("#a0a0a0")));

        // Store variable name in item data for editing
        item->setData(0, Qt::UserRole, name);
    }

    // Add placeholder local variable
    auto* tempItem = new QTreeWidgetItem(localGroup, {"tempChoice", "\"Option A\"", "QString"});
    tempItem->setForeground(1, QBrush(QColor("#ce9178")));
    tempItem->setForeground(2, QBrush(QColor("#a0a0a0")));
}

void NMDebugOverlayPanel::updateCallStackTab(const QStringList& stack) {
    m_currentCallStack = stack;

    m_callStackList->clear();

    for (int i = stack.size() - 1; i >= 0; --i) {  // Reverse order (top of stack first)
        const QString& frame = stack[i];
        auto* item = new QListWidgetItem(QString("%1. %2").arg(stack.size() - i).arg(frame));

        if (i == stack.size() - 1) {
            // Highlight current frame
            item->setForeground(QBrush(QColor("#0078d4")));
            item->setIcon(NMIconManager::instance().getIcon("arrow-right", 16));
        }

        m_callStackList->addItem(item);
    }
}

void NMDebugOverlayPanel::onVariablesChanged(const QVariantMap& variables) {
    updateVariablesTab(variables);
}

void NMDebugOverlayPanel::onCallStackChanged(const QStringList& stack) {
    updateCallStackTab(stack);
}

void NMDebugOverlayPanel::onPlayModeChanged(int mode) {
    // Update UI based on play mode
}

void NMDebugOverlayPanel::onVariableItemDoubleClicked(QTreeWidgetItem* item, int column) {
    if (!item->parent()) {
        // Clicked on a group, not a variable
        return;
    }

    auto& controller = NMPlayModeController::instance();

    if (!controller.isPaused()) {
        // Only allow editing when paused
        m_variablesTree->setToolTip("Variables can only be edited when playback is paused");
        return;
    }

    const QString varName = item->data(0, Qt::UserRole).toString();
    if (varName.isEmpty()) {
        return;  // Placeholder item
    }

    const QVariant currentValue = m_currentVariables.value(varName);
    editVariable(varName, currentValue);
}

void NMDebugOverlayPanel::editVariable(const QString& name, const QVariant& currentValue) {
    bool ok = false;
    QVariant newValue;

    if (currentValue.type() == QVariant::String) {
        newValue = QInputDialog::getText(
            this,
            "Edit Variable",
            QString("Enter new value for '%1':").arg(name),
            QLineEdit::Normal,
            currentValue.toString(),
            &ok
        );
    } else if (currentValue.type() == QVariant::Int) {
        newValue = QInputDialog::getInt(
            this,
            "Edit Variable",
            QString("Enter new value for '%1':").arg(name),
            currentValue.toInt(),
            -2147483647,
            2147483647,
            1,
            &ok
        );
    } else if (currentValue.type() == QVariant::Double) {
        newValue = QInputDialog::getDouble(
            this,
            "Edit Variable",
            QString("Enter new value for '%1':").arg(name),
            currentValue.toDouble(),
            -std::numeric_limits<double>::max(),
            std::numeric_limits<double>::max(),
            2,
            &ok
        );
    } else {
        // Unsupported type, edit as string
        newValue = QInputDialog::getText(
            this,
            "Edit Variable",
            QString("Enter new value for '%1':").arg(name),
            QLineEdit::Normal,
            currentValue.toString(),
            &ok
        );
    }

    if (ok) {
        NMPlayModeController::instance().setVariable(name, newValue);
    }
}

} // namespace NovelMind::editor::qt
