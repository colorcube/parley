/***************************************************************************
    Copyright 2008 Frederik Gladhorn <frederik.gladhorn@kdemail.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "entryfilter.h"


#include <krandom.h>

#include <KDebug>
#include <KDialog>
#include <KMessageBox>

#include <keduvocdocument.h>
#include <keduvocwordtype.h>
#include <keduvocexpression.h>

#include "documentsettings.h"
#include "testentry.h"

using namespace Practice;

EntryFilter::EntryFilter(QObject * parent, KEduVocDocument* doc)
    :QObject(parent)
    ,m_doc(doc)
    ,m_dialog(0)
{
    m_fromTranslation = Prefs::questionLanguage();
    m_toTranslation = Prefs::solutionLanguage();
    kDebug() << "Filter for " << m_fromTranslation << " to " << m_toTranslation;

    if (Prefs::practiceMode() == Prefs::EnumPracticeMode::ConjugationPractice) {
        DocumentSettings documentSettings(m_doc->url().url() + QString::number(m_toTranslation));
        documentSettings.readConfig();
        m_tenses = documentSettings.conjugationTenses();
        kDebug() << "Tenses" << m_tenses;
    }

    expireEntries();
}

void EntryFilter::expireEntries()
{
    if ( Prefs::expire() ) {
        int counter = 0;
        foreach (KEduVocExpression* entry, m_entries) {
            int grade = entry->translation(m_toTranslation)->grade();

            const QDateTime &date =  entry->translation(m_toTranslation)->practiceDate();

            const QDateTime &expireDate = QDateTime::currentDateTime().addSecs( -Prefs::expireItem(grade) );

            if ( date < expireDate && grade > 0) {
                // decrease the grade
                entry->translation(m_toTranslation)->decGrade();

                // prevent from endless dropping
                entry->translation(m_toTranslation)->setPracticeDate( QDateTime::currentDateTime().addSecs( -Prefs::expireItem( grade - 2) ) );
                counter++;
            }
        }
        kDebug() << "Expired words dropped their grade: " << counter;
    }
}

QList<TestEntry*> EntryFilter::entries()
{
    // set up the lists/sets of filtered vocabulary
    m_entries = m_doc->lesson()->entries(KEduVocLesson::Recursive).toSet();
    cleanupInvalid();

    static QString noEntriesError =
            i18n("The vocabulary document contains no entries that can be used for the chosen type"
                    " of practice.");

    kDebug() << "Document contains " << m_entries.count() << " valid entries.";
    if (m_entries.count() == 0) {
        KMessageBox::error(0, noEntriesError);
        return QList<TestEntry*>();
    }

    // FIXME the filtering needs to be done for each word or the grammar modes get included with written or somesuch

    /* FIXME
    if (Prefs::genderPractice())
    {
        if (m_doc->identifier(m_toTranslation).article().isEmpty())
        {
            KMessageBox::error(0, i18n("The vocabulary document contains no articles for the current language. Please add some in the Edit->Grammar menu."));
            return QList<KEduVocExpression*>();
        }
    }
    */

    lessonEntries();
    wordTypeEntries();
    blockedEntries();
    timesWrongEntries();
    timesPracticedEntries();
    minMaxGradeEntries();

    updateTotal();

    bool ignoreBlocked = false;
    if (m_currentSelection.count() == 0) {
        kDebug() << "Creating practice filter dialog.";
        m_dialog = new KDialog;
        m_dialog->setCaption(i18n("Start Practice"));
        QWidget *widget = new QWidget(m_dialog);
        ui.setupUi(widget);
        m_dialog->setMainWidget(widget);
        m_dialog->setButtons( KDialog::Ok | KDialog::Cancel );

        ui.lessonLabel->setText(QString::number(m_entriesLesson.count()));
        ui.wordTypeLabel->setText(QString::number(m_entriesWordType.count()));
        ui.blockedLabel->setText(QString::number(m_entriesNotBlocked.count()));
        ui.timesWrongLabel->setText(QString::number(m_entriesTimesWrong.count()));
        ui.timesPracticedLabel->setText(QString::number(m_entriesTimesPracticed.count()));
        ui.minMaxGradeLabel->setText(QString::number(m_entriesMinMaxGrade.count()));

        ui.documentTotalLabel->setText(QString::number(m_entries.count()));
        updateTotal();

        ui.lessonCheckBox->setChecked(!m_entriesLesson.count());
        ui.wordTypeCheckBox->setChecked(!m_entriesWordType.count());
        ui.blockedCheckBox->setChecked(!m_entriesNotBlocked.count());
        ui.timesWrongCheckBox->setChecked(!m_entriesTimesWrong.count());
        ui.timesPracticedCheckBox->setChecked(!m_entriesTimesPracticed.count());
        ui.minMaxGradeCheckBox->setChecked(!m_entriesMinMaxGrade.count());

        connect( ui.lessonCheckBox, SIGNAL(toggled(bool)), this, SLOT(checkBoxChanged(bool)));
        connect( ui.wordTypeCheckBox, SIGNAL(toggled(bool)), this, SLOT(checkBoxChanged(bool)));
        connect( ui.blockedCheckBox, SIGNAL(toggled(bool)), this, SLOT(checkBoxChanged(bool)));
        connect( ui.timesWrongCheckBox, SIGNAL(toggled(bool)), this, SLOT(checkBoxChanged(bool)));
        connect( ui.timesPracticedCheckBox, SIGNAL(toggled(bool)), this, SLOT(checkBoxChanged(bool)));
        connect( ui.minMaxGradeCheckBox, SIGNAL(toggled(bool)), this, SLOT(checkBoxChanged(bool)));

        updateTotal();

        if (!Prefs::wordTypesInPracticeEnabled()) {
            ui.wordTypeLabel->setVisible(false);
            ui.wordTypeCheckBox->setVisible(false);
        }

        if (m_dialog->exec() == QDialog::Rejected) {
            delete m_dialog;
            return QList<TestEntry*>();
        }
        ignoreBlocked = ui.blockedCheckBox->isChecked();
        delete m_dialog;
    }

    if (Prefs::practiceMode() == Prefs::EnumPracticeMode::ConjugationPractice) {
        QList< TestEntry* > ret = conjugationTestEntries(ignoreBlocked);
        if (ret.count() == 0) {
            KMessageBox::error(0, noEntriesError);
        }
        return ret;
    } else {
        QList<TestEntry*> testEntries;
        foreach (KEduVocExpression* entry, m_currentSelection){
            randomizedInsert(testEntries, new TestEntry(entry));
        }
        return testEntries;
    }
}

