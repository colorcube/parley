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


#ifndef GENDERBACKENDMODE_H
#define GENDERBACKENDMODE_H

#include "multiplechoicebackendmode.h"
#include "practice/sessionmanagerbase.h"

namespace Practice
{

class GenderBackendMode : public MultipleChoiceBackendMode
{
    Q_OBJECT

public:
    GenderBackendMode(AbstractFrontend *frontend,
                      QObject *parent, Practice::SessionManagerBase* sessionManager,
                      KEduVocDocument* doc);

    /** Start practicing a new word */
    bool setTestEntry(TestEntry* current) Q_DECL_OVERRIDE;

    grade_t currentPreGradeForEntry() const Q_DECL_OVERRIDE;
    grade_t currentGradeForEntry() const Q_DECL_OVERRIDE;

protected:
    void prepareChoices(TestEntry* current) Q_DECL_OVERRIDE;
    void updateGrades() Q_DECL_OVERRIDE;

private:
    /** Reference to the kvocdoc */
    const KEduVocDocument & m_doc;
    KEduVocArticle m_articles;
    QString m_masculine;
    QString m_feminine;
    QString m_neuter;
    bool m_neuterExists;
};

}

#endif
