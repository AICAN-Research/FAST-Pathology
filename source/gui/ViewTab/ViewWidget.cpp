//
// Created by dbouget on 02.11.2021.
//

#include "ViewWidget.h"

#include <FAST/Importers/WholeSlideImageImporter.hpp>
#include <FAST/Visualization/ImagePyramidRenderer/ImagePyramidRenderer.hpp>
#include <FAST/Data/ImagePyramid.hpp>
#include <FAST/Visualization/SegmentationRenderer/SegmentationRenderer.hpp>

namespace fast {
    ViewWidget::ViewWidget(QWidget *parent): QWidget(parent){
        this->setupInterface();
        this->setupConnections();
    }

    ViewWidget::~ViewWidget(){

    }

    void ViewWidget::setupInterface()
    {
        this->_main_layout = new QVBoxLayout(this);
        this->_main_layout->setAlignment(Qt::AlignTop);
        //this->_main_layout->setFixedWidth(200);

        // ComboBox in view section to set which image object to change
//        auto curr1PageWidget = new QWidget;
//        auto curr2PageWidget = new QWidget;
//        auto curr3PageWidget = new QWidget;
//        auto curr4PageWidget = new QWidget;
//        auto curr5PageWidget = new QWidget;

//        auto wsiPageWidget = new QWidget;

        this->_stacked_layout = new QStackedLayout;

        this->_stacked_widget= new QWidget(this);
        this->_stacked_widget->setLayout(this->_stacked_layout);

        this->_page_combobox = new QComboBox(this);
        this->_page_combobox->setFixedWidth(150);

        auto imageNameTexts = new QLabel(this);
        imageNameTexts->setText("Image: ");
        imageNameTexts->setFixedWidth(50);
        auto smallTextBox_imageName = new QHBoxLayout;
        smallTextBox_imageName->addWidget(imageNameTexts);
        smallTextBox_imageName->addWidget(this->_page_combobox);
        auto smallTextBoxWidget_imageName = new QWidget(this);
        smallTextBoxWidget_imageName->setFixedHeight(50);
        smallTextBoxWidget_imageName->setLayout(smallTextBox_imageName);

        this->_main_layout->insertWidget(0, smallTextBoxWidget_imageName);
        this->_main_layout->insertWidget(1, this->_stacked_widget);
        this->hide();
    }

    void ViewWidget::resetInterface()
    {
        this->_dynamic_widget_list.clear();
        this->_page_combobox->clear();
        this->hide();
    }

    void ViewWidget::setupConnections()
    {
//        QObject::connect(this->_page_combobox, &QComboBox::activated, this->_stacked_layout, &QStackedLayout::setCurrentIndex);
        QObject::connect(this->_page_combobox, SIGNAL(activated(int)), this->_stacked_layout, SLOT(setCurrentIndex(int)));
    }

    void ViewWidget::processTriggerUpdate(std::string process_name)
    {
        if (this->_dynamic_widget_list.empty())
        {
            auto dynamic_widget = new DynamicViewTabWidget("WSI", this);
            this->_dynamic_widget_list["WSI"] = dynamic_widget;
            this->_stacked_layout->addWidget(dynamic_widget);
            this->_page_combobox->insertItem(0, "WSI");
            this->show();
        }

        auto dynamic_widget = new DynamicViewTabWidget(process_name, this);
        this->_dynamic_widget_list[process_name] = dynamic_widget;
        this->_stacked_layout->addWidget(dynamic_widget);
        this->_page_combobox->insertItem(this->_page_combobox->count(), QString::fromStdString(process_name));
    }

