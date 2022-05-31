#include "ViewWidget.h"
#include <FAST/Importers/WholeSlideImageImporter.hpp>
#include <FAST/Visualization/ImagePyramidRenderer/ImagePyramidRenderer.hpp>
#include <FAST/Data/ImagePyramid.hpp>
#include <FAST/Visualization/SegmentationRenderer/SegmentationRenderer.hpp>
#include <FAST/Visualization/HeatmapRenderer/HeatmapRenderer.hpp>
#include <QCheckBox>

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

        this->_stacked_layout = new QStackedLayout;

        this->_stacked_widget= new QWidget(this);
        this->_stacked_widget->setLayout(this->_stacked_layout);

        this->_page_combobox = new QComboBox(this);

        _main_layout->addWidget(_page_combobox);
        _main_layout->addWidget(_stacked_widget);
        setVisible(false);
        hide();
    }

    void ViewWidget::resetInterface()
    {
        this->_page_combobox->clear();
        // Clear stacked layout
        auto layout = _stacked_layout;
        QLayoutItem *item;
        while((item = layout->takeAt(0))) {
            if (item->widget())
                delete item->widget();
            delete item;
        }
        // end clear
        this->setVisible(false);
        this->hide();
    }

    void ViewWidget::setupConnections()
    {
//        QObject::connect(this->_page_combobox, &QComboBox::activated, this->_stacked_layout, &QStackedLayout::setCurrentIndex);
        QObject::connect(this->_page_combobox, SIGNAL(activated(int)), this->_stacked_layout, SLOT(setCurrentIndex(int)));
    }

    void ViewWidget::deleteViewObjectReceived(std::string uid)
    {
        // If there was only one results in addition to the main WSI, and the said result is removed,
        // then the WSI is also removed, hence the full interface reset.
        if (this->_dynamic_widget_list.size() == 2)
        {
            this->resetInterface();
        }
        else
        {
            this->_page_combobox->removeItem(this->_page_combobox->currentIndex());
            this->_dynamic_widget_list.erase(this->_dynamic_widget_list.find(uid));
        }
        emit removeRendererFromViewRequested(uid);
        DataManager::GetInstance()->get_visible_image()->remove_renderer(uid);
    }

    void ViewWidget::processTriggerUpdate(std::string process_name)
    {
        if (this->_dynamic_widget_list.empty())
        {
            auto dynamic_widget = new DynamicViewTabWidget("WSI", this);
            // Not connecting the delete button for the WSI widget. Will be added with the first View results, and deleted if no more viewed results, automatically.
            this->_dynamic_widget_list["WSI"] = dynamic_widget;
            this->_stacked_layout->addWidget(dynamic_widget);
            this->_page_combobox->insertItem(0, "WSI");
            this->setVisible(true);
            this->show();
        }

        auto dynamic_widget = new DynamicViewTabWidget(process_name, this);
        QObject::connect(dynamic_widget, &DynamicViewTabWidget::deleteViewObjectTriggered, this, &ViewWidget::deleteViewObjectReceived);
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

    void ViewWidget::writeRendererAttributes(Result result) {
        if(result.renderer->getNameOfClass() == "ImagePyramidRenderer")
            return;
        auto project = DataManager::GetInstance()->getCurrentProject();
        const std::string saveFolder = join(project->getRootFolder(), "results", result.WSI_uid, result.pipelineName, result.name);
        std::ofstream file(join(saveFolder, "attributes.txt"), std::iostream::out);
        file << result.renderer->attributesToString();
        file.close();
    }

    void ViewWidget::setResults(std::vector<Result> results) {
        resetInterface();
        // Create layout for all results
        for(auto result : results) {
            auto renderer = result.renderer;
            auto page = new QWidget();
            auto layout = new QVBoxLayout();
            layout->setAlignment(Qt::AlignTop);
            page->setLayout(layout);
            _stacked_layout->addWidget(page);
            _page_combobox->addItem(QString::fromStdString(result.pipelineName) + ": " + QString::fromStdString(result.name));

            // Toggle renderer on and off
            auto toggleButton = new QPushButton();
            toggleButton->setText("Toggle");
            layout->addWidget(toggleButton);
            QObject::connect(toggleButton, &QPushButton::clicked, [renderer, result, this]() {
                renderer->setDisabled(!renderer->isDisabled());
                writeRendererAttributes(result);
            });

            auto rendererType = result.renderer->getNameOfClass();
            if(rendererType == "SegmentationRenderer") { // TODO Move to separate methods
                auto segRenderer = std::dynamic_pointer_cast<SegmentationRenderer>(renderer);

                // Opacity
                {
                    auto label = new QLabel();
                    label->setText("Opacity:");
                    layout->addWidget(label);
                    auto slider = new QSlider(Qt::Horizontal);
                    slider->setRange(0, 100);
                    slider->setValue(segRenderer->getOpacity()*100.0f);
                    QObject::connect(slider, &QSlider::valueChanged, [segRenderer, result, this](int i) {
                        segRenderer->setOpacity((float)i/100.0f, segRenderer->getBorderOpacity());
                        writeRendererAttributes(result);
                    });
                    layout->addWidget(slider);
                }

                // Border opacity
                {
                    auto label = new QLabel();
                    label->setText("Border opacity:");
                    layout->addWidget(label);
                    auto slider = new QSlider(Qt::Horizontal);
                    slider->setRange(0, 100);
                    slider->setValue(segRenderer->getBorderOpacity()*100.0f);
                    QObject::connect(slider, &QSlider::valueChanged, [segRenderer, result, this](int i) {
                        segRenderer->setBorderOpacity((float)i/100.0f);
                        writeRendererAttributes(result);
                    });
                    layout->addWidget(slider);
                }
                // Border radius
                {
                    auto label = new QLabel();
                    label->setText("Border radius:");
                    layout->addWidget(label);
                    auto slider = new QSlider(Qt::Horizontal);
                    slider->setRange(1, 32);
                    slider->setValue(segRenderer->getBorderRadius());
                    QObject::connect(slider, &QSlider::valueChanged, [segRenderer, result, this](int i) {
                        segRenderer->setBorderRadius(i);
                        writeRendererAttributes(result);
                    });
                    layout->addWidget(slider);
                }
            } else if(rendererType == "HeatmapRenderer") {
                auto heatmapRenderer = std::dynamic_pointer_cast<HeatmapRenderer>(renderer);

                // Max opacity
                {
                    auto label = new QLabel();
                    label->setText("Maximum Opacity:");
                    layout->addWidget(label);
                    auto slider = new QSlider(Qt::Horizontal);
                    slider->setRange(0, 100);
                    slider->setValue(heatmapRenderer->getMaxOpacity()*100.f);
                    QObject::connect(slider, &QSlider::valueChanged, [heatmapRenderer, result, this](int i) {
                        heatmapRenderer->setMaxOpacity((float)i/100.0f);
                        writeRendererAttributes(result);
                    });
                    layout->addWidget(slider);
                }

                // Min confidence
                {
                    auto label = new QLabel();
                    label->setText("Minimum Confidence:");
                    layout->addWidget(label);
                    auto slider = new QSlider(Qt::Horizontal);
                    slider->setRange(0, 100);
                    slider->setValue(heatmapRenderer->getMinConfidence()*100.0f);
                    QObject::connect(slider, &QSlider::valueChanged, [heatmapRenderer, result, this](int i) {
                        heatmapRenderer->setMinConfidence((float)i/100.0f);
                        writeRendererAttributes(result);
                    });
                    layout->addWidget(slider);
                }

                // Interpolation
                {
                    auto label = new QLabel();
                    label->setText("Interpolation:");
                    layout->addWidget(label);
                    auto checkbox = new QCheckBox();
                    checkbox->setChecked(heatmapRenderer->getInterpolation());
                    QObject::connect(checkbox, &QCheckBox::stateChanged, [heatmapRenderer, result, this](int i) {
                        heatmapRenderer->setInterpolation(!heatmapRenderer->getInterpolation());
                        writeRendererAttributes(result);
                    });
                    layout->addWidget(checkbox);
                }
            }
        }
        _page_combobox->adjustSize();
    }
} // End of namespace fast
