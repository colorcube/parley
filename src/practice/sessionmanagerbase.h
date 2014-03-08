/***************************************************************************
    copyright     : (C) 1999-2001 Ewald Arnold <kvoctrain@ewald-arnold.de>
                    (C) 2005-2007 Peter Hedlund <peter.hedlund@kdemail.net>
                    (C) 2007-2010 Frederik Gladhorn <gladhorn@kde.org>
                    (C) 2014      Inge Wallin <inge@lysator.liu.se>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PRACTICESESSIONMANAGERBASE_H
#define PRACTICESESSIONMANAGERBASE_H

// Qt
#include <QVector>

// kdelibs
#include <KRandomSequence>

// kdeedulibs
#include <keduvocexpression.h>

// Parley
#include "testentry.h"
#include "prefs.h"


class KEduVocDocument;
class PracticeDialog;

namespace Practice
{

class SessionManagerBase
{
public:
    /**
     * Create a collection of entries to be practiced.
     * @param doc
     */
    explicit SessionManagerBase(QWidget *parent);

    /**
     * destructor
     */
    virtual ~SessionManagerBase();

    /**
     * Prepare for practice using the entries in this document.
     *
     * The default implementation selects all available entries in the
     * document into the list allTestEntries and then calls
     * initializeTraining().
     */
    virtual void setDocument(KEduVocDocument *doc);

    /**
     * Initialize the lists of entries that will be used in the
     * training from the full set of available entries. Reimplement
     * this to create other types of training sessions.
     */
    virtual void initializeTraining() = 0;

    /**
     * Retun the title of the document.
     */
    QString title() const;


    // Should be called when starting and ending the practice session respectively.
    // The default implementations only start and stop the practice timer.
    virtual void practiceStarted();
    virtual void practiceFinished();

    /** the time in seconds */
    int totalTime();


    /**
     * Get the next entry to show to the user. The default
     * implementation refills the active entries up to the max number
     * and selects a random entry from them.
     *
     * @return TestEntry* the next entry to be practiced
     */
    virtual TestEntry* nextTrainingEntry();

    /** Finish the currently active entry */
    virtual void removeCurrentEntryFromPractice();


    /**
     * Get a list of all entries in the test - used by the summary dialog
     */
    QList<TestEntry*> allTestEntries() const;

    /**
     * The number of entries available for the practice session.
     * The actual session may be smaller.
     * @return
     */
    int allEntryCount() const;

    /**
     * The number of entries that are still to be practiced
     * @return
     */
    int activeEntryCount();

    /**
    * Get a list of all unanswered entries in the test
    */
    QList<TestEntry*> allUnansweredTestEntries();

    // ----------------------------------------------------------------
    // Statistics
    int statisticTotalCorrectFirstAttempt();
    int statisticTotalWrong();
    int statisticTotalUnanswered();

    /**
     * Puts some grades on the shell
     */
    void printStatistics();


    QStringList multipleChoiceAnswers(int numberChoices);

    //QString currentConjugationTense();

 protected:  // methods

    /**
     * Select appropriate entries for the practice (respect blocking settings etc)
     * m_allTestEntries will be filled by this.
     */
    void filterTestEntries();

    void setLanguages(int from, int to);

    /**
     * Find out if the given expression can be used as a multiple choice answer for the current entry
     * (i.e. if it's not the answer itself, not a synonym and has a different text)
     */
    bool isValidMultipleChoiceAnswer(KEduVocExpression *e);

protected:  // data

    KEduVocDocument *m_doc;
    QWidget *m_parent;
    int m_fromTranslation;
    int m_toTranslation;
    int m_testType;

    // ----------------------------------------------------------------
    // The following entries define the training

    /// All entries available in the document that fulfill the
    /// requirements set in the configuration and the grades in the
    /// entries themselves.
    QList<TestEntry*> m_allTestEntries;

    /// All entries that have not been entered into the active set.
    QList<TestEntry*> m_notAskedTestEntries;

    /// The list of entries that are being asked. If one of these is
    /// done, it can be deleted and an new one from
    /// m_notAskedTestEntries taken.
    QList<TestEntry*> m_currentEntries;

    // The index of the current entry in m_currentEntries.
    int m_currentEntry;

    QTime m_time;
    int m_totalTime;

    KRandomSequence m_randomSequence;

    friend class EntryFilter;
};

}

#endif // kvtquery_included
