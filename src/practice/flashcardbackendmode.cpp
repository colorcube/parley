/***************************************************************************
    Copyright 2009 Frederik Gladhorn <gladhorn@kde.org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "flashcardbackendmode.h"

#include <KLocalizedString>

using namespace Practice;

FlashCardBackendMode::FlashCardBackendMode(AbstractFrontend* frontend, QObject* parent)
    : AbstractBackendMode(frontend, parent)
{
}

bool FlashCardBackendMode::setTestEntry(TestEntry* current)
{
    Practice::AbstractBackendMode::setTestEntry(current);
    m_current = current;
    m_currentHint.clear();
    m_solutionVisible = false;
    m_frontend->showQuestion();
    return true;
}

void FlashCardBackendMode::checkAnswer()
{
    emit showSolution();
}

void FlashCardBackendMode::hintAction()
{
    QString solution = m_current->entry()->translation(m_current->languageTo())->text();
    m_currentHint = solution.left(m_currentHint.size() + 1);
    if (m_currentHint.size() == solution.size()) {
        // show solution
        m_frontend->showSolution();
        m_solutionVisible = true;
    } else {
        m_frontend->setHint(m_currentHint);
    }
}
