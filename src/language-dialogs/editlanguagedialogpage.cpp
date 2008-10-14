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

#include "editlanguagedialogpage.h"

#include "languagesettings.h"

#include <keduvocdocument.h>

#include <KStandardDirs>
#include <KLocale>
#include <QLabel>
#include <QtDBus>

/**
 *
 * @param parent
 */
EditLanguageDialogPage::EditLanguageDialogPage(KEduVocDocument* doc, int identifierIndex, QWidget * parent)
    : QWidget(parent)
{
    m_doc = doc;
    m_identifierIndex = identifierIndex;
    setupUi(this);

    initialize();

    identifierNameLineEdit->setText(m_doc->identifier(m_identifierIndex).name());

    connect(localeComboBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(localeChanged(const QString&)));

    connect(iconComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(iconChanged(int)));

    connect(identifierNameLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(languageNameChanged(const QString&)));
}


EditLanguageDialogPage::~EditLanguageDialogPage()
{
}


void EditLanguageDialogPage::initialize()
{
    // language codes
    QStringList codes = KGlobal::locale()->allLanguagesList();

    // qmap automatically sorts by keys
    QMap<QString, QString> languageCodeMap;
    foreach (const QString &code, codes){
        languageCodeMap[KGlobal::locale()->languageCodeToName(code)] = code;
    }
    // add the language, but also it's code as data
    foreach ( const QString &language, languageCodeMap.keys() ) {
        localeComboBox->addItem( language, languageCodeMap.value(language) );
    }

    localeComboBox->setCurrentIndex(localeComboBox->findData(
        m_doc->identifier(m_identifierIndex).locale()));

    // icons
    LanguageSettings currentSettings(m_doc->identifier(m_identifierIndex).locale());
    currentSettings.readConfig();
    QString currentIcon = currentSettings.icon();

    iconComboBox->addItem(i18n("No icon"));

    QStringList countrylist = KGlobal::locale()->allCountriesList();
    countrylist.sort();

    foreach(const QString &code, countrylist) {
        QString country = KGlobal::dirs()->findResource("locale",
                QString("l10n/%1/entry.desktop").arg(code));
        KConfig entry(country, KConfig::SimpleConfig);
        KConfigGroup group = entry.group("KCM Locale");
        QString name = group.readEntry("Name", i18n("without name"));

        QString pixmap = country;
        pixmap.truncate(pixmap.lastIndexOf('/'));
        pixmap += "/flag.png";

        iconComboBox->addItem(QIcon(pixmap), name, pixmap);
    }

    iconComboBox->setCurrentIndex(iconComboBox->findData(currentIcon));

    // keyboard layout
        // try to talk to kxbk - get a list of keyboard layouts

    QDBusInterface kxbk("org.kde.kxkb", "/kxkb", "org.kde.KXKB");
    QDBusReply<QStringList> reply = kxbk.call("getLayoutsList");
    if (reply.isValid()) {
        QStringList layouts = reply;
        layouts.prepend(QString());
        keyboardLayoutComboBox->clear();
        keyboardLayoutComboBox->addItems(layouts);
        keyboardLayoutComboBox->setEnabled(true);

        LanguageSettings settings(m_doc->identifier(m_identifierIndex).locale());
        settings.readConfig();

        if (!settings.keyboardLayout().isEmpty()) {
            keyboardLayoutComboBox->setCurrentIndex(keyboardLayoutComboBox->findText(settings.keyboardLayout()));
        } else {
            QDBusReply<QString> currentLayout = kxbk.call("getCurrentLayout");
            keyboardLayoutComboBox->setCurrentIndex(keyboardLayoutComboBox->findText(currentLayout));
        }
    } else {
        kDebug() << "kxkb dbus error";
        keyboardLayoutComboBox->setEnabled(false);
    }
}


void EditLanguageDialogPage::commitData()
{
    QString locale = localeComboBox->itemData(localeComboBox->currentIndex()).toString();

    // if no locales are found (kdebase not installed?) at least fall back to using the language name as locale.
    if (locale.isEmpty()) {
        locale = identifierNameLineEdit->text();
    }
    m_doc->identifier(m_identifierIndex).setLocale( locale );
    m_doc->identifier(m_identifierIndex).setName( identifierNameLineEdit->text() );

    QString icon = iconComboBox->itemData(iconComboBox->currentIndex()).toString();

    LanguageSettings settings(locale);
    settings.setIcon(icon);
    if ( keyboardLayoutComboBox->isEnabled() ) {
        settings.setKeyboardLayout( keyboardLayoutComboBox->currentText() );
    }
    settings.writeConfig();
}

void EditLanguageDialogPage::iconChanged(int iconIndex)
{
    emit iconSelected( iconComboBox->itemData(iconIndex).toString() );
}

void EditLanguageDialogPage::localeChanged(const QString & locale)
{
    identifierNameLineEdit->setText( locale );
}

void EditLanguageDialogPage::languageNameChanged(const QString &)
{
}


#include "editlanguagedialogpage.moc"