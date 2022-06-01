#include "ProcessWidget.h"
#include <FAST/Importers/WholeSlideImageImporter.hpp>
#include <FAST/Visualization/ImagePyramidRenderer/ImagePyramidRenderer.hpp>
#include <FAST/Data/ImagePyramid.hpp>
#include <FAST/Visualization/SegmentationRenderer/SegmentationRenderer.hpp>
#include <QGLContext>
#include <QProgressDialog>
#include <QThread>
#include <FAST/Visualization/View.hpp>
#include <FAST/Visualization/ComputationThread.hpp>
#include "source/logic/WholeSlideImage.h"
#include "source/gui/MainWindow.hpp"

namespace fast {
    ProcessWidget::ProcessWidget(MainWindow* mainWindow, QWidget* parent): QWidget(parent){
        m_mainWindow = mainWindow;
        m_computationThread = mainWindow->getComputationThread();
        this->_cwd = join(QDir::homePath().toStdString(), "fastpathology");
        m_view = mainWindow->getView(0);
        this->setupInterface();
        this->setupConnections();

        // Notify GUI when pipeline has finished
        QObject::connect(m_computationThread.get(), &ComputationThread::pipelineFinished, this, &ProcessWidget::done);

        // Stop whe critical error occurs in computation thread
        QObject::connect(m_computationThread.get(), &ComputationThread::criticalError, this, &ProcessWidget::stop);

        // Connection to show message in GUI in main thread
        QObject::connect(this, &ProcessWidget::messageSignal, this, &ProcessWidget::showMessage, Qt::QueuedConnection);
    }

    ProcessWidget::~ProcessWidget(){

    }

    void ProcessWidget::setupInterface()
    {
        this->_main_layout = new QVBoxLayout(this);
        this->_main_layout->setSpacing(6);
        this->_main_layout->setAlignment(Qt::AlignTop);

        auto label = new QLabel();
        label->setText("Select processing pipeline:");
        _main_layout->addWidget(label);

        _stacked_layout = new QStackedLayout;

        _stacked_widget= new QWidget(this);
        _stacked_widget->setLayout(_stacked_layout);

        _page_combobox = new QComboBox(this);

        _main_layout->addWidget(_page_combobox);
        _main_layout->addWidget(_stacked_widget);

        this->addPipelines();
    }

    void ProcessWidget::resetInterface()
    {
        return;
    }

    void ProcessWidget::setupConnections()
    {
        QObject::connect(_page_combobox, SIGNAL(activated(int)), _stacked_layout, SLOT(setCurrentIndex(int)));
    }

    void ProcessWidget::addModels()
    {
        //QString fileName = QFileDialog::getOpenFileName(
        QStringList ls = QFileDialog::getOpenFileNames(this, tr("Select Model"), nullptr,
                tr("Model Files (*.pb *.txt *.h5 *.xml *.mapping *.bin *.uff *.anchors *.onnx *.fpl"),
                nullptr, QFileDialog::DontUseNativeDialog
        ); // TODO: DontUseNativeDialog - this was necessary because I got wrong paths -> /run/user/1000/.../filename instead of actual path

        auto progDialog = QProgressDialog(this);
        progDialog.setRange(0, ls.count() - 1);
        progDialog.setVisible(true);
        progDialog.setModal(false);
        progDialog.setLabelText("Adding models...");
        QRect screenrect = this->screen()[0].geometry();
        progDialog.move(this->width() - progDialog.width() / 2, -this->width() / 2 - progDialog.width() / 2);
        progDialog.show();

        QCoreApplication::processEvents(QEventLoop::AllEvents, 0);
    }

    void ProcessWidget::addPipelines() {
        auto counter = 1;
        // Load pipelines and create one button for each.
        std::string pipelineFolder = this->_cwd + "/pipelines/";
        for(auto& filename : getDirectoryList(pipelineFolder)) {
            auto pipeline = Pipeline(join(pipelineFolder, filename));

            auto page = new QWidget();
            auto layout = new QVBoxLayout();
            layout->setAlignment(Qt::AlignTop);
            page->setLayout(layout);
            _stacked_layout->addWidget(page);
            _page_combobox->addItem(QString::fromStdString(pipeline.getName()));

            auto description = new QLabel();
            description->setText(QString::fromStdString(pipeline.getDescription()));
            description->setWordWrap(true);
            layout->addWidget(description);

            auto button = new QPushButton;
            button->setText("Run pipeline for this image");
            layout->addWidget(button);
            QObject::connect(button, &QPushButton::clicked, [=]() {
                runInThread(join(pipelineFolder, filename), pipeline.getName(), false);
            });

            auto batchButton = new QPushButton;
            batchButton->setText("Run pipeline for all images in project");
            layout->addWidget(batchButton);
            QObject::connect(batchButton, &QPushButton::clicked, [=]() {
                runInThread(join(pipelineFolder, filename), pipeline.getName(), true);
            });
        }
    }

