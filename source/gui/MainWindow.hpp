#pragma once

#include <FAST/Visualization/Window.hpp>
#include <QPushButton>
#include <QMainWindow>
#include <QDialog>
#include <QObject>
#include <QWidget>
#include <QCloseEvent>
#include <QNetworkReply>
#include <QMessageBox>
#include <QProgressDialog>
#include "source/utils/utilities.h"
#include "source/gui/MainSidePanelWidget.h"
#include "source/logic/DataManager.h"

QT_BEGIN_NAMESPACE
class QAction;
class QLabel;
class QMenuBar;
class QMenu;
class QPushButton;
class QTextEdit;
class QHBoxLayout;
class QVBoxLayout;
class QStackedWidget;
class QComboBox;
class QStackedLayout;
class QSplitter;
class QPlainTextEdit;
class QString;
class QScrollArea;
class QListWidget;
QT_END_NAMESPACE

namespace fast {
    class ImagePyramid;
    class View;
    class ViewWidget;

class MainWindow : public Window {
    FAST_OBJECT(MainWindow);
    Q_OBJECT
    public:
        bool advancedMode = false;

        std::string cwd;
        std::string tmpDirPath;

        std::unordered_map<std::string, std::string> metadata; // make metadata information a global variable

        QWidget *mainWidget;
        QMenu *runPipelineMenu;
        View *view;

        QHBoxLayout *mainLayout;
        QVBoxLayout *superLayout;
        QVBoxLayout *processLayout;

        // custom split
        std::vector<std::string> splitCustom(const std::string& s, const std::string& delimiter);

        // GUI RELATED STUFF
        /**
         * Creates the main menu bar of the platform.
         */
        void createMenubar();
        /**
         * Creates the OpenGL window for rendering WSI-related stuff.
         */
        void createOpenGLWindow();
        /**
         * Opens the default browser and directs the user to the FastPathology GitHub page to seek for assistance.
         */
        static void helpUrl();
        /**
         * Opens the default browser and directs the user to the FastPathology GitHub "Issues" section, to easily
         * report an issue/bug or request a feature.
         */
        static void reportIssueUrl();
        /**
         * Opens a simple dialog which contains information about the software.
         */
        void aboutProgram();

        // IMPORT RELATED STUFF
        /**
         * Opens a file explorer for selecting which deep learning modules from disk to import to the program.
         * All selected models will be automatically added to the Progress widget.
         */
        void addModels();
        /**
         * Imports models to the program, where selections are made from a drag-and-drop event.
         * @param fileNames
         */
        void addModelsDrag(const QList<QString> &fileNames);
        /**
         * Opens a file explorer for selecting which Pipelines from disk to import to the program.
         * All selected Pipelines will be automatically added to the default Pipelines and the Pipeline menu.
         */
        void addPipelines();

        /**
         * Load text pipeline from disk. Assumed to be stored in a .txt-like format. Both .txt and .fpl are supported.
         */
        void loadPipelines();

        /**
         * Prompts the user whether or not to "reset", if accepted it will refresh the software to the initial state,
         * deleting all WSIs and rendered images from the current run.
         */
        void reset();

        std::shared_ptr<ComputationThread> getComputationThread();

        std::shared_ptr<Project> getCurrentProject();

        std::string getRootFolder() const;
    protected:
        /**
         * Define the interface for the current global widget.
         */
        void setupInterface();
        /**
         * Define the connections for all elements inside the current global widget.
         */
        void setupConnections();

    private:
        MainWindow();

        std::string _application_name; /* */
        MainSidePanelWidget *_side_panel_widget; /* Main widget for the left-hand panel */
        std::map<std::string, QAction*> _file_menu_actions; /* Holder for all actions in the File main menu bar */
        QMenu* _pipeline_menu; /* */
        QMenu* _help_menu; /* */
        QAction* _file_menu_create_project_action;
        QAction* _file_menu_import_wsi_action;
        QAction* _file_menu_add_model_action;
        QAction* _file_menu_add_pipeline_action;
        QAction* _project_menu_create_project_action;
        QAction* _project_menu_open_project_action;
        QAction* _project_menu_save_project_action;
        QAction* _edit_menu_change_mode_action;
        QAction* _edit_menu_download_testdata_action;
        QAction* _pipeline_menu_import_action;
        QAction* _pipeline_menu_editor_action;
        QAction* _help_menu_about_action;

    public slots:
        void closeEvent (QCloseEvent *event);
        void resetDisplay();

        void changeWSIDisplayReceived(std::string uid_name, bool state);

        void updateAppTitleReceived(std::string title_suffix);
    };

}
