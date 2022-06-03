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

class MainWindow : public Window {
    FAST_OBJECT(MainWindow);
    Q_OBJECT
    public:
        bool advancedMode = false;

        std::string cwd;

        QWidget *mainWidget;

        QHBoxLayout *mainLayout;
        QVBoxLayout *superLayout;

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
         * Opens a file explorer for selecting which Pipelines from disk to import to the program.
         * All selected Pipelines will be automatically added to the default Pipelines and the Pipeline menu.
         */
        void addPipelines();

        /**
         * Prompts the user whether or not to "reset", if accepted it will refresh the software to the initial state,
         * deleting all WSIs and rendered images from the current run.
         */
        void reset();

        std::shared_ptr<ComputationThread> getComputationThread();

        std::shared_ptr<Project> getCurrentProject() const;
        std::string getCurrentWSIUID() const;
        std::shared_ptr<WholeSlideImage> getCurrentWSI() const;

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
        void downloadZipFile(std::string URL, std::string destination);

        View* view;
        std::shared_ptr<Project> m_project;
        std::string m_currentVisibleWSI; /* Unique id_name of the currently rendered (hence visible) WSI. */

        std::string _application_name; /* */
        MainSidePanelWidget *_side_panel_widget; /* Main widget for the left-hand panel */
        QMenu* _help_menu; /* */
        QAction* _file_menu_create_project_action;
        QAction* _file_menu_open_project_action;
        QAction* _file_menu_import_wsi_action;
        QAction* _file_menu_add_model_action;
        QAction* _file_menu_add_pipeline_action;
        QAction* _edit_menu_change_mode_action;
        QAction* _help_menu_about_action;

    public slots:
        void closeEvent (QCloseEvent *event);
        void resetDisplay();

        void changeWSIDisplayReceived(std::string uid_name);

        void updateAppTitleReceived(std::string title_suffix);

        void showSplashMenu(bool allowClose);
        void showSplashMenuWithClose();
    signals:
        void updateProjectTitle();
};

}
