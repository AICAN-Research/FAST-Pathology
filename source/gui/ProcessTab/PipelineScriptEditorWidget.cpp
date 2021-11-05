//
// Created by dbouget on 04.11.2021.
//

#include "PipelineScriptEditorWidget.h"

#include <FAST/PipelineEditor.hpp>

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
        this->_main_dialog = new QDialog(this);
        this->_main_dialog->setWindowTitle("Script Editor");

        this->_main_dialog_layout = new QVBoxLayout;
        this->_main_dialog->setLayout(this->_main_dialog_layout);

        this->_editor_textedit = new QPlainTextEdit(this);
        auto highlighter = new PipelineHighlighter(this->_editor_textedit->document());
        const QFont fixedFont("UbuntuMono");
        this->_editor_textedit->setFont(fixedFont);

        this->_main_dialog_layout->insertWidget(1, this->_editor_textedit);

        // center QDialog when opened
        QRect rect = this->_main_dialog->geometry();
        QRect parentRect = this->geometry();
        rect.moveTo(this->mapToGlobal(QPoint(parentRect.x() + parentRect.width() - rect.width(), parentRect.y())));
        this->_main_dialog->resize(600, 800);
        this->setupInterfaceWidgetMenu();
        this->_main_dialog->show();
    }
    void PipelineScriptEditorWidget::setupInterfaceWidgetMenu()
    {
        this->_main_menubar = new QMenuBar(this->_main_dialog);
        auto toolBar = new QToolBar();

        this->_main_dialog_layout->setMenuBar(this->_main_menubar);

        this->_file_menu = this->_main_menubar->addMenu(tr("&File"));
        auto fileToolBar = new QToolBar;

        const QIcon newIcon = QIcon::fromTheme("document-new", QIcon(":/images/new.png"));
        this->_file_menu_new_action = new QAction(newIcon, tr("&New"), this);
        this->_file_menu_new_action->setShortcuts(QKeySequence::New);
        this->_file_menu_new_action->setStatusTip(tr("Create a new file"));
        this->_file_menu->addAction(this->_file_menu_new_action);

        const QIcon openIcon = QIcon::fromTheme("document-open", QIcon(":/images/open.png"));
        this->_file_menu_open_action = new QAction(openIcon, tr("&Open..."), this);
        this->_file_menu_open_action->setShortcuts(QKeySequence::Open);
        this->_file_menu_open_action->setStatusTip(tr("Open an existing file"));
        this->_file_menu->addAction(this->_file_menu_open_action);
        fileToolBar->addAction(this->_file_menu_open_action);

        const QIcon saveIcon = QIcon::fromTheme("document-save", QIcon(":/images/save.png"));
        this->_file_menu_save_action = new QAction(saveIcon, tr("&Save"), this);
        this->_file_menu_save_action->setShortcuts(QKeySequence::Save);
        this->_file_menu_save_action->setStatusTip(tr("Save the document to disk"));
        this->_file_menu->addAction(this->_file_menu_save_action);
        fileToolBar->addAction(this->_file_menu_save_action);

        const QIcon saveAsIcon = QIcon::fromTheme("document-save-as");
        this->_file_menu_saveas_action = new QAction(saveAsIcon, tr("Save &As..."), this);
        this->_file_menu_saveas_action->setShortcuts(QKeySequence::SaveAs);
        this->_file_menu_saveas_action->setStatusTip(tr("Save the document under a new name"));
        this->_file_menu->addAction(this->_file_menu_saveas_action);
        this->_file_menu->addSeparator();

        this->_edit_menu = this->_main_menubar->addMenu(tr("&Edit"));
        auto editToolBar = new QToolBar(tr("Edit"));

        #ifndef QT_NO_CLIPBOARD
            const QIcon cutIcon = QIcon::fromTheme("edit-cut", QIcon(":/images/cut.png"));
            this->_edit_menu_cut_action = new QAction(cutIcon, tr("Cu&t"), this);
            this->_edit_menu_cut_action->setShortcuts(QKeySequence::Cut);
            this->_edit_menu_cut_action->setStatusTip(tr("Cut the current selection's contents to the clipboard"));
            this->_edit_menu->addAction(this->_edit_menu_cut_action);
            editToolBar->addAction(this->_edit_menu_cut_action);

            const QIcon copyIcon = QIcon::fromTheme("edit-copy", QIcon(":/images/copy.png"));
            this->_edit_menu_copy_action = new QAction(copyIcon, tr("&Copy"), this);
            this->_edit_menu_copy_action->setShortcuts(QKeySequence::Copy);
            this->_edit_menu_copy_action->setStatusTip(tr("Copy the current selection's contents to the clipboard"));
            this->_edit_menu->addAction(this->_edit_menu_copy_action);
            editToolBar->addAction(this->_edit_menu_copy_action);

            const QIcon pasteIcon = QIcon::fromTheme("edit-paste", QIcon(":/images/paste.png"));
            this->_edit_menu_paste_action = new QAction(pasteIcon, tr("&Paste"), this);
            this->_edit_menu_paste_action->setShortcuts(QKeySequence::Paste);
            this->_edit_menu_paste_action->setStatusTip(tr("Paste the clipboard's contents into the current selection"));
            this->_edit_menu->addAction(this->_edit_menu_paste_action);
            editToolBar->addAction(this->_edit_menu_paste_action);

            this->_main_menubar->addSeparator();
        #endif // !QT_NO_CLIPBOARD
        #ifndef QT_NO_CLIPBOARD
            this->_edit_menu_cut_action->setEnabled(false);
            this->_edit_menu_copy_action->setEnabled(false);
        #endif // !QT_NO_CLIPBOARD
    }

    void PipelineScriptEditorWidget::setupConnections()
    {
        QObject::connect(this->_file_menu_new_action, &QAction::triggered, this, &PipelineScriptEditorWidget::newFileScript);
        QObject::connect(this->_file_menu_open_action, &QAction::triggered, this, &PipelineScriptEditorWidget::openScript);
        QObject::connect(this->_file_menu_save_action, &QAction::triggered, this, &PipelineScriptEditorWidget::saveScript);
        QObject::connect(this->_file_menu_saveas_action, &QAction::triggered, this, &PipelineScriptEditorWidget::saveAsScript);

        #ifndef QT_NO_CLIPBOARD
            QObject::connect(this->_edit_menu_cut_action, &QAction::triggered, this->_editor_textedit, &QPlainTextEdit::cut);
            QObject::connect(this->_edit_menu_copy_action, &QAction::triggered, this->_editor_textedit, &QPlainTextEdit::copy);
            QObject::connect(this->_edit_menu_paste_action, &QAction::triggered, this->_editor_textedit, &QPlainTextEdit::paste);
        #endif // !QT_NO_CLIPBOARD
        #ifndef QT_NO_CLIPBOARD
            QObject::connect(this->_editor_textedit, &QPlainTextEdit::copyAvailable, this->_edit_menu_cut_action, &QAction::setEnabled);
            QObject::connect(this->_editor_textedit, &QPlainTextEdit::copyAvailable, this->_edit_menu_copy_action, &QAction::setEnabled);
        #endif // !QT_NO_CLIPBOARD
    }

    void PipelineScriptEditorWidget::newFileScript()
    {
        if (this->maybeSaveScript())
        {
            this->_editor_textedit->clear();
            this->setCurrentFileScript(QString());
        }
    }

    void PipelineScriptEditorWidget::openScript() {
        if (this->maybeSaveScript())
        {
            auto fileName = QFileDialog::getOpenFileName(this, tr("Open File"), nullptr, tr("WSI Files (*.fpl *.txt)"),
                                                         nullptr, QFileDialog::DontUseNativeDialog);
            if (!fileName.isEmpty())
                this->loadFileScript(fileName);
        }
    }

    void PipelineScriptEditorWidget::loadFileScript(const QString &fileName)
    {
        QFile file(fileName);
        if (!file.open(QFile::ReadOnly | QFile::Text)) { // handle read only files
            QMessageBox::warning(this->_main_dialog, tr("Application"),
                                 tr("Cannot read file %1:\n%2.")
                                         .arg(QDir::toNativeSeparators(fileName), file.errorString()));
            return;
        }

        QTextStream in(&file);
    #ifndef QT_NO_CURSOR
        QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    #endif
        this->_editor_textedit->setPlainText(in.readAll());
    #ifndef QT_NO_CURSOR
        QGuiApplication::restoreOverrideCursor();
    #endif

        this->setCurrentFileScript(fileName);
    }

    bool PipelineScriptEditorWidget::saveScript()
    {
        std::cout << "Saving...: " << this->_current_script_filename.toStdString() << std::endl;
        if (this->_current_script_filename.isEmpty())
            return saveAsScript();
        else
        {
            std::cout << "We are saving, not save as..." << std::endl;
            return saveFileScript(this->_current_script_filename);
        }
    }

    bool PipelineScriptEditorWidget::saveAsScript()
    {
        QFileDialog dialog(this->_main_dialog);
        //dialog.DontUseNativeDialog;
        dialog.setOption(QFileDialog::DontUseNativeDialog);
        dialog.setWindowModality(Qt::WindowModal);
        dialog.setAcceptMode(QFileDialog::AcceptSave);
        if (dialog.exec() != QDialog::Accepted)
            return false;
        return saveFileScript(dialog.selectedFiles().first());
    }

    bool PipelineScriptEditorWidget::saveFileScript(const QString &fileName)
    {
        QString errorMessage;

        QGuiApplication::setOverrideCursor(Qt::WaitCursor);
        QSaveFile file(fileName);
        if (file.open(QFile::WriteOnly | QFile::Text))
        {
            QTextStream out(&file);
            out << this->_editor_textedit->toPlainText();
            if (!file.commit())
            {
                errorMessage = tr("Cannot write file %1:\n%2.")
                        .arg(QDir::toNativeSeparators(fileName), file.errorString());
            }
        }
        else
        {
            errorMessage = tr("Cannot open file %1 for writing:\n%2.")
                    .arg(QDir::toNativeSeparators(fileName), file.errorString());
        }
        QGuiApplication::restoreOverrideCursor();

        if (!errorMessage.isEmpty())
        {
            QMessageBox::warning(this->_main_dialog, tr("Application"), errorMessage);
            return false;
        }

        setCurrentFileScript(fileName);
        return true;
    }

    bool PipelineScriptEditorWidget::maybeSaveScript()
    {
        if (!this->_editor_textedit->document()->isModified())
            return true;
        const QMessageBox::StandardButton ret = QMessageBox::warning(this->_main_dialog, tr("Application"),
                                       tr("The document has been modified.\n"
                                          "Do you want to save your changes?"),
                                       QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        switch (ret) {
            case QMessageBox::Save:
                return saveScript();
            case QMessageBox::Cancel:
                return false;
            default:
                break;
        }
        return true;
    }

    void PipelineScriptEditorWidget::setCurrentFileScript(const QString &fileName)
    {
        this->_current_script_filename = fileName;
        this->_editor_textedit->document()->setModified(false);
        this->_editor_textedit->setWindowModified(false);

        QString shownName = this->_current_script_filename;
        if (this->_current_script_filename.isEmpty())
            shownName = "untitled.txt";
        this->_editor_textedit->setWindowFilePath(shownName);
    }
} // End of namespace fast
