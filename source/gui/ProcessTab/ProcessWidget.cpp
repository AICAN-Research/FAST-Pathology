//
// Created by dbouget on 02.11.2021.
//

#include "ProcessWidget.h"

#include <FAST/Importers/WholeSlideImageImporter.hpp>
#include <FAST/Visualization/ImagePyramidRenderer/ImagePyramidRenderer.hpp>
#include <FAST/Data/ImagePyramid.hpp>

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

//        processWidget = new QWidget(this);
//        processWidget->setLayout(processLayout);

        this->_tissue_seg_pushbutton = new QPushButton(this);
        this->_tissue_seg_pushbutton->setText("Segment Tissue");
        //segTissueButton->setFixedWidth(200);
        this->_tissue_seg_pushbutton->setFixedHeight(50);

        // add tissue segmentation to widget (should always exist as built-in FAST)
        this->_main_layout->insertWidget(0, this->_tissue_seg_pushbutton);

        QDir directory(QString::fromStdString(this->_cwd + "data/Models/"));
        QStringList paths = directory.entryList(QStringList() << "*.txt" << "*.TXT",QDir::Files);
        int counter=1;
        foreach(QString currFile, paths) {

            // current model
            std::string modelName = splitCustom(currFile.toStdString(), ".")[0];

            // get metadata of current model
            std::map<std::string, std::string> metadata = getModelMetadata(modelName);

            auto someButton = new QPushButton(this);
            someButton->setText(QString::fromStdString(metadata["task"]));
            //predGradeButton->setFixedWidth(200);
            someButton->setFixedHeight(50);
//            QObject::connect(someButton, &QPushButton::clicked, std::bind(&MainWindow::pixelClassifier_wrapper, this, modelName));
            someButton->show();

            this->_main_layout->insertWidget(counter, someButton);
            // @TODO. Has to be thought further, when I have models to run.
            this->_specific_seg_models_pushbutton_map[std::to_string(counter)] = someButton;

            counter++;
        }

//        std::vector<string> modelPaths;
    }

    void ProcessWidget::resetInterface()
    {
        return;
    }

    void ProcessWidget::setupConnections()
    {
        QObject::connect(this->_tissue_seg_pushbutton, &QPushButton::clicked, this, &ProcessWidget::segmentTissue);
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
        QStringList ls = QFileDialog::getOpenFileNames(this, tr("Select Pipeline"), nullptr,
                tr("Pipeline Files (*.fpl)"),
                nullptr, QFileDialog::DontUseNativeDialog
        );

//        // now iterate over all selected files and add selected files and corresponding ones to Pipelines/
//        for (QString& fileName : ls) {

//            if (fileName == "")
//                continue;

//            std::string someFile = splitCustom(fileName.toStdString(), "/").back();
//            std::string oldLocation = splitCustom(fileName.toStdString(), someFile)[0];
//            std::string newLocation = cwd + "data/Pipelines/";
//            std::string newPath = cwd + "data/Pipelines/" + someFile;
//            if (fileExists(newPath)) {
//                std::cout << fileName.toStdString() << " : " << "File with the same name already exists in folder, didn't transfer..." << std::endl;
//                continue;
//            } else {
//                if (fileExists(fileName.toStdString())) {
//                    QFile::copy(fileName, QString::fromStdString(newPath));
//                }
//            }
//            // should update runPipelineMenu as new pipelines are being added
//            //runPipelineMenu->addAction(QString::fromStdString(splitCustom(someFile, ".")[0]), this, &MainWindow::runPipeline);
//            //auto currentAction = new QAction(QString::fromStdString(splitCustom(someFile, ".")[0]));
//            auto currentAction = runPipelineMenu->addAction(QString::fromStdString(splitCustom(someFile, ".")[0]));
//            QObject::connect(currentAction, &QAction::triggered, std::bind(&MainWindow::runPipeline, this, someFile));

//            //auto currentAction = runPipelineMenu->addAction(currentFpl); //QString::fromStdString(splitCustom(splitCustom(currentFpl.toStdString(), "/")[-1], ".")[0]));
//            //QObject::connect(currentAction, &QAction::triggered, std::bind(&MainWindow::runPipeline, this, cwd + "data/Pipelines/" + currentFpl.toStdString()));
//        }
    }

    bool ProcessWidget::segmentTissue()
    {
        // @TODO. All of it should be within a ProcessManager most likely, no reason to have it inside here.

        // @TODO. Does the segmentation run only on the visible WSI, or on all loaded WSIs showing on the side list?
        if (DataManager::GetInstance()->isEmpty()) {
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
    }
}
