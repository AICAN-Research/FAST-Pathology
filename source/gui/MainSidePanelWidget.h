#pragma once

#include <QWidget>
#include <QSignalMapper>
#include <QStackedWidget>
#include <QToolBar>
#include <QPainter>
#include "source/gui/ProjectTab/ProjectWidget.h"
#include "source/gui/ProcessTab/ProcessWidget.h"
#include "source/gui/ViewTab/ViewWidget.h"
#include "source/gui/StatsTab/StatsWidget.h"
#include "source/gui/ExportTab/ExportWidget.h"
#include "source/logic/ProcessManager.h"

namespace fast{
    class MainWindow;
    class MainSidePanelWidget: public QWidget{
    Q_OBJECT
    public:
        MainSidePanelWidget(MainWindow* mainWindow, QWidget *parent = nullptr);
        ~MainSidePanelWidget();
        /**
         * Set the interface in its default state.
         */
        void resetInterface();

    protected:
        void setUpInterface();
        /**
         * Define the connections for all elements inside the current global widget.
         */
        void setupConnections();

    signals:
        void filesDropped(const QList<QString> &);
        void changeWSIDisplayTriggered(std::string, bool);
        void resetDisplay();
        void newAppTitle(std::string);
        void createProjectTriggered();
        void openProjectTriggered();
        void saveProjectTriggered();
        void selectFilesTriggered();
        void downloadTestDataTriggered();
        void addModelsTriggered();
        void addPipelinesTriggered();
        void editorPipelinesTriggered();
        void addRendererToViewRequested(const std::string&);
        void removeRendererFromViewRequested(const std::string&);
        void runPipelineEmitted(QString);

    public slots:
        /**
         * To change GUI mode. By default it is in diagnostics mode. In advanced mode, additional options are included,
         * such as possibility to set parameters to methods and such. Toggling will update the title of the program
         * and the Qt "mode" button on the down-left of the program.
         */
        void setApplicationMode();

    public:
        ProjectWidget *_project_widget;
        ProcessWidget *_process_widget;
        ViewWidget *_view_widget;
        StatsWidget *_stats_widget;
        ExportWidget *_export_widget;

    private:
        QStackedWidget *_container_stacked_widget;
        QPushButton* _app_mode_pushbutton;
        MainWindow* m_mainWindow;
    };
}
