//
// Created by dbouget on 02.11.2021.
//

#ifndef FASTPATHOLOGY_PROCESSWIDGET_H
#define FASTPATHOLOGY_PROCESSWIDGET_H

#include <string>
#include <iostream>
#include <fstream>
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QCoreApplication>
#include <QTextStream>
#include <QMessageBox>
#include <QProgressDialog>
#include <QListWidgetItem>
#include <QScrollArea>
#include <QLabel>
#include <QColorDialog>
#include <QComboBox>
#include <QGroupBox>
#include <QPlainTextEdit>
#include <FAST/Visualization/Renderer.hpp>
#include "source/logic/DataManager.h"
#include "source/logic/ProcessManager.h"
#include "source/utils/utilities.h"
#include "source/utils/qutilities.h"
#include "source/gui/ProcessTab/PipelineScriptEditorWidget.h"


namespace fast {
    class WholeSlideImageImporter;
    class ImagePyramid;
    class ImagePyramidRenderer;
    class Renderer;
    class SegmentationRenderer;

class ProcessWidget: public QWidget {
Q_OBJECT
public:
    ProcessWidget(QWidget* parent=0);
    ~ProcessWidget();
    /**
     * Set the interface in its default state.
     */
    void resetInterface();

    std::map<std::string, std::string> getModelMetadata(std::string modelName);
protected:
    /**
     * Define the interface for the current global widget.
     */
    void setupInterface();
    /**
     * Define the connections for all elements inside the current global widget.
     */
    void setupConnections();

signals:
    void processTriggered(std::string);
    void addRendererToViewRequested(const std::string&);

public slots:
    bool segmentTissue();
    void addModels();
    void addPipelines();
    /**
     * Defines and creates the script editor widget.
     */
    void editorPipelinesReceived();

private slots:
    bool processStartEventReceived(std::string process_name);

private:
    QVBoxLayout* _main_layout; /* Principal layout holder for the current custom QWidget */
    QPushButton* _tissue_seg_pushbutton; /* Specific push button for tissue segmentation */
    std::map<std::string, QPushButton*> _specific_seg_models_pushbutton_map; /* Map with custom/specific segmentation push buttons for the available models */

private:
    std::string _cwd; /* Holder for the main folder containing models? */
};

}
#endif //FASTPATHOLOGY_PROCESSTWIDGET_H
