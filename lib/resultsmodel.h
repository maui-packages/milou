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

#ifndef NEPOMUK2_RESULTSMODEL_H
#define NEPOMUK2_RESULTSMODEL_H

#include "nepomuk_finder_export.h"

#include <QAbstractListModel>
#include <QThreadPool>

#include <Nepomuk2/Query/Result>

namespace Nepomuk2 {

class QueryRunnable;

class NEPOMUK_FINDER_EXPORT ResultsModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString queryString READ queryString WRITE setQueryString)
    Q_PROPERTY(int queryLimit READ queryLimit WRITE setQueryLimit)

public:
    enum Roles {
        UrlRole = Qt::UserRole + 1,
        ModifiedRole,
        CreatedRole
    };

    explicit ResultsModel(QObject* parent = 0);

    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;

    QString queryString();
    int queryLimit();

public slots:
    void setQueryString(const QString& string);
    void setQueryLimit(int limit);

private slots:
    void slotQueryResult(const Query::Result& result);
    void slotQueryFinished(QueryRunnable* runnable);

private:
    QList<Query::Result> m_results;

    QueryRunnable* m_queryTask;

    QString m_queryString;
    int m_queryLimit;
};
}

#endif // NEPOMUK2_RESULTSMODEL_H
