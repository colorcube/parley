/***************************************************************************
    Copyright 2008 Frederik Gladhorn <frederik.gladhorn@kdemail.net>
    Copyright 2011 Jan Gerrit Marker <jangerrit@weiler-marker.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <keduvocdocument.h>
#include <keduvocexpression.h>
#include <keduvocwordtype.h>
#include <keduvocdeclension.h>

#include "declensionwidget.h"
#include "conjugationwidget.h"

#include "inflectionwidget.h"

using namespace Editor;

InflectionWidget::InflectionWidget(QWidget* parent): QStackedWidget(parent)
{
    m_wordTypeWidget = new QWidget(this);
    QVBoxLayout *wordTypeLayout = new QVBoxLayout();

    QLabel *label = new QLabel();
    wordTypeLayout->addWidget(label);

    QPushButton *toVerb = new QPushButton(i18n("Verb"));
    toVerb->setObjectName("toVerb");
    connect(toVerb, SIGNAL(pressed()), this, SLOT(setWordType()));
    wordTypeLayout->addWidget(toVerb, 1);

    QPushButton *toNoun = new QPushButton(i18n("Noun"));
    toNoun->setObjectName("toNoun");
    connect(toNoun, SIGNAL(pressed()), this, SLOT(setWordType()));
    wordTypeLayout->addWidget(toNoun, 1);

    QPushButton *toAdjective = new QPushButton(i18n("Adjective"));
    toAdjective->setObjectName("toAdjective");
    connect(toAdjective, SIGNAL(pressed()), this, SLOT(setWordType()));
    wordTypeLayout->addWidget(toAdjective, 1);

    wordTypeLayout->addStretch();

    m_wordTypeWidget->setDisabled(true);
    m_wordTypeWidget->setLayout(wordTypeLayout);

    m_conjugationWidget = new ConjugationWidget(this);
    m_declensionWidget = new DeclensionWidget(this);

    addWidget(m_wordTypeWidget);
    addWidget(m_conjugationWidget);
    addWidget(m_declensionWidget);
}

void InflectionWidget::setDocument(KEduVocDocument* doc)
{
    m_doc = doc;
    m_conjugationWidget->setDocument(doc);
    m_declensionWidget->setDocument(doc);
}

void InflectionWidget::setTranslation(KEduVocExpression* entry, int translation)
{
    m_entry = entry;
    m_translation = translation;

    if (!entry || !entry->translation(translation)) {
        m_wordTypeWidget->setDisabled(true);
        setCurrentWidget(m_wordTypeWidget);
        return;
    }
    if (!entry->translation(translation)->wordType() ||
        !(entry->translation(translation)->wordType()->wordType() & KEduVocWordFlag::Verb
         || entry->translation(translation)->wordType()->wordType() & KEduVocWordFlag::Noun
         || entry->translation(translation)->wordType()->wordType() & KEduVocWordFlag::Adjective
        )
       ) {
        qobject_cast<QLabel*>(m_wordTypeWidget->layout()->itemAt(0)->widget())->setText(i18n("\"%1\" is a:", entry->translation(translation)->text()));
        m_wordTypeWidget->setEnabled(true);
        setCurrentWidget(m_wordTypeWidget);
        return;
    }

    m_conjugationWidget->setTranslation(entry, translation);
    m_declensionWidget->setTranslation(entry, translation);

    if (entry->translation(translation)->wordType()->wordType() & KEduVocWordFlag::Verb) {
        setCurrentWidget(m_conjugationWidget);
    }
    else {
        setCurrentWidget(m_declensionWidget);
    }
}

void InflectionWidget::setWordType()
{
    if (!sender()) {
        return;
    }

    KEduVocWordType* container = 0;

    if (sender()->objectName() == "toVerb") {
        container = m_doc->wordTypeContainer()->childOfType(KEduVocWordFlag::Verb);
    }
    else if (sender()->objectName() == "toNoun") {
        KEduVocWordFlags type = KEduVocWordFlag::Noun;

        QPointer<KDialog> getGenderDialog = new KDialog(this);
        getGenderDialog->setCaption(i18n("Please select the noun's gender"));
        getGenderDialog->setButtons(KDialog::Ok | KDialog::Cancel);

        KComboBox *genderComboBox = new KComboBox;
        genderComboBox->addItem(i18n("Masculine"), KEduVocWordFlag::Masculine);
        genderComboBox->addItem(i18n("Neuter"), KEduVocWordFlag::Neuter);
        genderComboBox->addItem(i18n("Feminine"), KEduVocWordFlag::Feminine);

        getGenderDialog->setMainWidget(genderComboBox);

        if (getGenderDialog->exec() == KDialog::Accepted)
        {
            type = (KEduVocWordFlags)(type|genderComboBox->itemData(genderComboBox->currentIndex()).toInt());
        }

        delete getGenderDialog;

        container = m_doc->wordTypeContainer()->childOfType(type);
    }
    else if (sender()->objectName() == "toAdjective") {
        container = m_doc->wordTypeContainer()->childOfType(KEduVocWordFlag::Adjective);
    }

    if (container) {
        m_entry->translation(m_translation)->setWordType(container);
        setTranslation(m_entry, m_translation);
    }
}