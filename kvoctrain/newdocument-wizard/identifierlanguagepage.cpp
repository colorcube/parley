/***************************************************************************

                             kvtnewdocumentwizardlanguagepage

    -----------------------------------------------------------------------

    copyright     : (C) 2007 Frederik Gladhorn <frederik.gladhorn@kdemail.net>

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

#include "identifierlanguagepage.h"

#include <QWizardPage>
#include <QLabel>
#include <QLineEdit>
#include <QTreeView>
#include <QCheckBox>
#include <QVBoxLayout>

#include <KComboBox>
#include <KLocale>


#include "kvtnewdocumentwizard.h"

// IdentifierLanguagePage::IdentifierLanguagePage(bool isFirstLanguagePage, QWizard *parent)
//  : QWizardPage(parent)
// {
//     m_languageLabel = new QLineEdit;
//     m_languageShortLabel = new QLineEdit;
//
//     m_isFirstLanguagePage = isFirstLanguagePage;
//
//     QVBoxLayout *layout = new QVBoxLayout;
//     if ( isFirstLanguagePage ) {
//         setTitle(i18n("First Language"));
//         //setSubTitle(i18n("Now select the first language (e.g. the one you know already)."));
//
//         registerField("firstLanguagePage.language", m_languageLabel);
//         registerField("firstLanguagePage.languageShort", m_languageShortLabel);
//     } else {
//         setTitle(i18n("Second Language"));
//         //setSubTitle(i18n("Now select the language that you want to learn."));
//         registerField("secondLanguagePage.language", m_languageLabel);
//         registerField("secondLanguagePage.languageShort", m_languageShortLabel);
//         m_beforeSelectionLabel = new QLabel;
//         layout->addWidget(m_beforeSelectionLabel);
//     }
//
//     KVTLanguageView *languageView = new KVTLanguageView(this);
//     connect(languageView, SIGNAL(signalLanguageChanged(const QString&, const QString&)), this, SLOT(slotLanguageChanged(const QString&, const QString&)));
//     connect(languageView, SIGNAL(signalActivated()), parent, SLOT(next()));
//     layout->addWidget(languageView);
//
//     m_currentLabel = new QLabel(i18n("Your selection:"));
//     layout->addWidget(m_currentLabel);
//
//     /*
//     if ( !isFirstLanguagePage ) {*/
//         /* //More confusing than helping:
//         QLabel *labelHint = new QLabel(i18n("Please note that you can "
//             "add more languages later.\n"
//             "Use Vocabulary->AppendLanguage to do that."));
//         labelHint->setWordWrap(true);
//         layout->addWidget(labelHint);*/
//         /*
//         m_moreLanguagesCheckbox=new QCheckBox(i18n("I want to add yet another language"));
//         layout->addWidget(m_moreLanguagesCheckbox);
//
//     }*/
//
//     //layout->addWidget(m_languageLabel);
//     //layout->addWidget(m_languageShortLabel);
//     setLayout(layout);
// }

/*
void IdentifierLanguagePage::initializePage()
{
kDebug() << "initializePage" << field("firstLanguagePage.language").toString();
    if (!m_isFirstLanguagePage) {
        m_beforeSelectionLabel->setText(
            i18n("So far you have chosen ") +
            field("firstLanguagePage.language").toString() +
            i18n("\nPlease add a second language:")
            );
    }
}


void IdentifierLanguagePage::slotLanguageChanged(const QString &language, const QString &languageShort)
{
    m_languageLabel->setText(language);
    m_languageShortLabel->setText(languageShort);
    m_currentLabel->setText(i18n("Your selection: ") + language);
    kDebug() << "slotLanguageChanged: " << language << " = " << languageShort;
}*/

/**
 *
 * @param parent
 */
IdentifierLanguagePage::IdentifierLanguagePage(QWizard * parent)
    : QWizardPage(parent)
{


    setTitle(i18n("Identifier and language selection"));

    setupUi(this);

    QStringList codes = KGlobal::locale()->allLanguagesList();
    QStringList languageNames;
    foreach (QString code, codes){
        languageNames.append( KGlobal::locale()->languageCodeToName(code) );
    }


    languageComboBox->addItems(languageNames);

}

IdentifierLanguagePage::~ IdentifierLanguagePage()
{
//     delete ui;
}


#include "identifierlanguagepage.moc"
