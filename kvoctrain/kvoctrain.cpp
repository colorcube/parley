/***************************************************************************

                      main part of kvoctrain

    -----------------------------------------------------------------------

    begin         : Thu Mar 11 20:50:53 MET 1999

    copyright     : (C) 1999-2001 Ewald Arnold <kvoctrain@ewald-arnold.de>
                    (C) 2001 The KDE-EDU team
                    (C) 2004-2007 Peter Hedlund <peter.hedlund@kdemail.net>

    -----------------------------------------------------------------------

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QFile>
#include <QTimer>
#include <QPixmap>
#include <QKeyEvent>
#include <QApplication>
#include <QClipboard>
#include <QProgressBar>
#include <QSplitter>

#include <kapplication.h>
#include <kstatusbar.h>
#include <klineedit.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kselectaction.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kprinter.h>
#include <krecentfilesaction.h>
#include <kinputdialog.h>

#include <time.h>
#include <ctype.h>

#include "kvoctrain.h"
#include "query-dialogs/RandomQueryDlg.h"
#include "query-dialogs/MCQueryDlg.h"
#include "query-dialogs/AdjQueryDlg.h"
#include "query-dialogs/VerbQueryDlg.h"
#include "query-dialogs/ArtQueryDlg.h"
#include "query-dialogs/SimpleQueryDlg.h"
#include "entry-dialogs/EntryDlg.h"
#include "docprop-dialogs/DocPropDlg.h"
#include "docprop-dialogs/DocPropLangDlg.h"
#include "statistics-dialogs/StatisticsDialog.h"
#include "common-dialogs/kvoctrainprefs.h"
#include "prefs.h"
#include "languagesettings.h"

#define MAX_LESSON       25
#define THRESH_LESSON    KV_MIN_GRADE

#define START_QUERY                 1
#define START_MULTIPLE              2
#define START_ARTICLE               3
#define START_VERB                  4
#define START_ADJECTIVE             5
#define START_SYNONYM               6
#define START_ANTONYM               7
#define START_EXAMPLE               8
#define START_PARAPHRASE            9


void KVocTrainApp::saveOptions()
{
  fileOpenRecent->saveEntries(KGlobal::config()->group("Recent Files"));

  if (m_tableView)
  {
    Prefs::setCurrentRow(m_tableView->currentIndex().row());
    Prefs::setCurrentCol(m_tableView->currentIndex().column());
  }

  if (m_lessonSelectionCombo)
    Prefs::setLessonEditingSelection(m_lessonSelectionCombo->currentIndex());

  if (m_mainSplitter)
    Prefs::setMainWindowSplitter(m_mainSplitter->sizes());

  saveLanguages();
  Prefs::writeConfig();
}

void KVocTrainApp::saveLanguages()
{
  Prefs::setNumLangSet(m_languages.count());
  for (int i = 0 ; i < m_languages.count(); i++)
  {
    LanguageSettings languageSettings(QString::number(i));
    languageSettings.setShortId(m_languages.shortId(i));
    languageSettings.setShort2Id(m_languages.shortId2(i));
    languageSettings.setLongId(m_languages.longId(i));
    languageSettings.setPixmapFile(m_languages.pixmapFile(i));
    languageSettings.setKeyboardLayout(m_languages.keyboardLayout(i));
    languageSettings.writeConfig();
  }
}

void KVocTrainApp::readOptions()
{
  fileOpenRecent->loadEntries(KGlobal::config()->group( "Recent Files") );
  readLanguages();
}

void KVocTrainApp::readLanguages()
{
  m_languages.clear();
  int ls = Prefs::numLangSet();
  for (int i = 0 ; i < ls; i++)
  {
    LanguageSettings languageSettings(QString::number(i));
    languageSettings.readConfig();

    QString shortId = languageSettings.shortId();
    if (shortId.simplified().length() == 0) {
      shortId.setNum (i);
      shortId.insert (0, "id");
    }

    QString longId = languageSettings.longId();
    if (longId.simplified().length() == 0) {
      longId.setNum (i);
      longId.insert (0, "ident");
    }

    m_languages.addLanguage(shortId, longId, languageSettings.pixmapFile(), languageSettings.short2Id(),    languageSettings.keyboardLayout());
  }
}
void KVocTrainApp::saveProperties(KConfigGroup &config )
{
  saveOptions();
  if (m_doc) {
    config.writeEntry("Filename", m_doc->url().path());
    config.writeEntry("Title", m_doc->title());
    config.writeEntry("Modified", m_doc->isModified());

    QString filename=m_doc->url().path();
    QString tempname = kapp->tempSaveName(filename);
    saveDocProps(m_doc);
    m_doc->saveAs(this, KUrl(tempname), KEduVocDocument::automatic, "KVocTrain");
  }
}


void KVocTrainApp::readProperties(const KConfigGroup &config)
{
  QString filename = config.readEntry("Filename");
  QString title = config.readEntry("Title");
  bool modified = config.readEntry("Modified", false);
  if (modified){
    bool b_canRecover;
    QString tempname = kapp->checkRecoverFile(filename,b_canRecover);

    if (b_canRecover){
      m_doc = new KEduVocDocument(this);
      m_doc->setUrl(KUrl(tempname));
      removeProgressBar();
      m_doc->setModified();
      m_doc->setTitle(title);
      m_doc->setUrl(KUrl(filename));
      setCaption(m_doc->title(), m_doc->isModified());
      QFile::remove(tempname);
    }
  }
  else if (!filename.isEmpty()){
    m_doc = new KEduVocDocument(this);
    m_doc->setUrl(KUrl(filename));
    removeProgressBar();
    setCaption(m_doc->title(), m_doc->isModified());
  }

  show();
}

/*void KVocTrainApp::slotSaveOptions()
{
   saveOptions(true);
}
*/

KVocTrainApp::~KVocTrainApp()
{
   removeEntryDlg();
   delete m_doc;
}


void KVocTrainApp::slotCancelSelection ()
{
  m_tableView->clearSelection();
}


void KVocTrainApp::slotSelectAll ()
{
  m_tableView->selectAll();
}


void KVocTrainApp::slotEditRow()
{
  slotEditEntry2(m_tableView->currentIndex());
}


void KVocTrainApp::slotEditCallBack(int res)
{
  switch (res) {
    case EntryDlg::EditCancel:
      removeEntryDlg();
    break;

    case EntryDlg::EditApply:
      commitEntryDlg(true);
      if (Prefs::smartAppend())
      {
        int row = m_tableView->currentIndex().row();
        if (row == m_tableModel->rowCount(QModelIndex()) - 1)
        {
          int col = m_tableView->currentIndex().column();
          if (col < m_tableModel->columnCount(QModelIndex()) - 1 && col >= KV_COL_ORG)
          {
            int lesson = m_doc->entry(row)->lesson();
            if (lesson >= m_lessonsComboBox->count())
              lesson = qMax (0, m_lessonsComboBox->count()-1);
            slotChooseLesson(lesson);

            QString exp;
            exp = m_doc->entry(row)->translation(col+1-KV_COL_ORG);
            if (exp.isEmpty())
              m_tableView->setCurrentIndex(m_tableModel->index(row, col + 1));
          }
          else
            slotAppendRow();
        }
      }
    break;

    case EntryDlg::EditUndo:
      int row, col;
      QModelIndexList tabsel;
      entryDlg->getCell(row, col, tabsel);
      setDataEntryDlg(row, col);
    break;
  }
}


