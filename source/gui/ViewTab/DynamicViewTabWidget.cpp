//
// Created by dbouget on 04.11.2021.
//

#include <FAST/Visualization/HeatmapRenderer/HeatmapRenderer.hpp>
#include <FAST/Visualization/SegmentationRenderer/SegmentationRenderer.hpp>
#include <FAST/Visualization/BoundingBoxRenderer/BoundingBoxRenderer.hpp>
#include <FAST/Visualization/ImagePyramidRenderer/ImagePyramidRenderer.hpp>

#include "DynamicViewTabWidget.h"

namespace fast
{
    DynamicViewTabWidget::DynamicViewTabWidget(const std::string& renderer_name, QWidget *parent): _renderer_name(renderer_name), QWidget(parent){
        this->_current_class = 0;
        this->setupInterface();
        this->setupConnections();
    }

    DynamicViewTabWidget::~DynamicViewTabWidget(){

    }

    void DynamicViewTabWidget::setupInterface()
    {
        this->_main_layout = new QVBoxLayout(this);
        this->_main_layout->setAlignment(Qt::AlignTop);

        this->_toggle_image_pushbutton = new QPushButton(this);
        this->_toggle_image_pushbutton->setText("Toggle image");
        this->_toggle_image_pushbutton->setFixedHeight(50);
        this->_toggle_image_pushbutton->setCheckable(true);
        this->_toggle_image_pushbutton->setChecked(true);

        // for WSI this should be grayed out, shouldn't be able to change it
        this->_opacity_slider = new QSlider(Qt::Horizontal, this);
        this->_opacity_slider->setMinimum(0);
        this->_opacity_slider->setMaximum(20);
        this->_opacity_slider->setValue(8);
        this->_opacity_slider->setTickInterval(1);
        if(this->_renderer_name == "WSI")
            this->_opacity_slider->setEnabled(false);

        this->_tissue_label = new QLabel(this);
        std::string label_text = this->_renderer_name;
        label_text[0] = toupper(label_text[0]);
        this->_tissue_label->setText(QString::fromStdString(label_text + ": "));
        this->_tissue_label->setFixedWidth(50);

        this->_tissue_textbox_layout = new QHBoxLayout;
        this->_tissue_textbox_layout->addWidget(this->_tissue_label);
        this->_tissue_textbox_layout->addWidget(this->_opacity_slider);

        this->_tissue_textbox_widget = new QWidget(this);
        this->_tissue_textbox_widget->setFixedHeight(50);
        this->_tissue_textbox_widget->setLayout(this->_tissue_textbox_layout);

        // make QColorDialog for manually setting color to different classes
        auto colorSetWidget = new QColorDialog(this);
        colorSetWidget->setOption(QColorDialog::DontUseNativeDialog, true);

        this->_set_color_pushbutton = new QPushButton(this);
        this->_set_color_pushbutton->setFixedHeight(50);
        this->_set_color_pushbutton->setText("Set color");
        this->_set_color_pushbutton->setChecked(true);

        // to be able to set which classes to show/hide
        int channel_value = 0;

        this->_toggle_class_pushbutton = new QPushButton(this);
        this->_toggle_class_pushbutton->setText("Toggle class");
        this->_toggle_class_pushbutton->setFixedHeight(50);
        this->_toggle_class_pushbutton->setCheckable(true);
        this->_toggle_class_pushbutton->setChecked(true);

        this->_delete_tab_pushbutton = new QPushButton(this);
        this->_delete_tab_pushbutton->setText("Delete object");
        this->_delete_tab_pushbutton->setFixedHeight(50);
        this->_delete_tab_pushbutton->setStyleSheet("color: black; background-color: red");


        this->_class_list_combobox = new QComboBox(this);
        this->_classes.clear();

        // additional options for setting which classes to show and colors
        if (DataManager::GetInstance()->get_visible_image()->get_renderer_type(this->_renderer_name) == "HeatmapRenderer")
        {
            // get metadata of current model
            std::map<std::string, std::string> metadata = ProcessManager::GetInstance()->get_model(this->_renderer_name)->get_model_metadata();
            std::vector someVector = splitCustom(metadata["class_names"], ";");
            for (const auto & i : someVector){
                this->_classes.append(QString::fromStdString(i));
            }
            this->_class_list_combobox->clear();
            this->_class_list_combobox->update();
            this->_class_list_combobox->insertItems(0, this->_classes);

            QObject::connect(this->_set_color_pushbutton, &QPushButton::clicked, [=]() {
                auto rgb = colorSetWidget->getColor().toRgb();

                auto someRenderer = std::dynamic_pointer_cast<HeatmapRenderer>(DataManager::GetInstance()->get_visible_image()->get_renderer(this->_renderer_name));
                someRenderer->setChannelColor(this->_class_list_combobox->currentIndex(), Color((float)(rgb.red() / 255.0f), (float)(rgb.green() / 255.0f), (float)(rgb.blue() / 255.0f)));
            });
        }
        else if(DataManager::GetInstance()->get_visible_image()->get_renderer_type(this->_renderer_name) == "SegmentationRenderer")
        {
            for (const auto & i : { 1 }) { //{ 0, 1 }) {  // TODO: Supports only binary images (where class of interest = 1)
                this->_classes.append(QString::number(i));
            }
            this->_class_list_combobox->clear();
            this->_class_list_combobox->update();
            this->_class_list_combobox->insertItems(0, this->_classes);

            QObject::connect(this->_set_color_pushbutton, &QPushButton::clicked, [=]() {
                auto rgb = colorSetWidget->getColor().toRgb();
                auto someRenderer = std::dynamic_pointer_cast<SegmentationRenderer>(DataManager::GetInstance()->get_visible_image()->get_renderer(this->_renderer_name));
                //auto currImage = someRenderer->updateAndGetOutputData<Image>();
                //currImage->
                //std::cout << "window: " << someRenderer->updateAndGetOutputData<Image>() << std::endl;
                //auto vals = someRenderer->getIntensityWindow();

                // TODO: Supports only binary images (where class of interest = 1)
                someRenderer->setColor(this->_class_list_combobox->currentIndex() + 1, Color((float)(rgb.red() / 255.0f), (float)(rgb.green() / 255.0f), (float)(rgb.blue() / 255.0f)));
            });
        // TODO: Need to add proper options in SegmentationPyramidRenderer and SegmentationRenderer for toggling and setting which classes to show, do to this properly...
        }
        else if (DataManager::GetInstance()->get_visible_image()->get_renderer_type(this->_renderer_name) == "SegmentationPyramidRenderer")
        {
            // get metadata of current model
            std::map<std::string, std::string> metadata = ProcessManager::GetInstance()->get_model(this->_renderer_name)->get_model_metadata();
            std::vector someVector = splitCustom(metadata["class_names"], ";");
            for (const auto & i : someVector) {
                this->_classes.append(QString::fromStdString(i));
            }
            this->_class_list_combobox->clear();
            this->_class_list_combobox->update();
            this->_class_list_combobox->insertItems(0, this->_classes);

            QObject::connect(this->_set_color_pushbutton, &QPushButton::clicked, [=]() {
                auto rgb = colorSetWidget->getColor().toRgb();
                auto someRenderer = std::dynamic_pointer_cast<SegmentationRenderer>(DataManager::GetInstance()->get_visible_image()->get_renderer(this->_renderer_name));
                someRenderer->setColor(this->_class_list_combobox->currentIndex() + 1, Color((float)(rgb.red() / 255.0f), (float)(rgb.green() / 255.0f), (float)(rgb.blue() / 255.0f)));
                //someRenderer->setChannelColor(this->_class_list_combobox->currentIndex(), Color((float)(rgb.red() / 255.0f), (float)(rgb.green() / 255.0f), (float)(rgb.blue() / 255.0f)));
            });
        }
        else if (DataManager::GetInstance()->get_visible_image()->get_renderer_type(this->_renderer_name) == "BoundingBoxRenderer")
        {
            // get metadata of current model
            std::map<std::string, std::string> metadata = ProcessManager::GetInstance()->get_model(this->_renderer_name)->get_model_metadata();
            std::vector someVector = splitCustom(metadata["class_names"], ";");
            for (const auto & i : someVector) {
                this->_classes.append(QString::fromStdString(i));
            }
            this->_class_list_combobox->clear();
            this->_class_list_combobox->update();
            this->_class_list_combobox->insertItems(0, this->_classes);

            QObject::connect(this->_set_color_pushbutton, &QPushButton::clicked, [=]() {
                auto rgb = colorSetWidget->getColor().toRgb();
                auto someRenderer = std::dynamic_pointer_cast<BoundingBoxRenderer>(DataManager::GetInstance()->get_visible_image()->get_renderer(this->_renderer_name));
                //someRenderer->setLabelColor(this->_class_list_combobox->currentIndex(), Color((float)(rgb.red() / 255.0f), (float)(rgb.green() / 255.0f), (float)(rgb.blue() / 255.0f)));
            });
        }
        else if (DataManager::GetInstance()->get_visible_image()->get_renderer_type(this->_renderer_name) == "ImagePyramidRenderer")
        {
            // Main image case, nothing to do here.
        }
        else
        {
            simpleInfoPrompt(QString::fromStdString("Invalid renderer used with type: " + DataManager::GetInstance()->get_visible_image()->get_renderer_type(this->_renderer_name)), this);
        }

        this->_class_selector_label = new QLabel(this);
        this->_class_selector_label->setText("Class: ");
        this->_class_selector_label->setFixedWidth(50);
        this->_class_selector_layout = new QHBoxLayout;
        this->_class_selector_layout->addWidget(this->_class_selector_label);
        this->_class_selector_layout->addWidget(this->_class_list_combobox);

        auto smallTextBoxWidget_imageName = new QWidget;
        smallTextBoxWidget_imageName->setFixedHeight(50);
        smallTextBoxWidget_imageName->setLayout(this->_class_selector_layout);
        auto biggerTextBox_imageName = new QVBoxLayout;
        biggerTextBox_imageName->addWidget(smallTextBoxWidget_imageName);
        biggerTextBox_imageName->setAlignment(Qt::AlignTop);

        this->_toggle_color_layout = new QHBoxLayout;
        this->_toggle_color_layout->addWidget(this->_toggle_class_pushbutton);
        this->_toggle_color_layout->addWidget(this->_set_color_pushbutton);
//        auto smallToggleColorBox = new QWidget;
//        smallToggleColorBox->setLayout(this->_toggle_color_layout);
//        auto biggerTextBoxWidget_imageName = new QWidget;
//        biggerTextBoxWidget_imageName->setLayout(this->_toggle_color_layout);

        // disable some features for specific renderer types
        if (DataManager::GetInstance()->get_visible_image()->get_renderer_type(this->_renderer_name) == "ImagePyramidRenderer")
        {
            this->_opacity_slider->setDisabled(true);
            colorSetWidget->setDisabled(true);
//            biggerTextBoxWidget_imageName->setDisabled(true);
        }
        else if (DataManager::GetInstance()->get_visible_image()->get_renderer_type(this->_renderer_name) == "SegmentationRenderer")
        {
            this->_toggle_class_pushbutton->setDisabled(true);
        }
        else if (DataManager::GetInstance()->get_visible_image()->get_renderer_type(this->_renderer_name) == "SegmentationPyramidRenderer")
        {
            this->_toggle_class_pushbutton->setDisabled(true);
            //colorSetWidget->setDisabled(true);
            //biggerTextBoxWidget_imageName->setDisabled(true);
        }
        else if (DataManager::GetInstance()->get_visible_image()->get_renderer_type(this->_renderer_name) == "BoundingBoxRenderer")
        {
            1;
        }
        else
        {
            colorSetWidget->setDisabled(false);
        }

        this->_renderer_groupbox = new QGroupBox(tr("Modify image"), this);
        this->_all_view_layout = new QVBoxLayout;
        this->_all_view_layout->addWidget(this->_toggle_image_pushbutton);
        this->_all_view_layout->addWidget(this->_tissue_textbox_widget);
        this->_renderer_groupbox->setLayout(this->_all_view_layout);

        this->_class_groupbox = new QGroupBox(tr("Modify class"), this);
        this->_class_view_layout = new QVBoxLayout;
        //this->_class_view_layout->addWidget(smallTextBoxWidget_imageName);
        //this->_class_view_layout->addWidget(biggerTextBoxWidget_imageName);
        this->_class_view_layout->addLayout(this->_class_selector_layout);
        this->_class_view_layout->addLayout(this->_toggle_color_layout);
        this->_class_groupbox->setLayout(this->_class_view_layout);

        this->_main_layout->addWidget(this->_renderer_groupbox);
        this->_main_layout->addWidget(this->_class_groupbox);
        this->_main_layout->addWidget(colorSetWidget);
        this->_main_layout->addWidget(this->_delete_tab_pushbutton);
    }

