#pragma once

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
#include <QProgressDialog>
#include <QComboBox>
#include <QGroupBox>
#include <QPlainTextEdit>
#include <FAST/Visualization/Renderer.hpp>
#include "source/logic/DataManager.h"
#include "source/logic/ProcessManager.h"
#include "source/utils/utilities.h"
#include "source/utils/qutilities.h"
#include "source/gui/ProcessTab/PipelineScriptEditorWidget.h"
#include "source/gui/ProcessTab/PipelineRunnerWidget.h"


namespace fast {

class View;
class ComputationThread;
class MainWindow;
class ImagePyramid;

class ProcessWidget: public QWidget {
Q_OBJECT
public:
    ProcessWidget(MainWindow* mainWindow, QWidget* parent=nullptr);
    ~ProcessWidget();
    /**
     * Set the interface in its default state.
     */
    void resetInterface();

    std::map<std::string, std::string> getModelMetadata(std::string modelName);
    void done();
    void stop();
    void stopProcessing();
    void selectWSI(std::shared_ptr<ImagePyramid> WSI);
    void loadResults(int i);
    void processPipeline(std::string pipelinePath, std::shared_ptr<ImagePyramid> WSI);
    void batchProcessPipeline(std::string pipelinePath);
    void saveResults();
    void showMessage(QString msg);
    void runInThread(std::string pipelineFilename, std::string pipelineName, bool runForAll);
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
    void runPipelineEmitted(QString);
    void messageSignal(QString);

public slots:
    bool segmentTissue();
    void addModels();
    void addPipelines();
    /**
     * Defines and creates the script editor widget.
     */
    void editorPipelinesReceived();
    void deletePipelineReceived(QString pipeline_uid);
    void runPipelineReceived(QString pipeline_uid);
private slots:
    bool processStartEventReceived(std::string process_name);

private:
    QVBoxLayout* _main_layout; /* Principal layout holder for the current custom QWidget */
    std::map<std::string, PipelineRunnerWidget*> _pipeline_runners_map;

    bool m_procesessing = false;
    bool m_batchProcesessing = false;
    int m_currentWSI = 0;
    std::unique_ptr<Pipeline> m_runningPipeline;
    QProgressDialog* m_progressDialog;
    std::string _cwd; /* Holder for the main folder containing models? */
    MainWindow* m_mainWindow;
    View* m_view;
    std::shared_ptr<ComputationThread> m_computationThread;
};

}