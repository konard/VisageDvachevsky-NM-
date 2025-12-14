#include "NovelMind/editor/qt/panels/nm_inspector_panel.hpp"
#include "NovelMind/editor/qt/nm_style_manager.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QFrame>

namespace NovelMind::editor::qt {

// ============================================================================
// NMPropertyGroup
// ============================================================================

NMPropertyGroup::NMPropertyGroup(const QString& title, QWidget* parent)
    : QWidget(parent)
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Header
    m_header = new QWidget(this);
    m_header->setCursor(Qt::PointingHandCursor);
    auto* headerLayout = new QHBoxLayout(m_header);
    headerLayout->setContentsMargins(4, 4, 4, 4);

    m_expandIcon = new QLabel(QString::fromUtf8("\u25BC"), m_header);  // Down arrow
    m_expandIcon->setFixedWidth(16);
    headerLayout->addWidget(m_expandIcon);

    auto* titleLabel = new QLabel(title, m_header);
    QFont boldFont = titleLabel->font();
    boldFont.setBold(true);
    titleLabel->setFont(boldFont);
    headerLayout->addWidget(titleLabel);

    headerLayout->addStretch();

    // Make header clickable
    m_header->installEventFilter(this);

    mainLayout->addWidget(m_header);

    // Separator
    auto* separator = new QFrame(this);
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(separator);

    // Content area
    m_content = new QWidget(this);
    m_contentLayout = new QVBoxLayout(m_content);
    m_contentLayout->setContentsMargins(8, 4, 8, 8);
    m_contentLayout->setSpacing(4);

    mainLayout->addWidget(m_content);

    // Connect header click
    connect(m_header, &QWidget::destroyed, []{});  // Placeholder for event filter
}

void NMPropertyGroup::setExpanded(bool expanded)
{
    m_expanded = expanded;
    m_content->setVisible(expanded);
    m_expandIcon->setText(expanded ? QString::fromUtf8("\u25BC") : QString::fromUtf8("\u25B6"));
}

void NMPropertyGroup::addProperty(const QString& name, const QString& value)
{
    auto* row = new QWidget(m_content);
    auto* rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setSpacing(8);

    auto* nameLabel = new QLabel(name + ":", row);
    nameLabel->setMinimumWidth(100);
    nameLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    const auto& palette = NMStyleManager::instance().palette();
    nameLabel->setStyleSheet(QString("color: %1;").arg(NMStyleManager::colorToStyleString(palette.textSecondary)));

    auto* valueLabel = new QLabel(value, row);
    valueLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

    rowLayout->addWidget(nameLabel);
    rowLayout->addWidget(valueLabel, 1);

    m_contentLayout->addWidget(row);
}

void NMPropertyGroup::addProperty(const QString& name, QWidget* widget)
{
    auto* row = new QWidget(m_content);
    auto* rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setSpacing(8);

    auto* nameLabel = new QLabel(name + ":", row);
    nameLabel->setMinimumWidth(100);
    nameLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    const auto& palette = NMStyleManager::instance().palette();
    nameLabel->setStyleSheet(QString("color: %1;").arg(NMStyleManager::colorToStyleString(palette.textSecondary)));

    rowLayout->addWidget(nameLabel);
    rowLayout->addWidget(widget, 1);

    m_contentLayout->addWidget(row);
}

void NMPropertyGroup::clearProperties()
{
    QLayoutItem* item;
    while ((item = m_contentLayout->takeAt(0)) != nullptr)
    {
        delete item->widget();
        delete item;
    }
}

void NMPropertyGroup::onHeaderClicked()
{
    setExpanded(!m_expanded);
}

// ============================================================================
// NMInspectorPanel
// ============================================================================

NMInspectorPanel::NMInspectorPanel(QWidget* parent)
    : NMDockPanel(tr("Inspector"), parent)
{
    setPanelId("Inspector");
    setupContent();
}

NMInspectorPanel::~NMInspectorPanel() = default;

void NMInspectorPanel::onInitialize()
{
    showNoSelection();
}

void NMInspectorPanel::onUpdate(double /*deltaTime*/)
{
    // No continuous update needed
}

void NMInspectorPanel::clear()
{
    // Remove all groups
    for (auto* group : m_groups)
    {
        m_mainLayout->removeWidget(group);
        delete group;
    }
    m_groups.clear();

    m_headerLabel->clear();
}

void NMInspectorPanel::inspectObject(const QString& objectType, const QString& objectId)
{
    clear();
    m_noSelectionLabel->hide();

    // Set header
    m_headerLabel->setText(QString("<b>%1</b><br><span style='color: gray;'>%2</span>")
                          .arg(objectType)
                          .arg(objectId));
    m_headerLabel->show();

    // Add demo properties based on type
    auto* transformGroup = addGroup(tr("Transform"));
    transformGroup->addProperty(tr("Position X"), "0.0");
    transformGroup->addProperty(tr("Position Y"), "0.0");
    transformGroup->addProperty(tr("Rotation"), "0.0");
    transformGroup->addProperty(tr("Scale X"), "1.0");
    transformGroup->addProperty(tr("Scale Y"), "1.0");

    auto* renderGroup = addGroup(tr("Rendering"));
    renderGroup->addProperty(tr("Visible"), "true");
    renderGroup->addProperty(tr("Alpha"), "1.0");
    renderGroup->addProperty(tr("Z-Order"), "0");

    if (objectType == "Dialogue" || objectType == "Choice")
    {
        auto* dialogueGroup = addGroup(tr("Dialogue"));
        dialogueGroup->addProperty(tr("Speaker"), "Narrator");
        dialogueGroup->addProperty(tr("Text"), objectId);
        dialogueGroup->addProperty(tr("Voice Clip"), "(none)");
    }

    // Add spacer at the end
    m_mainLayout->addStretch();
}

NMPropertyGroup* NMInspectorPanel::addGroup(const QString& title)
{
    auto* group = new NMPropertyGroup(title, m_scrollContent);
    m_mainLayout->addWidget(group);
    m_groups.append(group);
    return group;
}

void NMInspectorPanel::showNoSelection()
{
    clear();
    m_headerLabel->hide();
    m_noSelectionLabel->show();
}

void NMInspectorPanel::setupContent()
{
    auto* container = new QWidget(this);
    auto* containerLayout = new QVBoxLayout(container);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setSpacing(0);

    // Header
    m_headerLabel = new QLabel(container);
    m_headerLabel->setWordWrap(true);
    m_headerLabel->setTextFormat(Qt::RichText);
    m_headerLabel->setMargin(8);
    m_headerLabel->hide();
    containerLayout->addWidget(m_headerLabel);

    // Scroll area
    m_scrollArea = new QScrollArea(container);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);

    m_scrollContent = new QWidget(m_scrollArea);
    m_mainLayout = new QVBoxLayout(m_scrollContent);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(8);
    m_mainLayout->setAlignment(Qt::AlignTop);

    m_scrollArea->setWidget(m_scrollContent);
    containerLayout->addWidget(m_scrollArea, 1);

    // No selection label
    m_noSelectionLabel = new QLabel(tr("Select an object to view its properties"), container);
    m_noSelectionLabel->setAlignment(Qt::AlignCenter);
    m_noSelectionLabel->setWordWrap(true);

    const auto& palette = NMStyleManager::instance().palette();
    m_noSelectionLabel->setStyleSheet(QString("color: %1; padding: 20px;")
                                      .arg(NMStyleManager::colorToStyleString(palette.textSecondary)));

    m_mainLayout->addWidget(m_noSelectionLabel);

    setContentWidget(container);
}

} // namespace NovelMind::editor::qt
