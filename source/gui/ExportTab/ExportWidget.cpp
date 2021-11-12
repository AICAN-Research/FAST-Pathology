//
// Created by dbouget on 12.11.2021.
//

#include "ExportWidget.h"

namespace fast
{
    ExportWidget::ExportWidget(QWidget *parent): QWidget(parent){
    }

    ExportWidget::~ExportWidget(){

    }

    void ExportWidget::setupInterface()
    {
        this->_main_layout = new QVBoxLayout(this);
        this->_main_layout->setAlignment(Qt::AlignTop);
        // @TODO. Is this export tab actually necessary, since all results and thumbnails will be stored automatically?

//        //auto wsiPageWidget = new QWidget;
//        exportStackedLayout = new QStackedLayout;

//        auto exportStackedWidget = new QWidget;
//        exportStackedWidget->setLayout(exportStackedLayout);

//        exportComboBox = new QComboBox;
//        exportComboBox->setFixedWidth(150);
//        connect(exportComboBox, SIGNAL(activated(int)), exportStackedLayout, SLOT(setCurrentIndex(int)));

//        QStringList itemsInComboBox;
//        for (int index = 0; index < pageComboBox->count(); index++) {
//            std::cout << "some item: " << pageComboBox->itemText(index).toStdString() << std::endl;
//            itemsInComboBox << pageComboBox->itemText(index);
//        }

//        auto saveThumbnailButton = new QPushButton(mWidget);
//        saveThumbnailButton->setText("Save thumbnail");
//        saveThumbnailButton->setFixedHeight(50);
//        QObject::connect(saveThumbnailButton, &QPushButton::clicked, std::bind(&MainWindow::saveThumbnail, this));

//        auto saveTissueButton = new QPushButton(mWidget);
//        saveTissueButton->setText("Save tissue mask");
//        saveTissueButton->setFixedHeight(50);
//        QObject::connect(saveTissueButton, &QPushButton::clicked, std::bind(&MainWindow::saveTissueSegmentation, this));

//        auto saveTumorButton = new QPushButton(mWidget);
//        saveTumorButton->setText("Save tumor mask");
//        saveTumorButton->setFixedHeight(50);
//        QObject::connect(saveTumorButton, &QPushButton::clicked, std::bind(&MainWindow::saveTumor, this));

//        auto imageNameTexts = new QLabel(mWidget);
//        imageNameTexts->setText("Results: ");
//        imageNameTexts->setFixedWidth(75);
//        auto smallTextBox_imageName = new QHBoxLayout;
//        smallTextBox_imageName->addWidget(imageNameTexts);
//        smallTextBox_imageName->addWidget(exportComboBox);
//        auto smallTextBoxWidget_imageName = new QWidget(mWidget);
//        smallTextBoxWidget_imageName->setFixedHeight(50);
//        smallTextBoxWidget_imageName->setLayout(smallTextBox_imageName);

//        this->_main_layout->addWidget(smallTextBoxWidget_imageName);
//        this->_main_layout->addWidget(saveThumbnailButton);
//        this->_main_layout->addWidget(saveTissueButton);
//        this->_main_layout->addWidget(saveTumorButton);
    }

    void ExportWidget::resetInterface()
    {
    }

    void ExportWidget::setupConnections()
    {
    }
} // End of namespace fast
