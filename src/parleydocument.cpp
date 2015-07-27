/***************************************************************************

    Copyright 2007-2008 Frederik Gladhorn <frederik.gladhorn@kdemail.net>

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "parleydocument.h"

#include "../config-parley.h"
#include "parleymainwindow.h"
#include "editor/editor.h"
#include "version.h"
#include "prefs.h"
#include "languagesettings.h"

#include "dashboard/dashboard.h"

#include <keduvoclesson.h>
#include <keduvocleitnerbox.h>
#include <keduvocexpression.h>
#include <keduvocwordtype.h>
#include <keduvocvocabularyview.h>
#include <keduvocdocumentproperties.h>
#include <keduvoclanguageproperties.h>

#include <QFileDialog>
#include <QMimeDatabase>
#include <KRecentFilesAction>
#include <QStandardPaths>
#include <kns3/downloaddialog.h>
#include <kns3/uploaddialog.h>
#include <KEMailSettings>
#include <KMessageBox>
#include <KProcess>
#include <QTemporaryDir>

#include <QDialog>
#include <QDialogButtonBox>
#include <QTimer>
#include <QCheckBox>

#ifdef HAVE_LIBXSLT
#include "exportdialog.h"
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#endif

namespace DocumentHelper
{
void fetchGrammar(KEduVocDocument* doc, int languageIndex)
{
    QString locale = doc->identifier(languageIndex).locale();

    QUrl location(QUrl::fromUserInput(QString("http://edu.kde.org/parley/locale/") + locale + QString(".kvtml")) );

    KEduVocDocument grammarDoc;
    if (grammarDoc.open(location) == KEduVocDocument::NoError) {
        doc->identifier(languageIndex).setArticle(grammarDoc.identifier(0).article());
        doc->identifier(languageIndex).setPersonalPronouns(grammarDoc.identifier(0).personalPronouns());
        // @todo        m_doc->identifier(index).setDeclension(grammarDoc.identifier(0).declension());
        doc->identifier(languageIndex).setTenseList(grammarDoc.identifier(0).tenseList());
    } else {
        qDebug() << "Download of " << location << " failed.";
    }
}
} // namespace DocumentHelper

ParleyDocument::ParleyDocument(ParleyMainWindow* parleyMainWindow)
    : QObject(parleyMainWindow)
    , m_parleyApp(parleyMainWindow)
    , m_backupTimer(0)
    , m_doc(new KEduVocDocument(this))
{
}

ParleyDocument::~ParleyDocument()
{
    close();
    delete m_backupTimer;
    m_doc->deleteLater();
    emit documentChanged(0);
}

void ParleyDocument::loadLanguageSettings( QString locale ) {
    LanguageSettings settings( locale );
    settings.load();
}

void ParleyDocument::loadEditorFont( QString locale, KEduVocLanguagePropertiesPage* page ) {
    LanguageSettings settings( locale );
    settings.load();
    page->setEditorFont( settings.editorFont() );
}

void ParleyDocument::loadPracticeFont( QString locale, KEduVocLanguagePropertiesPage* page ) {
    LanguageSettings settings( locale );
    settings.load();
    page->setPracticeFont( settings.practiceFont() );
}

void ParleyDocument::loadKeyboardLayout( QString locale, KEduVocLanguagePropertiesPage* page ) {
    LanguageSettings settings( locale );
    settings.load();
    page->setKeyboardLayout( settings.keyboardLayout() );
}

void ParleyDocument::loadSpellChecker( QString locale, KEduVocLanguagePropertiesPage* page ) {
    LanguageSettings settings( locale );
    settings.load();
    page->setSpellChecker( settings.spellChecker() );
}

void ParleyDocument::saveEditorFont( QString locale, QFont font ) {
    LanguageSettings settings( locale );
    settings.load();
    settings.setEditorFont( font );
}

void ParleyDocument::savePracticeFont( QString locale, QFont font ) {
    LanguageSettings settings( locale );
    settings.load();
    settings.setPracticeFont( font );
}

void ParleyDocument::saveKeyboardLayout( QString locale, QString keyboardLayout ) {
    LanguageSettings settings( locale );
    settings.load();
    settings.setKeyboardLayout( keyboardLayout );
}

void ParleyDocument::saveSpellChecker( QString locale, QString spellChecker ) {
    LanguageSettings settings( locale );
    settings.load();
    settings.setSpellChecker( spellChecker );
}

void ParleyDocument::storeSettings( QString locale ) {
    LanguageSettings settings( locale );
    settings.load();
    settings.save();
}


KEduVocDocument *ParleyDocument::document()
{
    // If there is no present vocabulary document, create an empty one.
    if (!m_doc) {
        m_doc = new KEduVocDocument();
    }

    return m_doc;
}

void ParleyDocument::setTitle(const QString& title)
{
    m_doc->setTitle(title);
}

void ParleyDocument::slotFileNew()
{
    if (queryClose()) {
        newDocument(true);
    }
}

void ParleyDocument::newDocument(bool wizard)
{
    KEduVocDocument *newDoc = new KEduVocDocument();

    initializeDefaultGrammar(newDoc);
    setDefaultDocumentProperties(newDoc);
    bool showGrammarDialog = false;
    bool fetchGrammarOnline = false;
    if (wizard) {
        KEduVocDocumentProperties* titleAuthorWidget = new KEduVocDocumentProperties( newDoc, true, m_parleyApp );

        QDialogButtonBox * button_dialog = new QDialogButtonBox;
        button_dialog->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel );

        QVBoxLayout *layout = new QVBoxLayout;
        layout->addWidget( titleAuthorWidget );
        layout->addWidget( button_dialog );

        QDialog * titleAuthorDialog = new QDialog;
        titleAuthorDialog->setLayout( layout );
        titleAuthorDialog->setWindowTitle(i18nc("@title:window document properties", "Properties for %1", newDoc->url().url()));

        connect(titleAuthorDialog, &QDialog::accepted, titleAuthorWidget, &KEduVocDocumentProperties::accept);
        connect(button_dialog, &QDialogButtonBox::accepted, titleAuthorDialog, &QDialog::accept);
        connect(button_dialog, &QDialogButtonBox::rejected, titleAuthorDialog, &QDialog::reject);

        if (titleAuthorDialog->exec()) {
            showGrammarDialog = titleAuthorWidget->isGrammarCheckBoxChecked();
            fetchGrammarOnline = titleAuthorWidget->isDownloadGrammarCheckBoxChecked();
            delete titleAuthorDialog;
        } else {
            delete titleAuthorDialog;
            delete newDoc;
            return;
        }
    }

    close();
    if (m_doc) {
        m_doc->deleteLater();
    }
    m_doc = newDoc;
    emit documentChanged(m_doc);
    enableAutoBackup(Prefs::autoBackup());

    if (fetchGrammarOnline) {
        DocumentHelper::fetchGrammar(m_doc, 0);
        DocumentHelper::fetchGrammar(m_doc, 1);
    }
    if (showGrammarDialog) {
        languageProperties();
    }

    m_parleyApp->showEditor();
}

void ParleyDocument::slotFileOpen()
{
    if ( queryClose() ) {
        ///@todo frameworks check filters are fixed in kvocdoc
        ///@todo frameworks check how to add child checkbox to this dialog
        QFileDialog dialog( m_parleyApp,  i18n("Open Vocabulary Collection"), QString(), KEduVocDocument::pattern(KEduVocDocument::Reading) );
        QCheckBox *practiceCheckBox = new QCheckBox(i18n("Open in practice &mode"), &dialog);
        practiceCheckBox->setChecked(m_parleyApp->currentComponent() != ParleyMainWindow::EditorComponent);
        dialog.setFileMode( QFileDialog::ExistingFile );
        if (dialog.exec()
            && !dialog.selectedFiles().isEmpty()
            && open(QUrl::fromLocalFile( dialog.selectedFiles().first() ))
            ) {
            if (practiceCheckBox->isChecked()) {
                m_parleyApp->showPracticeConfiguration();
            } else {
                m_parleyApp->showEditor();
            }
        }
    }
}

void ParleyDocument::slotFileOpenRecent(const QUrl& url)
{
    if ( queryClose() && open(url)) {
        m_parleyApp->showEditor(); ///@todo: start practice directly depending on current component
    }
}

bool ParleyDocument::open(const QUrl & url)
{
    if (url.path().isEmpty()) {
        return false;
    }

    close();

    if (m_doc) {
        m_doc->deleteLater();
    }
    m_doc = new KEduVocDocument(this);
    emit documentChanged(m_doc);
    m_doc->setCsvDelimiter(Prefs::separator());

    bool isSuccess = false, isError = false;

    KEduVocDocument::ErrorCode ret = m_doc->open(url,  KEduVocDocument::FileDefaultHandling);
    switch ( ret ) {
    case KEduVocDocument::NoError :
        isSuccess = true;
        break;
    case KEduVocDocument::FileLocked :
    {
        int exit = KMessageBox::warningYesNo(
            m_parleyApp, i18n("The vocabulary collection is locked by another process.  You can open the file if you take over the lock, but you will lose any changes from the other process.\n\nDo you want to take over the lock?\n"), i18n( "Take Over Lock" ) ) ;
        if ( exit == KMessageBox::Yes ) { //attempt to steal lock

            ret = m_doc->open(url, KEduVocDocument::FileIgnoreLock);
            if ( ret == KEduVocDocument::NoError ) {
                qDebug() << "Lock stolen";
                isSuccess = true;
            } else {
                isError = true;
            }
        } else { //Don't Steal continue work without saving !!!
        }

        break;
    }
    default:
        isError = true;
    }

    if ( isSuccess ) {
        qDebug() << "Open success.";
        //m_parleyApp->editor()->updateDocument();
        m_parleyApp->addRecentFile(url, m_doc->title());

        enableAutoBackup(Prefs::autoBackup());

    } else {
        if ( isError ) {
            KMessageBox::error(
                m_parleyApp, i18n("Opening collection \"%1\" resulted in an error: %2", m_doc->url().url(),
                                  m_doc->errorDescription(ret)), i18nc("@title:window", "Open Collection"));
        }
        m_doc->deleteLater();
        emit documentChanged(0);
        m_doc = 0;
    }

    return isSuccess;
}

void ParleyDocument::close()
{
    enableAutoBackup(false);
    if (m_doc) {
        emit documentChanged(0);
        m_doc->deleteLater();
        m_doc = 0;
    }
}

bool ParleyDocument::queryClose()
{
    if ( (m_doc == NULL ) || !m_doc->isModified()) {
        return true;
    }

    Prefs::self()->save();

    bool canSave = Prefs::autoSave(); //save without asking

    if (!canSave) {
        int exit = KMessageBox::warningYesNoCancel(
            m_parleyApp, i18n("Vocabulary is modified.\n\nSave file before exit?\n"),
            "", KStandardGuiItem::save(), KStandardGuiItem::discard());
        switch ( exit ) {
        case KMessageBox::Yes :
            canSave = true;   // save and exit
            break;
        case KMessageBox::No :
            canSave = false;  // don't save but exit
            break;
        case KMessageBox::Continue :
        default:
            return false;  // continue work without saving !!!
        }
    }

    if (canSave) {
        save();       // save and exit
    }

    close();
    return true;
}


void ParleyDocument::openGHNS()
{
    if (m_parleyApp->queryClose()) {
        QDir downloadDir(  QStandardPaths::writableLocation( QStandardPaths::DataLocation ) + '/' + "kvtml/" );

        downloadDir.mkpath(downloadDir.absolutePath());

        QUrl url = QUrl::fromUserInput( QFileDialog::getOpenFileName(
            m_parleyApp
            , i18n("Open Downloaded Vocabulary Collection")
            , downloadDir.path()
            , KEduVocDocument::pattern(KEduVocDocument::Reading) ) );

        if (open(url)) {
            m_parleyApp->showPracticeConfiguration();
        }
    }
}

void ParleyDocument::save()
{
    if (m_doc->url().fileName() == i18n("Untitled")) {
        saveAs();
        return;
    }

    // remove previous backup
    QFile::remove(m_doc->url().toLocalFile() + '~');
    QFile::copy(QFile::encodeName(m_doc->url().toLocalFile()), QFile::encodeName(m_doc->url().toLocalFile() + '~'));

    m_doc->setCsvDelimiter(Prefs::separator());

    emit statesNeedSaving();

    QString newgenerator = QString::fromLatin1("Parley ") + PARLEY_VERSION_STRING ;
    m_doc->setGenerator(newgenerator);

    bool isSuccess = false,  isError = false ;

    KEduVocDocument::ErrorCode ret = m_doc->saveAs(
        m_doc->url() , KEduVocDocument::Automatic, KEduVocDocument::FileIgnoreLock);

    switch ( ret ) {
    case KEduVocDocument::NoError :
        isSuccess = true;
        break;
    case KEduVocDocument::FileLocked :
    {
        int exit = KMessageBox::warningYesNo(
            m_parleyApp, i18n("File \"%1\" is locked by another process.  You can save to the file if you take over the lock, but you will lose any changes from the other process.\n\nDo you want to take over the lock?\n"
                , m_doc->url().url()), "");
        if ( exit == KMessageBox::Yes ) {
            m_doc->setGenerator(newgenerator );
            ret = m_doc->saveAs(m_doc->url() , KEduVocDocument::Automatic, KEduVocDocument::FileIgnoreLock);

            if ( ret == KEduVocDocument::NoError ) {
                isSuccess = true;
                qDebug() << "Lock stolen";
            } else {
                isError = true;
            }
        } else {
            //Intentionally empty else. Try to saveAs another filename
        }
        break;
    }
    default:
        isError = true;
    }

    if ( isSuccess ) {
        m_parleyApp->addRecentFile(m_doc->url(), m_doc->title());
        enableAutoBackup(Prefs::autoBackup());
    } else {
        if ( isError ) {
            KMessageBox::error(
                m_parleyApp, i18n("Writing file \"%1\" resulted in an error: %2", m_doc->url().url(),
                                  m_doc->errorDescription(ret)), i18nc("@title:window", "Save File"));
        }
        qDebug() << "Save failed trying save as for "<< m_doc->url().url();
        saveAs();
    }

}

void ParleyDocument::saveAs(QUrl url)
{
    if (!m_doc) {
        return;
    }

    if (url.isEmpty()) {
        url = QUrl::fromLocalFile( QFileDialog::getSaveFileName(
            m_parleyApp->parentWidget()
            , i18n("Save Vocabulary As")
            , QString()
            , KEduVocDocument::pattern(KEduVocDocument::Writing)) );

        if (url.isEmpty()) {
            return;
        }
    }

    QFileInfo fileinfo(url.toLocalFile());
    if (fileinfo.exists()) {
        if (KMessageBox::warningContinueCancel(
                0, i18n("<qt>The file<p><b>%1</b></p>already exists. Do you want to overwrite it?</qt>",
                        url.toLocalFile()), QString(), KStandardGuiItem::overwrite()) == KMessageBox::Cancel) {
            return;
        }
    }

    QString msg = i18nc("@info:status saving a file", "Saving %1", url.toLocalFile());

    QFile::remove(url.toLocalFile() + '~'); // remove previous backup
    QFile::copy(QFile::encodeName(url.toLocalFile()), QFile::encodeName(QString(url.toLocalFile() + '~')));

    m_doc->setCsvDelimiter(Prefs::separator());

    if (!url.toLocalFile().contains('.')) {
        url.fromLocalFile(url.toLocalFile() + QString::fromLatin1(".kvtml"));
    }

    bool isSuccess = false,  isError = false;
    m_doc->setGenerator("Parley");
    int ret = m_doc->saveAs(url, KEduVocDocument::Automatic);
    switch ( ret ) {
    case KEduVocDocument::NoError :
        isSuccess = true;
        break;
    case KEduVocDocument::FileLocked :
    {
        int exit = KMessageBox::warningYesNo(
            m_parleyApp, i18n("File \"%1\" is locked by another process.  You can save to the file if you take over the lock, but you will lose any changes from the other process.\n\nDo you want to take over the lock?\n"
                , m_doc->url().url()), "");
        if ( exit == KMessageBox::Yes ) { //attempt lock steal
            m_doc->setGenerator(QString::fromLatin1("Parley ") + PARLEY_VERSION_STRING );
            ret = m_doc->saveAs(
                m_doc->url() , KEduVocDocument::Automatic, KEduVocDocument::FileIgnoreLock);

            if ( ret == KEduVocDocument::NoError ) {
                isSuccess = true;
                qDebug() << "Lock stolen";
            } else {
                isError = true;
            }
            break;
        } else { //don't steal the lock
        }

        break;
    }
    default:
        isError = true;
        break;
    }

    if ( isSuccess ) {
        qDebug() << "SaveAs success.";
        m_parleyApp->addRecentFile(m_doc->url(), m_doc->title());
        emit statesNeedSaving();

    } else {
        qDebug() << "SaveAs failed for "<< m_doc->url().url()<<" \nwhy "<< m_doc->errorDescription(ret);
        if ( isError ) {
            KMessageBox::error(
                m_parleyApp, i18n("Writing file \"%1\" resulted in an error: %2",
                                  m_doc->url().url(), m_doc->errorDescription(ret)), i18nc("@title:window", "Save File"));

        }
    }

}

void ParleyDocument::initializeDefaultGrammar(KEduVocDocument *doc)
{
    KEduVocWordType *root = doc->wordTypeContainer();
    KEduVocWordType *noun = new KEduVocWordType(i18n("Noun"), root);
    noun->setWordType(KEduVocWordFlag::Noun);
    root->appendChildContainer(noun);

    KEduVocWordType *nounChild = new KEduVocWordType(i18n("Masculine"), noun);
    nounChild->setWordType(KEduVocWordFlag::Noun | KEduVocWordFlag::Masculine);
    noun->appendChildContainer(nounChild);
    nounChild = new KEduVocWordType(i18n("Feminine"), noun);
    nounChild->setWordType(KEduVocWordFlag::Noun | KEduVocWordFlag::Feminine);
    noun->appendChildContainer(nounChild);
    nounChild = new KEduVocWordType(i18n("Neuter"), noun);
    nounChild->setWordType(KEduVocWordFlag::Noun | KEduVocWordFlag::Neuter);
    noun->appendChildContainer(nounChild);

    KEduVocWordType *verb = new KEduVocWordType(i18n("Verb"), root);
    verb->setWordType(KEduVocWordFlag::Verb);
    root->appendChildContainer(verb);

    KEduVocWordType *adjective = new KEduVocWordType(i18n("Adjective"), root);
    adjective->setWordType(KEduVocWordFlag::Adjective);
    root->appendChildContainer(adjective);

    KEduVocWordType *adverb = new KEduVocWordType(i18n("Adverb"), root);
    adverb->setWordType(KEduVocWordFlag::Adverb);
    root->appendChildContainer(adverb);

    KEduVocWordType *conjunction = new KEduVocWordType(i18n("Conjunction"), root);
    conjunction->setWordType(KEduVocWordFlag::Conjunction);
    root->appendChildContainer(conjunction);
}


void ParleyDocument::setDefaultDocumentProperties(KEduVocDocument *doc)
{
    KEMailSettings emailSettings;
    emailSettings.setProfile(emailSettings.defaultProfileName());
    doc->setAuthor(emailSettings.getSetting(KEMailSettings::RealName));
    doc->setAuthorContact(emailSettings.getSetting(KEMailSettings::EmailAddress));

    doc->setLicense(i18n("Public Domain"));
    doc->setCategory(i18n("Languages"));

    QString locale = QLocale().name();

    doc->appendIdentifier();
    doc->appendIdentifier();
    doc->identifier(0).setName(QLocale( locale ).nativeLanguageName( ) );
    doc->identifier(0).setLocale(locale);
    doc->identifier(1).setName(i18n("A Second Language"));
    doc->identifier(1).setLocale(locale);

    KEduVocLesson* lesson = new KEduVocLesson(i18n("Lesson 1"), doc->lesson());
    doc->lesson()->appendChildContainer(lesson);

    // add some entries
    for (int i = 0; i < 15 ; i++) {
        lesson->appendEntry(new KEduVocExpression());
    }

    doc->setModified(false);
}

void ParleyDocument::slotGHNS()
{
    QMimeDatabase db;
    QString fileName;
    KNS3::DownloadDialog newStuffDialog(ParleyMainWindow::instance());
    newStuffDialog.exec();
    KNS3::Entry::List entries = newStuffDialog.installedEntries();
    int numberInstalled = entries.size();
    foreach (const KNS3::Entry & entry, entries) {
        // check mime type and if kvtml, open it
        foreach (const QString & file, entry.installedFiles()) {
            QMimeType mimeType = db.mimeTypeForFile(file);
            qDebug() << "KNS2 file of mime type:" << db.mimeTypeForFile(file).name();
            if (mimeType.inherits("application/x-kvtml")) {
                ParleyMainWindow::instance()->addRecentFile(file, QString()); ///@todo: title!
                fileName = file;
            }
        }
    }

    // to enable the display in the welcome screen
    Prefs::self()->save();
    m_parleyApp->updateRecentFilesModel();
    if (numberInstalled > 1) {
        openGHNS();
    } else if (numberInstalled == 1) {
        if (open(QUrl::fromUserInput(fileName)))
            m_parleyApp->showPracticeConfiguration();
        else
            KMessageBox::error(m_parleyApp, i18n("Could not open vocabulary collection \"%1\"", entries.first().name()));
    }
}

void ParleyDocument::documentProperties()
{
    KEduVocDocumentProperties* titleAuthorWidget = new KEduVocDocumentProperties( m_doc, false, m_parleyApp );

    QDialogButtonBox * button_dialog = new QDialogButtonBox;
    button_dialog->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel );

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget( titleAuthorWidget );
    layout->addWidget( button_dialog );

    QDialog * titleAuthorDialog = new QDialog;
    titleAuthorDialog->setLayout( layout );
    titleAuthorDialog->setWindowTitle(i18nc("@title:window document properties", "Properties for %1", m_doc->url().url()));

    // the language options are only shown, when this is used to create a new document.
    titleAuthorWidget->setLanguageGroupBoxVisible(false);

    connect(titleAuthorDialog, &QDialog::accepted, titleAuthorWidget, &KEduVocDocumentProperties::accept);
    connect(button_dialog, &QDialogButtonBox::accepted, titleAuthorDialog, &QDialog::accept);
    connect(button_dialog, &QDialogButtonBox::rejected, titleAuthorDialog, &QDialog::reject);

    titleAuthorDialog->exec();
    delete titleAuthorDialog;
    emit documentChanged(m_doc);
}

void ParleyDocument::languageProperties()
{
    KEduVocLanguageProperties *properties = new KEduVocLanguageProperties( m_doc, m_parleyApp );
    connect( properties, &KEduVocLanguageProperties::languageSettingsChanged, this, &ParleyDocument::loadLanguageSettings );
    connect( properties, &KEduVocLanguageProperties::editorFontChanged, this, &ParleyDocument::loadEditorFont );
    connect( properties, &KEduVocLanguageProperties::practiceFontChanged, this, &ParleyDocument::loadPracticeFont );
    connect( properties, &KEduVocLanguageProperties::keyboardLayoutChanged, this, &ParleyDocument::loadKeyboardLayout );
    connect( properties, &KEduVocLanguageProperties::spellCheckerChanged, this, &ParleyDocument::loadSpellChecker );
    connect( properties, &KEduVocLanguageProperties::storeEditorFont, this, &ParleyDocument::saveEditorFont );
    connect( properties, &KEduVocLanguageProperties::storePracticeFont, this, &ParleyDocument::savePracticeFont );
    connect( properties, &KEduVocLanguageProperties::storeKeyboardLayout, this, &ParleyDocument::saveKeyboardLayout );
    connect( properties, &KEduVocLanguageProperties::storeSpellChecker, this, &ParleyDocument::saveSpellChecker );;
    connect( properties, &KEduVocLanguageProperties::saveSettings, this, &ParleyDocument::storeSettings );

    if (properties->exec() == QDialog::Accepted) {
        emit languagesChanged();
    }
}

void ParleyDocument::uploadFile()
{
    // save file to temp location
    QTemporaryDir dir;
    if ( ! dir.isValid() ) {
        KMessageBox::error(m_parleyApp, i18n("Could not create temporary directory \"%1\"", dir.path() ));
        return;
    }
    QUrl url(QUrl::fromUserInput(dir.path() + m_doc->url().fileName()) );
    qDebug() << "save in " << url;
    m_doc->setGenerator("Parley");
    if ( m_doc->saveAs(url, KEduVocDocument::Automatic) != KEduVocDocument::NoError ){
        KMessageBox::error(m_parleyApp, i18n("Could not save vocabulary collection \"%1\"", url.toString() ));
        return;
    }

    KEduVocDocument tempDoc(this);
    if ( tempDoc.open(url) != KEduVocDocument::NoError ) {
        KMessageBox::error(m_parleyApp, i18n("Could not open vocabulary collection \"%1\"", url.toString() ));
        return;
    }

    // remove grades
    tempDoc.lesson()->resetGrades(-1, KEduVocContainer::Recursive);
    m_doc->setGenerator("Parley");
    if ( tempDoc.saveAs(url, KEduVocDocument::Automatic) != KEduVocDocument::NoError ) {
        KMessageBox::error(m_parleyApp, i18n("Could not save vocabulary collection \"%1\"", url.toString() ));
        return;
    }

    // upload
    KNS3::UploadDialog dialog(ParleyMainWindow::instance());
    dialog.setUploadFile(url);
    dialog.exec();
}


void ParleyDocument::exportDialog()
{
#ifdef HAVE_LIBXSLT
    ExportDialog dialog(this, m_parleyApp);
    dialog.exec();
#endif
}

void ParleyDocument::slotFileMerge()
{
    ///@todo as soon as some brave soul descends into the lib and implements merging this should be enabled
//     KUrl url = KFileDialog::getOpenUrl(QString(), KEduVocDocument::pattern(KEduVocDocument::Reading), parentWidget(), i18n("Merge Vocabulary File"));
    //
//     if (!url.isEmpty()) {
//         QString msg = i18n("Loading %1", url.path());
//         slotStatusMsg(msg);
    //
//         KEduVocDocument *new_doc = new KEduVocDocument(this);
//         new_doc->setCsvDelimiter(Prefs::separator());
//         new_doc->open(url);
    //
//         m_doc->merge(new_doc, true);
    //
//         KEduVocWordFlag::setTenseNames(m_doc->tenseDescriptions());
//         KVTUsage::setUsageNames(m_doc->usageDescriptions());
    //
//         delete(new_doc);
//         m_recentFilesAction->addUrl(url);
//         m_tableModel->reset();
//         m_lessonModel->setDocument(m_doc);
//         m_tableView->adjustContent();
//     }
}

void ParleyDocument::enableAutoBackup(bool enable)
{
    if (!enable) {
        if (m_backupTimer) {
            m_backupTimer->stop();
        }
    } else {
        if (!m_backupTimer) {
            m_backupTimer = new QTimer(this);
            connect(m_backupTimer, &QTimer::timeout, this, &ParleyDocument::save);
        }
        m_backupTimer->start(Prefs::backupTime() * 60 * 1000);
    }
}
