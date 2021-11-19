//
// Created by dbouget on 02.11.2021.
//

#include "ProcessWidget.h"

#include <FAST/Importers/WholeSlideImageImporter.hpp>
#include <FAST/Visualization/ImagePyramidRenderer/ImagePyramidRenderer.hpp>
#include <FAST/Data/ImagePyramid.hpp>
#include <FAST/Visualization/SegmentationRenderer/SegmentationRenderer.hpp>

namespace fast {
    ProcessWidget::ProcessWidget(QWidget *parent): QWidget(parent){
        this->_cwd = QDir::homePath().toStdString();
        this->_cwd += "/fastpathology/";
        this->setupInterface();
        this->setupConnections();
    }

    ProcessWidget::~ProcessWidget(){

    }

    void ProcessWidget::setupInterface()
    {
        this->_main_layout = new QVBoxLayout(this);
        this->_main_layout->setSpacing(6);
        this->_main_layout->setAlignment(Qt::AlignTop);

//        this->_tissue_seg_pushbutton = new QPushButton(this);
//        this->_tissue_seg_pushbutton->setText("Segment Tissue");
//        //segTissueButton->setFixedWidth(200);
//        this->_tissue_seg_pushbutton->setFixedHeight(50);

//        // add tissue segmentation to widget (should always exist as built-in FAST)
//        this->_main_layout->insertWidget(0, this->_tissue_seg_pushbutton);

//        QDir directory(QString::fromStdString(this->_cwd + "data/Models/"));
//        QStringList model_dirs = directory.entryList(QDir::NoDot | QDir::NoDotDot | QDir::Dirs);
//        int counter=1;
//        foreach(QString model_dir, model_dirs)
//        {
//            ProcessManager::GetInstance()->importModel(model_dir.toStdString());

//            auto someButton = new QPushButton(this);
//            someButton->setText(QString::fromStdString(ProcessManager::GetInstance()->get_model(model_dir.toStdString())->get_model_metadata()["task"]));
//            //predGradeButton->setFixedWidth(200);
//            someButton->setFixedHeight(50);
////            QObject::connect(someButton, &QPushButton::clicked, std::bind(&MainWindow::pixelClassifier_wrapper, this, modelName));
//            someButton->show();

//            this->_main_layout->insertWidget(counter, someButton);
//            // @TODO. Has to be thought further, maybe not needed to store those buttons?
//            this->_specific_seg_models_pushbutton_map[std::to_string(counter)] = someButton;
//            QObject::connect(someButton, &QPushButton::clicked, std::bind(&ProcessWidget::processStartEventReceived, this, model_dir.toStdString()));

//            counter++;
//        }

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
        std::ifstream infile(this->_cwd + "data/Models/" + modelName + ".txt");
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

//        int counter = 0;
//        // now iterate over all selected files and add selected files and corresponding ones to Models/
//        for (QString& fileName : ls) {

//            if (fileName == "")
//                return;

//            std::string someFile = splitCustom(fileName.toStdString(), "/").back(); // TODO: Need to make this only split on last "/"
//            std::string oldLocation = splitCustom(fileName.toStdString(), someFile)[0];
//            std::string newLocation = cwd + "data/Models/";

//            std::vector<string> names = splitCustom(someFile, ".");
//            string fileNameNoFormat = names[0];
//            string formatName = names[1];

//            // copy selected file to Models folder
//            // check if file already exists in new folder, if yes, print warning, and stop
//            string newPath = cwd + "data/Models/" + someFile;
//            if (fileExists(newPath)) {
//                std::cout << "file with the same name already exists in folder, didn't transfer... " << std::endl;
//                progDialog.setValue(counter);
//                counter++;
//                continue;
//            }

//            // check which corresponding model files that exist, except from the one that is chosen
//            std::vector<std::string> allowedFileFormats{"txt", "pb", "h5", "mapping", "xml", "bin", "uff", "anchors", "onnx", "fpl"};

//            std::cout << "Copy test" << std::endl;
//            foreach(std::string currExtension, allowedFileFormats) {
//                std::string oldPath = oldLocation + fileNameNoFormat + "." + currExtension;
//                if (fileExists(oldPath)) {
//                    QFile::copy(QString::fromStdString(oldPath), QString::fromStdString(cwd + "data/Models/" + fileNameNoFormat + "." + currExtension));
//                }
//            }

//            // when models are added, ProcessWidget should be updated by adding the new widget to ProcessWidget layout
//            // current model
//            modelName = fileNameNoFormat;

//            // get metadata of current model
//            std::map<std::string, std::string> metadata = getModelMetadata(modelName);

//            auto someButton = new QPushButton(mWidget);
//            someButton->setText(QString::fromStdString(metadata["task"]));
//            //predGradeButton->setFixedWidth(200);
//            someButton->setFixedHeight(50);
//            QObject::connect(someButton, &QPushButton::clicked,
//                             std::bind(&MainWindow::pixelClassifier_wrapper, this, modelName));
//            someButton->show();

//            processLayout->insertWidget(processLayout->count(), someButton);

//            progDialog.setValue(counter);
//            QCoreApplication::processEvents(QEventLoop::AllEvents, 0);
//            counter++;
//        }
    }

    void ProcessWidget::addPipelines()
    {
        auto counter = 1;
        auto pipelines = ProcessManager::GetInstance()->getAllPipelines();
        for(auto it = pipelines.begin(); it != pipelines.end(); it++)
        {
            std::string pipeline_uid = it->first;
            auto runner_widget = new PipelineRunnerWidget(QString::fromStdString(pipeline_uid), this);
            this->_main_layout->insertWidget(counter, runner_widget);
            QObject::connect(runner_widget, &PipelineRunnerWidget::runPipelineEmitted, this, &ProcessWidget::runPipelineReceived);
            QObject::connect(runner_widget, &PipelineRunnerWidget::deletePipelineEmitted, this, &ProcessWidget::deletePipelineReceived);
            this->_pipeline_runners_map[pipeline_uid] = runner_widget;
            counter++;
        }
    }

    void ProcessWidget::runPipelineReceived(QString pipeline_uid)
    {

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
}