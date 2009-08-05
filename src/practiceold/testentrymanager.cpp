/***************************************************************************

                             manage queries

    -----------------------------------------------------------------------

    begin         : Sat Jul 11 20:50:53 MET 1999

    copyright     : (C) 1999-2001 Ewald Arnold <kvoctrain@ewald-arnold.de>
                    (C) 2005-2007 Peter Hedlund <peter.hedlund@kdemail.net>
                    (C) 2007-2009 Frederik Gladhorn <gladhorn@kde.org>

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

#include "testentrymanager.h"


#include "practiceold/entryfilter.h"

// for the enum
#include "practicedialog.h"

#include <prefs.h>
#include <klocale.h>
#include <kconfig.h>

#include <keduvoclesson.h>
#include <keduvocexpression.h>
#include <keduvocdocument.h>
#include <QDateTime>


TestEntryManager::TestEntryManager(KEduVocDocument* doc, QWidget* parent)
    :m_doc(doc)
    ,m_parent(parent)
    ,m_fromTranslation(Prefs::questionLanguage())
    ,m_toTranslation(Prefs::solutionLanguage())
    ,m_testType(Prefs::testType())
    ,m_currentEntry(0)
    ,m_randomSequence(QDateTime::currentDateTime().toTime_t())
{
    TestEntry::setGradeTo(m_toTranslation);

    // write grades of mono lingual tests into translation(i).gradeFrom(i)
    if ( m_testType == Prefs::EnumTestType::WrittenTest ||
        m_testType == Prefs::EnumTestType::MultipleChoiceTest) {
        TestEntry::setGradeFrom(m_fromTranslation);
    } else {
        TestEntry::setGradeFrom(m_toTranslation);
    }

    // don't crash when trying to start practicing a document containing only one language
    if (m_doc->identifierCount() < 2) {
        KMessageBox::error(0, i18n("The vocabulary document contains no entries that can be used for the chosen type of practice."));
        return;
    }

    kDebug() << "Test from: " << m_doc->identifier(m_fromTranslation).name()
        << " to: " << m_doc->identifier(m_toTranslation).name();

    filterTestEntries();
    kDebug() << "Found " << m_allTestEntries.count() << " entries after filtering.";

    m_notAskedTestEntries = m_allTestEntries;

    for ( int i = 0; i < qMin(m_notAskedTestEntries.count(), Prefs::testNumberOfEntries() ); i++ ) {
        m_currentEntries.append( m_notAskedTestEntries.takeAt(0) );
    }
}

TestEntryManager::~TestEntryManager()
{
    qDeleteAll(m_allTestEntries);
}

void TestEntryManager::filterTestEntries()
{
    EntryFilter filter(m_parent, m_doc);
    QList<KEduVocExpression*> allEntries = filter.entries();

    if (allEntries.count() == 0) {
        return;
    }

    foreach (KEduVocExpression* entry, allEntries){
        m_allTestEntries.append(new TestEntry(entry));
    }
}

void TestEntryManager::currentEntryFinished()
{
    m_currentEntries.takeAt(m_currentEntry);
}

void TestEntryManager::printStatistics()
{
    kDebug() << "Test statistics: ";
    foreach ( TestEntry* entry, m_allTestEntries ) {
        kDebug()
            << " asked: " << entry->statisticCount()
            << " +" << entry->statisticGoodCount() << " -" << entry->statisticBadCount()
            << " ->+" << entry->statisticSkipKnown() << " ->-" << entry->statisticSkipUnknown()
            << " time:" << entry->statisticTimeout()
            << "Entry: " << entry->entry()->translation(0)->text();
    }
}

int TestEntryManager::totalEntryCount()
{
    return m_allTestEntries.count();
}

int TestEntryManager::activeEntryCount()
{
    return m_notAskedTestEntries.count() + m_currentEntries.count();
}

int TestEntryManager::statisticTotalCorrectFirstAttempt()
{
    int count = 0;
    foreach(TestEntry* entry, m_allTestEntries) {
        if ( entry->statisticCorrectAtFirstAttempt() ) {
            count++;
        }
    }
    return count;
}

int TestEntryManager::statisticTotalWrong()
{
    int count = 0;
    foreach(TestEntry* entry, m_allTestEntries) {
        if ( entry->statisticBadCount() ) {
            count++;
        }
    }
    return count;
}

int TestEntryManager::statisticTotalUnanswered()
{
    int count = 0;
    foreach(TestEntry* entry, m_allTestEntries) {
        if ( entry->statisticCount() == 0 ) {
            count++;
        }
    }
    return count;
}

int TestEntryManager::statisticTotalSkipKnown()
{
    int count = 0;
    foreach(TestEntry* entry, m_allTestEntries) {
        if ( entry->statisticSkipKnown() ) {
            count++;
        }
    }
    return count;
}

int TestEntryManager::statisticTotalSkipUnknown()
{
    int count = 0;
    foreach(TestEntry* entry, m_allTestEntries) {
        if ( entry->statisticSkipUnknown() ) {
            count++;
        }
    }
    return count;
}


TestEntry* TestEntryManager::getNextEntry()
{
    // refill current entries
    while ( m_currentEntries.count() < Prefs::testNumberOfEntries() &&
            m_notAskedTestEntries.count() > 0 ) {
        m_currentEntries.append( m_notAskedTestEntries.takeAt(0) );
    }

    int lastEntry = m_currentEntry;
    // return one of the current entries
    if ( m_currentEntries.count() > 0 ) {
        // one of the current words (by random)
        m_currentEntry = m_randomSequence.getLong(m_currentEntries.count());
        // do not allow to ask the same entry twice in a row
        if ( m_currentEntries.count() > 1 ) {
            while ( m_currentEntry == lastEntry ) {
                m_currentEntry = m_randomSequence.getLong(m_currentEntries.count());
            }
        }

        kDebug() << "nextEntry: " << m_currentEntry << " = " << m_currentEntries.value(m_currentEntry)->entry()->translation(0)->text() << " (" << m_currentEntries.count() + m_notAskedTestEntries.count() << "entries remaining)";

        return m_currentEntries.value(m_currentEntry);
    } else {
        return 0;
    }
}
