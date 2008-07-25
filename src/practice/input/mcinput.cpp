//
// C++ Implementation: mcinput
//
// Description:
//
//
// Author: David Capel <wot.narg@gmail.com>, (C) 2008
//
/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "mcinput.h"

#include "../practiceentry.h"
#include "prefs.h"

#include <KDebug>
#include <KSvgRenderer>
#include <KRandom>
#include <QRadioButton>
#include <QVBoxLayout>
#include <KRandomSequence>
#include <QString>
#include <KLocalizedString>
#include <QStringList>
#include <QGraphicsView>

#include "keduvocwordtype.h"


MCInput::MCInput(KSvgRenderer * renderer, QGraphicsView * view, const QString& elementId, QWidget* parent)
        : QGroupBox(parent),
        m_renderer(renderer)
{
    if (!renderer->elementExists(elementId))
    {
        setVisible(false);
        kDebug() << "!! Element id doesn't exist:";
        kDebug() << elementId << ":" << renderer->elementExists(elementId);
    }

     QRect bounds = m_renderer->boundsOnElement(elementId).toRect();
     setGeometry(view->mapToScene(bounds).boundingRect().toRect());

    setAttribute(Qt::WA_OpaquePaintEvent, true);
}

void MCInput::slotShortcutTriggered(int shortcutNumber)
{
    if (shortcutNumber > Prefs::numberMultipleChoiceAnswers())
        return; // bogus false positive

    // Shortcut number 0 is triggered by return/enter and is used for activating the currently selected option.
    // Therefore, we check if any buttons are checked, and if so, emit the signal
    // if none are checked, we ignore this shortcut
    if (shortcutNumber == 0)
        // we emit only if a button is checked
        foreach(QRadioButton* b, findChildren<QRadioButton*>())
            if (b->isChecked())
            {
                emit triggered();
                return;
            }

    foreach(QRadioButton* b, findChildren<QRadioButton*>())
    {
        if (b->text().startsWith(QString("&%1 ").arg(shortcutNumber)))
        {
            b->setChecked(true);
            emit triggered();
            return;
        }
    }
    // we didn't find anything.
}

void MCInput::setAvailableAnswers(const QStringList list)
{
    // clean up from last time
    delete layout();

    foreach(QRadioButton* b, findChildren<QRadioButton*>())
    {
        delete b;
    }


    if (list.size() == 0)
    {
        kDebug() << "Source list empty. Aborted.";
        return;
    }

    // start fresh and new!

    QVBoxLayout *vbox = new QVBoxLayout;

    int n = 1;
    foreach(QString s, list)
    {
        vbox->addWidget(new QRadioButton(QString("&%1 %2").arg(n++).arg(s)));
    }

     vbox->addStretch(1);
     setLayout(vbox);
}


MCInput::~MCInput()
{
    foreach(QRadioButton* b, findChildren<QRadioButton*>())
    {
        delete b;
    }
}

void MCInput::slotEmitAnswer()
{
    foreach(QRadioButton* b, findChildren<QRadioButton*>())
        if (b->isChecked())
        {
            emit signalAnswer(b->text().remove(QRegExp("^&\\d ")));
        }
    emit signalAnswer(""); // none were selected.
}