void KVocTrainApp::commitEntryDlg(bool force)
{
  if (entryDlg == 0) {
    kError() << "KVocTrainApp::commitEntryDlg: entryDlg == 0\n";
    return;
  }

  if (!force && entryDlg->isModified() && !Prefs::autoEntryApply()) {
    if (KMessageBox::No == KMessageBox::warningYesNo(this,
                  i18n("The entry dialog contains unsaved changes.\n"
                      "Do you want to apply or discard your changes?"),
                  i18n("Unsaved Changes"),
                  KStandardGuiItem::apply(), KStandardGuiItem::discard())) {
      return;
    }
  }

  int row, col;

  QModelIndexList tabsel;
  entryDlg->getCell(row, col, tabsel);
  int hasSel = tabsel.count() > 1;

  fillLessonBox();

  if (!hasSel) {
    KEduVocExpression *expr = m_doc->entry(row);
    if (col >= KV_COL_ORG) {
      m_tableModel->setData(m_tableModel->index(row, col), entryDlg->getExpr(), Qt::EditRole);
      col -= KV_EXTRA_COLS;

      expr->setRemark(col, entryDlg->getRemark());
      expr->setPronunciation(col, entryDlg->getPronounce());
      expr->setSynonym(col, entryDlg->getSynonym());
      expr->setAntonym(col, entryDlg->getAntonym());
      expr->setExample(col, entryDlg->getExample());
      expr->setUsageLabel(col, entryDlg->getUsageLabel());
      expr->setParaphrase(col, entryDlg->getParaphrase());
      expr->setConjugation(col, entryDlg->getConjugation());
      expr->setComparison(col, entryDlg->getComparison());
      expr->setMultipleChoice(col, entryDlg->getMultipleChoice());
      expr->setFauxAmi(col, entryDlg->getFromFauxAmi(), false);
      expr->setFauxAmi(col, entryDlg->getToFauxAmi(), true);
      expr->setGrade(col, entryDlg->getFromGrade(), false);
      expr->setGrade(col, entryDlg->getToGrade(), true);
      expr->setQueryCount(col, entryDlg->getFromQCount(), false);
      expr->setQueryCount(col, entryDlg->getToQCount(), true);
      expr->setBadCount(col, entryDlg->getFromBCount(), false);
      expr->setBadCount(col, entryDlg->getToBCount(), true);
      expr->setQueryDate(col, entryDlg->getFromDate(), false);
      expr->setQueryDate(col, entryDlg->getToDate(), true);
      expr->setType(col, entryDlg->getType());

      for (int j = 0; j <= expr->translationCount(); j++)
        if (expr->type(j).isEmpty())
          expr->setType(j, entryDlg->getType());

      for (int j = 0; j <= expr->translationCount(); j++)
        if (KVTQuery::getMainType(expr->type(j))
              !=
            KVTQuery::getMainType(entryDlg->getType()))
          expr->setType(j, entryDlg->getType());
    }
    m_tableModel->setData(m_tableModel->index(row, 0), entryDlg->getLesson(), Qt::EditRole);
    expr->setActive(entryDlg->getActive());

    entryDlg->setModified(false);
    m_doc->setModified(true);

  }
  else {
    col -= KV_EXTRA_COLS;
    foreach(QModelIndex selIndex, tabsel) {
      QModelIndex index = m_sortFilterModel->mapToSource(selIndex);
      KEduVocExpression *expr = m_doc->entry(index.row());

      if (col >= 0) {
        // only updated "common" props in multimode
        if (entryDlg->fromGradeIsModified())
          expr->setGrade(col, entryDlg->getFromGrade(), false);
        if (entryDlg->toGradeIsModified())
          expr->setGrade(col, entryDlg->getToGrade(), true);
        if (entryDlg->fromQueryCountIsModified())
          expr->setQueryCount(col, entryDlg->getFromQCount(), false);
        if (entryDlg->toQueryCountIsModified())
          expr->setQueryCount(col, entryDlg->getToQCount(), true);
        if (entryDlg->fromBadCountIsModified())
          expr->setBadCount(col, entryDlg->getFromBCount(), false);
        if (entryDlg->toBadCountIsModified())
          expr->setBadCount(col, entryDlg->getToBCount(), true);
        if (entryDlg->fromDateIsModified())
          expr->setQueryDate(col, entryDlg->getFromDate(), false);
        if (entryDlg->toDateIsModified())
          expr->setQueryDate(col, entryDlg->getToDate(), true);
        if (entryDlg->usageIsModified())
          for (int j = 0; j <= expr->translationCount(); j++)
            expr->setUsageLabel (j, entryDlg->getUsageLabel());
        if (entryDlg->typeIsModified())
          for (int j = 0; j <= expr->translationCount(); j++)
            expr->setType(j, entryDlg->getType());
      }

      if (entryDlg->activeIsModified())
        expr->setActive(entryDlg->getActive());
      if (entryDlg->lessonIsModified())
        m_tableModel->setData(m_tableModel->index(index.row(), 0), entryDlg->getLesson(), Qt::EditRole);
    }
    entryDlg->setModified(false);
    m_doc->setModified(true);
  }
}


void KVocTrainApp::removeEntryDlg()
{
  if (entryDlg != 0) {
    kDebug() << "Removing entry dialog" << endl;
    commitEntryDlg(false);
    delete entryDlg;
    entryDlg = 0;
  }

  m_tableView->setEditTriggers(QAbstractItemView::AnyKeyPressed | QAbstractItemView::EditKeyPressed | QAbstractItemView::DoubleClicked);
}


void KVocTrainApp::slotEditEntry(int row, int col)
{
  if (entryDlg == 0) {
    entryDlg = new EntryDlg(this, m_doc, querymanager);
    connect(entryDlg, SIGNAL(sigEditChoice(int)), this, SLOT(slotEditCallBack(int)));
    entryDlg->show();
  }

  if (entryDlg->isModified()) {
    commitEntryDlg(false);
  }

  setDataEntryDlg(row, col);
}


void KVocTrainApp::slotEditEntry2(const QModelIndex & index)
{
  if (index.isValid()) {
    QModelIndex docIndex = m_sortFilterModel->mapToSource(index);
    slotEditEntry(docIndex.row(), docIndex.column());
  }
}


