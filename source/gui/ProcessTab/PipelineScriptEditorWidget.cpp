//
// Created by dbouget on 04.11.2021.
//

#include "PipelineScriptEditorWidget.h"

namespace fast {
    PipelineScriptEditorWidget::PipelineScriptEditorWidget(QWidget *parent): QWidget(parent)
    {
        this->setupInterface();
        this->setupConnections();
    }

    PipelineScriptEditorWidget::~PipelineScriptEditorWidget()
    {

    }

    void PipelineScriptEditorWidget::setupInterface()
    {
//        auto backgroundLayout = new QVBoxLayout;

//        auto scriptEditorWidget = new QDialog(this);
//        scriptEditorWidget->setWindowTitle("Script Editor");
//        backgroundLayout->addWidget(scriptEditorWidget);

//        auto scriptLayout = new QVBoxLayout;
//        scriptEditorWidget->setLayout(scriptLayout);

//        auto scriptEditor = new QPlainTextEdit(this);
//        auto highlighter = new PipelineHighlighter(scriptEditor->document());
//        const QFont fixedFont("UbuntuMono");
//        scriptEditor->setFont(fixedFont);

//        scriptLayout->insertWidget(1, scriptEditor);

//        // center QDialog when opened
//        QRect rect = scriptEditorWidget->geometry();
//        QRect parentRect = this->geometry();
//        rect.moveTo(this->mapToGlobal(QPoint(parentRect.x() + parentRect.width() - rect.width(), parentRect.y())));
//        scriptEditorWidget->resize(600, 800);

//        auto menuBar = new QMenuBar(scriptEditorWidget);
//        auto toolBar = new QToolBar();

//        scriptLayout->setMenuBar(menuBar);

//        auto fileMenu = menuBar->addMenu(tr("&File"));
//        auto fileToolBar = new QToolBar;

//        const QIcon newIcon = QIcon::fromTheme("document-new", QIcon(":/images/new.png"));
//        auto newAct = new QAction(newIcon, tr("&New"), this);
//        newAct->setShortcuts(QKeySequence::New);
//        newAct->setStatusTip(tr("Create a new file"));
//        QObject::connect(newAct, &QAction::triggered, this, &ProcessWidget::newFileScript);
//        fileMenu->addAction(newAct);

//        const QIcon openIcon = QIcon::fromTheme("document-open", QIcon(":/images/open.png"));
//        auto openAct = new QAction(openIcon, tr("&Open..."), this);
//        openAct->setShortcuts(QKeySequence::Open);
//        openAct->setStatusTip(tr("Open an existing file"));
//        QObject::connect(openAct, &QAction::triggered, this, &ProcessWidget::openScript);
//        fileMenu->addAction(openAct);
//        fileToolBar->addAction(openAct);

//        const QIcon saveIcon = QIcon::fromTheme("document-save", QIcon(":/images/save.png"));
//        auto saveAct = new QAction(saveIcon, tr("&Save"), this);
//        saveAct->setShortcuts(QKeySequence::Save);
//        saveAct->setStatusTip(tr("Save the document to disk"));
//        QObject::connect(saveAct, &QAction::triggered, this, &ProcessWidget::saveScript);
//        fileMenu->addAction(saveAct);
//        fileToolBar->addAction(saveAct);

//        const QIcon saveAsIcon = QIcon::fromTheme("document-save-as");
//        QAction *saveAsAct = fileMenu->addAction(saveAsIcon, tr("Save &As..."), this, &ProcessWidget::saveAsScript);
//        saveAsAct->setShortcuts(QKeySequence::SaveAs);
//        saveAsAct->setStatusTip(tr("Save the document under a new name"));

//        fileMenu->addSeparator();

//        QMenu *editMenu = menuBar->addMenu(tr("&Edit"));
//        auto editToolBar = new QToolBar(tr("Edit"));

//    #ifndef QT_NO_CLIPBOARD
//        const QIcon cutIcon = QIcon::fromTheme("edit-cut", QIcon(":/images/cut.png"));
//        auto cutAct = new QAction(cutIcon, tr("Cu&t"), this);

//        cutAct->setShortcuts(QKeySequence::Cut);
//        cutAct->setStatusTip(tr("Cut the current selection's contents to the "
//                                "clipboard"));
//        QObject::connect(cutAct, &QAction::triggered, scriptEditor, &QPlainTextEdit::cut);
//        editMenu->addAction(cutAct);
//        editToolBar->addAction(cutAct);

//        const QIcon copyIcon = QIcon::fromTheme("edit-copy", QIcon(":/images/copy.png"));
//        auto copyAct = new QAction(copyIcon, tr("&Copy"), this);
//        copyAct->setShortcuts(QKeySequence::Copy);
//        copyAct->setStatusTip(tr("Copy the current selection's contents to the "
//                                 "clipboard"));
//        QObject::connect(copyAct, &QAction::triggered, scriptEditor, &QPlainTextEdit::copy);
//        editMenu->addAction(copyAct);
//        editToolBar->addAction(copyAct);

//        const QIcon pasteIcon = QIcon::fromTheme("edit-paste", QIcon(":/images/paste.png"));
//        auto pasteAct = new QAction(pasteIcon, tr("&Paste"), this);
//        pasteAct->setShortcuts(QKeySequence::Paste);
//        pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current "
//                                  "selection"));
//        QObject::connect(pasteAct, &QAction::triggered, scriptEditor, &QPlainTextEdit::paste);
//        editMenu->addAction(pasteAct);
//        editToolBar->addAction(pasteAct);

//        menuBar->addSeparator();

//    #endif // !QT_NO_CLIPBOARD

//    #ifndef QT_NO_CLIPBOARD
//        cutAct->setEnabled(false);
//        copyAct->setEnabled(false);
//        QObject::connect(scriptEditor, &QPlainTextEdit::copyAvailable, cutAct, &QAction::setEnabled);
//        QObject::connect(scriptEditor, &QPlainTextEdit::copyAvailable, copyAct, &QAction::setEnabled);
//    #endif // !QT_NO_CLIPBOARD

//        scriptEditorWidget->show();
    }

    void PipelineScriptEditorWidget::setupConnections()
    {

    }

    void PipelineScriptEditorWidget::newFileScript()
    {
//        if (maybeSaveScript()) {
//            scriptEditor->clear();
//            setCurrentFileScript(QString());
//        }
    }
} // End of namespace fast
