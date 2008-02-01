/***************************************************************************

    Copyright 2006, 2007 Peter Hedlund <peter.hedlund@kdemail.net>
    Copyright 2007-2008 Frederik Gladhorn <frederik.gladhorn@kdemail.net>

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "vocabularyview.h"

#include "vocabularymodel.h"
#include "vocabularyfilter.h"
#include "vocabularydelegate.h"

#include "parley.h"
#include "prefs.h"

#include <keduvoctranslation.h>
#include <keduvocexpression.h>

#include <QHeaderView>
#include <QPainter>
#include <QResizeEvent>

#include <KComponentData>
#include <KGlobalSettings>
#include <KAction>
#include <KActionMenu>
#include <KActionCollection>
#include <KToggleAction>
#include <KLocale>
#include <KMessageBox>

VocabularyView::VocabularyView(ParleyApp * parent)
    : QTableView(parent)
{
    m_model = 0;



    horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
//     setSelectionMode(QAbstractItemView::ExtendedSelection);
//     setSelectionBehavior(QAbstractItemView::SelectRows);
    setEditTriggers(QAbstractItemView::AnyKeyPressed | QAbstractItemView::EditKeyPressed | QAbstractItemView::DoubleClicked);

    setSortingEnabled(true);

    VocabularyDelegate *vocabularyDelegate = new VocabularyDelegate;
    setItemDelegate(vocabularyDelegate);

    setFrameStyle(QFrame::NoFrame);
    setAlternatingRowColors(true);

    // Enable context menus
    setContextMenuPolicy(Qt::ActionsContextMenu);
    horizontalHeader()->setContextMenuPolicy(Qt::ActionsContextMenu);

    //horizontalHeader()->setStretchLastSection(true);

    setWordWrap(true);
    setDragEnabled(true);

    // create actions
    m_vocabularyColumnsActionMenu = new KActionMenu(this);

    m_appendEntryAction = new KAction(this);
    parent->actionCollection()->addAction("edit_append", m_appendEntryAction);
    m_appendEntryAction->setIcon(KIcon("list-add-card"));
    m_appendEntryAction->setText(i18n("&Add New Entry"));
    connect(m_appendEntryAction, SIGNAL(triggered(bool)), SLOT(appendEntry()));
    m_appendEntryAction->setShortcut(QKeySequence(Qt::Key_Insert));
    m_appendEntryAction->setWhatsThis(i18n("Append a new row to the vocabulary"));
    m_appendEntryAction->setToolTip(m_appendEntryAction->whatsThis());
    m_appendEntryAction->setStatusTip(m_appendEntryAction->whatsThis());
    addAction(m_appendEntryAction);

    m_deleteEntriesAction = new KAction(this);
    parent->actionCollection()->addAction("edit_remove_selected_area", m_deleteEntriesAction);
    m_deleteEntriesAction->setIcon(KIcon("list-remove-card"));
    m_deleteEntriesAction->setText(i18n("&Delete Entry"));
    connect(m_deleteEntriesAction, SIGNAL(triggered(bool)), this, SLOT(deleteSelectedEntries()));
    m_deleteEntriesAction->setShortcut(QKeySequence(Qt::Key_Delete));
    m_deleteEntriesAction->setWhatsThis(i18n("Delete the selected rows"));
    m_deleteEntriesAction->setToolTip(m_deleteEntriesAction->whatsThis());
    m_deleteEntriesAction->setStatusTip(m_deleteEntriesAction->whatsThis());
    addAction(m_deleteEntriesAction);

    KAction* editCopy = KStandardAction::copy(this, SLOT(slotEditCopy()), parent->actionCollection());
    editCopy->setWhatsThis(i18n("Copy"));
    editCopy->setToolTip(editCopy->whatsThis());
    editCopy->setStatusTip(editCopy->whatsThis());

    KAction* editCut = KStandardAction::cut(this, SLOT(slotCutEntry()), parent->actionCollection());
    editCut->setWhatsThis(i18n("Copy"));
    editCut->setToolTip(editCut->whatsThis());
    editCut->setStatusTip(editCut->whatsThis());

    KAction* editPaste = KStandardAction::paste(this, SLOT(slotEditPaste()), parent->actionCollection());
    editPaste->setWhatsThis(i18n("Paste"));
    editPaste->setToolTip(editPaste->whatsThis());
    editPaste->setStatusTip(editPaste->whatsThis());

    KAction* editSelectAll = KStandardAction::selectAll(this, SLOT(selectAll()), parent->actionCollection());
    editSelectAll->setWhatsThis(i18n("Select all rows"));
    editSelectAll->setToolTip(editSelectAll->whatsThis());
    editSelectAll->setStatusTip(editSelectAll->whatsThis());

    KAction* editClearSelection = KStandardAction::deselect(this, SLOT(clearSelection()), parent->actionCollection());
    editClearSelection->setWhatsThis(i18n("Deselect all rows"));
    editClearSelection->setToolTip(editClearSelection->whatsThis());
    editClearSelection->setStatusTip(editClearSelection->whatsThis());
}


void VocabularyView::setModel(VocabularyFilter * model)
{
    QTableView::setModel(model);
    m_model = model;
//     setCurrentIndex(model->index(0, 0));
//     scrollTo(currentIndex());
//     connect(verticalHeader(), SIGNAL(sectionResized(int, int, int)), this, SLOT(verticalHeaderResized(int, int, int)));
//     connect(horizontalHeader(), SIGNAL(sectionResized(int, int, int)), this, SLOT(horizontalHeaderResized(int, int, int)));
    connect(selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(slotCurrentChanged(const QModelIndex &, const QModelIndex &)));
}

// void VocabularyView::slotCurrentColumnChanged(const QModelIndex & current, const QModelIndex & previous)
// {
//     Q_UNUSED(previous);
//     m_delegate->setCurrentIndex(current);
//     reset();
// }


// void VocabularyView::print(QPrinter * pPrinter)
// {
//     ///@todo revisit and check before release
//     QPainter painter;
//     painter.setBrush(Qt::NoBrush);
//     int res = pPrinter->resolution();
//     int marg = res;
// 
//     int pw;
//     int c = 0;
//     int currentWidth;
//     int startCol;
//     int pageNum = 1;
// 
//     int colCount = model()->columnCount(QModelIndex());
//     int hh = horizontalHeader()->height();
//     int tPos = marg + hh;
// 
//     QStyleOptionViewItem option;
//     option.initFrom(this);
// 
//     painter.begin(pPrinter);
//     QRect w = painter.window();
//     QRect cr;
// 
//     pw = w.width() - (2 * marg);
// 
//     while (c < colCount) {
//         startCol = c;
//         currentWidth = marg;
// 
//         while ((currentWidth < pw) && (c < colCount)) {
//             currentWidth = currentWidth + columnWidth(c);
//             c++;
//         }
//         if (c < colCount)
//             c--;
// 
//         newPage(painter, res, startCol, c);
// 
//         for (int rc = 0; rc < model()->rowCount(QModelIndex()); ++rc) {
//             int rh = rowHeight(rc);
// 
//             painter.resetMatrix();
//             painter.setFont(Prefs::tableFont());
//             painter.translate(marg, tPos);
// 
//             for (int i = startCol; i <= c && i < colCount; ++i) {
//                 cr.setRect(0, 0, columnWidth(i), rh);
//                 option.displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
//                 option.rect = cr;
//                 painter.drawRect(cr);
//                 painter.save();
// 
//                 itemDelegate()->paint(&painter, option, model()->index(rc, i));
//                 painter.restore();
// 
//                 painter.translate(columnWidth(i), 0);
//             }
// 
//             tPos = tPos + rowHeight(rc);
// 
//             if (tPos + rowHeight(rc + 1) > w.height() - marg) {
//                 endOfPage(painter, pageNum++, res);
//                 tPos = marg + hh;
//                 pPrinter->newPage();
//                 newPage(painter, res, startCol, c);
//             }
//         }
//         endOfPage(painter, pageNum++, res);
// 
//         if (c < colCount) {
//             pPrinter->newPage();
//             tPos = marg + hh;
//             c++;
//         }
//     }
//     painter.end();
// }
// 
// void VocabularyView::newPage(QPainter & painter, int res, int startCol, int endCol)
// {
//     int marg = res;
//     int hh = horizontalHeader()->height();
//     int cw;
//     QRect cr;
//     QRect w = painter.window();
//     painter.resetMatrix();
//     painter.setFont(KGlobalSettings::generalFont());
//     painter.drawText(marg, marg - 20, KGlobal::caption());
//     painter.translate(marg, marg);
//     painter.drawLine(0 , 0, 0, hh);
//     for (int i = startCol; i <= endCol && i < model()->columnCount(QModelIndex()); ++i) {
//         cw = columnWidth(i);
//         painter.drawLine(0, 0, cw, 0);
//         painter.drawLine(cw, 0, cw, hh);
//         cr.setRect(3, 0, columnWidth(i)- 6, hh);
//         painter.drawText(cr, Qt::AlignLeft | Qt::AlignVCenter, model()->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString());
//         painter.translate(cw, 0);
//     }
// }
// 
// void VocabularyView::endOfPage(QPainter & painter, int pageNum, int res)
// {
//     painter.resetMatrix();
//     painter.setFont(KGlobalSettings::generalFont());
//     QRect w = painter.window();
//     QRect r = painter.boundingRect(0, 0, 0, 0, Qt::AlignLeft, QString::number(pageNum));
//     painter.drawText((w.width()/2) - (r.width()/2), w.height() - res + 20, QString::number(pageNum));
// }
// 
// void VocabularyView::resizeEvent(QResizeEvent * event)
// {
//     QWidget::resizeEvent(event);
// 
// //     if (event == 0) {
// //         return;
// //     }
// 
//     if ( !model() || (model()->columnCount() < 3) ) {
//         return;
//     }
// 
//     QHeaderView * header = horizontalHeader();
//     int colCount =  model()->columnCount(QModelIndex());
//     int oldWidth = 0;
// 
//     for (int i = 0; i < colCount; ++i)
//         oldWidth += header->sectionSize(i);
// 
//     int newWidth = viewport()->width();
// 
//     switch (Prefs::headerResizeMode()) {
//     case Prefs::EnumHeaderResizeMode::Automatic: {
//             // lesson is only half as wide as a original/translation
//             // exclude fixed size of "mark"-column
//             int activeColumnVisible = 0;
//             if ( Prefs::tableActiveColumnVisible() ) {
//                 newWidth -= KV_COLWIDTH_MARK;
//                 header->resizeSection(KV_COL_MARK, KV_COLWIDTH_MARK);
//             }
// 
//             int columnsHalfWidth;
//             if ( Prefs::tableLessonColumnVisible() ) {
//                 // total width / ((total number of columns - active column) * 2 -1 lesson only half) 
//                 columnsHalfWidth = newWidth / ((colCount - 1) * 2 -1);
//                 header->resizeSection(KV_COL_LESS, columnsHalfWidth);
//             } else {
//                 columnsHalfWidth = newWidth / ((colCount - 2) * 2);
//             }
// 
//             for (int currentColumn = KV_COL_TRANS; currentColumn < colCount; currentColumn++) {
//                 header->resizeSection(currentColumn, 2 * columnsHalfWidth);
//             }
//         }
//         break;
// 
//     case Prefs::EnumHeaderResizeMode::Percent: {
//             float grow = float(newWidth) / float(oldWidth);
//             header->resizeSection(KV_COL_MARK, KV_COLWIDTH_MARK);
// 
//             int remainder = newWidth - KV_COLWIDTH_MARK;
//             int x = qMax(HEADER_MINSIZE, (int)((header->sectionSize(KV_COL_LESS) * grow) + 0.5));
//             header->resizeSection(KV_COL_LESS, x);
//             remainder -= x;
// 
//             for (int i = KV_COL_TRANS; i < colCount - 1; i++) {
//                 x = qMax(HEADER_MINSIZE, (int)((header->sectionSize(i) * grow) + 0.5));
//                 remainder -= x;
//                 header->resizeSection(i, x);
//             }
//             header->resizeSection(colCount - 1, qMax(HEADER_MINSIZE, remainder));
//         }
//         break;
// 
//     case Prefs::EnumHeaderResizeMode::Fixed:
//         // do nothing
//         break;
//     }
// }
// 
// void VocabularyView::showEvent(QShowEvent * event)
// {
//     QWidget::showEvent(event);
//     QResizeEvent rsEvent(size(), size());
//     resizeEvent(&rsEvent);
// }

void VocabularyView::slotCurrentChanged(const QModelIndex & current, const QModelIndex & previous)
{
    Q_UNUSED(previous);
    KEduVocExpression* entry = 0;
    if ( current.isValid() ) {
        entry =  model()->data(current, VocabularyModel::EntryRole).value<KEduVocExpression*>();
    }
    emit translationChanged(entry, VocabularyModel::translation(current.column()));
}

void VocabularyView::reset()
{
    QTableView::reset();
    kDebug() << "reset";

    foreach( KAction* oldAction, m_columnActionMap.keys() ) {
        m_columnActionMap.remove(oldAction);
        delete oldAction;
    }

    for( int i = 0; i < model()->columnCount(QModelIndex()); i++) {
        KActionMenu* currentTranslationAction;
        KAction* columnAction;

        if(VocabularyModel::columnType(i) == VocabularyModel::Translation) {
            currentTranslationAction = new KActionMenu(
                model()->headerData(i, Qt::Horizontal).toString(), this);
            columnAction = currentTranslationAction;
            m_vocabularyColumnsActionMenu->addAction(currentTranslationAction);
            connect (columnAction, SIGNAL(triggered(bool)),
                this, SLOT(slotToggleColumn(bool)));
        } else {
            columnAction = new KToggleAction(model()->headerData(i, Qt::Horizontal).toString(), this);
            currentTranslationAction->addAction(columnAction);
            connect (columnAction, SIGNAL(triggered(bool)),
                this, SLOT(slotToggleColumn(bool)));
            setColumnHidden(i, true);
        }
        m_columnActionMap[columnAction] = i;
    }

//     KAction* toggleColumn = new KAction("Column count: ", this);
//     m_columnActionMap
//     m_vocabularyColumnsActionMenu->addAction(toggleColumn);

    resizeColumnsToContents ();
}

void VocabularyView::slotToggleColumn(bool show)
{
    setColumnHidden(m_columnActionMap[(KAction*)sender()], !show);
}

void VocabularyView::appendEntry()
{
    QModelIndex newIndex = m_model->appendEntry();
    scrollTo(newIndex);
    selectionModel()->select(newIndex, QItemSelectionModel::ClearAndSelect);
    selectionModel()->setCurrentIndex(newIndex, QItemSelectionModel::ClearAndSelect);
    edit(newIndex);
}

void VocabularyView::appendChar(const QChar &c)
{
    const QModelIndex &index = selectionModel()->currentIndex();
    m_model->setData(index, m_model->data(index).toString() + c);
}

void VocabularyView::deleteSelectedEntries()
{
    QSet<int> rows;
    foreach (QModelIndex index, selectionModel()->selectedIndexes()) {
        rows.insert(index.row());
    }

    bool del = KMessageBox::Continue == KMessageBox::warningContinueCancel(this, i18np("Do you really want to delete the selected entry?", "Do you really want to delete the selected %1 entries?", rows.count()), i18n("Delete"), KStandardGuiItem::del());

    if (del) {
        while (!selectionModel()->selectedIndexes().isEmpty()) {
            m_model->removeRows(selectionModel()->selectedIndexes()[0].row(), 1, QModelIndex());
        }
    }
}

void VocabularyView::slotEditCopy()
{
//     slotStatusMsg(i18n("Copying selection to clipboard..."));

/*    QApplication::setOverrideCursor(Qt::WaitCursor);

    QString textToCopy;
    QModelIndexList selectedRows = m_tableView->selectionModel()->selectedRows(0);

    foreach(const QModelIndex &idx, selectedRows) {
    bool sep = false;
    for (int i = KV_COL_TRANS; i < m_tableModel->columnCount(QModelIndex()); i++) {
    if (!sep)
    sep = true;
    else
    textToCopy += '\t';

    QModelIndex mappedIndex = m_sortFilterModel->mapToSource(m_sortFilterModel->index(idx.row(), i));
    textToCopy += m_tableModel->data(mappedIndex, Qt::DisplayRole).toString();
}
    if (!textToCopy.isEmpty())
    textToCopy += '\n';
}

    if (!textToCopy.isEmpty())
    QApplication::clipboard()->setText(textToCopy);

    QApplication::restoreOverrideCursor();*/