void EntryFilter::checkBoxChanged(bool filter)
{
    Q_UNUSED(filter)
    updateTotal();
}

void EntryFilter::updateTotal()
{
    QSet< KEduVocExpression * > selected = m_entries;
    if (!m_dialog || !ui.lessonCheckBox->isChecked()) {
        selected = selected.intersect(m_entriesLesson);
    }
    if (!m_dialog || !ui.wordTypeCheckBox->isChecked()) {
        selected = selected.intersect(m_entriesWordType);
    }
    if (!m_dialog || !ui.blockedCheckBox->isChecked()) {
        selected = selected.intersect(m_entriesNotBlocked);
    }
    if (!m_dialog || !ui.timesWrongCheckBox->isChecked()) {
        selected = selected.intersect(m_entriesTimesWrong);
    }
    if (!m_dialog || !ui.timesPracticedCheckBox->isChecked()) {
        selected = selected.intersect(m_entriesTimesPracticed);
    }
    if (!m_dialog || !ui.minMaxGradeCheckBox->isChecked()) {
        selected = selected.intersect(m_entriesMinMaxGrade);
    }

    if (m_dialog) {
        ui.totalLabel->setText(QString::number(selected.count()));
        m_dialog->enableButtonOk(selected.count() > 0);
    }

    m_currentSelection = selected;
}

void EntryFilter::lessonEntries()
{
    foreach(KEduVocExpression* entry, m_entries) {
        if (entry->lesson()->inPractice()) {
            m_entriesLesson.insert(entry);
        }
    }
}

void EntryFilter::wordTypeEntries()
{
    if (Prefs::wordTypesInPracticeEnabled()) {
        foreach(KEduVocExpression* entry, m_entries) {
            if (entry->translation(m_toTranslation)->wordType()) {
                if(entry->translation(m_toTranslation)->wordType()->inPractice()) {
                    m_entriesWordType.insert(entry);
                }
            }
        }
    } else {
        m_entriesWordType = m_entries;
    }
}

void EntryFilter::blockedEntries()
{
    if (!Prefs::block()) {
        m_entriesNotBlocked = m_entries;
        return;
    }

    switch (Prefs::practiceMode()) {
    case Prefs::EnumPracticeMode::ConjugationPractice:
        foreach(KEduVocExpression* entry, m_entries) {
            if (!isConjugationBlocked(entry->translation(m_toTranslation))) {
                m_entriesNotBlocked.insert(entry);
            }
        }
        break;
    case Prefs::EnumPracticeMode::GenderPractice:
        foreach(KEduVocExpression* entry, m_entries) {
            KEduVocText article = entry->translation(m_toTranslation)->article();
            if (!isBlocked(&article)) {
                m_entriesNotBlocked.insert(entry);
            }
        }
        break;
    case Prefs::EnumPracticeMode::ComparisonPractice:
        foreach(KEduVocExpression* entry, m_entries) {
            KEduVocTranslation* translation = entry->translation(m_toTranslation);
            KEduVocText comparative = translation->comparativeForm();
            KEduVocText superlative = translation->superlativeForm();
            if (!isBlocked(&(comparative))
                || !isBlocked(&(superlative)) )
            {
                m_entriesNotBlocked.insert(entry);
            }
        }
        break;
    default:
        foreach(KEduVocExpression* entry, m_entries) {
            if (!isBlocked(entry->translation(m_toTranslation))) {
                m_entriesNotBlocked.insert(entry);
            }
        }
        break;
    }
}

