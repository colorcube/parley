/***************************************************************************

                         edit common properties

    -----------------------------------------------------------------------

    begin         : Mon Jun 28 21:02:16 1999

    copyright     : (C) 1999-2001 Ewald Arnold <kvoctrain@ewald-arnold.de>
                    (C) 2005-2007 Peter Hedlund <peter.hedlund@kdemail.net>
                    (C) 2007 Frederik Gladhorn <frederik.gladhorn@kdemail.net>

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

#ifndef CommonEntryPage_included
#define CommonEntryPage_included


#include "ui_CommonEntryPageForm.h"

// kvtquery.h is still needed for usages
#include "practice/testentrymanager.h"

#include <keduvocwordtype.h>
#include <keduvocdocument.h>

class CommonEntryPage : public QWidget, public Ui::CommonEntryPageForm
{
    Q_OBJECT
public:
    explicit CommonEntryPage(KEduVocDocument *doc, QWidget* parent = 0);

    void setData(const QList<int>& entries, int currentTranslation);
    void commitData();

    bool isModified();

signals:
    void signalTypeSelected(const QString &);

private:
    void updateMainTypeBoxContents();
    void setLessonBox(int lesson);
    void setUsageSelection(const QStringList& selectionList);

private slots:
    // to get noted upon modification
    void slotDataChanged();
    void slotDataChanged(int);
    void slotDataChanged(const QString&);

    /// Updates the usage label and emits sigModified()
    void slotUsageChanged();
    /// Updates the subtype box and emits sigModified()
    void slotTypeBoxChanged(const QString &mainType);

    /**
     * Append a char from the phonetics dialog
     * @param  the new letter
     */
    void phoneticSelected(wchar_t);
    void invokeTypeDlg();
    void invokePronDlg();
    void invokeUsageDlg();
    void slotSubDialogClosed();

signals:
    void sigModified();

private:
    KEduVocDocument     *m_doc;

    KDialog             *subDialog;

    /// Column in the document - corresponds to the language (-KV_COL_TRANS)
    int               m_currentTranslation;
    /// Selection in the doc - if more than one row is selected behavior is different
    QList<int>        m_entries;
    bool m_usageIsModified;
    KEduVocWordType   m_wordTypes;
};
#endif // CommonEntryPage_included