//     slotStatusMsg(IDS_DEFAULT);
}

void VocabularyView::slotEditPaste()
{
    /// @todo make the pasted stuff visible by making the corresponding lesson visible, if it is not (?)
//     slotStatusMsg(i18n("Inserting clipboard contents..."));

/*    QApplication::setOverrideCursor(Qt::WaitCursor);
    QString s;
    QString textToPaste = QApplication::clipboard()->text();

    QTextStream ts;
    ts.setString(&textToPaste, QIODevice::Text);

    QString num;

    QModelIndexList selectedRows = m_tableView->selectionModel()->selectedRows();
    int lastSelectedRow;
    if(!selectedRows.isEmpty())
    lastSelectedRow = m_sortFilterModel->mapToSource(selectedRows.back()).row() + 1;
    else
    lastSelectedRow = m_tableModel->rowCount(QModelIndex());

    int count = 0;
    while (!ts.atEnd()) {
    s = ts.readLine();
    if (!s.isEmpty()) {
    m_tableModel->insertRows(lastSelectedRow + count, 1);
    QStringList sl = s.split('\t', QString::KeepEmptyParts);

    for (int i = 0; i < sl.count(); ++i) {
    m_tableModel->setData(m_tableModel->index(lastSelectedRow + count, i + KV_COL_TRANS), sl[i], Qt::EditRole);
//                 m_tableModel->setData(m_tableModel->index(lastSelectedRow + count, i + KV_COL_TRANS), m_document->document()->currentLesson(), KVTTableModel::LessonRole);
    m_tableModel->setData(m_tableModel->index(lastSelectedRow + count, i + KV_COL_TRANS), m_lessonDockWidget->selectedLesson(), KVTTableModel::LessonRole);
}
}
    count++;
}

    QApplication::restoreOverrideCursor();
//     slotStatusMsg(IDS_DEFAULT);

    m_deleteEntriesAction->setEnabled(m_sortFilterModel->rowCount(QModelIndex()) > 0);*/
}

