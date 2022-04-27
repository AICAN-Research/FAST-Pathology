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
        this->setupStylesheets();
    }

    ViewWidget::~ViewWidget(){

    }

    void ViewWidget::setupInterface()
    {
        this->_main_layout = new QVBoxLayout(this);
        this->_main_layout->setAlignment(Qt::AlignTop);
        //this->_main_layout->setFixedWidth(200);

        this->_layers_stacked_widget = new QStackedWidget(this);

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
        this->_main_layout->insertWidget(1, this->_layers_stacked_widget);
        this->setVisible(false);
        this->hide();
    }

    void ViewWidget::resetInterface()
    {
        this->_dynamic_widget_list.clear();
        this->_page_combobox->clear();
        for(int i = this->_layers_stacked_widget->count(); i >= 0; i--)
        {
            QWidget* widget = this->_layers_stacked_widget->widget(i);
            this->_layers_stacked_widget->removeWidget(widget);
            widget->deleteLater();
        }

        this->setVisible(false);
        this->hide();
    }

    void ViewWidget::setupConnections()
    {
        QObject::connect(this->_page_combobox, SIGNAL(activated(int)), this->_layers_stacked_widget, SLOT(setCurrentIndex(int)));
    }

    void ViewWidget::setupStylesheets()
    {

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
        
        // @TODO. When pipeline results can be properly collected, this call should be updated.
        DataManager::GetInstance()->get_visible_image()->remove_renderer(uid);
    }

    void ViewWidget::processTriggerUpdate(std::string process_name)
    {
        if (this->_dynamic_widget_list.empty())
        {
            auto dynamic_widget = new DynamicViewTabWidget("WSI", this);
            // Not connecting the delete button for the WSI widget. Will be added with the first View results, and deleted if no more viewed results, automatically.
            this->_dynamic_widget_list["WSI"] = dynamic_widget;
            this->_layers_stacked_widget->addWidget(dynamic_widget);
            this->_page_combobox->insertItem(0, "WSI");
            this->setVisible(true);
            this->show();
        }

        auto dynamic_widget = new DynamicViewTabWidget(process_name, this);
        QObject::connect(dynamic_widget, &DynamicViewTabWidget::deleteViewObjectTriggered, this, &ViewWidget::deleteViewObjectReceived);
        this->_dynamic_widget_list[process_name] = dynamic_widget;
        this->_layers_stacked_widget->addWidget(dynamic_widget);
        this->_page_combobox->insertItem(this->_page_combobox->count(), QString::fromStdString(process_name));
    }

    void ViewWidget::onChangeWSIDisplay(std::string wsi_uid, bool state)
    {
        // Cleaning the panel from any previous results
        this->resetInterface();

        // If the wsi_uid has been toggled, its results should be queried from the DataManager to populate the area
        if(state)
        {

        }
    }
} // End of namespace fast
