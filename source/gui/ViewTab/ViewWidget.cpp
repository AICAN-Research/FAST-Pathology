//
// Created by dbouget on 02.11.2021.
//

#include "ViewWidget.h"

#include <FAST/Importers/WholeSlideImageImporter.hpp>
#include <FAST/Visualization/ImagePyramidRenderer/ImagePyramidRenderer.hpp>
#include <FAST/Data/ImagePyramid.hpp>

namespace fast {
    ViewWidget::ViewWidget(QWidget *parent): QWidget(parent){
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

        this->_stacked_layout = new QStackedLayout(this);

        this->_stacked_widget= new QWidget(this);
        this->_stacked_widget->setLayout(this->_stacked_layout);

        this->_page_combobox = new QComboBox(this);
        this->_page_combobox->setFixedWidth(150);

        auto imageNameTexts = new QLabel(this);
        imageNameTexts->setText("Image: ");
        imageNameTexts->setFixedWidth(50);
        auto smallTextBox_imageName = new QHBoxLayout(this);
        smallTextBox_imageName->addWidget(imageNameTexts);
        smallTextBox_imageName->addWidget(this->_page_combobox);
        auto smallTextBoxWidget_imageName = new QWidget(this);
        smallTextBoxWidget_imageName->setFixedHeight(50);
        smallTextBoxWidget_imageName->setLayout(smallTextBox_imageName);

        this->_main_layout->insertWidget(0, smallTextBoxWidget_imageName);
        this->_main_layout->insertWidget(1, this->_stacked_widget);
    }

    void ViewWidget::resetInterface()
    {
        return;
    }

    void ViewWidget::setupConnections()
    {
//        QObject::connect(this->_page_combobox, &QComboBox::activated, this->_stacked_layout, &QStackedLayout::setCurrentIndex);
        QObject::connect(this->_page_combobox, SIGNAL(activated(int)), this->_stacked_layout, SLOT(setCurrentIndex(int)));
    }

} // End of namespace fast
