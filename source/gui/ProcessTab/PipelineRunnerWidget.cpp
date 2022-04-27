//
// Created by dbouget on 16.11.2021.
//

#include "PipelineRunnerWidget.h"


namespace fast
{
    PipelineRunnerWidget::PipelineRunnerWidget(QString id, QWidget *parent): _pipeline_uid(id), QWidget(parent)
    {
        this->setupInterface();
        this->setupConnections();
    }

    PipelineRunnerWidget::~PipelineRunnerWidget()
    {

    }

    void PipelineRunnerWidget::setupInterface()
    {
        this->_main_layout = new QHBoxLayout(this);
        this->_run_pushbutton = new QPushButton();
        this->_run_pushbutton->setText(this->_pipeline_uid);
        this->_run_pushbutton->setFixedHeight(50);

        this->_edit_pushbutton = new QPushButton();
//        auto pixmap = QPixmap(QString::fromStdString(":/data/Icons/cogwheel_process_icon.png"));
        QPixmap editPix(QString::fromStdString(":/data/Icons/cogwheel_process_icon.png"));
        this->_edit_pushbutton->setIcon(QIcon(editPix));
        // this->_edit_pushbutton->setIconSize(editPix.rect().size());
        this->_edit_pushbutton->setIconSize(QSize(40, 40));
        this->_edit_pushbutton->setFixedHeight(50);
        this->_edit_pushbutton->setFixedWidth(50);
        this->_edit_pushbutton->setToolTip(QString::fromStdString("Edit the pipeline..."));

        this->_delete_pushbutton = new QPushButton();
        this->_delete_pushbutton->setIcon(QIcon(QString::fromStdString(":/data/Icons/trash_delete_icon.jpeg")));
        this->_delete_pushbutton->setIconSize(editPix.rect().size());
        this->_delete_pushbutton->setFixedHeight(50);
        this->_delete_pushbutton->setFixedWidth(50);
        this->_delete_pushbutton->setToolTip(QString::fromStdString("Delete the pipeline..."));

        this->_main_layout->addWidget(this->_run_pushbutton);
        this->_main_layout->addWidget(this->_edit_pushbutton);
        this->_main_layout->addWidget(this->_delete_pushbutton);
    }


    void PipelineRunnerWidget::setupConnections()
    {
        QObject::connect(this->_run_pushbutton, &QPushButton::clicked, this, &PipelineRunnerWidget::runPipelineReceived);
//        QObject::connect(this->_run_pushbutton, &QPushButton::clicked, std::bind(&PipelineRunnerWidget::runPipelineEmitted, this, this->_pipeline_uid));
        QObject::connect(this->_edit_pushbutton, &QPushButton::clicked, this, &PipelineRunnerWidget::editPipelineReceived);
        QObject::connect(this->_edit_pushbutton, &QPushButton::clicked, std::bind(&PipelineRunnerWidget::editPipelineEmitted, this, this->_pipeline_uid));
        QObject::connect(this->_delete_pushbutton, &QPushButton::clicked, std::bind(&PipelineRunnerWidget::deletePipelineEmitted, this, this->_pipeline_uid));
    }

    void PipelineRunnerWidget::runPipelineReceived()
    {
        std::map<std::string, std::string> arguments;
        arguments["filename"] = DataManager::GetInstance()->get_visible_image()->get_filename();
        arguments["exportPath"] = "/home/dbouget/Desktop/fp-autoexport.tiff";
        arguments["modelPath"] = ProcessManager::GetInstance()->get_models_filepath();
        ProcessManager::GetInstance()->get_pipeline(this->_pipeline_uid.toStdString())->setParameters(arguments);
        ProcessManager::GetInstance()->runPipeline(this->_pipeline_uid.toStdString());

//        connect(ProcessManager::GetInstance()->get_pipeline(this->_pipeline_uid.toStdString()).get(), &PipelineProcess::currentPipelineFinished, this, &PipelineRunnerWidget::onPipelineFinished);
//        DataManager::GetInstance()->get_visible_image()->insert_renderer(this->_pipeline_uid.toStdString(), "SegmentationRenderer", *ProcessManager::GetInstance()->get_pipeline(this->_pipeline_uid.toStdString())->getRenderers().begin());
        emit runPipelineEmitted(this->_pipeline_uid);
//        emit addRendererToViewRequested(this->_pipeline_uid.toStdString());
    }

    void PipelineRunnerWidget::editPipelineReceived()
    {
        //@TODO. When started from here, should disable the menu options to create a new script or load an existing script, as it is only meant for editing the current one.
        auto editor = new PipelineScriptEditorWidget(QString::fromStdString(ProcessManager::GetInstance()->get_pipeline(this->_pipeline_uid.toStdString())->getPipelineFullFilename()), this);
    }

    void PipelineRunnerWidget::onPipelineFinished()
    {
        std::cout<<"The pipeline is fully finished."<<std::endl;
    }
} // End of namespace fast
