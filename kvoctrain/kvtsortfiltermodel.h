//
// C++ Interface: kvtsortfiltermodel
//
// Description: 
//
//
// Author:  (C) 2006 Peter Hedlund <peter.hedlund@kdemail.net>
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef KVTSORTFILTERMODEL_H
#define KVTSORTFILTERMODEL_H

#include <QSortFilterProxyModel>


/**
  @author Peter Hedlund <peter.hedlund@kdemail.net>
*/

class KVTTableModel;

class KVTSortFilterModel : public QSortFilterProxyModel
{
Q_OBJECT
public:

  KVTSortFilterModel(QObject *parent = 0);

  void setSourceModel(KVTTableModel * sourceModel);
  KVTTableModel * sourceModel () const;

protected:
  //bool filterAcceptsColumn(int source_column, const QModelIndex & source_parent) const;
  //bool filterAcceptsRow(int source_row, const QModelIndex & source_parent) const;

private:
  KVTTableModel * m_sourceModel;
};

#endif