    void ViewWidget::createDynamicViewWidget(std::string someName, std::string modelName)
    {
        auto dynamicViewWidget = new QWidget(this);

        auto imageButton = new QPushButton(dynamicViewWidget);
        imageButton->setText("Toggle image");
        imageButton->setFixedHeight(50);
        imageButton->setCheckable(true);
        imageButton->setChecked(true);
        QObject::connect(imageButton, &QPushButton::clicked, std::bind(&ViewWidget::toggleRenderer, this, someName));

        // for WSI this should be grayed out, shouldn't be able to change it
        auto opacitySlider = new QSlider(Qt::Horizontal, dynamicViewWidget);
        opacitySlider->setMinimum(0);
        opacitySlider->setMaximum(20);
        opacitySlider->setValue(8);
        opacitySlider->setTickInterval(1);
        QObject::connect(opacitySlider, &QSlider::valueChanged, std::bind(&ViewWidget::opacityRenderer, this, std::placeholders::_1, someName));

        auto label_tissue = new QLabel(dynamicViewWidget);
        std::string tmpSomeName = someName;
        tmpSomeName[0] = toupper(tmpSomeName[0]);
        label_tissue->setText(QString::fromStdString(tmpSomeName + ": "));
        label_tissue->setFixedWidth(50);
        auto smallTextBox_tissue = new QHBoxLayout(dynamicViewWidget);
        smallTextBox_tissue->addWidget(label_tissue);
        smallTextBox_tissue->addWidget(opacitySlider);
        auto smallTextBoxWidget_tissue = new QWidget(dynamicViewWidget);
        smallTextBoxWidget_tissue->setFixedHeight(50);
        smallTextBoxWidget_tissue->setLayout(smallTextBox_tissue);

        // make QColorDialog for manually setting color to different classes
        auto colorSetWidget = new QColorDialog(dynamicViewWidget);
        colorSetWidget->setOption(QColorDialog::DontUseNativeDialog, true);

        auto colorButton = new QPushButton(dynamicViewWidget);
        colorButton->setFixedHeight(50);
        colorButton->setText("Set color");
        colorButton->setChecked(true);

        // to be able to set which classes to show/hide
        int channel_value = 0;

        auto toggleShowButton = new QPushButton(dynamicViewWidget);
        toggleShowButton->setText("Toggle class");
        toggleShowButton->setFixedHeight(50);
        toggleShowButton->setCheckable(true);
        toggleShowButton->setChecked(true);
//        QObject::connect(toggleShowButton, &QPushButton::clicked, std::bind(&MainWindow::hideChannel, this, someName));

        auto deleteButton = new QPushButton(dynamicViewWidget);
        deleteButton->setText("Delete object");
        deleteButton->setFixedHeight(50);
//        QObject::connect(deleteButton, &QPushButton::clicked, std::bind(&MainWindow::deleteViewObject, this, someName));
        deleteButton->setStyleSheet("color: black; background-color: red");


        auto currComboBox = new QComboBox(dynamicViewWidget);

//        // additional options for setting which classes to show and colors
//        if (m_rendererTypeList[someName] == "HeatmapRenderer") {
//            // get metadata of current model
//            std::map<std::string, std::string> metadata = getModelMetadata(modelName);
//            std::vector someVector = splitCustom(metadata["class_names"], ";");
//            // clear vector first
//            currentClassesInUse.clear();
//            for (const auto & i : someVector){
//                currentClassesInUse.append(QString::fromStdString(i));
//            }
//            currComboBox->clear();
//            currComboBox->update();
//            currComboBox->insertItems(0, currentClassesInUse);

//            QObject::connect(colorButton, &QPushButton::clicked, [=]() {
//                auto rgb = colorSetWidget->getColor().toRgb();

//                auto someRenderer = std::dynamic_pointer_cast<HeatmapRenderer>(m_rendererList[someName]);
//                someRenderer->setChannelColor(currComboBox->currentIndex(), Color((float)(rgb.red() / 255.0f), (float)(rgb.green() / 255.0f), (float)(rgb.blue() / 255.0f)));
//            });
//        } else if(m_rendererTypeList[someName] == "SegmentationRenderer") {
//            // clear vector first
//            currentClassesInUse.clear();
//            for (const auto & i : { 1 }) { //{ 0, 1 }) {  // TODO: Supports only binary images (where class of interest = 1)
//                currentClassesInUse.append(QString::number(i));
//            }
//            currComboBox->clear();
//            currComboBox->update();
//            currComboBox->insertItems(0, currentClassesInUse);

//            QObject::connect(colorButton, &QPushButton::clicked, [=]() {
//                auto rgb = colorSetWidget->getColor().toRgb();
//                auto someRenderer = std::dynamic_pointer_cast<SegmentationRenderer>(m_rendererList[someName]);
//                //auto currImage = someRenderer->updateAndGetOutputData<Image>();
//                //currImage->
//                //std::cout << "window: " << someRenderer->updateAndGetOutputData<Image>() << std::endl;
//                //auto vals = someRenderer->getIntensityWindow();

//                // TODO: Supports only binary images (where class of interest = 1)
//                someRenderer->setColor(currComboBox->currentIndex() + 1, Color((float)(rgb.red() / 255.0f), (float)(rgb.green() / 255.0f), (float)(rgb.blue() / 255.0f)));
//            });
//        // TODO: Need to add proper options in SegmentationPyramidRenderer and SegmentationRenderer for toggling and setting which classes to show, do to this properly...
//        } else if (m_rendererTypeList[someName] == "SegmentationPyramidRenderer") {
//            // get metadata of current model
//            std::map<std::string, std::string> metadata = getModelMetadata(modelName);
//            std::vector someVector = splitCustom(metadata["class_names"], ";");
//            // clear vector first
//            currentClassesInUse.clear();
//            for (const auto & i : someVector) {
//                currentClassesInUse.append(QString::fromStdString(i));
//            }
//            currComboBox->clear();
//            currComboBox->update();
//            currComboBox->insertItems(0, currentClassesInUse);

//            QObject::connect(colorButton, &QPushButton::clicked, [=]() {
//                auto rgb = colorSetWidget->getColor().toRgb();
//                auto someRenderer = std::dynamic_pointer_cast<SegmentationRenderer>(m_rendererList[someName]);
//                someRenderer->setColor(currComboBox->currentIndex() + 1, Color((float)(rgb.red() / 255.0f), (float)(rgb.green() / 255.0f), (float)(rgb.blue() / 255.0f)));
//                //someRenderer->setChannelColor(currComboBox->currentIndex(), Color((float)(rgb.red() / 255.0f), (float)(rgb.green() / 255.0f), (float)(rgb.blue() / 255.0f)));
//            });
//        } else if (m_rendererTypeList[someName] == "BoundingBoxRenderer") {
//            // get metadata of current model
//            std::map<std::string, std::string> metadata = getModelMetadata(modelName);
//            std::vector someVector = splitCustom(metadata["class_names"], ";");
//            // clear vector first
//            currentClassesInUse.clear();
//            for (const auto & i : someVector) {
//                currentClassesInUse.append(QString::fromStdString(i));
//            }
//            currComboBox->clear();
//            currComboBox->update();
//            currComboBox->insertItems(0, currentClassesInUse);

//            QObject::connect(colorButton, &QPushButton::clicked, [=]() {
//                auto rgb = colorSetWidget->getColor().toRgb();
//                auto someRenderer = std::dynamic_pointer_cast<BoundingBoxRenderer>(m_rendererList[someName]);
//                //someRenderer->setLabelColor(currComboBox->currentIndex(), Color((float)(rgb.red() / 255.0f), (float)(rgb.green() / 255.0f), (float)(rgb.blue() / 255.0f)));
//            });
//        } else {
//            simpleInfoPrompt("Invalid renderer used...");
//        }

//        connect(currComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateChannelValue(int)));

//        auto imageNameTexts = new QLabel();
//        imageNameTexts->setText("Class: ");
//        imageNameTexts->setFixedWidth(50);
//        auto smallTextBox_imageName = new QHBoxLayout;
//        smallTextBox_imageName->addWidget(imageNameTexts);
//        smallTextBox_imageName->addWidget(currComboBox);

//        auto smallTextBoxWidget_imageName = new QWidget;
//        smallTextBoxWidget_imageName->setFixedHeight(50);
//        smallTextBoxWidget_imageName->setLayout(smallTextBox_imageName);
//        auto biggerTextBox_imageName = new QVBoxLayout;
//        biggerTextBox_imageName->addWidget(smallTextBoxWidget_imageName);
//        biggerTextBox_imageName->setAlignment(Qt::AlignTop);

//        auto smallToggleColorBox_layout = new QHBoxLayout;
//        smallToggleColorBox_layout->addWidget(toggleShowButton);
//        smallToggleColorBox_layout->addWidget(colorButton);
//        auto smallToggleColorBox = new QWidget;
//        smallToggleColorBox->setLayout(smallToggleColorBox_layout);
//        auto biggerTextBoxWidget_imageName = new QWidget;
//        biggerTextBoxWidget_imageName->setLayout(smallToggleColorBox_layout);

//        // disable some features for specific renderer types
//        if (m_rendererTypeList[someName] == "ImagePyramidRenderer") {
//            opacitySlider->setDisabled(true);
//            colorSetWidget->setDisabled(true);
//            biggerTextBoxWidget_imageName->setDisabled(true);
//        } else if (m_rendererTypeList[someName] == "SegmentationRenderer") {
//            toggleShowButton->setDisabled(true);
//        } else if (m_rendererTypeList[someName] == "SegmentationPyramidRenderer") {
//            toggleShowButton->setDisabled(true);
//            //colorSetWidget->setDisabled(true);
//            //biggerTextBoxWidget_imageName->setDisabled(true);
//        } else if (m_rendererTypeList[someName] == "BoundingBoxRenderer") {
//            1;
//        } else {
//            colorSetWidget->setDisabled(false);
//        }

        auto allBox = new QGroupBox(tr("Modify image"), this);
        auto classBox = new QGroupBox(tr("Modify class"), this);

        auto dynamicViewLayout = new QVBoxLayout;
        dynamicViewLayout->setAlignment(Qt::AlignTop);
        dynamicViewWidget->setLayout(dynamicViewLayout);

        auto allViewLayout = new QVBoxLayout;
        allViewLayout->addWidget(imageButton);
        allViewLayout->addWidget(smallTextBoxWidget_tissue);

        allBox->setLayout(allViewLayout);

        // clear vector first
        QObject::connect(colorButton, &QPushButton::clicked, [=]() {
            auto rgb = colorSetWidget->getColor().toRgb();
            auto someRenderer = std::dynamic_pointer_cast<SegmentationRenderer>(DataManager::GetInstance()->get_visible_image()->get_renderer(someName));
            //auto currImage = someRenderer->updateAndGetOutputData<Image>();
            //currImage->
            //std::cout << "window: " << someRenderer->updateAndGetOutputData<Image>() << std::endl;
            //auto vals = someRenderer->getIntensityWindow();

            // TODO: Supports only binary images (where class of interest = 1)
            someRenderer->setColor(currComboBox->currentIndex() + 1, Color((float)(rgb.red() / 255.0f), (float)(rgb.green() / 255.0f), (float)(rgb.blue() / 255.0f)));
        });

        auto classViewLayout = new QVBoxLayout;
//        classViewLayout->addWidget(smallTextBoxWidget_imageName);
//        classViewLayout->addWidget(biggerTextBoxWidget_imageName);

        classBox->setLayout(classViewLayout);

        dynamicViewLayout->addWidget(allBox);
        dynamicViewLayout->addWidget(classBox);
        dynamicViewLayout->addWidget(colorSetWidget);
        dynamicViewLayout->addWidget(deleteButton);

        this->_stacked_layout->addWidget(dynamicViewWidget);
//        // add widget to QComboBox
//        pageComboBox->addItem(tr(tmpSomeName.c_str()));
//        stackedLayout->addWidget(dynamicViewWidget);
    }

