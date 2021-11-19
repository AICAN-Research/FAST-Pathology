//
// Created by dbouget on 16.11.2021.
//

#ifndef FASTPATHOLOGY_PIPELINERUNNERWIDGET_H
#define FASTPATHOLOGY_PIPELINERUNNERWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QString>
#include <QIcon>
#include <QPixmap>

#include "source/gui/ProcessTab/PipelineScriptEditorWidget.h"
#include "source/logic/ProcessManager.h"
#include "source/logic/DataManager.h"

namespace fast
{

    class PipelineRunnerWidget: public QWidget
    {
        Q_OBJECT
        public:
            PipelineRunnerWidget(QString id, QWidget* parent=0);
            ~PipelineRunnerWidget();
            /**
             * Define the interface for the current global widget.
             */
            void setupInterface();
            /**
             * Set the interface in its default state.
             */
            void resetInterface();

            /**
             * Define the connections for all elements inside the current global widget.
             */
            void setupConnections();

        signals:
            void runPipelineEmitted(QString);
            void editPipelineEmitted(QString);
            void deletePipelineEmitted(QString);
            void addRendererToViewRequested(const std::string&);

        private slots:
            void runPipelineReceived();
            void editPipelineReceived();

        private:
            QString _pipeline_uid;
            QHBoxLayout* _main_layout;
            QPushButton* _run_pushbutton;
            QPushButton* _edit_pushbutton;
            QPushButton* _delete_pushbutton;

    };
} // End of namespace fast

#endif FASTPATHOLOGY_PIPELINERUNNERWIDGET_H
