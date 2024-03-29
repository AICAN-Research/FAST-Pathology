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
#include "source/utils/utilities.h"
#include "source/utils/qutilities.h"
#include "source/gui/ProcessTab/PipelineScriptEditorWidget.h"
#include <FAST/Pipeline.hpp>

class QStackedLayout;

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

    void done();
    void stop();
    void stopProcessing();
    void selectWSI(std::shared_ptr<ImagePyramid> WSI);
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
    void messageSignal(QString);
    void pipelineFinished(std::string uid);
public slots:
    void refreshPipelines(QString currentFilename = "");
    /**
     * Defines and creates the script editor widget.
     */
    void editorPipelinesReceived();
    /**
     * Opens a file explorer for selecting which deep learning modules from disk to import to the program.
     * All selected models will be automatically added to the Progress widget.
     */
    void addModelsFromDisk();
    /**
     * Opens a file explorer for selecting which Pipelines from disk to import to the program.
     * All selected Pipelines will be automatically added to the default Pipelines and the Pipeline menu.
     */
    void addPipelinesFromDisk();
    /**
     * @brief Update progress dialog
     */
    void updateProgress();
private:
    QVBoxLayout* _main_layout; /* Principal layout holder for the current custom QWidget */
    QStackedLayout* _stacked_layout;
    QWidget* _stacked_widget;
    QComboBox* _page_combobox;

    bool m_procesessing = false;
    bool m_batchProcesessing = false;
    int m_currentWSI = 0;
    std::shared_ptr<Pipeline> m_runningPipeline;
    QProgressDialog* m_progressDialog;
    std::string _cwd; /* Holder for the main folder containing models? */
    MainWindow* m_mainWindow;
    View* m_view;
    std::shared_ptr<ComputationThread> m_computationThread;
};

}