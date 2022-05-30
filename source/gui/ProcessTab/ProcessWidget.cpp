//
// Created by dbouget on 02.11.2021.
//

#include "ProcessWidget.h"
#include <FAST/Importers/WholeSlideImageImporter.hpp>
#include <FAST/Visualization/ImagePyramidRenderer/ImagePyramidRenderer.hpp>
#include <FAST/Data/ImagePyramid.hpp>
#include <FAST/Visualization/SegmentationRenderer/SegmentationRenderer.hpp>
#include <QGLContext>
#include <QProgressDialog>
#include <QThread>
#include <FAST/Visualization/View.hpp>
#include <FAST/Importers/WholeSlideImageImporter.hpp>
#include <FAST/Visualization/ImagePyramidRenderer/ImagePyramidRenderer.hpp>
#include <FAST/Visualization/SegmentationRenderer/SegmentationRenderer.hpp>
#include <FAST/Visualization/HeatmapRenderer/HeatmapRenderer.hpp>
#include <FAST/Importers/TIFFImagePyramidImporter.hpp>
#include <FAST/Exporters/TIFFImagePyramidExporter.hpp>
#include <FAST/Importers/MetaImageImporter.hpp>
#include <FAST/Exporters/MetaImageExporter.hpp>
#include <FAST/Importers/HDF5TensorImporter.hpp>
#include <FAST/Exporters/HDF5TensorExporter.hpp>
#include <FAST/Visualization/ComputationThread.hpp>
#include "source/logic/WholeSlideImage.h"

namespace fast {
    ProcessWidget::ProcessWidget(View* view, std::shared_ptr<ComputationThread> compThread, QWidget* parent): QWidget(parent){
        m_computationThread = compThread;
        this->_cwd = join(QDir::homePath().toStdString(), "fastpathology");
        m_view = view;
        this->setupInterface();
        this->setupConnections();

        // Notify GUI when pipeline has finished
        QObject::connect(compThread.get(), &ComputationThread::pipelineFinished, this, &ProcessWidget::done);

        // Stop whe critical error occurs in computation thread
        QObject::connect(compThread.get(), &ComputationThread::criticalError, this, &ProcessWidget::stop);

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
        this->addPipelines();
    }

    void ProcessWidget::resetInterface()
    {
        return;
    }

    void ProcessWidget::setupConnections()
    {
//        QObject::connect(this->_tissue_seg_pushbutton, &QPushButton::clicked, this, &ProcessWidget::segmentTissue);
    }