bool EntryFilter::isConjugationBlocked(KEduVocTranslation* translation) const
{
    foreach(const QString& tense, translation->conjugationTenses()) {
        if(m_tenses.contains(tense)) {
            QList<KEduVocWordFlags> pronouns = translation->conjugation(tense).keys();
            foreach(const KEduVocWordFlags& pronoun, pronouns) {
                KEduVocText grade = translation->conjugation(tense).conjugation(pronoun);
                if (!isBlocked(&(grade))) {
                    // just need to find any form that is not blocked for generating test entries
                    // exact filtering is done later in conjugationTestEntries
                    return false;
                }
            }
        }
    }
    return true;
}

bool EntryFilter::isBlocked(const KEduVocText* const text) const
{
    grade_t grade = text->grade();
    // always include lowest level and where blocking is off
    if (grade == KV_NORM_GRADE || Prefs::blockItem(grade) == 0) {
        return false;
    } else {
        QDateTime date = text->practiceDate();
        if (date.addSecs(Prefs::blockItem(grade)) < QDateTime::currentDateTime()) {
            return false;
        }
    }

    return true;
}

void EntryFilter::timesWrongEntries()
{
    foreach(KEduVocExpression* entry, m_entries) {
        if (entry->translation(m_toTranslation)->badCount() >= Prefs::practiceMinimumWrongCount() && entry->translation(m_toTranslation)->badCount() <= Prefs::practiceMaximumWrongCount()) {
            m_entriesTimesWrong.insert(entry);
        }
    }
}

void EntryFilter::timesPracticedEntries()
{
    foreach(KEduVocExpression* entry, m_entries) {
        if (entry->translation(m_toTranslation)->practiceCount() >= Prefs::practiceMinimumTimesAsked() && entry->translation(m_toTranslation)->practiceCount() <= Prefs::practiceMaximumTimesAsked()) {
            m_entriesTimesPracticed.insert(entry);
        }
    }
}

void EntryFilter::minMaxGradeEntries()
{
    foreach(KEduVocExpression* entry, m_entries) {
        int grade =
                entry->translation(m_toTranslation)->grade();
        if (grade >= Prefs::practiceMinimumGrade() && grade <= Prefs::practiceMaximumGrade()) {
            m_entriesMinMaxGrade.insert(entry);
        }
    }
}
 /*
    if (m_testType == Prefs::EnumTestType::ArticleTest) {
    KMessageBox::information(0,
    i18n("You selected to practice the genders of nouns, but no appropriate nouns could be found. Use \"Edit Entry\" and select Noun as word type and the gender."),
    i18n("No valid word type found"));
    return;
}
    if (m_testType == Prefs::EnumTestType::ComparisonTest) {
    KMessageBox::information(0,
    i18n("You selected to practice comparison forms, but no adjectives or adverbs containing comparison forms could be found. Use \"Edit Entry\" and select Adverb or Adjective as word type and enter the comparison forms."),
    i18n("No valid word type found"));
    return;
}
    if (m_testType == Prefs::EnumTestType::ConjugationTest) {
    KMessageBox::information(0, i18n("You selected to practice conjugations, but no vocabulary containing conjugations in the tenses you selected could be found. Use \"Edit Entry\" and select Verb as word type and enter the conjugation forms."), i18n("No valid word type found"));
    return;
}
}

    if ( removeTestEntryList.count() == m_entries.count() ) {
    if ( KMessageBox::questionYesNo(0, i18n("<p>The lessons you selected for the practice contain no entries when the threshold settings are respected.</p><p>Hint: To configure the thresholds use the \"Threshold Page\" in the \"Configure Practice\" dialog.</p><p>Would you like to ignore the threshold setting?</p>"), i18n("No Entries with Current Threshold Settings") ) == KMessageBox::No ) {
    return;
}

 */

