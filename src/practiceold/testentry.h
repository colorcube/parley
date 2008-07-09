/***************************************************************************
    Copyright 2007 Frederik Gladhorn <frederik.gladhorn@kdemail.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef TESTENTRY_H
#define TESTENTRY_H

#include <QFlags>

class KEduVocExpression;

class TestEntry
{
public:
    enum ErrorType {
        SpellingMistake       =   0x1, /** < misspelled */
        CapitalizationMistake =   0x2, /** < capitalization error (whAt) */
        AccentMistake         =   0x4, /** < an accent is missing or wrong (é) */
        ArticleWrong          =   0x8, /** < solution is correct with the article interchanged */
        ArticleMissing        =   0x10, /** < solution is correct with the article missing*/
        FalseFriend           =  0x20, /** < a false friend */
        Synonym               =  0x40, /** < a synonym (may be correct) */
        Empty                 =  0x80, /** < empty answer string */
        UnrelatedWord         = 0x100, /** < a valid word but no connection to the solution */
        Incomplete            = 0x200, /** < the part that was entered is right, but not complete */
        Correct               = 0x400, /** < no error, solution was right */
        UnknownMistake        = 0x800  /** < no idea */
    };

    Q_DECLARE_FLAGS(ErrorTypes, ErrorType)


    TestEntry(KEduVocExpression *entry)
    {
        m_entry = entry;
        m_answeredCorrectInSequence = 0;
        m_statisticCount = 0;
        m_statisticGoodCount = 0;
        m_statisticBadCount = 0;
        m_statisticTimeout = 0;
        m_statisticSkipUnknown = 0;
        m_statisticSkipKnown = 0;
        m_canSwitchDirection = false;
        m_correctAtFirstAttempt = false;
        m_lastError = UnknownMistake;
    }

    void incGoodCount();
    void incBadCount();
    void incTimeout();
    void incSkipUnknown();
    void incSkipKnown();

    int answeredCorrectInSequence();
    int statisticCount();
    int statisticGoodCount();
    int statisticBadCount();
    int statisticSkipKnown();
    int statisticSkipUnknown();
    int statisticTimeout();
    bool statisticCorrectAtFirstAttempt();

    void setLastErrors(ErrorTypes errorTypes);
    ErrorTypes lastErrors();

    void setLastPercentage(double percent);
    double lastPercentage();

    int totalEntries();


    static void setGradeFrom(int from);
    static void setGradeTo(int to);
    static int gradeFrom();
    static int gradeTo();

    KEduVocExpression *entry();

private:
    void update();

    // these are for the CURRENT query only, so we can display statistics.
    int m_statisticCount;
    int m_statisticGoodCount;
    int m_statisticBadCount;
    int m_statisticTimeout;
    int m_statisticSkipUnknown;
    int m_statisticSkipKnown;
    int m_answeredCorrectInSequence;
    bool m_canSwitchDirection;
    bool m_correctAtFirstAttempt;

    double m_lastPercentage;
    ErrorTypes m_lastError;

//     QStringList m_correctParts;
//     QStringList m_wrongParts;

    static int m_gradeTo;
    static int m_gradeFrom;

    /// the entry itself
    KEduVocExpression *m_entry;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(TestEntry::ErrorTypes)

#endif