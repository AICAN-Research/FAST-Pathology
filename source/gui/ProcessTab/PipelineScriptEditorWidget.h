//
// Created by dbouget on 04.11.2021.
//

#ifndef FASTPATHOLOGY_PIPELINESCRIPTEDITORWIDGET_H
#define FASTPATHOLOGY_PIPELINESCRIPTEDITORWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QDialog>
#include <QPlainTextEdit>
#include <QMenuBar>
#include <QToolBar>
#include <QIcon>
#include <QKeySequence>
#include <QAction>
#include <QMessageBox>
#include <QTextStream>
#include <QGuiApplication>
#include <QFileDialog>
#include <QDir>
#include <QSaveFile>
#include <iostream>

namespace fast {
    class PipelineHighlighter;

class PipelineScriptEditorWidget: public QWidget {
Q_OBJECT
public:
    PipelineScriptEditorWidget(QWidget* parent=0);
    ~PipelineScriptEditorWidget();
    /**
     * Define the interface for the current global widget.
     */
    void setupInterface();
    /**
     * Defined and created the menubar related to the script editor.
     */
    void setupInterfaceWidgetMenu();

    /**
     * Set the interface in its default state.
     */
    void resetInterface();

    /**
     * Define the connections for all elements inside the current global widget.
     */
    void setupConnections();

private:
    /**
     * Sets the title of the script editor widget to the current file name.
     * @param fileName
     */
    void setCurrentFileScript(const QString &fileName);

private slots:
    /**
     * Create a new file from the script editor. Opens a blank document. If already unsaved document exist,
     * it will prompt whether the user wish to save the unsaved changes.
     */
    void newFileScript();
    /**
     * Opens file explorer to select which script to use in the script editor.
     */
    void openScript();
    /**
     * Loads the script file from disk, relevant for the script editor.
     * @param fileName
     */
    void loadFileScript(const QString &fileName);
    /**
     * Save the current document in the script editor.
     * @return
     */
    bool saveScript();
    /**
     * To save the current document as a completely new document, from the script editor.
     * @return
     */
    bool saveAsScript();
    /**
     * Actually saves document in script editor, given a save/saveAs event.
     * @param fileName
     * @return
     */
    bool saveFileScript(const QString &fileName);
    /**
     * Prompt given to the user if they wish the current changes in the script editor or not.
     * @return
     */
    bool maybeSaveScript();

private:
    QString _current_script_filename; /* */
    QDialog* _main_dialog; /* */
    QVBoxLayout* _main_dialog_layout; /* */
    QPlainTextEdit* _editor_textedit; /* */
    QMenuBar* _main_menubar; /* */
    QMenu* _file_menu; /* */
    QMenu* _edit_menu; /* */
    QAction* _file_menu_new_action; /* */
    QAction* _file_menu_open_action; /* */
    QAction* _file_menu_save_action; /* */
    QAction* _file_menu_saveas_action; /* */
    QAction* _edit_menu_cut_action; /* */
    QAction* _edit_menu_copy_action; /* */
    QAction* _edit_menu_paste_action; /* */
};
} // End of namespace fast

#endif FASTPATHOLOGY_PIPELINESCRIPTEDITORWIDGET_H
