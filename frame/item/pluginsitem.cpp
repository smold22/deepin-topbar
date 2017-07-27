#include "pluginsitem.h"
#include <QHBoxLayout>
#include <QDBusPendingReply>
#include <QDBusObjectPath>
#include <QPoint>
#include <QMouseEvent>
#include <QScreen>
#include <QApplication>
#include <QDesktopWidget>

PluginsItem::PluginsItem(PluginsItemInterface * const pluginInter, const QString &itemKey, QWidget *parent) :
    Item(parent),
    m_pluginInter(pluginInter),
    m_centralWidget(pluginInter->itemWidget(itemKey)),
    m_itemKey(itemKey),
    m_eventMonitor(new EventMonitor(this))
{
    m_eventMonitor->start();

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);

    mainLayout->addWidget(m_centralWidget);

    setLayout(mainLayout);

    connect(m_eventMonitor, &EventMonitor::buttonPress, this, [=] (int x, int y) {
        if (!containsPoint(QPoint(x, y))) {
            ItemPopupWindow *popup = PopupWindow.get();
            if (popup->isVisible())
                m_pluginInter->popupHide();
        }
    });
}

PluginsItem::~PluginsItem()
{

}

PluginsItem::ItemType PluginsItem::itemType() const
{
    if (m_pluginInter->pluginName() == "indicator")
        return Indicator;
    if (m_pluginInter->pluginName() == "datetime")
        return DateTime;
    if (m_pluginInter->pluginName() == "notify")
        return Notify;

    return Plugin;
}

const QString PluginsItem::name() const
{
    return m_pluginInter->pluginName();
}

void PluginsItem::finished()
{
    m_pluginInter->finished();
}

const QRect PluginsItem::popupMarkGeometry() const
{
    return QRect(mapToGlobal(pos()), size());
}

QWidget *PluginsItem::popupTips()
{
    return m_pluginInter->itemPopupApplet(m_itemKey);
}

PluginsItemInterface *PluginsItem::itemInter()
{
    return m_pluginInter;
}

void PluginsItem::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        showTips();
}

void PluginsItem::showTips()
{
    QWidget * const content = popupTips();
    if (!content)
        return;

    showPopupWindow(content);
}

void PluginsItem::showPopupWindow(QWidget * const content)
{
    ItemPopupWindow *popup = PopupWindow.get();
    popup->setContent(content);
    m_pluginInter->popupShow();
    PopupWindow->setVisible(true);

    popup->setRect(popupMarkGeometry());
}

void PluginsItem::hidePopup()
{
    ItemPopupWindow *popup = PopupWindow.get();
    PopupWindow->setVisible(false);
    popup->setVisible(false);
}

bool PluginsItem::containsPoint(const QPoint &point) const
{
    QRect screen = QApplication::desktop()->screenGeometry(QApplication::desktop()->primaryScreen());
    QRect r(screen.x(), screen.y(), screen.width(), 27);

    // if click self;
    QRect self(m_pluginInter->itemWidget("")->mapToGlobal(m_pluginInter->itemWidget("")->pos()), m_pluginInter->itemWidget("")->size());
    if (isVisible() && self.contains(point))
        return false;

    if (r.contains(point) || geometry().contains(point))
        return true;
    return false;
}
