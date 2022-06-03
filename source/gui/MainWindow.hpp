#pragma once

#include <FAST/Visualization/Window.hpp>
#include <QPushButton>
#include <QMainWindow>
#include <QDialog>
#include <QObject>
#include <QWidget>
#include <QCloseEvent>
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
         * Creates the OpenGL window for rendering WSI-related stuff.
         */
        void createOpenGLWindow();

        // IMPORT RELATED STUFF


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

        View* view;
        std::shared_ptr<Project> m_project;
        std::string m_currentVisibleWSI; /* Unique id_name of the currently rendered (hence visible) WSI. */

        std::string _application_name; /* */
        MainSidePanelWidget *_side_panel_widget; /* Main widget for the left-hand panel */

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