    bool ViewWidget::hideChannel(const std::string& name) {
//        if (m_rendererTypeList[name] != "HeatmapRenderer") {
//            return false;
//        }
//        if (!DataManager::GetInstance()->get_visible_image()->has_renderer(name))
//            return false;
//        else
//        {
//            background_flag = !background_flag;
//            auto someRenderer = std::dynamic_pointer_cast<HeatmapRenderer>(getRenderer(name));
//            someRenderer->setChannelHidden(channel_value, background_flag);
//            return true;
//        }
        return false;
    }

    void ViewWidget::toggleRenderer(std::string someName)
    {
        auto renderer = DataManager::GetInstance()->get_visible_image()->get_renderer("tissue");
        renderer->setDisabled(!renderer->isDisabled());
    }

    bool ViewWidget::opacityRenderer(int value, const std::string& name) {
        if (!DataManager::GetInstance()->get_visible_image()->has_renderer(name))
            return false;
        else
        {
            auto someRenderer = std::dynamic_pointer_cast<SegmentationRenderer>(DataManager::GetInstance()->get_visible_image()->get_renderer(name));
            someRenderer->setOpacity((float) value / 20.0f);

//            if (m_rendererTypeList[name] == "SegmentationRenderer") {
//                auto someRenderer = std::dynamic_pointer_cast<SegmentationRenderer>(getRenderer(name));
//                someRenderer->setOpacity((float) value / 20.0f);
//            } else if (m_rendererTypeList[name] == "SegmentationRenderer") { // FIXME: Apparently, this doesn't change opacity
//                auto someRenderer = std::dynamic_pointer_cast<SegmentationRenderer>(getRenderer(name));
//                someRenderer->setOpacity((float) value / 20.0f);
//            } else {
//                auto someRenderer = std::dynamic_pointer_cast<HeatmapRenderer>(getRenderer(name));
//                someRenderer->setMaxOpacity((float) value / 20.0f);
//                //someRenderer->setMinConfidence(0.9);
//            }
            return true;
        }
    }
} // End of namespace fast