void EntryFilter::cleanupInvalid()
{
    Prefs::EnumPracticeMode::type mode = Prefs::practiceMode();
    bool wordTypeNeeded = (mode == Prefs::EnumPracticeMode::GenderPractice) ||
        (mode == Prefs::EnumPracticeMode::ComparisonPractice) ||
        (mode == Prefs::EnumPracticeMode::ConjugationPractice);

    QSet<KEduVocExpression*>::iterator i = m_entries.begin();
    while (i != m_entries.end()) {
        // remove empty entries
        if ((*i)->translation(m_toTranslation)->text().isEmpty()
              || (*i)->translation(m_fromTranslation)->text().isEmpty()) {
            i = m_entries.erase(i);
            continue;
        }
               
        // for grammar stuff we need the word to have its word type set, else continue
        if (wordTypeNeeded && !(*i)->translation(m_toTranslation)->wordType()) {
            i = m_entries.erase(i);
            continue;
        }
        
        // Grammar modes need different things:
        switch (Prefs::practiceMode()) {     
        
        // example sentences: need the example sentence to exist
        case Prefs::EnumPracticeMode::ExampleSentencesPractice:
            if ((*i)->translation(m_toTranslation)->example().simplified().isEmpty()){
                i = m_entries.erase(i);
                continue;
            }
            break;
            
        case Prefs::EnumPracticeMode::GenderPractice:
            if (!((*i)->translation(m_toTranslation)->wordType()->wordType() & KEduVocWordFlag::Noun)) {
                i = m_entries.erase(i);
                continue;
            }
            break;

        case Prefs::EnumPracticeMode::ComparisonPractice:
            if (
                // only adjective/adverb
                (((*i)->translation(m_toTranslation)->wordType()->wordType() != KEduVocWordFlag::Adjective)
                    && ((*i)->translation(m_toTranslation)->wordType()->wordType() != KEduVocWordFlag::Adverb))
                // at least one comparison forms is there
                || ((*i)->translation(m_toTranslation)->comparative().isEmpty() || (*i)->translation(m_toTranslation)->superlative().isEmpty())) {
                i = m_entries.erase(i);
                continue;
            }
            break;

        case Prefs::EnumPracticeMode::ConjugationPractice: {
            KEduVocTranslation* translation = (*i)->translation(m_toTranslation);
            bool erase = false;

            // Remove entries which are not verbs
            if (translation->wordType()->wordType() != KEduVocWordFlag::Verb) {
                erase = true;
            }

            // Remove entries which don't have any of the tenses which are configured for practice
            QSet<QString> practice_tenses = QSet<QString>::fromList(m_tenses);
            QSet<QString> existing_tenses;
            foreach(const QString& tense, translation->conjugationTenses()) {
                if (!translation->conjugation(tense).isEmpty()) {
                    existing_tenses << tense;
                }
            }
            if (existing_tenses.intersect(practice_tenses).isEmpty()) erase = true;

            if (erase) {
                i = m_entries.erase(i);
                continue;
            }
            break;
        }
        default:
            break;
        } // switch
        i++;
    } // for
    kDebug() << "Invalid items removed. Remaining: " << m_entries.count();
}

QList< TestEntry* > EntryFilter::conjugationTestEntries(bool ignoreBlocked) const
{
    kDebug() << "Filtering conjugation entries for tenses... " << m_tenses;

    // TODO CM make this configurable
    enum MODE
    {
        M_SEPARATE,
        M_COMPLETE
    };
    MODE mode = M_SEPARATE;

    QList<TestEntry*> testEntries;
    foreach (KEduVocExpression* entry, m_currentSelection) {
        foreach(const QString& tense, entry->translation(m_toTranslation)->conjugationTenses()) {

            // Only include tenses which are both non-empty and which should be practiced
            if (!m_tenses.contains(tense)) {
                continue;
            }
            KEduVocConjugation& conjugation = entry->translation(m_toTranslation)->conjugation(tense);
            if (conjugation.isEmpty()) {
                continue;
            }

            bool blocked = true;
            QList<KEduVocWordFlags> pronouns = conjugation.keys();
            foreach(const KEduVocWordFlags& pronoun, pronouns) {
                KEduVocText* grade = &conjugation.conjugation(pronoun);
                if (ignoreBlocked || !isBlocked(grade)) {
                    blocked = false;

                    if (mode == M_SEPARATE)
                    {
                        TestEntry* testEntry = new TestEntry(entry);
                        testEntry->setConjugationTense(tense);
                        QList<KEduVocWordFlags> list;
                        list << pronoun;
                        testEntry->setConjugationPronouns(list);
                        randomizedInsert(testEntries, testEntry);
                    }
                }
            }

            if (!blocked && mode == M_COMPLETE) {
                TestEntry* testEntry = new TestEntry(entry);
                testEntry->setConjugationTense(tense);
                testEntry->setConjugationPronouns(pronouns);
                randomizedInsert(testEntries, testEntry);
            }
        }
    }
    return testEntries;
}


void EntryFilter::randomizedInsert(QList<TestEntry*>& list, TestEntry* entry)
{
    list.insert(KRandom::random() % (list.size()+1), entry);
}

#include "entryfilter.moc"