void KVocTrainApp::setDataEntryDlg(int row, int col)
{
  if (entryDlg == 0) {
    kError() << "KVocTrainApp::setDataEntryDlg: entryDlg == 0\n";
    return;
  }

  if ((row < 0) || (col < 0) || (m_tableModel->rowCount(QModelIndex()) <= 0))
    return;

  QString text, title;

  KEduVocExpression *expr = m_doc->entry(row);

  if (expr == 0)
    return; // entry delete in the meantime

  int lesson = expr->lesson();
  if (lesson >= m_lessonsComboBox->count())
    lesson = qMax (0, m_lessonsComboBox->count() - 1);

  bool hasSelection = (m_tableView->selectionModel()->selectedRows().count() > 1);
  kDebug() << hasSelection << endl;

  col -= KV_EXTRA_COLS;

  EntryDlg::EnableType et;

  if (col < 0)
    et = EntryDlg::EnableOnlyCommon;
  else if (col == 0)
    et = EntryDlg::EnableOnlyOriginal;
  else
    et = EntryDlg::EnableAll;

  if (col < 0) {
    title = i18n("Edit General Properties");

    entryDlg->setData(et,
                      hasSelection,
                      0,
                      0,
                      0,
                      0,
                      0,
                      0,
                      QDateTime(),
                      QDateTime(),
                      QString(),
                      QString(),
                      QString(),
                      lesson,
                      QString(),
                      m_doc->entry(row)->type(0),
                      QString(),
                      QString(),
                      QString(),
                      QString(),
                      QString(),
                      QString(),
                      m_doc->conjugation(0),
                      KEduVocConjugation(),
                      KEduVocComparison(),
                      KEduVocMultipleChoice(),
                      title,
                      m_doc->entry(row)->isActive());
  }
  else {
    text = m_tableModel->data(m_tableModel->index(row, col + KV_EXTRA_COLS), Qt::DisplayRole).toString();
    //col -= KV_EXTRA_COLS;

    if (col == 0)
      title = i18n("Edit Properties for Original");
    else
      title = i18n("Edit Properties of a Translation");

    entryDlg->setData(et,
                      hasSelection,
                      m_doc->entry(row)->grade(col, false),
                      m_doc->entry(row)->grade(col, true),
                      m_doc->entry(row)->queryCount(col, false),
                      m_doc->entry(row)->queryCount(col, true),
                      m_doc->entry(row)->badCount(col, false),
                      m_doc->entry(row)->badCount(col, true),
                      m_doc->entry(row)->queryDate(col, false),
                      m_doc->entry(row)->queryDate(col, true),
                      m_doc->entry(row)->fauxAmi(col, false),
                      m_doc->entry(row)->fauxAmi(col, true),
                      text,
                      lesson,
                      m_doc->entry(row)->remark(col),
                      m_doc->entry(row)->type(col),
                      m_doc->entry(row)->pronunciation(col),
                      m_doc->entry(row)->synonym(col),
                      m_doc->entry(row)->antonym(col),
                      m_doc->entry(row)->example(col),
                      m_doc->entry(row)->usageLabel(col),
                      m_doc->entry(row)->paraphrase(col),
                      m_doc->conjugation(col),
                      m_doc->entry(row)->conjugation(col),
                      m_doc->entry(row)->comparison(col),
                      m_doc->entry(row)->multipleChoice(col),
                      title,
                      m_doc->entry(row)->isActive());
  }

  entryDlg->setCell(row, col + KV_EXTRA_COLS, m_tableView->selectionModel()->selectedRows());
  m_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
}


void KVocTrainApp::slotDocumentProperties()
{
  int old_lessons = (int) m_lessonsComboBox->count();
  int old_types = (int) m_doc->typeDescriptions().count();
  int old_tenses = (int) m_doc->tenseDescriptions().count();
  int old_usages = (int) m_doc->usageDescriptions().count();
  QList<int> old_lessonsinquery = m_doc->lessonsInQuery();

  DocPropsDlg ddlg(m_doc, this);

  if (ddlg.exec() == QDialog::Accepted) {
    QList<int> typeIndex;
    QList<int> tenseIndex;
    QList<int> usageIndex;
    QList<int> lessonIndex;
    QStringList new_typeStr;
    QStringList new_tenseStr;
    QStringList new_usageStr;
    QStringList new_lessonStr;
    QList<int> new_lessonsinquery;

    m_doc->setTitle(ddlg.getTitle());
    m_doc->setAuthor(ddlg.getAuthor());
    m_doc->setLicense(ddlg.getLicense());
    m_doc->setDocumentRemark(ddlg.getDocRemark());

    slotStatusMsg(i18n("Updating lesson indices..."));
    QApplication::setOverrideCursor(Qt::WaitCursor);

    ddlg.getLesson(new_lessonStr, lessonIndex);
    ddlg.getTypeNames(new_typeStr, typeIndex);
    ddlg.getTenseNames(new_tenseStr, tenseIndex);
    ddlg.getUsageLabels(new_usageStr, usageIndex);

    new_lessonsinquery = old_lessonsinquery;
    LessOptPage::cleanUnused(m_doc, lessonIndex, old_lessons, new_lessonsinquery);

    slotStatusMsg(i18n("Updating type indices..."));
    TypeOptPage::cleanUnused(m_doc, typeIndex, old_types);
    KVTQuery::setTypeNames(new_typeStr);

    slotStatusMsg(i18n("Updating tense indices..."));
    TenseOptPage::cleanUnused(m_doc, tenseIndex, old_tenses);
    KEduVocConjugation::setTenseNames(new_tenseStr);

    slotStatusMsg(i18nc("usage (area) of an expression", "Updating usage label indices..."));
    UsageOptPage::cleanUnused(m_doc, usageIndex, old_usages);
    KVTUsage::setUsageNames(new_usageStr);

    m_doc->setSortingEnabled(ddlg.getSorting());
    m_tableView->setSortingEnabled(m_doc->isSortingEnabled());
    m_doc->setTypeDescriptions(new_typeStr);
    m_doc->setTenseDescriptions(new_tenseStr);
    m_doc->setUsageDescriptions(new_usageStr);
    m_doc->setLessonDescriptions(new_lessonStr);
    m_doc->setLessonsInQuery(new_lessonsinquery);
    m_doc->setModified();

    querymanager.setLessonItems(new_lessonsinquery);
    fillLessonBox();
    m_tableModel->reset();
    setCaption(m_doc->title(), m_doc->isModified());
    QApplication::restoreOverrideCursor();
    slotStatusMsg(IDS_DEFAULT);
  }
}


void KVocTrainApp::slotDocPropsLang()
{
  DocPropsLangDlg ldlg(m_doc, m_languages);

  if (ldlg.exec() == QDialog::Accepted) {
    for (int i = 0; i < m_doc->identifierCount(); i++) {
      m_doc->setArticle(i, ldlg.getArticle(i));
      m_doc->setConjugation(i, ldlg.getConjugation(i));
    }

    m_doc->setModified();
    m_tableModel->reset();
    setCaption(m_doc->title(), m_doc->isModified()); 
    slotStatusMsg(IDS_DEFAULT);
  }
}


void KVocTrainApp::slotModifiedDoc(bool /*mod*/)
{
  setCaption(m_doc->title(), m_doc->isModified());
  slotStatusMsg(IDS_DEFAULT);
}


