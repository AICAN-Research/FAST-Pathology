//
// Created by dbouget on 04.11.2021.
//

#ifndef FASTPATHOLOGY_PIPELINESCRIPTEDITORWIDGET_H
#define FASTPATHOLOGY_PIPELINESCRIPTEDITORWIDGET_H

#include <QWidget>

namespace fast {
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
     * Set the interface in its default state.
     */
    void resetInterface();

    /**
     * Define the connections for all elements inside the current global widget.
     */
    void setupConnections();

private:
    void newFileScript();
};
} // End of namespace fast

#endif FASTPATHOLOGY_PIPELINESCRIPTEDITORWIDGET_H
