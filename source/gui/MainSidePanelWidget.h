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
        ViewWidget* getViewWidget();
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
        void selectFilesTriggered();
        void addModelsTriggered();
        void editorPipelinesTriggered();
        void loadProject();
        void showMenu();
    public:
        ProjectWidget *_project_widget;
        ProcessWidget *_process_widget;
        ViewWidget *_view_widget;
        //StatsWidget *_stats_widget;
        //ExportWidget *_export_widget;

    private:
        QStackedWidget *_container_stacked_widget;
        MainWindow* m_mainWindow;
    };
}
