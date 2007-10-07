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

#include "languagedialog.h"

#include "editlanguagedialogpage.h"
#include "languagesettings.h"

#include <keduvocdocument.h>

#include <KMessageBox>
#include <QLabel>
#include <KLocale>


/**
 *
 * @param parent
 */
LanguageDialog::LanguageDialog(KEduVocDocument* doc, QWidget * parent)
    : KPageDialog(parent)
{
    m_doc = doc;

    setCaption(i18n("Edit Languages"));
    setFaceType( List );
    setButtons(User1|User2|Ok|Cancel);

    setButtonText(User2, i18n("Add language"));
//     setButtonIcon(User2, KIcon("insert_table_col"));
    setButtonText(User1, i18n("Remove language"));
//     setButtonIcon(User1, KIcon("delete_table_col"));

    connect( this, SIGNAL(user2Clicked()), this, SLOT(slotAppendIdentifier()));
    connect( this, SIGNAL(user1Clicked()), this, SLOT(slotDeleteIdentifier()));

    for ( int i = 0; i < m_doc->identifierCount(); i++ ) {
        createPage(i);
    }
}

LanguageDialog::~LanguageDialog()
{}

void LanguageDialog::accept()
{
    emit signalCommitData();
//     emit deletePages();

    qSort(m_deleteList.begin(), m_deleteList.end());
    for ( int identifierIndex = m_deleteList.count() - 1; identifierIndex >= 0; identifierIndex-- ) {
        kDebug() << "Delete Language: " << m_deleteList.value(identifierIndex);
        if ( KMessageBox::warningYesNo(this, i18n("Really delete language: %1?", m_doc->identifier(identifierIndex).name()), i18n("Remove Language")) == KMessageBox::Yes ) {
            m_doc->removeIdentifier(m_deleteList.value(identifierIndex));
        }
    }
    m_doc->setModified();
    KDialog::accept();
}

void LanguageDialog::slotAppendIdentifier()
{
    kDebug() << "Append identifier";

    // we cheat here. we directly append the identifier and remove it again, if cancel is selected.
    int i = m_doc->appendIdentifier();

    KPageWidgetItem* newPage = createPage( i );
    setCurrentPage( newPage );

    m_appendList.append(i);
}

void LanguageDialog::slotDeleteIdentifier()
{
    int index = m_pages.indexOf(currentPage());
    m_deleteList.append(index);
    removePage(currentPage());
}

void LanguageDialog::pageIconChanged(const QString & newIcon)
{
    currentPage()->setIcon( KIcon(newIcon) );
}

void LanguageDialog::pageNameChanged(const QString & newName)
{
    currentPage()->setName( newName );
}

KPageWidgetItem*  LanguageDialog::createPage(int i)
{
    EditLanguageDialogPage* editPageWidget = new EditLanguageDialogPage(m_doc, i, this);

    KPageWidgetItem* editPage = new KPageWidgetItem( editPageWidget,  m_doc->identifier(i).name() );
    editPage->setHeader( i18nc("Edit language properties", "Properties for %1", m_doc->identifier(i).name() ) );

    m_pages.append(editPage);

    // icons
    LanguageSettings currentSettings(m_doc->identifier(i).locale());
    currentSettings.readConfig();
    QString currentIcon = currentSettings.icon();
    editPage->setIcon( KIcon( currentIcon ) );

    addPage( editPage );

    connect(this, SIGNAL(signalCommitData()), editPageWidget, SLOT(commitData()));

    connect(editPageWidget->identifierNameLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(pageNameChanged(const QString&)));

    connect(editPageWidget, SIGNAL(iconSelected(const QString&)), this, SLOT(pageIconChanged(const QString&)));

    return editPage;
}

void LanguageDialog::reject()
{
    foreach(int identifierIndex, m_appendList) {
        m_doc->removeIdentifier(identifierIndex);
    }
    KDialog::reject();
}


#include "languagedialog.moc"