    void ProcessWidget::runInThread(std::string pipelineFilename, std::string pipelineName, bool runForAll) {
        stopProcessing(); // Have to stop any renderers etc first.

        // Create a GL context for the thread which is sharing with the context of the view
        // We have to this because we need an OpenGL context when parsing the pipeline and thereby creating renderers
        auto view = m_view;
        auto context = new QGLContext(View::getGLFormat(), view);
        context->create(view->context());

        if (!context->isValid())
            throw Exception("The custom Qt GL context is invalid!");

        if (!context->isSharing())
            throw Exception("The custom Qt GL context is not sharing!");

        auto thread = new QThread();
        context->moveToThread(thread);
        QObject::connect(thread, &QThread::started, [=](){
            context->makeCurrent();
            if (runForAll) {
                batchProcessPipeline(pipelineFilename);
            } else {
                processPipeline(pipelineFilename, nullptr);
            }
            context->doneCurrent(); // Must call done here for some reason..
            thread->quit();
        });
        int size = m_mainWindow->getCurrentProject()->getWSICountInProject();
        // TODO lock tabs etc while running.
        m_progressDialog = new QProgressDialog(("Running pipeline " + pipelineName + " ..").c_str(), "Cancel", 0, runForAll ? size : 1, this);
        m_progressDialog->setWindowTitle("Running..");
        m_progressDialog->setAutoClose(true);
        m_progressDialog->show();
        QObject::connect(m_progressDialog, &QProgressDialog::canceled, [this, thread]() {
            std::cout << "canceled.." << std::endl;
            thread->wait();
            std::cout << "done waiting" << std::endl;
            stop();
        });
        thread->start();
        // TODO should delete QThread safely somehow..
    }

    void ProcessWidget::stopProcessing() {
        m_procesessing = false;
        std::cout << "stopping pipeline" << std::endl;
        m_view->stopPipeline();
        std::cout << "done" << std::endl;
        std::cout << "removing renderers.." << std::endl;
        m_view->removeAllRenderers();
        std::cout << "done" << std::endl;
    }

    void ProcessWidget::stop() {
        if(m_progressDialog)
            m_progressDialog->setValue(m_progressDialog->maximum()); // Close progress dialog
        m_batchProcesessing = false;
        stopProcessing();
        selectWSI(m_mainWindow->getCurrentWSI()->get_image_pyramid());
    }

    void ProcessWidget::showMessage(QString msg) {
        // This must happen in main thread
        QMessageBox msgBox;
        msgBox.setText(msg);
        msgBox.exec();
    }

    void ProcessWidget::done() {
        if(m_procesessing)
            saveResults();
        if(m_batchProcesessing) {
            if(m_currentWSI == m_mainWindow->getCurrentProject()->getWSICountInProject()-1) {
                // All processed. Stop.
                m_progressDialog->setValue(m_mainWindow->getCurrentProject()->getWSICountInProject());
                m_batchProcesessing = false;
                m_procesessing = false;
                emit messageSignal("Batch processing is done!");
                emit pipelineFinished(m_mainWindow->getCurrentProject()->getAllWsiUids()[0]);
            } else {
                // Run next
                m_currentWSI += 1;
                m_progressDialog->setValue(m_currentWSI);
                std::cout << "Processing WSI " << m_currentWSI << std::endl;
                processPipeline(m_runningPipeline->getFilename(), m_mainWindow->getCurrentProject()->getImage(m_currentWSI)->get_image_pyramid());
            }
        } else if(m_procesessing) {
            m_progressDialog->setValue(1);
            emit messageSignal("Processing is done!");
            emit pipelineFinished(m_mainWindow->getCurrentProject()->getAllWsiUids()[m_currentWSI]);
            m_procesessing = false;
        }
    }

    void ProcessWidget::processPipeline(std::string pipelinePath, std::shared_ptr<ImagePyramid> WSI) {
        std::cout << "Processing pipeline: " << pipelinePath << std::endl;
        stopProcessing();
        m_procesessing = true;
        auto view = m_view;

        // Load pipeline and give it a WSI
        std::cout << "Loading pipeline in thread: " << std::this_thread::get_id() << std::endl;
        m_runningPipeline = std::make_shared<Pipeline>(pipelinePath);
        std::cout << "OK" << std::endl;
        try {
            std::cout << "parsing" << std::endl;
            if(!WSI) {
                auto uids = m_mainWindow->getCurrentProject()->getAllWsiUids();
                auto currentUID = m_mainWindow->getCurrentWSIUID();
                if(currentUID.empty())
                    return;
                for(int i = 0; i < uids.size(); ++i) {
                    if(uids[i] == currentUID) {
                        m_currentWSI = i;
                    }
                }
                WSI = m_mainWindow->getCurrentProject()->getImage(currentUID)->get_image_pyramid();
            }
            m_runningPipeline->parse({{"WSI", WSI}});
            std::cout << "OK" << std::endl;
        } catch(Exception &e) {
            m_procesessing = false;
            m_batchProcesessing = false;
            m_runningPipeline.reset();
            // Syntax error in pipeline file. Raise error and return to avoid crash.
            std::string msg = "Error parsing pipeline! " + std::string(e.what());
            emit messageSignal(msg.c_str());
            return;
        }
        std::cout << "Done" << std::endl;

        for(auto renderer : m_runningPipeline->getRenderers()) {
            view->addRenderer(renderer);
        }
        m_computationThread->reset();
    }

    void ProcessWidget::batchProcessPipeline(std::string pipelineFilename) {
        m_currentWSI = 0;
        m_batchProcesessing = true;
        auto uid = m_mainWindow->getCurrentProject()->getAllWsiUids()[m_currentWSI];
        processPipeline(pipelineFilename, m_mainWindow->getCurrentProject()->getImage(uid)->get_image_pyramid());
    }

    void ProcessWidget::selectWSI(std::shared_ptr<ImagePyramid> WSI) {
        stopProcessing();
        auto view = m_view;
        view->removeAllRenderers();
        auto renderer = ImagePyramidRenderer::create()
                ->connect(WSI);
        view->addRenderer(renderer);
    }

    void ProcessWidget::saveResults() {
        auto pipelineData = m_runningPipeline->getAllPipelineOutputData();
        m_mainWindow->getCurrentProject()->saveResults(m_mainWindow->getCurrentProject()->getAllWsiUids()[m_currentWSI], m_runningPipeline, pipelineData);
    }

    void ProcessWidget::editorPipelinesReceived()
    {
        auto editor = new PipelineScriptEditorWidget(this);
    }
}