void VocabularyView::slotCutEntry()
{
    // there's no need to reinvent the wheel ;)
    slotEditCopy();

    // but we won't ask the user whether to delete or not.. we'll just cut
//     if (m_tableView->selectionModel()->selectedRows().count() == 1) {
//         int currentRow = m_tableView->currentIndex().row();
//         int currentColumn = m_tableView->currentIndex().column();
//         m_sortFilterModel->removeRows(m_tableView->currentIndex().row(), 1, QModelIndex());
//         m_tableView->selectionModel()->setCurrentIndex(m_sortFilterModel->index(currentRow, currentColumn), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
//     } else {
//         int currentRow = m_tableView->currentIndex().row();
//         int currentColumn = m_tableView->currentIndex().column();
//         int rowCount = m_sortFilterModel->rowCount(QModelIndex());
//         // Must count backwards otherwise entry-numbering goes wrong when
//         // deleting.
//         for (int i = rowCount - 1; i >= 0; i--)
//             if (m_tableView->selectionModel()->isRowSelected(i, QModelIndex()))
//                 m_sortFilterModel->removeRows(i, 1, QModelIndex());
//         m_tableView->selectionModel()->setCurrentIndex(m_sortFilterModel->index(currentRow, currentColumn), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
//     }
//     m_deleteEntriesAction->setEnabled(m_sortFilterModel->rowCount(QModelIndex()) > 0);
}

KActionMenu * VocabularyView::columnsActionMenu()
{
    return m_vocabularyColumnsActionMenu;
}

#include "vocabularyview.moc"

