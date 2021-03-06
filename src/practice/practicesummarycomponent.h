/***************************************************************************
    Copyright 2007-2009 Frederik Gladhorn <gladhorn@kde.org>
 ***************************************************************************

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PRACTICESUMMARYDIALOG_H
#define PRACTICESUMMARYDIALOG_H

#include "practice/sessionmanagerbase.h"

#include "ui_practicesummarywidget.h"
#include <KXmlGuiWindow>

namespace Practice
{

class PracticeSummaryComponent : public KXmlGuiWindow, Ui::PracticeSummaryWidget
{
    Q_OBJECT

public:
    PracticeSummaryComponent(SessionManagerBase* sessionManager, QWidget *parent);
    ~PracticeSummaryComponent();

public slots:
    void exportResults();

private:
    void initActions(QWidget* parleyMainWindow);
    void setupDetailsTable();

    SessionManagerBase* m_sessionManager;

    class SortedAttemptTableWidgetItem;
};
}

#endif
