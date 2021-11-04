//
// Created by dbouget on 06.10.2021.
//

#ifndef FASTPATHOLOGY_MAINSIDEPANELWIDGET_H
#define FASTPATHOLOGY_MAINSIDEPANELWIDGET_H

#include <QWidget>
#include <QSignalMapper>
#include <QStackedWidget>
#include <QToolBar>
#include <QPainter>
#include "source/gui/ProjectTab/ProjectWidget.h"
#include "source/gui/ProcessTab/ProcessWidget.h"
#include "source/gui/ViewTab/ViewWidget.h"
#include "source/gui/StatsTab/StatsWidget.h"
#include "source/logic/ProcessManager.h"

namespace fast{
    class MainSidePanelWidget: public QWidget{
    Q_OBJECT
    public:
        MainSidePanelWidget(QWidget *parent=0);
        ~MainSidePanelWidget();
    protected:
        void setUpInterface();
        /**
         * Set the interface in its default state.
         */
        void resetInterface();

        /**
         * Define the connections for all elements inside the current global widget.
         */
        void setupConnections();

    signals:
        void newImageDisplay(std::string, bool);
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

    private:
        QStackedWidget *_container_stacked_widget;
        QPushButton* _app_mode_pushbutton;
    };
}

#endif //FASTPATHOLOGY_MAINSIDEPANELWIDGET_H