void KVocTrainApp::slotRemoveRow()
{
  if (m_tableView->selectionModel()->selectedRows().count() == 1) {
    if (KMessageBox::Continue == KMessageBox::warningContinueCancel(this, i18n("Do you really want to delete the selected entry?"), "", KStandardGuiItem::del()))
    {
      int currentRow = m_tableView->currentIndex().row();
      int currentColumn = m_tableView->currentIndex().column();
      m_tableModel->removeRows(m_tableView->currentIndex().row(), 1, QModelIndex());
      m_tableView->selectionModel()->setCurrentIndex(m_tableModel->index(currentRow, currentColumn), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    }
  }
  else {
    if (KMessageBox::Continue == KMessageBox::warningContinueCancel(this, i18n("Do you really want to delete the selected entries?"), "", KStandardGuiItem::del()))
    {
      int currentRow = m_tableView->currentIndex().row();
      int currentColumn = m_tableView->currentIndex().column();
      int rowCount = m_tableModel->rowCount(QModelIndex());
      // Must count backwards otherwise entry-numbering goes wrong when
      // deleting.
      for (int i = rowCount - 1; i >= 0; i--)
        if (m_tableView->selectionModel()->isRowSelected(i, QModelIndex()))
          m_tableModel->removeRows(i, 1, QModelIndex());
      m_tableView->selectionModel()->setCurrentIndex(m_tableModel->index(currentRow, currentColumn), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    }
  }
  editDelete->setEnabled(m_tableModel->rowCount(QModelIndex()) > 0);
}


void KVocTrainApp::slotAppendRow ()
{
  m_tableModel->insertRows(m_tableModel->rowCount(QModelIndex()), 1, QModelIndex());
  m_tableView->selectionModel()->setCurrentIndex(m_tableModel->index(m_tableModel->rowCount(QModelIndex()) - 1, KV_COL_ORG), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
  m_tableModel->setData(m_tableView->currentIndex(), m_currentLesson, KVTTableModel::LessonRole);
  editDelete->setEnabled(m_tableModel->rowCount(QModelIndex()) > 0);
}


void KVocTrainApp::keyReleaseEvent(QKeyEvent *e)
{
  switch(e->key()) {
    case Qt::Key_Control:  
      controlActive = false;
      break;
  }
}


void KVocTrainApp::keyPressEvent(QKeyEvent *e)
{
  controlActive = (e->modifiers() & Qt::ControlModifier) !=0;

  switch(e->key()) {
    case Qt::Key_Plus:
      if (controlActive) {
        int less = m_lessonsComboBox->currentIndex();
        if (less == m_lessonsComboBox->count() - 1)
          m_lessonsComboBox->setCurrentIndex(0);
        else
          m_lessonsComboBox->setCurrentIndex(less+1);
        slotChooseLesson(m_lessonsComboBox->currentIndex());
      }
    break;

    case Qt::Key_Minus:
      if (controlActive) {
        int less = m_lessonsComboBox->currentIndex();
        if (less == 0)
          m_lessonsComboBox->setCurrentIndex(m_lessonsComboBox->count() - 1);
        else
          m_lessonsComboBox->setCurrentIndex(less - 1);
        slotChooseLesson(m_lessonsComboBox->currentIndex());
      }
    break;

    case Qt::Key_Control:
      controlActive = true;
      break;

    case Qt::Key_Tab:
      if (m_tableView->hasFocus())  {
        m_searchLine->setFocus();
        m_searchLine->selectAll();
      }
      else
        m_tableView->setFocus();
    break;

    case Qt::Key_Backtab:
      if (m_searchLine->hasFocus())
        m_tableView->setFocus();
      else {
        m_searchLine->setFocus();
        m_searchLine->selectAll();
      }
    break;

    default:
      bool found = false;
      if (!found)
        e->ignore();
  }
  slotStatusMsg(IDS_DEFAULT);
}


void KVocTrainApp::slotChooseLesson(int idx)
{
  m_currentLesson = idx;
  m_doc->setCurrentLesson(idx);
  m_doc->setModified(true);
}


void KVocTrainApp::slotCreateLesson(int header)
{
  QList<int> sel;
  m_doc->setModified();
  for (int i = 0; i < m_tableModel->rowCount(QModelIndex()); i++) {
    KEduVocExpression *kv = m_doc->entry(i);
    kv->setLesson(0);
    if (kv->grade(header) > THRESH_LESSON && !kv->translation(header).isEmpty())
      sel.push_back(i);
  }

  int cnt = 0;
  while (cnt < MAX_LESSON && sel.size() != 0) {
    int nr = m_randomSequence.getLong(sel.size());
    KEduVocExpression *kv = m_doc->entry(sel[nr]);
    // include non-lesson and non-empty string
    if (kv->lesson() == 0) {
      kv->setLesson(1);
      sel.erase (sel.begin() + nr);
      cnt++;
    }
  }
  m_tableModel->reset();
}


void KVocTrainApp::slotShowStatistics()
{
  KVTStatisticsDialog sdlg(m_languages, m_tableModel, this);
  sdlg.exec();
}


void KVocTrainApp::slotCleanVocabulary()
{
  prepareProgressBar();
  QApplication::setOverrideCursor(Qt::WaitCursor);
  int num = m_doc->cleanUp();
  QApplication::restoreOverrideCursor();
  removeProgressBar();

  slotStatusMsg(IDS_DEFAULT);

  if (num != 0) {
    m_tableModel->reset();
    QString s = i18np("1 entry with the same content was found and removed.",
                      "%1 entries with the same content were found and removed.", num);

    KMessageBox::information(this, s, i18n("Remove Duplicates"));
  }
}


void KVocTrainApp::slotCreateRandom()
{
  bool ok = false;
  int res = KInputDialog::getInteger(i18n("Entries per Lesson"), i18n("Enter number of entries per lesson:"), Prefs::entriesPerLesson(), 1, 1000, 1, &ok, this);
  if (!ok)
    return;

  Prefs::setEntriesPerLesson(res);

  slotStatusMsg(i18n("Creating random lessons..."));
  QApplication::setOverrideCursor(Qt::WaitCursor);

  QList<KEduVocExpression*> randomList;
  for (int i = 0; i < m_doc->entryCount(); i++) {
    KEduVocExpression *expr = m_doc->entry(i);
    if (expr->lesson() == 0)
      randomList.append(expr);
  }

  if (randomList.size() != 0) {
    int less_no = m_lessonsComboBox->count();
    QString s;
    s.setNum(less_no);
    s.prepend("- ");
    m_lessonsComboBox->addItem(s);
    int less_cnt = Prefs::entriesPerLesson();
    while (randomList.size() != 0) {
      if (--less_cnt <= 0) {
        less_cnt = Prefs::entriesPerLesson();
        less_no++;
        s.setNum(less_no);
        s.prepend("- ");
        m_lessonsComboBox->addItem(s);
      }
      int nr = m_randomSequence.getLong(randomList.size());
      randomList.takeAt(nr)->setLesson(less_no);
    }

    QStringList new_lessonStr;
    for (int i = 1; i < m_lessonsComboBox->count(); i++)
      new_lessonStr.append(m_lessonsComboBox->itemText(i));
    m_doc->setLessonDescriptions(new_lessonStr);
    m_tableModel->reset();
    m_doc->setModified ();
  }
  QApplication::restoreOverrideCursor();
  slotStatusMsg(IDS_DEFAULT);
}


void KVocTrainApp::slotGeneralOptions()
{
  slotGeneralOptionsPage(-1);
}


void KVocTrainApp::slotGeneralOptionsPage(int index)
{
  KVocTrainPrefs* dialog = new KVocTrainPrefs(m_languages, &querymanager, this, "settings",  Prefs::self());
  connect(dialog, SIGNAL(settingsChanged(const QString &)), this, SLOT(slotApplyPreferences()));
  if (index >= 0)
    dialog->selectLanguagePage();
  dialog->show();
}


void KVocTrainApp::slotApplyPreferences()
{
  if (Prefs::autoBackup())
    QTimer::singleShot(Prefs::backupTime() * 60 * 1000, this, SLOT(slotTimeOutBackup()));

  if (m_pronunciationStatusBarLabel)
    m_pronunciationStatusBarLabel->setFont(Prefs::iPAFont());

  m_tableView->setFont(Prefs::tableFont());
  m_tableView->reset();

  readLanguages();
  m_tableModel->setLanguages(m_languages);
  m_tableModel->reset();
}


void KVocTrainApp::slotAppendLanguage(int index)
{
  if (index >= m_languages.count()){

    QString msg = i18n("To add a language which is not listed in the menu, you must first define its properties in the "
                       "general options dialog.\n\nWould you like to add a new language?");
    if (KMessageBox::Yes == KMessageBox::questionYesNo(this, msg))
    {
      slotGeneralOptionsPage(1);
    }
    return;
  }

  m_tableModel->insertColumns(m_tableModel->columnCount(QModelIndex()), 1, QModelIndex());
  m_tableModel->setHeaderData(m_tableModel->columnCount(QModelIndex()) - 1, Qt::Horizontal, m_languages.shortId(index), Qt::EditRole);
}


void KVocTrainApp::slotAssignLanguage(QAction * action)
{
  int column = qvariant_cast<QPoint>(action->data()).x() + KV_EXTRA_COLS;
  int index  = qvariant_cast<QPoint>(action->data()).y();

  if (index >= (int) m_languages.count())
    return;

  m_tableModel->setHeaderData(column, Qt::Horizontal, m_languages.shortId(index), Qt::EditRole);
}

void KVocTrainApp::slotRemoveLanguage(int index)
{
  int column = index + KV_EXTRA_COLS + 1;
  QString name = m_tableModel->headerData(column, Qt::Horizontal, Qt::DisplayRole).toString();

  QString msg = i18n("You are about to delete a language permanently.\nDo you really want to delete '%1'?", name);

  int result = KMessageBox::warningContinueCancel(this, msg, "", KStandardGuiItem::del());
  if (result == KMessageBox::Continue)
    m_tableModel->removeColumns(column, 1, QModelIndex());
}

void KVocTrainApp::slotSearch(const QString& s)
{
  if (s.length() == 0) {
    m_sortFilterModel->delSearchFilter();
    m_tableModel->reset();
    return;
  }
  QString searchterm = QString(s);
  searchterm.replace(QString(" "), QString("|"));
  QRegExp searchRegExp = QRegExp(".*(" + searchterm + ").*");
  searchRegExp.setCaseSensitivity(Qt::CaseInsensitive);
  m_sortFilterModel->setSearchRegExp(searchRegExp);
  m_tableModel->reset();
}


void KVocTrainApp::slotStatusMsg(const QString &/*text*/)
{
//
}


void KVocTrainApp::aboutToShowLearn()
{
  QMenu *identifierSubMenu;
  QMenu *identifierSubSubMenu;
  QAction *typeAction;

  QString mapString = QString("%1%2%3");
  QChar fillChar = QLatin1Char('0');

  learningMenu->clear();

  if (m_learningMapper != 0)
    delete m_learningMapper;
  m_learningMapper = 0;
  m_learningMapper = new QSignalMapper(this);
  connect(m_learningMapper, SIGNAL(mapped(const QString &)), this, SLOT(slotLearningMapperTriggered(const QString &)));

  int columns = m_tableModel->columnCount(QModelIndex()) - KV_EXTRA_COLS;

  //collect needed data
  QStringList titles;
  QList<QPixmap> icons;
  for (int i = 0; i < columns; i++) {
    titles.append(m_tableModel->headerData(i + KV_EXTRA_COLS, Qt::Horizontal, Qt::DisplayRole).toString());
    icons.append(m_tableModel->headerData(i + KV_EXTRA_COLS, Qt::Horizontal, Qt::DecorationRole).value<QPixmap>());
  }

  for (int j = 0; j < columns; j++) {

    //top level item for each identifier
    identifierSubMenu = learningMenu->addMenu(icons[j], titles[j]);

    //Random and multiple choice items
    if (j == 0) {
      identifierSubSubMenu = identifierSubMenu->addMenu(KIcon("run_query"), i18n("Create Random &Query"));
      for (int k = 1; k < columns; k++) {
        typeAction = identifierSubSubMenu->addAction(icons[k], i18n("From %1", titles[k]), m_learningMapper, SLOT(map()));
        typeAction->setWhatsThis(i18n("Creates and starts query from %1 to %2", titles[k], titles[0]));
        typeAction->setToolTip(typeAction->whatsThis());
        typeAction->setStatusTip(typeAction->whatsThis());
        m_learningMapper->setMapping(typeAction, mapString.arg(START_QUERY, 3, 10, fillChar).arg(j, 3, 10, fillChar).arg(k, 3, 10, fillChar));
      }

      identifierSubSubMenu = identifierSubMenu->addMenu(KIcon("run_multi"), i18n("Create &Multiple Choice"));
      for (int k = 1; k < columns; k++) {
        typeAction = identifierSubSubMenu->addAction(icons[k], i18n("From %1", titles[k]), m_learningMapper, SLOT(map()));
        typeAction->setWhatsThis(i18n("Creates and starts multiple choice from %1 to %2", titles[k], titles[0]));
        typeAction->setToolTip(typeAction->whatsThis());
        typeAction->setStatusTip(typeAction->whatsThis());
        m_learningMapper->setMapping(typeAction, mapString.arg(START_MULTIPLE, 3, 10, fillChar).arg(j, 3, 10, fillChar).arg(k, 3, 10, fillChar));
      }
    }
    else
    {
      typeAction = identifierSubMenu->addAction(KIcon("run_query"), i18n("Create Random &Query"), m_learningMapper, SLOT(map()));
      typeAction->setWhatsThis(i18n("Creates and starts query from %1 to %2", titles[0], titles[j]));
      typeAction->setToolTip(typeAction->whatsThis());
      typeAction->setStatusTip(typeAction->whatsThis());
      m_learningMapper->setMapping(typeAction, mapString.arg(START_QUERY, 3, 10, fillChar).arg(j, 3, 10, fillChar).arg(0, 3, 10, fillChar));

      typeAction = identifierSubMenu->addAction(KIcon("run_multi"), i18n("Create &Multiple Choice"), m_learningMapper, SLOT(map()));
      typeAction->setWhatsThis(i18n("Creates and starts multiple choice from %1 to %2", titles[0], titles[j]));
      typeAction->setToolTip(typeAction->whatsThis());
      typeAction->setStatusTip(typeAction->whatsThis());
      m_learningMapper->setMapping(typeAction, mapString.arg(START_MULTIPLE, 3, 10, fillChar).arg(j, 3, 10, fillChar).arg(0, 3, 10, fillChar));
    }

    identifierSubMenu->addSeparator();

    //Special queries items for each identifier
    typeAction = identifierSubMenu->addAction(i18n("&Verbs"), m_learningMapper, SLOT(map()));
    typeAction->setWhatsThis(i18n("Starts training with verbs"));
    typeAction->setToolTip(typeAction->whatsThis());
    typeAction->setStatusTip(typeAction->whatsThis());
    m_learningMapper->setMapping(typeAction, mapString.arg(START_VERB, 3, 10, fillChar).arg(j, 3, 10, fillChar).arg(0, 3, 10, fillChar));

    typeAction = identifierSubMenu->addAction(i18n("&Articles"), m_learningMapper, SLOT(map()));
    typeAction->setWhatsThis(i18n("Starts training with articles"));
    typeAction->setToolTip(typeAction->whatsThis());
    typeAction->setStatusTip(typeAction->whatsThis());
    m_learningMapper->setMapping(typeAction, mapString.arg(START_ARTICLE, 3, 10, fillChar).arg(j, 3, 10, fillChar).arg(0, 3, 10, fillChar));

    typeAction = identifierSubMenu->addAction(i18n("&Comparisons"), m_learningMapper, SLOT(map()));
    typeAction->setWhatsThis(i18n("Starts training with adjectives"));
    typeAction->setToolTip(typeAction->whatsThis());
    typeAction->setStatusTip(typeAction->whatsThis());
    m_learningMapper->setMapping(typeAction, mapString.arg(START_ADJECTIVE, 3, 10, fillChar).arg(j, 3, 10, fillChar).arg(0, 3, 10, fillChar));

    identifierSubMenu->addSeparator();

    typeAction = identifierSubMenu->addAction(i18n("S&ynonyms"), m_learningMapper, SLOT(map()));
    typeAction->setWhatsThis(i18n("Starts training with synonyms"));
    typeAction->setToolTip(typeAction->whatsThis());
    typeAction->setStatusTip(typeAction->whatsThis());
    m_learningMapper->setMapping(typeAction, mapString.arg(START_SYNONYM, 3, 10, fillChar).arg(j, 3, 10, fillChar).arg(0, 3, 10, fillChar));

    typeAction = identifierSubMenu->addAction(i18n("A&ntonyms"), m_learningMapper, SLOT(map()));
    typeAction->setWhatsThis(i18n("Starts training with antonyms"));
    typeAction->setToolTip(typeAction->whatsThis());
    typeAction->setStatusTip(typeAction->whatsThis());
    m_learningMapper->setMapping(typeAction, mapString.arg(START_ANTONYM, 3, 10, fillChar).arg(j, 3, 10, fillChar).arg(0, 3, 10, fillChar));

    typeAction = identifierSubMenu->addAction(i18n("E&xamples"), m_learningMapper, SLOT(map()));
    typeAction->setWhatsThis(i18n("Starts training with examples"));
    typeAction->setToolTip(typeAction->whatsThis());
    typeAction->setStatusTip(typeAction->whatsThis());
    m_learningMapper->setMapping(typeAction, mapString.arg(START_EXAMPLE, 3, 10, fillChar).arg(j, 3, 10, fillChar).arg(0, 3, 10, fillChar));

    typeAction = identifierSubMenu->addAction(i18n("&Paraphrase"), m_learningMapper, SLOT(map()));
    typeAction->setWhatsThis(i18n("Starts training with paraphrases"));
    typeAction->setToolTip(typeAction->whatsThis());
    typeAction->setStatusTip(typeAction->whatsThis());
    m_learningMapper->setMapping(typeAction, mapString.arg(START_PARAPHRASE, 3, 10, fillChar).arg(j, 3, 10, fillChar).arg(0, 3, 10, fillChar));
  }

  learningMenu->addSeparator();
  learningMenu->addAction(learningResumeQuery);
  learningMenu->addAction(learningResumeMultipleChoice);
}


void KVocTrainApp::aboutToShowVocabAppendLanguage()
{
  if (m_doc != 0)
  {
    vocabAppendLanguage->removeAllActions();

    QStringList names;
    for (int i = 0; i < m_languages.count(); i++)
    {
      if (m_languages.longId(i).isEmpty())
        names.append(m_languages.shortId(i));
      else
        names.append(m_languages.longId(i));
    }

    QAction *action = 0;

    for (int i = 0; i < m_languages.count(); i++)
    {
      if (!m_languages.pixmapFile(i).isEmpty() && !m_languages.longId(i).isEmpty())
        action = new QAction(QIcon(QPixmap(m_languages.pixmapFile(i))), names[i], vocabAppendLanguage->selectableActionGroup());
      else
        action = new QAction(names[i], vocabAppendLanguage->selectableActionGroup());
      action->setWhatsThis(i18n("Add the language '%1' to the vocabulary", names[i]));
      action->setToolTip(action->whatsThis());
      action->setStatusTip(action->whatsThis());
      vocabAppendLanguage->addAction(action);
    }

    action = new QAction("", vocabAppendLanguage->selectableActionGroup());
    action->setSeparator(true);
    vocabAppendLanguage->addAction(action);

    action = new QAction(i18n("&New Language..."), vocabAppendLanguage->selectableActionGroup());
    action->setWhatsThis(i18n("Add a new language to the vocabulary"));
    action->setToolTip(action->whatsThis());
    action->setStatusTip(action->whatsThis());
    vocabAppendLanguage->addAction(action); 
  }
}


void KVocTrainApp::aboutToShowVocabSetLanguage()
{
  if (m_doc != 0)
  {
    vocabSetLanguage->removeAllActions();
    //QMenu * set_m = vocabSetLanguage->menu();

    QStringList names;
    for (int i = 0; i < m_languages.count(); i++)
    {
      if (m_languages.longId(i).isEmpty())
        names.append(m_languages.shortId(i));
      else
        names.append(m_languages.longId(i));
    }

    QAction *action = 0;
    KSelectAction *selAction = 0;

    for (int column = 0; column < m_doc->identifierCount(); ++column)
    {
      if (column == 0)
        selAction = new KSelectAction(i18n("&Original"), vocabSetLanguage->selectableActionGroup());
      else {
        if (m_doc->identifierCount() <= 2)
          selAction = new KSelectAction(i18n("&Translation"), vocabSetLanguage->selectableActionGroup());
        else
          selAction = new KSelectAction(i18n("&%1. Translation", column), vocabSetLanguage->selectableActionGroup());
      }
      connect(selAction, SIGNAL(triggered(QAction *)), this, SLOT(slotAssignLanguage(QAction *)));
      vocabSetLanguage->addAction(selAction);

      for (int i = 0; i < m_languages.count(); i++)
      {
        if (!m_languages.pixmapFile(i).isEmpty() && !m_languages.longId(i).isEmpty())
          action = new QAction(QIcon(QPixmap(m_languages.pixmapFile(i))), names[i], selAction->selectableActionGroup());
        else
          action = new QAction(names[i], selAction->selectableActionGroup());
        action->setData(QVariant(QPoint(column, i))); //QPair doesn't work with QVariant
        action->setWhatsThis(i18n("Assign the language '%1' to the selected column", names[i]));
        action->setToolTip(action->whatsThis());
        action->setStatusTip(action->whatsThis());
        selAction->addAction(action);
      }
    }
  }
}


void KVocTrainApp::aboutToShowVocabRemoveLanguage()
{
  if (m_doc != 0)
  {
    vocabRemoveLanguage->removeAllActions();

    QStringList names;
    for (int j = 1; j < (int) m_doc->identifierCount(); j++)
    {
      int i;
      if ((i = m_languages.indexShortId(m_doc->identifier(j))) >= 0)
        names.append(m_languages.longId(i));
      else
        names.append(m_doc->identifier(j));
    }

    QAction *action = 0;

    for (int i = 1; i < (int) m_doc->identifierCount(); i++)
    {
      int j;
      if ((j = m_languages.indexShortId(m_doc->identifier(i))) >= 0 && !m_languages.pixmapFile(j).isEmpty() && !m_languages.longId(j).isEmpty())
        action = new QAction(QIcon(QPixmap(m_languages.pixmapFile(j))), names[i - 1], vocabRemoveLanguage->selectableActionGroup());
      else
        action = new QAction(names[i - 1], vocabRemoveLanguage->selectableActionGroup());
      action->setWhatsThis(i18n("Permanently remove the language '%1' from the vocabulary", names[i - 1]));
      action->setToolTip(action->whatsThis());
      action->setStatusTip(action->whatsThis());
      vocabRemoveLanguage->addAction(action);
    }
  }
}


void KVocTrainApp::slotStatusHelpMsg(const QString &text)
{
  ///////////////////////////////////////////////////////////////////
  // change status message of whole statusbar temporary (text, msec)
  if (pbar == 0 || !pbar->isVisible())
    statusBar()->showMessage(text, 3000);
}

void KVocTrainApp::slotFilePrint()
{
  slotStatusMsg(i18n("Printing..."));
  KPrinter printer;
  printer.setFullPage(true);
  if (printer.setup(this))
    m_tableView->print(&printer);
  slotStatusMsg(i18n("Ready"));
}


QList<int> KVocTrainApp::csvOrder()
{
  QList<int> csv_order;
  QStringList languageList = Prefs::pasteOrder();

  if (!Prefs::useCurrent())
  {
    foreach(const QString &language, languageList)
    {
      int j = m_doc->indexOfIdentifier(language);
      if (j >= 0)
        csv_order.append(j);
      else
        csv_order.append(-1);
    }
  }
/*
  for (int i = 0; i < csv_order.size(); i++)
    cout << csv_order[i] << " ";
  cout << endl;
*/

/*  perhaps skip missing ??
  for (int i = csv_order.size()-1; i >= 0; i--)
    if (csv_order[i] == -1)
      csv_order.erase(csv_order.begin() + i);
*/
  // append indices from doc if no order given
  for (int i = 0; i < m_doc->identifierCount(); i++)
    if (qFind (csv_order.begin(), csv_order.end(), i) == csv_order.end())
       csv_order.append(i);
/*
  if (csv_order.size() > doc->numIdentifiers())
    csv_order.erase(csv_order.begin() + doc->numIdentifiers(), csv_order.end());
*/

  // remove trailing garbage
  for (int i = csv_order.size()-1; i >= 0; i--) {
    if (csv_order[i] != -1)
      break;
    else
      csv_order.erase(csv_order.begin() + i);
  }

  return csv_order;
}


void KVocTrainApp::slotEditCopy()
{
  slotStatusMsg(i18n("Copying selection to clipboard..."));

  QApplication::setOverrideCursor(Qt::WaitCursor);

  QString textToCopy;
  QList<int> csv_order = csvOrder();
  QModelIndexList selectedRows = m_tableView->selectionModel()->selectedRows(0);

  foreach(const QModelIndex &idx, selectedRows)
  {
    bool sep = false;
    foreach(int i, csv_order)
    {
      if (!sep)
        sep = true;
      else
        textToCopy += Prefs::separator();

      if (i >= 0)
        textToCopy += m_tableModel->data(m_tableModel->index(idx.row(), i + KV_COL_ORG), Qt::DisplayRole).toString();
    }
    if (!textToCopy.isEmpty())
      textToCopy += '\n';
  }

  if (!textToCopy.isEmpty())
    QApplication::clipboard()->setText(textToCopy);

  QApplication::restoreOverrideCursor();
  slotStatusMsg(IDS_DEFAULT);
}


void KVocTrainApp::slotEditPaste()
{
  slotStatusMsg(i18n("Inserting clipboard contents..."));

  QApplication::setOverrideCursor(Qt::WaitCursor);
  QString s;
  QString textToPaste = QApplication::clipboard()->text();

  QTextStream ts;
  ts.setString(&textToPaste, QIODevice::Text);
  QList<int> csv_order = csvOrder();

  QString num;

  while (!ts.atEnd()) {
    s = ts.readLine();
    kDebug() << s << endl;
    // similar block in kvd_csv.cpp::loadFromCsv()

    if (!s.isEmpty()) {
      m_tableModel->insertRows(m_tableModel->rowCount(QModelIndex()), 1, QModelIndex());
      QStringList sl = s.split(Prefs::separator(), QString::KeepEmptyParts);
      ///@todo check this function
      if (csv_order.count() > 0) {
        // now move columns according to paste-order
        int j = 0;
        foreach(int i, csv_order)
        {
          kDebug() << "i= " << i << " j= " << j << endl;
          if (j < sl.count()) {
            m_tableModel->setData(m_tableModel->index(m_tableModel->rowCount(QModelIndex()) - 1, i + KV_COL_ORG), sl[j], Qt::EditRole);
            m_tableModel->setData(m_tableModel->index(m_tableModel->rowCount(QModelIndex()) - 1, i + KV_COL_ORG), m_currentLesson, KVTTableModel::LessonRole);
          }
          j++;
        }
      }
      else {
        for (int i = 0; i < sl.count(); ++i)
        {
          m_tableModel->setData(m_tableModel->index(m_tableModel->rowCount(QModelIndex()) - 1, i + KV_COL_ORG), sl[i], Qt::EditRole);
          m_tableModel->setData(m_tableModel->index(m_tableModel->rowCount(QModelIndex()) - 1, i + KV_COL_ORG), m_currentLesson, KVTTableModel::LessonRole);
        }
      }
    }
  }

  QApplication::restoreOverrideCursor();
  slotStatusMsg(IDS_DEFAULT);
}

void KVocTrainApp::slotCurrentChanged(const QModelIndex & current, const QModelIndex & previous)
{
  Q_UNUSED(previous);
  if (!current.isValid())
    return;

  int column = current.column() - KV_EXTRA_COLS;
  int row = current.row();
  bool noData = false;
  KEduVocExpression *expr = 0;

  if (m_doc->entryCount() <= row || m_doc->identifierCount() <= column || row < 0 || column < 0)
    noData = true;
  else
    expr = m_doc->entry(row);

  if (m_remarkStatusBarLabel != 0)
    m_remarkStatusBarLabel->setText(i18nc("Abbreviation for R)emark","R: %1", noData ? QString() : expr->remark(column)));
  if (m_pronunciationStatusBarLabel != 0)
    m_pronunciationStatusBarLabel->setText(i18nc("Abbreviation for P)ronouncation","P: %1", noData ? QString() : expr->pronunciation(column)));
  if (m_typeStatusBarLabel != 0)
    m_typeStatusBarLabel->setText(i18nc("Abbreviation for T)ype of word", "T: %1", noData ? QString() : KVTQuery::typeStr(expr->type(column))));

  if (entryDlg != 0) {
    slotEditEntry2(current);
  }
}


void KVocTrainApp::updateTableFilter()
{
  QModelIndexList indexes = m_lessonView->selectionModel()->selectedIndexes();
  // oops - this crashes if there is no selection - there should always be a current lesson!!!
  if (indexes.empty()) {
    kDebug() << "WARNING - NO SELECTION FOR ACTIVE LESSON! THIS SHOULD NOT HAPPEN!" << endl;
    return;
  }
  QModelIndex current = indexes.at(0); // should be one item selected anyway...

  m_doc->setCurrentLesson(current.row()+1);
  int comboState = m_lessonSelectionCombo->currentIndex();

  // do the columns play a role here??? I mean... I also wanto to use this for search - to filter out results... so this should only apply for the lesson column. How do I do something with the search at the same time?

  //QRegExp myReg("(Lektion 09)|(Lektion 04)");
  QString lessonStrings;
  QStringList description;

  switch (comboState) {
    case Prefs::EnumLessonEditingSelection::CurrentLesson:
      m_sortFilterModel->setLessonRegExp( QRegExp(m_lessonModel->data(current, Qt::DisplayRole).toString(), Qt::CaseInsensitive, QRegExp::FixedString) );
      break;
    case Prefs::EnumLessonEditingSelection::LessonsInQuery:
      description = m_doc->lessonDescriptions();
      //kDebug << lessonStrings << endl;
      lessonStrings.append("(");
      foreach(int lesson, m_doc->lessonsInQuery()){
        lessonStrings.append(m_doc->lessonDescriptions().at(lesson-1));
        lessonStrings.append(")|(");
      }
      lessonStrings.remove(lessonStrings.length()-2, 2); // remove the last "|("
      if(lessonStrings.compare("")==0) // in this case select none to be consistent!
      {
        lessonStrings = "$^"; // hoping no-one hase a lesson called "" for now. It's your own fault, if you call them like this ;) this is generally unfortunate... maybe I should forbid it and default back to "New lesson 1".
      }
      //m_sortFilterModel->setFilterRegExp(lessonStrings);

      m_sortFilterModel->setLessonRegExp(QRegExp(lessonStrings));
      break;
    case Prefs::EnumLessonEditingSelection::AllLessons:
      m_sortFilterModel->setLessonRegExp(QRegExp());
      break;
  }
  m_doc->setModified();
  m_tableModel->reset();
}

void KVocTrainApp::slotLessonSelectionComboChanged(int index)
{
  Q_UNUSED(index);
  updateTableFilter();
}

void KVocTrainApp::slotCurrentLessonChanged(const QModelIndex &current, const QModelIndex &previous)
{
  Q_UNUSED(previous);
  Q_UNUSED(current);
  updateTableFilter();
}


void KVocTrainApp::slotLearningMapperTriggered(const QString & mapString)
{
  int cmd     = mapString.mid(0, 3).toInt();
  int header1 = mapString.mid(3, 3).toInt();
  int header2 = mapString.mid(6, 3).toInt();
  kDebug() << mapString << " " << cmd << " " << header1 << " " << header2 << endl;

  switch (cmd) {

    case START_QUERY:
      delete randomQueryDlg;
      randomQueryDlg = 0;
      m_queryType = KVTQuery::RandomQuery;
      slotStartQuery(header1 ? m_doc->identifier(header1) : m_doc->originalIdentifier(),
                     header2 ? m_doc->identifier(header2) : m_doc->originalIdentifier(), true);
    break;

    case START_MULTIPLE:
      delete mcQueryDlg;
      mcQueryDlg = 0;
      m_queryType = KVTQuery::MultipleChoiceQuery;
      slotStartQuery(header1 ? m_doc->identifier(header1) : m_doc->originalIdentifier(),
                     header2 ? m_doc->identifier(header2) : m_doc->originalIdentifier(), true);
    break;

    case START_VERB: {
      delete verbQueryDlg;
      verbQueryDlg = 0;
      m_queryType = KVTQuery::ConjugationQuery;
      slotStartTypeQuery(header1, QM_VERB);
    }
    break;

    case START_ARTICLE: {
      delete artQueryDlg;
      artQueryDlg = 0;
      m_queryType = KVTQuery::ArticlesQuery;
      slotStartTypeQuery(header1, QM_NOUN);
    }
    break;

    case START_ADJECTIVE: {
      delete adjQueryDlg;
      adjQueryDlg = 0;
      m_queryType = KVTQuery::ComparisonQuery;
      slotStartTypeQuery(header1, QM_ADJ);
    }
    break;

    case START_SYNONYM: {
      delete simpleQueryDlg;
      simpleQueryDlg = 0;
      slotStartPropertyQuery(header1, KVTQuery::SynonymQuery);
    }
    break;

    case START_ANTONYM: {
      delete simpleQueryDlg;
      simpleQueryDlg = 0;
      slotStartPropertyQuery(header1, KVTQuery::AntonymQuery);
    }
    break;

    case START_EXAMPLE: {
      delete simpleQueryDlg;
      simpleQueryDlg = 0;
      slotStartPropertyQuery(header1, KVTQuery::ExampleQuery);
    }
    break;

    case START_PARAPHRASE: {
      delete simpleQueryDlg;
      simpleQueryDlg = 0;
      slotStartPropertyQuery(header1, KVTQuery::ParaphraseQuery);
    }
    break;

    default:
       kError() << "Unknown command" << endl;

  }
  slotStatusMsg(IDS_DEFAULT);
}

void KVocTrainApp::slotConfigShowSearch()
{
  if (m_searchWidget) {
    m_searchWidget->setVisible(m_searchWidget->isHidden());
    Prefs::setShowSearch(m_searchWidget->isVisible());
  }
}

#include "kvoctrain.moc"