    std::map<std::string, std::string> ProcessWidget::getModelMetadata(std::string modelName) {
        // parse corresponding txt file for relevant information regarding model
        std::ifstream infile(this->_cwd + "data/models/" + modelName + ".txt");
        std::string key, value, str;
        std::string delimiter = ":";
        std::map<std::string, std::string> metadata;
        while (std::getline(infile, str))
        {
            std::vector<std::string> v = split(str, delimiter);
            //key = v[0];
            //value = v[1];
            //metadata[key] = value;
            metadata.emplace(std::move(v[0]), std::move(v[1]));
        }
        return metadata;
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

            auto button = new QPushButton;
            button->setText(("Run: " + pipeline.getName()).c_str());
            _main_layout->addWidget(button);
            QObject::connect(button, &QPushButton::clicked, [=]() {
                runInThread(join(pipelineFolder, filename), pipeline.getName(), false);
            });

            auto batchButton = new QPushButton;
            batchButton->setText(("Run for entire project: " + pipeline.getName()).c_str());
            _main_layout->addWidget(batchButton);
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
                processPipeline(pipelineFilename);
            }
            context->doneCurrent(); // Must call done here for some reason..
            thread->quit();
        });
        m_progressDialog = new QProgressDialog(("Running pipeline " + pipelineName + " ..").c_str(), "Cancel", 0, runForAll ? m_WSIs.size() : 1, this);
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
            selectWSI(m_currentWSI);
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
            if(m_currentWSI == m_WSIs.size()-1) {
                // All processed. Stop.
                m_progressDialog->setValue(m_WSIs.size());
                m_batchProcesessing = false;
                m_procesessing = false;
                emit messageSignal("Batch processing is done!");
            } else {
                // Run next
                m_currentWSI += 1;
                m_progressDialog->setValue(m_currentWSI);
                std::cout << "Processing WSI " << m_currentWSI << std::endl;
                processPipeline(m_runningPipeline->getFilename());
            }
        } else if(m_procesessing) {
            m_progressDialog->setValue(1);
            emit messageSignal("Processing is done!");
            m_procesessing = false;
        }
    }

    void ProcessWidget::processPipeline(std::string pipelinePath) {
        std::cout << "Processing pipeline: " << pipelinePath << std::endl;
        stopProcessing();
        m_procesessing = true;
        auto view = m_view;

        // Load pipeline and give it a WSI
        std::cout << "Loading pipeline in thread: " << std::this_thread::get_id() << std::endl;
        m_runningPipeline = std::make_unique<Pipeline>(pipelinePath);
        std::cout << "OK" << std::endl;
        try {
            std::cout << "parsing" << std::endl;
            m_runningPipeline->parse({{"WSI", m_WSIs[m_currentWSI].second}});
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
        processPipeline(pipelineFilename);
    }

    void ProcessWidget::selectWSI(int i) {
        stopProcessing();
        m_currentWSI = i;
        auto view = m_view;
        view->removeAllRenderers();
        auto renderer = ImagePyramidRenderer::create()->connect(m_WSIs[m_currentWSI].second);
        view->addRenderer(renderer);
    }

    void ProcessWidget::saveResults() {
        auto pipelineData = m_runningPipeline->getAllPipelineOutputData();
        for(auto data : pipelineData) {
            const std::string dataTypeName = data.second->getNameOfClass();
            const std::string saveFolder = join(this->_cwd, "results", m_WSIs[m_currentWSI].first, m_runningPipeline->getName());
            createDirectories(saveFolder);
            std::cout << "Saving " << dataTypeName << " data to " << saveFolder << std::endl;
            if(dataTypeName == "ImagePyramid") {
                const std::string saveFilename = join(saveFolder, data.first + ".tiff");
                auto exporter = TIFFImagePyramidExporter::create(saveFilename)
                        ->connect(data.second);
                exporter->run();
            } else if(dataTypeName == "Image") {
                const std::string saveFilename = join(saveFolder, data.first + ".mhd");
                auto exporter = MetaImageExporter::create(saveFilename)
                        ->connect(data.second);
                exporter->run();
            } else if(dataTypeName == "Tensor") {
                const std::string saveFilename = join(saveFolder, data.first + ".hdf5");
                auto exporter = HDF5TensorExporter::create(saveFilename)
                        ->connect(data.second);
                exporter->run();
            } else {
                std::cout << "Unsupported data to export " << dataTypeName << std::endl;
            }

            // TODO handle multiple renderes somehow
            std::ofstream file(join(saveFolder, "attributes.txt"), std::iostream::out);
            for(auto renderer : m_runningPipeline->getRenderers()) {
                if(renderer->getNameOfClass() != "ImagePyramidRenderer")
                    file << renderer->attributesToString();
            }
            file.close();
        }
    }

    void ProcessWidget::loadResults(int i) {
        selectWSI(i);
        auto view = m_view;
        // Load any results for current WSI
        const std::string saveFolder = join(_cwd, "results", m_WSIs[m_currentWSI].first);
        for(auto pipelineName : getDirectoryList(saveFolder, false, true)) {
            const std::string folder = join(saveFolder, pipelineName);
            for(auto filename : getDirectoryList(folder, true, false)) {
                const std::string path = join(folder, filename);
                const std::string extension = filename.substr(filename.rfind('.'));
                Renderer::pointer renderer;
                if(extension == ".tiff") {
                    auto importer = TIFFImagePyramidImporter::create(path);
                    renderer = SegmentationRenderer::create()->connect(importer);
                } else if(extension == ".mhd") {
                    auto importer = MetaImageImporter::create(path);
                    renderer = SegmentationRenderer::create()->connect(importer);
                } else if(extension == ".hdf5") {
                    auto importer = HDF5TensorImporter::create(path);
                    renderer = HeatmapRenderer::create()->connect(importer);
                }
                if(renderer) {
                    // Read attributes from txt file
                    std::ifstream file(join(folder, "attributes.txt"), std::iostream::in);
                    if(!file.is_open())
                        throw Exception("Error reading " + join(folder, "attributes.txt"));
                    do {
                        std::string line;
                        std::getline(file, line);
                        trim(line);
                        std::cout << line << std::endl;

                        std::vector<std::string> tokens = split(line);
                        if(tokens[0] != "Attribute")
                            break;

                        if(tokens.size() < 3)
                            throw Exception("Expecting at least 3 items on attribute line when parsing object " + renderer->getNameOfClass() + " but got " + line);

                        std::string name = tokens[1];

                        std::shared_ptr<Attribute> attribute = renderer->getAttribute(name);
                        std::string attributeValues = line.substr(line.find(name) + name.size());
                        trim(attributeValues);
                        attribute->parseInput(attributeValues);
                        std::cout << "Set attribute " << name << " to " << attributeValues  << " for object " << renderer->getNameOfClass() << std::endl;
                    } while(!file.eof());
                    renderer->loadAttributes();
                    view->addRenderer(renderer);
                }
            }
        }
    }

    void ProcessWidget::deletePipelineReceived(QString pipeline_uid)
    {
        this->_main_layout->removeWidget(this->_pipeline_runners_map[pipeline_uid.toStdString()]);
        this->_pipeline_runners_map.erase(this->_pipeline_runners_map.find(pipeline_uid.toStdString()));
        QCoreApplication::processEvents(QEventLoop::AllEvents, 0);
    }

    bool ProcessWidget::segmentTissue()
    {
        // @TODO. All of it should be within a ProcessManager most likely, no reason to have it inside here.

        // @TODO. Does the segmentation run only on the visible WSI, or on all loaded WSIs showing on the side list?
        if (DataManager::GetInstance()->getCurrentProject()->isProjectEmpty()) {
            std::cout << "Requires a WSI to be rendered in order to perform the analysis." << std::endl;
            return false;
        }

        if (DataManager::GetInstance()->getVisibleImageName() == "")
        {
            std::cout << "Requires a WSI to be rendered in order to perform the analysis." << std::endl;
            return false;
        }

        auto visible_image = DataManager::GetInstance()->get_visible_image();

        // prompt if you want to run the analysis again, if it has already been ran
        if (visible_image->has_renderer("tissue")) {
            simpleInfoPrompt("Tissue segmentation on current WSI has already been performed.", this);
            return false;
        }

        ProcessManager::GetInstance()->runProcess(DataManager::GetInstance()->getVisibleImageName(), "tissue");
        emit processTriggered("tissue");
        emit addRendererToViewRequested("tissue");
    }

    bool ProcessWidget::processStartEventReceived(std::string process_name)
    {
        if (DataManager::GetInstance()->getCurrentProject()->isProjectEmpty()) {
            std::cout << "Requires a WSI to be rendered in order to perform the analysis." << std::endl;
            simpleInfoPrompt(QString("Requires a WSI to be rendered in order to perform the analysis."), this);
            return false;
        }

        if (DataManager::GetInstance()->getVisibleImageName() == "")
        {
            std::cout << "Requires a WSI to be rendered in order to perform the analysis." << std::endl;
            return false;
        }

        auto visible_image = DataManager::GetInstance()->get_visible_image();

        // prompt if you want to run the analysis again, if it has already been ran
        if (visible_image->has_renderer(process_name)) {
            simpleInfoPrompt("Segmentation on current WSI has already been performed.", this);
            return false;
        }

//        if(ProcessManager::GetInstance()->get_advanced_mode_status())

        auto progDialog = QProgressDialog();
        progDialog.setRange(0, 1); //currentWSIs.size() <= total number of WSIs to process
        //progDialog.setContentsMargins(0, 0, 0, 0);
        progDialog.setValue(0);
        progDialog.setVisible(true);
        progDialog.setModal(false);
        progDialog.setLabelText("Running inference...");
        //QRect screenrect = mWidget->screen()[0].geometry();
        progDialog.move(this->width() - progDialog.width() * 1.1, progDialog.height() * 0.1);
        progDialog.show();

        QCoreApplication::processEvents(QEventLoop::AllEvents, 0);

        ProcessManager::GetInstance()->runProcess(DataManager::GetInstance()->getVisibleImageName(), process_name);
        emit processTriggered(process_name);
        emit addRendererToViewRequested(process_name);
    }

    void ProcessWidget::editorPipelinesReceived()
    {
        auto editor = new PipelineScriptEditorWidget(this);
    }

    void ProcessWidget::runPipelineReceived(QString pipeline_uid)
    {
        emit processTriggered(pipeline_uid.toStdString());
        emit runPipelineEmitted(pipeline_uid);
    }

    void ProcessWidget::updateWSIs() {
        std::cout << "Running update WSIs" << std::endl;
        auto manager = DataManager::GetInstance();
        auto WSI = manager->get_visible_image();
        if(WSI) {
            std::cout << "Adding a WSI" << std::endl;
            m_WSIs.push_back({WSI->get_filename(), WSI->get_image_pyramid()});
        }
        for(auto uid : manager->getCurrentProject()->getAllWsiUids()) {
            auto WSI = manager->getCurrentProject()->getImage(uid);
            m_WSIs.push_back({WSI->get_filename(), WSI->get_image_pyramid()});
        }
    }
}
