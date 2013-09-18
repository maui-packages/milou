/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright 2013  Vishesh Handa <me@vhanda.in>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "preview.h"
#include "previewplugin.h"

#include <KService>
#include <KServiceTypeTrader>
#include <KDebug>
#include <kparts/mainwindow.h>

#include <QPainter>
#include <QGraphicsProxyWidget>
#include <QTextEdit>
#include <QTimer>
#include <QDeclarativeContext>
#include <QDeclarativeEngine>

Preview::Preview(QDeclarativeItem* parent)
    : QDeclarativeItem(parent)
    , m_loaded(false)
    , m_declarativeItem(0)
{
    setFlag(QGraphicsItem::ItemHasNoContents, false);

    m_plugins = allPlugins();
    foreach(Milou::PreviewPlugin* plugin, m_plugins) {
        connect(plugin, SIGNAL(previewGenerated(QWidget*)),
                this, SLOT(slotPreviewGenerated(QWidget*)));
        connect(plugin, SIGNAL(previewGenerated(QDeclarativeItem*)),
                this, SLOT(slotPreviewGenerated(QDeclarativeItem*)));
    }

    m_proxyWidget = new QGraphicsProxyWidget(this);

    // When the object is created, it doesn't have a QDeclarativeContext
    QTimer::singleShot(0, this, SLOT(setPluginContexts()));
}

void Preview::setPluginContexts()
{
    QDeclarativeContext* context = QDeclarativeEngine::contextForObject(this);
    foreach(Milou::PreviewPlugin* plugin, m_plugins) {
        plugin->setContext(context);
    }
}

Preview::~Preview()
{
}

void Preview::paint(QPainter* painter, const QStyleOptionGraphicsItem* item, QWidget* widget)
{
    Q_UNUSED(item);
    Q_UNUSED(widget);

    if (!m_pixmap.isNull())
        painter->drawPixmap(x(), y(), width(), height(), m_pixmap);
}

void Preview::refresh()
{
    if (m_oldUrl == m_url && m_oldMimetype == m_mimetype) {
        emit loadingFinished();
        return;
    }

    m_loaded = false;
    KUrl url(m_url);
    foreach (Milou::PreviewPlugin* plugin, m_plugins) {
        foreach (const QString& mime, plugin->mimetypes()) {
            if (m_mimetype.startsWith(mime)) {
                plugin->setUrl(url);
                plugin->setMimetype(m_mimetype);
                plugin->setHighlight(m_highlight);
                plugin->generatePreview();
            }
        }
    }
}

void Preview::slotPreviewGenerated(QWidget* widget)
{
    clear();

    m_proxyWidget->setWidget(widget);

    setWidth(widget->width());
    setHeight(widget->height());

    m_loaded = true;
    emit loadingFinished();
}

void Preview::slotPreviewGenerated(QDeclarativeItem* item)
{
    clear();

    m_declarativeItem = item;
    item->setParentItem(this);

    setWidth(item->width());
    setHeight(item->height());

    m_loaded = true;
    emit loadingFinished();
}

void Preview::clear()
{
    if (m_proxyWidget->widget()) {
        m_proxyWidget->widget()->deleteLater();
        m_proxyWidget->setWidget(0);
    }
    if (m_declarativeItem) {
        m_declarativeItem->deleteLater();
        m_declarativeItem = 0;
    }
}

QString Preview::mimetype() const
{
    return m_mimetype;
}

void Preview::setMimetype(const QString& mime)
{
    if (m_mimetype != mime) {
        m_oldMimetype = m_mimetype;
        m_mimetype = mime;
    }
}

void Preview::setUrl(const QString& url)
{
    if (m_url != url) {
        m_oldUrl = m_url;
        m_url = url;
    }
}

QString Preview::url() const
{
    return m_url;
}

void Preview::setHighlight(const QString& highlight)
{
    m_highlight = highlight;
}

QString Preview::highlight() const
{
    return m_highlight;
}

QList<Milou::PreviewPlugin*> Preview::allPlugins()
{
    KService::List serviceList = KServiceTypeTrader::self()->query("MilouPreviewPlugin");
    QList<Milou::PreviewPlugin*> plugins;

    KService::List::const_iterator it;
    for (it = serviceList.constBegin(); it != serviceList.constEnd(); it++) {
        KService::Ptr service = *it;

        QString error;
        Milou::PreviewPlugin* p = service->createInstance<Milou::PreviewPlugin>(this, QVariantList(), &error);
        if(!p) {
            kError() << "Could not create PreviewPlugin:" << service->library();
            kError() << error;
            continue;
        }

        plugins << p;
    }

    return plugins;
}