    void DynamicViewTabWidget::resetInterface()
    {
        return;
    }

    void DynamicViewTabWidget::setupConnections()
    {
        QObject::connect(this->_toggle_image_pushbutton, &QPushButton::clicked, this, &DynamicViewTabWidget::toggleRendererReceived);
        //QObject::connect(this->_opacity_slider, &QSlider::valueChanged, std::bind(&DynamicViewTabWidget::opacityRendererReceived, this, std::placeholders::_1, someName));
        QObject::connect(this->_opacity_slider, &QSlider::valueChanged, this, &DynamicViewTabWidget::opacityRendererReceived);
        QObject::connect(this->_toggle_class_pushbutton, &QPushButton::clicked, this, &DynamicViewTabWidget::hideChannelReceived);
        //QObject::connect(this->_class_list_combobox, &QComboBox::currentIndexChanged, this, updateCurrentClassIndexReceived);
        QObject::connect(this->_class_list_combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateCurrentClassIndexReceived(int)));
        QObject::connect(this->_delete_tab_pushbutton, &QPushButton::clicked, std::bind(&DynamicViewTabWidget::deleteViewObjectTriggered, this, this->_renderer_name));
    }

    void DynamicViewTabWidget::updateCurrentClassIndexReceived(int index)
    {
        this->_current_class = (uint) index;
    }

    void DynamicViewTabWidget::toggleRendererReceived()
    {
        if(DataManager::GetInstance()->get_visible_image()->has_renderer(this->_renderer_name))
        {
            auto renderer = DataManager::GetInstance()->get_visible_image()->get_renderer(this->_renderer_name);
            renderer->setDisabled(!renderer->isDisabled());
        }
    }

    bool DynamicViewTabWidget::opacityRendererReceived(int value)
    {
        if (!DataManager::GetInstance()->get_visible_image()->has_renderer(this->_renderer_name))
            return false;
        else
        {
            if (DataManager::GetInstance()->get_visible_image()->get_renderer_type(this->_renderer_name) == "SegmentationRenderer")
            {
                auto someRenderer = std::dynamic_pointer_cast<SegmentationRenderer>(DataManager::GetInstance()->get_visible_image()->get_renderer(this->_renderer_name));
                someRenderer->setOpacity((float) value / 20.0f);
            }
            else if (DataManager::GetInstance()->get_visible_image()->get_renderer_type(this->_renderer_name) == "SegmentationRenderer")
            { // FIXME: Apparently, this doesn't change opacity
                auto someRenderer = std::dynamic_pointer_cast<SegmentationRenderer>(DataManager::GetInstance()->get_visible_image()->get_renderer(this->_renderer_name));
                someRenderer->setOpacity((float) value / 20.0f);
            }
            else if (DataManager::GetInstance()->get_visible_image()->get_renderer_type(this->_renderer_name) == "HeatmapRenderer")
            {
                auto someRenderer = std::dynamic_pointer_cast<HeatmapRenderer>(DataManager::GetInstance()->get_visible_image()->get_renderer(this->_renderer_name));
                someRenderer->setMaxOpacity((float) value / 20.0f);
                //someRenderer->setMinConfidence(0.9);
            }
            else
                return false;
            return true;
        }
    }

    bool DynamicViewTabWidget::hideChannelReceived() {
        if (!DataManager::GetInstance()->get_visible_image()->has_renderer(this->_renderer_name) || DataManager::GetInstance()->get_visible_image()->get_renderer_type(this->_renderer_name) != "HeatmapRenderer")
            return false;
        else
        {
//            background_flag = !background_flag;
            auto someRenderer = std::dynamic_pointer_cast<HeatmapRenderer>(DataManager::GetInstance()->get_visible_image()->get_renderer(this->_renderer_name));
            someRenderer->setChannelHidden(this->_current_class, true); // @TODO. Should the background_flag be for each channel?
            return true;
        }
    }

} // End of namespace fast
