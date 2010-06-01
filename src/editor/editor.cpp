/***************************************************************************
    Copyright 1999-2001 Ewald Arnold <kvoctrain@ewald-arnold.de>
    Copyright 2004-2007 Peter Hedlund <peter.hedlund@kdemail.net>
    Copyright 2007-2009 Frederik Gladhorn <gladhorn@kde.org>
    Copyright 2008 Daniel Laidig <d.laidig@gmx.de>
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "editor.h"

#include "../config-parley.h"

#include "vocabulary/vocabularymodel.h"
#include "vocabulary/vocabularyview.h"
#include "vocabulary/vocabularyfilter.h"
#include "vocabulary/containerview.h"
#include "vocabulary/lessonview.h"
#include "vocabulary/wordtypeview.h"
#include "vocabulary/containermodel.h"
#include "vocabulary/lessonmodel.h"
#include "vocabulary/wordtypemodel.h"

#include "entry-dialogs/multiplechoicewidget.h"
#include "entry-dialogs/comparisonwidget.h"
#include "entry-dialogs/conjugationwidget.h"
#include "entry-dialogs/declensionwidget.h"
#include "entry-dialogs/imagechooserwidget.h"
#include "entry-dialogs/audiowidget.h"
#include "entry-dialogs/browserwidget.h"
#include "entry-dialogs/synonymwidget.h"
#include "entry-dialogs/summarywordwidget.h"
#include "entry-dialogs/latexwidget.h"

#include "settings/parleyprefs.h"
#include "prefs.h"

#include "scripts/scriptdialog.h"
#include "scripts/translator.h"

#include "parleyactions.h"
#include <KActionCollection>
#include <KToggleAction>
#include <KActionMenu>
#include <KCharSelect>

#include <QtGui/QDockWidget>
#include <QtGui/QHeaderView>
#include <QtGui/QStackedWidget>

#include "modeltest/modeltest.h"

using namespace Editor;

EditorWindow::EditorWindow(ParleyMainWindow* parent)
    :KXmlGuiWindow(parent), m_mainWindow(parent)
{
    // KXmlGui
    setXMLFile("editorui.rc");
    setObjectName("Editor");

    setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

    initView();
    initModel();

    initDockWidgets();
    initActions();
    initScripts();

    KConfigGroup cfg(KSharedConfig::openConfig("parleyrc"), objectName());
    applyMainWindowSettings(cfg);

    connect(parent->parleyDocument(), SIGNAL(documentChanged(KEduVocDocument*)), this, SLOT(updateDocument(KEduVocDocument*)));
    connect(parent->parleyDocument(), SIGNAL(languagesChanged()), this, SLOT(slotLanguagesChanged()));
    connect(parent->parleyDocument(), SIGNAL(statesNeedSaving()), this, SLOT(saveState()));
    connect(parent, SIGNAL(preferencesChanged()), this, SLOT(applyPrefs()));
}

EditorWindow::~EditorWindow()
{
    saveState();
    KConfigGroup cfg(KSharedConfig::openConfig("parleyrc"), objectName());
    saveMainWindowSettings(cfg);
}

void EditorWindow::updateDocument(KEduVocDocument *doc)
{
    m_vocabularyView->setDocument(doc);
    m_vocabularyModel->setDocument(doc);

    m_lessonModel->setDocument(doc);
    m_wordTypeModel->setDocument(doc);

    m_summaryWordWidget->slotDocumentChanged(doc);
    m_conjugationWidget->setDocument(doc);
    m_comparisonWidget->setDocument(doc);

    if (!m_mainWindow->parleyDocument()->document()) {
        return;
    }

    // expand the root items
    m_lessonView->expandToDepth(0);
    m_wordTypeView->expandToDepth(0);

    connect(m_mainWindow->parleyDocument()->document(), SIGNAL(docModified(bool)), m_mainWindow, SLOT(slotUpdateWindowCaption()));

    connect(m_vocabularyView->selectionModel(), 
                SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
            m_summaryWordWidget, SLOT(slotSelectionChanged(const QItemSelection &, const QItemSelection &)));
    connect(m_vocabularyView->selectionModel(),
                SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
            m_latexWidget, SLOT(slotSelectionChanged(const QItemSelection &, const QItemSelection &)));

    setCaption(m_mainWindow->parleyDocument()->document()->url().fileName(), false);

    m_mainWindow->slotUpdateWindowCaption();

///@todo remove this!
// at the moment creates a new test every time a model is created. this is good because we get the basic sanity check then.
// temporarily disabled because somehow with the welcome screen this crashes Parley when using open recent
//     new ModelTest(m_vocabularyModel, this);
}


void EditorWindow::initDockWidgets()
{
// Lesson dock
    QDockWidget *lessonDockWidget = new QDockWidget(i18n("Lessons"), this);
    lessonDockWidget->setObjectName("LessonDock");
    m_lessonView = new LessonView(this);
    lessonDockWidget->setWidget(m_lessonView);
    addDockWidget(Qt::LeftDockWidgetArea, lessonDockWidget);
    m_dockWidgets.append(lessonDockWidget);
    actionCollection()->addAction("show_lesson_dock", lessonDockWidget->toggleViewAction());

    m_lessonModel = new LessonModel(this);
///@todo remove before release
    new ModelTest(m_lessonModel, this);

    m_lessonView->setModel(m_lessonModel);
    m_lessonView->setToolTip(i18n("Right click to add, delete, or rename lessons. \n"
            "With the checkboxes you can select which lessons you want to practice. \n"
            "Only checked lessons [x] will be asked in the tests!"));

    connect(m_lessonView, SIGNAL(selectedLessonChanged(KEduVocLesson*)),
        m_vocabularyModel, SLOT(setLesson(KEduVocLesson*)));

    connect(m_lessonView, SIGNAL(signalShowContainer(KEduVocContainer*)),
        m_vocabularyModel, SLOT(showContainer(KEduVocContainer*)));

    connect(m_vocabularyView, SIGNAL(translationChanged(KEduVocExpression*, int)),
        m_lessonView, SLOT(setTranslation(KEduVocExpression*, int)));


// Word types dock
    QDockWidget* wordTypeDockWidget = new QDockWidget(i18n("Word Types"), this);
    wordTypeDockWidget->setObjectName( "WordTypeDock" );
    m_wordTypeView = new WordTypeView(this);
    wordTypeDockWidget->setWidget(m_wordTypeView);
    addDockWidget( Qt::LeftDockWidgetArea, wordTypeDockWidget );
    m_dockWidgets.append(wordTypeDockWidget);

    m_wordTypeModel = new WordTypeModel(this);
    wordTypeDockWidget->setVisible(false);
    actionCollection()->addAction("show_wordtype_dock", wordTypeDockWidget->toggleViewAction());

///@todo test, should be fixed with the lesson one though
///@todo remove before release
//     new ModelTest(m_wordTypeModel, this);

    m_wordTypeView->setModel(m_wordTypeModel);

    connect(m_wordTypeView, SIGNAL(selectedWordTypeChanged(KEduVocWordType*)),
        m_vocabularyModel, SLOT(setWordType(KEduVocWordType*)));

    connect(m_wordTypeView, SIGNAL(signalShowContainer(KEduVocContainer*)),
        m_vocabularyModel, SLOT(showContainer(KEduVocContainer*)));

    connect(m_vocabularyView, SIGNAL(translationChanged(KEduVocExpression*, int)),
        m_wordTypeView, SLOT(setTranslation(KEduVocExpression*, int)));

// Conjugations
    QDockWidget *conjugationDock = new QDockWidget(i18n("Conjugation"), this);
    conjugationDock->setObjectName("ConjugationDock");
    m_conjugationWidget = new ConjugationWidget(this);
    conjugationDock->setWidget(m_conjugationWidget);
    addDockWidget(Qt::RightDockWidgetArea, conjugationDock);
    m_dockWidgets.append(conjugationDock);
    conjugationDock->setVisible(false);
    actionCollection()->addAction("show_conjugation_dock", conjugationDock->toggleViewAction());
    connect(m_vocabularyView, SIGNAL(translationChanged(KEduVocExpression*, int)),
        m_conjugationWidget, SLOT(setTranslation(KEduVocExpression*, int)));

// Declensions
//     QDockWidget *declensionDock = new QDockWidget(i18n("Declension"), this);
//     declensionDock->setObjectName("DeclensionDock");
//     DeclensionWidget *declensionWidget = new DeclensionWidget(this);
//     declensionDock->setWidget(declensionWidget);
//     addDockWidget(Qt::RightDockWidgetArea, declensionDock);
//     m_dockWidgets.append(declensionDock);
//     actionCollection()->addAction("show_declension_dock", declensionDock->toggleViewAction());
//     declensionDock->setVisible(false);
//     connect(m_mainWindow->parleyDocument(), SIGNAL(documentChanged(KEduVocDocument*)),
//             declensionWidget, SLOT(setDocument(KEduVocDocument*)));
//     connect(m_vocabularyView, SIGNAL(translationChanged(KEduVocExpression*, int)),
//             declensionWidget, SLOT(setTranslation(KEduVocExpression*, int)));

// Comparison forms
    QDockWidget *comparisonDock = new QDockWidget(i18n("Comparison forms"), this);
    comparisonDock->setObjectName("ComparisonDock");
    m_comparisonWidget = new ComparisonWidget(this);
    comparisonDock->setWidget(m_comparisonWidget);
    addDockWidget(Qt::RightDockWidgetArea, comparisonDock);
    m_dockWidgets.append(comparisonDock);
    actionCollection()->addAction("show_comparison_dock", comparisonDock->toggleViewAction());
    comparisonDock->setVisible(false);
    connect(m_vocabularyView, SIGNAL(translationChanged(KEduVocExpression*, int)),
        m_comparisonWidget, SLOT(setTranslation(KEduVocExpression*, int)));

// Multiple choice
    QDockWidget *multipleChoiceDock = new QDockWidget(i18n("Multiple Choice"), this);
    multipleChoiceDock->setObjectName("MultipleChoiceDock");
    MultipleChoiceWidget *multipleChoiceWidget = new MultipleChoiceWidget(this);
    multipleChoiceDock->setWidget(multipleChoiceWidget);
    addDockWidget(Qt::RightDockWidgetArea, multipleChoiceDock);
    m_dockWidgets.append(multipleChoiceDock);
    actionCollection()->addAction("show_multiplechoice_dock", multipleChoiceDock->toggleViewAction());
    multipleChoiceDock->setVisible(false);
    connect(m_vocabularyView, SIGNAL(translationChanged(KEduVocExpression*, int)),
        multipleChoiceWidget, SLOT(setTranslation(KEduVocExpression*, int)));

// Synonym (and the same for antonym and false friends)
    QDockWidget *synonymDock = new QDockWidget(i18n("Synonyms"), this);
    synonymDock->setObjectName("SynonymDock");
    SynonymWidget *synonymWidget = new SynonymWidget(SynonymWidget::Synonym, this);
    synonymDock->setWidget(synonymWidget);
    addDockWidget(Qt::RightDockWidgetArea, synonymDock);
    m_dockWidgets.append(synonymDock);
    actionCollection()->addAction("show_synonym_dock", synonymDock->toggleViewAction());
    synonymDock->setVisible(false);
    connect(m_vocabularyView, SIGNAL(translationChanged(KEduVocExpression*, int)),
            synonymWidget, SLOT(setTranslation(KEduVocExpression*, int)));

    QDockWidget *antonymDock = new QDockWidget(i18n("Antonyms"), this);
    antonymDock->setObjectName("AntonymDock");
    SynonymWidget *antonymWidget = new SynonymWidget(SynonymWidget::Antonym, this);
    antonymDock->setWidget(antonymWidget);
    addDockWidget(Qt::RightDockWidgetArea, antonymDock);
    m_dockWidgets.append(antonymDock);
    actionCollection()->addAction("show_antonym_dock", antonymDock->toggleViewAction());
    antonymDock->setVisible(false);
    connect(m_vocabularyView, SIGNAL(translationChanged(KEduVocExpression*, int)),
            antonymWidget, SLOT(setTranslation(KEduVocExpression*, int)));

    QDockWidget *falseFriendDock = new QDockWidget(i18n("False Friends"), this);
    falseFriendDock->setObjectName("FalseFriendDock");
    SynonymWidget *falseFriendWidget = new SynonymWidget(SynonymWidget::FalseFriend, this);
    falseFriendDock->setWidget(falseFriendWidget);
    addDockWidget(Qt::RightDockWidgetArea, falseFriendDock);
    m_dockWidgets.append(falseFriendDock);
    actionCollection()->addAction("show_falsefriend_dock", falseFriendDock->toggleViewAction());
    falseFriendDock->setVisible(false);
    connect(m_vocabularyView, SIGNAL(translationChanged(KEduVocExpression*, int)),
            falseFriendWidget, SLOT(setTranslation(KEduVocExpression*, int)));

// Pronunciation symbols - Use KCharSelect
    QDockWidget *charSelectDock = new QDockWidget(i18n("Phonetic Symbols"), this);
    charSelectDock->setObjectName("IPADock");
    KCharSelect *charSelectWidget = new KCharSelect(this,  KCharSelect::SearchLine | KCharSelect::BlockCombos | KCharSelect::CharacterTable);
    charSelectWidget->setCurrentChar(0x0250);
    charSelectDock->setWidget(charSelectWidget);
    addDockWidget(Qt::BottomDockWidgetArea, charSelectDock);
    m_dockWidgets.append(charSelectDock);
    actionCollection()->addAction("show_pronunciation_dock", charSelectDock->toggleViewAction());
    charSelectDock->setVisible(false);
    connect(charSelectWidget, SIGNAL(charSelected(const QChar &)), m_vocabularyView, SLOT(appendChar(const QChar &)));

// Image
    QDockWidget *imageDock = new QDockWidget(i18n("Image"), this);
    imageDock->setObjectName("ImageDock");
    ImageChooserWidget *imageChooserWidget = new ImageChooserWidget(this);
    imageDock->setWidget(imageChooserWidget);
    addDockWidget(Qt::RightDockWidgetArea, imageDock);
    m_dockWidgets.append(imageDock);
    actionCollection()->addAction("show_image_dock", imageDock->toggleViewAction());
    imageDock->setVisible(false);
    connect(m_vocabularyView, SIGNAL(translationChanged(KEduVocExpression*, int)),
        imageChooserWidget, SLOT(setTranslation(KEduVocExpression*, int)));

// Summary word
    QDockWidget *summaryDock = new QDockWidget(i18n("Summary"), this);
    summaryDock->setObjectName("SummaryDock");
    m_summaryWordWidget = new SummaryWordWidget(m_vocabularyFilter, m_mainWindow->parleyDocument()->document(), this);
    summaryDock->setWidget(m_summaryWordWidget);
    addDockWidget(Qt::RightDockWidgetArea, summaryDock);
    actionCollection()->addAction("show_summary_dock", summaryDock->toggleViewAction());
    summaryDock->setVisible(false);
    m_dockWidgets.append(summaryDock);
    connect(m_vocabularyView, SIGNAL(translationChanged(KEduVocExpression*, int)),
            m_summaryWordWidget, SLOT(setTranslation(KEduVocExpression*, int)));

// Sound
    QDockWidget *audioDock = new QDockWidget(i18n("Sound"), this);
    audioDock->setObjectName("AudioDock");
    AudioWidget *audioWidget = new AudioWidget(this);
    audioDock->setWidget(audioWidget);
    addDockWidget(Qt::RightDockWidgetArea, audioDock);
    m_dockWidgets.append(audioDock);
    actionCollection()->addAction("show_audio_dock", audioDock->toggleViewAction());
    audioDock->setVisible(false);
    connect(m_vocabularyView, SIGNAL(translationChanged(KEduVocExpression*, int)),
        audioWidget, SLOT(setTranslation(KEduVocExpression*, int)));

// browser
    QDockWidget *browserDock = new QDockWidget(i18n("Internet"), this);
    browserDock->setObjectName("BrowserDock");
    //TinyWebBrowser *browserWidget = new TinyWebBrowser(this);
    BrowserWidget *htmlPart = new BrowserWidget(browserDock);
    browserDock->setWidget(htmlPart);
    addDockWidget(Qt::RightDockWidgetArea, browserDock);
    m_dockWidgets.append(browserDock);
    actionCollection()->addAction("show_browser_dock", browserDock->toggleViewAction());
    browserDock->setVisible(false);
    connect(m_vocabularyView, SIGNAL(translationChanged(KEduVocExpression*, int)),
            htmlPart, SLOT(setTranslation(KEduVocExpression*, int)));

// LaTeX
    QDockWidget *latexDock = new QDockWidget(i18n("LaTeX"), this);
    latexDock->setObjectName("LatexDock");
    m_latexWidget = new LatexWidget(m_vocabularyFilter, m_mainWindow->parleyDocument()->document(), this);
    latexDock->setWidget(m_latexWidget);
    addDockWidget(Qt::RightDockWidgetArea, latexDock);
    actionCollection()->addAction("show_latex_dock", latexDock->toggleViewAction());
    latexDock->setVisible(false);
    m_dockWidgets.append(latexDock);
    connect(m_vocabularyView, SIGNAL(translationChanged(KEduVocExpression*, int)),
            m_latexWidget, SLOT(setTranslation(KEduVocExpression*, int)));

// Grades
//     QDockWidget *gradeDock = new QDockWidget(i18n("Grade"), this);
//     gradeDock->setObjectName("gradeDock");
//     QLabel *gradeWidget = new QLabel("grade placeholder", this);
//     gradeDock->setWidget(gradeWidget);
//     addDockWidget(Qt::RightDockWidgetArea, gradeDock);
//     connect(this, SIGNAL(signalSetData(KEduVocTranslation*)), m_declensionWidget, SLOT(setTranslation(KEduVocTranslation*)));

}

void EditorWindow::initActions()
{
    ParleyActions::create(ParleyActions::RemoveGrades, this, SLOT(removeGrades()), actionCollection());
    ParleyActions::create(ParleyActions::CheckSpelling, m_vocabularyView, SLOT(checkSpelling()), actionCollection());
    ParleyActions::create(ParleyActions::ToggleShowSublessons, m_vocabularyModel, SLOT(showEntriesOfSubcontainers(bool)), actionCollection());
    ParleyActions::create(ParleyActions::AutomaticTranslation, m_vocabularyModel, SLOT(automaticTranslation(bool)), actionCollection());
    ParleyActions::create(ParleyActions::StartPractice, m_mainWindow, SLOT(showPracticeConfiguration()), actionCollection());
    actionCollection()->action("practice_start")->setText(i18n("Practice"));
    actionCollection()->action("practice_start")->setToolTip(i18n("Practice"));
    ParleyActions::create(ParleyActions::ConfigurePractice, m_mainWindow, SLOT(configurePractice()), actionCollection());
    ParleyActions::create(ParleyActions::ToggleSearchBar, this, SLOT(slotConfigShowSearch()), actionCollection());
    ParleyActions::create(ParleyActions::SearchVocabulary, this, SLOT(startSearch()), actionCollection());
    ParleyActions::create(ParleyActions::ShowScriptManager, this, SLOT(slotShowScriptManager()), actionCollection());
    ParleyActions::create(ParleyActions::LanguagesProperties, m_mainWindow->parleyDocument(), SLOT(languageProperties()), actionCollection());
    ParleyActions::createUploadAction(m_mainWindow->parleyDocument(), SLOT(uploadFile()), actionCollection());
}

void EditorWindow::initModel()
{
    m_vocabularyModel = new VocabularyModel(this);
    m_vocabularyFilter = new VocabularyFilter(this);
    m_vocabularyFilter->setSourceModel(m_vocabularyModel);
    m_vocabularyView->setModel(m_vocabularyFilter);

//    connect(m_mainWindow->parleyDocument(), SIGNAL(documentChanged(KEduVocDocument*)), m_vocabularyModel, SLOT(setDocument(KEduVocDocument*)));
//    connect(m_mainWindow->parleyDocument(), SIGNAL(documentChanged(KEduVocDocument*)), m_vocabularyView, SLOT(setDocument(KEduVocDocument*)));
    connect(m_searchLine, SIGNAL(textChanged(const QString&)), m_vocabularyFilter, SLOT(setSearchString(const QString&)));
}

/**
 * This initializes the main widgets and table.
 */
void EditorWindow::initView()
{
    QWidget *mainWidget = new QWidget(this);
    setCentralWidget(mainWidget);
    QVBoxLayout *topLayout = new QVBoxLayout(mainWidget);
    topLayout->setMargin(KDialog::marginHint());
    topLayout->setSpacing(KDialog::spacingHint());

    m_searchLine = new KLineEdit(this);
    m_searchLine->show();
    m_searchLine->setFocusPolicy(Qt::ClickFocus);
    m_searchLine->setClearButtonShown(true);
    m_searchLine->setClickMessage(i18n("Enter search terms here"));

//     m_searchLine->setToolTip(i18n("Search your vocabuary"));

    QLabel *label = new QLabel(i18n("S&earch:"), this);
    label->setBuddy(m_searchLine);
    label->show();

    m_searchWidget = new QWidget(this);
    QHBoxLayout* layout = new QHBoxLayout(m_searchWidget);
    layout->setSpacing(KDialog::spacingHint());
    layout->setMargin(0);
    layout->addWidget(label);
    layout->addWidget(m_searchLine);

///@todo     centralWidget()-> delete layout
    QVBoxLayout * rightLayout = new QVBoxLayout();
    rightLayout->setSpacing(KDialog::spacingHint());
    rightLayout->setMargin(0);
    rightLayout->addWidget(m_searchWidget);
    m_searchWidget->setVisible(Prefs::showSearch());

    m_vocabularyView = new VocabularyView(this);
    m_vocabularyView->setFont(Prefs::tableFont());
    rightLayout->addWidget(m_vocabularyView, 1, 0);

    topLayout->addLayout(rightLayout);
}

void EditorWindow::slotConfigShowSearch()
{
    m_searchWidget->setVisible(m_searchWidget->isHidden());
    Prefs::setShowSearch(m_searchWidget->isVisible());
}

void EditorWindow::startSearch()
{
    m_searchWidget->setVisible(true);
    m_searchLine->setFocus();
}

void EditorWindow::slotShowScriptManager() {
    ScriptDialog * dialog = new ScriptDialog(m_scriptManager);
    dialog->show();
}

void EditorWindow::applyPrefs()
{
    m_vocabularyView->setFont(Prefs::tableFont());
    m_vocabularyView->reset();
}

void EditorWindow::removeGrades()
{
    m_mainWindow->parleyDocument()->document()->lesson()->resetGrades(-1, KEduVocContainer::Recursive);
}

void EditorWindow::initScripts()
{
    m_scriptManager = new ScriptManager(this);

    m_vocabularyView->setTranslator(m_scriptManager->translator());

    //Load scripts
    m_scriptManager->loadScripts();
}

void EditorWindow::saveState()
{
    m_vocabularyView->saveColumnVisibility();
}

void EditorWindow::slotLanguagesChanged()
{
    m_vocabularyModel->resetLanguages();
}

#include "editor.moc"
