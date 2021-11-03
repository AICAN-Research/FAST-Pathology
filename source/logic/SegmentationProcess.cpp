//
// Created by dbouget on 03.11.2021.
//

#include "SegmentationProcess.h"
#include <FAST/Importers/WholeSlideImageImporter.hpp>
#include <FAST/Visualization/ImagePyramidRenderer/ImagePyramidRenderer.hpp>
#include <FAST/Data/ImagePyramid.hpp>
#include <FAST/Data/Image.hpp>


namespace fast{
    SegmentationProcess::SegmentationProcess(const std::string image_uid): _image_uid(image_uid)
    {
        this->_stop_flag = false;
    }

    SegmentationProcess::~SegmentationProcess()
    {
    }

    bool SegmentationProcess::segmentTissue() {
        return false;
        // @TODO. Does the segmentation run only on the visible WSI, or on all loaded WSIs showing on the side list?
//        if (DataManager::GetInstance()->isEmpty()) {
//            std::cout << "Requires a WSI to be rendered in order to perform the analysis." << std::endl;
//            return false;
//        }

//        if (DataManager::GetInstance()->getVisibleImageName() == "")
//        {
//            std::cout << "Requires a WSI to be rendered in order to perform the analysis." << std::endl;
//            return false;
//        }

//        auto visible_image = DataManager::GetInstance()->get_visible_image();

//        // prompt if you want to run the analysis again, if it has already been ran
//        if (visible_image->has_renderer("tissue")) {
//            simpleInfoPrompt("Tissue segmentation on current WSI has already been performed.", this);
//            return false;
//        }

//        // basic thresholding (with morph. post-proc.) based on euclidean distance from the color white
//        auto tissueSegmentation = TissueSegmentation::New();
//        tissueSegmentation->setInputData(visible_image->get_image_pyramid());

//        this->_stop_flag = false;
//        if (this->_advanced_mode) {
//            // option for setting parameters
//            QDialog paramDialog;
//            paramDialog.setStyleSheet(mWidget->styleSheet()); // transfer style sheet from parent
//            QFormLayout form(&paramDialog);
//            form.addRow(new QLabel("Please, set the parameters for this analysis: "));

//            // threshold : for WSI this should be grayed out, shouldn't be able to change it
//            auto threshSlider = new QSlider(Qt::Horizontal, dynamicViewWidget);
//            threshSlider->setFixedWidth(150);
//            threshSlider->setMinimum(0);
//            threshSlider->setMaximum(255);
//            threshSlider->setValue(tissueSegmentation->getThreshold());
//            threshSlider->setTickInterval(1);
//            QObject::connect(threshSlider, &QSlider::valueChanged, [=](int newValue){tissueSegmentation->setThreshold(newValue);});

//            auto currValue = new QLabel;
//            currValue->setText(QString::fromStdString(std::to_string(tissueSegmentation->getThreshold())));
//            currValue->setFixedWidth(50);
//            QObject::connect(threshSlider, &QSlider::valueChanged, [=](int newValue){currValue->setText(QString::fromStdString(std::to_string(tissueSegmentation->getThreshold())));});

//            // threshold
//            auto threshWidget = new QWidget;
//            auto sliderLayout = new QHBoxLayout;
//            threshWidget->setLayout(sliderLayout);
//            sliderLayout->addWidget(threshSlider);
//            sliderLayout->addWidget(currValue);

//            std::string tempTissueName = "temporaryTissue";

//            QObject::connect(threshSlider, &QSlider::valueChanged, [=](int newValue) {
//                const int step = 2;
//                threshSlider->setValue(newValue);
//                tissueSegmentation->setThreshold(newValue);
//                auto checkFlag = true;

//                if (checkFlag) {
//                    auto temporaryTissueSegmentation = TissueSegmentation::New();
//                    temporaryTissueSegmentation->setInputData(m_image);
//                    temporaryTissueSegmentation->setThreshold(tissueSegmentation->getThreshold());
//                    temporaryTissueSegmentation->setErode(tissueSegmentation->getErode());
//                    temporaryTissueSegmentation->setDilate(tissueSegmentation->getDilate());

//                    auto someRenderer = SegmentationRenderer::New();
//                    someRenderer->setColor(1, Color(255.0f / 255.0f, 127.0f / 255.0f, 80.0f / 255.0f));
//                    someRenderer->setInputData(temporaryTissueSegmentation->updateAndGetOutputData<Image>());
//                    someRenderer->setOpacity(0.4f);
//                    someRenderer->update();

//                    if (hasRenderer(tempTissueName)) {
//                        auto currRenderer = m_rendererList[tempTissueName];
//                        getView(0)->removeRenderer(currRenderer);
//                        m_rendererList.erase(tempTissueName);
//                    }
//                    insertRenderer(tempTissueName, someRenderer);
//                }
//            });

//            // dilation
//            auto dilateSlider = new QSlider(Qt::Horizontal, dynamicViewWidget);
//            dilateSlider->setFixedWidth(150);
//            dilateSlider->setMinimum(1);
//            dilateSlider->setMaximum(28);
//            dilateSlider->setValue(tissueSegmentation->getDilate());
//            dilateSlider->setSingleStep(2);
//            QObject::connect(dilateSlider, &QSlider::valueChanged, [=](int newValue) {
//                const int step = 2;
//                bool checkFlag = false;
//                if (newValue < 3) {
//                    dilateSlider->setValue(0);
//                    tissueSegmentation->setDilate(0);
//                    checkFlag = true;
//                } else {
//                    if (newValue % 2 != 0) {
//                        dilateSlider->setValue(newValue);
//                        tissueSegmentation->setDilate(newValue);
//                        checkFlag = true;
//                    }
//                }

//                if (checkFlag) {
//                    auto temporaryTissueSegmentation = TissueSegmentation::New();
//                    temporaryTissueSegmentation->setInputData(m_image);
//                    temporaryTissueSegmentation->setThreshold(tissueSegmentation->getThreshold());
//                    temporaryTissueSegmentation->setErode(tissueSegmentation->getErode());
//                    temporaryTissueSegmentation->setDilate(tissueSegmentation->getDilate());

//                    auto someRenderer = SegmentationRenderer::New();
//                    someRenderer->setColor(1, Color(255.0f / 255.0f, 127.0f / 255.0f, 80.0f / 255.0f));
//                    someRenderer->setInputData(temporaryTissueSegmentation->updateAndGetOutputData<Image>());
//                    someRenderer->setOpacity(0.4f);
//                    someRenderer->update();

//                    if (hasRenderer(tempTissueName)) {
//                        auto currRenderer = m_rendererList[tempTissueName];
//                        getView(0)->removeRenderer(currRenderer);
//                        m_rendererList.erase(tempTissueName);
//                    }
//                    insertRenderer(tempTissueName, someRenderer);
//                }
//            });

//            auto currDilateValue = new QLabel;
//            currDilateValue->setText(QString::fromStdString(std::to_string(tissueSegmentation->getDilate())));
//            currDilateValue->setFixedWidth(50);
//            QObject::connect(dilateSlider, &QSlider::valueChanged, [=](int newValue){currDilateValue->setText(QString::fromStdString(std::to_string(tissueSegmentation->getDilate())));});

//            auto dilateWidget = new QWidget;
//            auto dilateSliderLayout = new QHBoxLayout;
//            dilateWidget->setLayout(dilateSliderLayout);
//            dilateSliderLayout->addWidget(dilateSlider);
//            dilateSliderLayout->addWidget(currDilateValue);

//            // erosion
//            auto erodeSlider = new QSlider(Qt::Horizontal, dynamicViewWidget);
//            erodeSlider->setFixedWidth(150);
//            erodeSlider->setMinimum(1);
//            erodeSlider->setMaximum(28);
//            erodeSlider->setValue(tissueSegmentation->getErode());
//            erodeSlider->setSingleStep(2);
//            QObject::connect(erodeSlider, &QSlider::valueChanged, [=](int newValue) {
//                bool checkFlag = false;
//                if (newValue < 3) {
//                    erodeSlider->setValue(0);
//                    tissueSegmentation->setErode(0);
//                    checkFlag = true;
//                } else {
//                    if (newValue % 2 != 0) {
//                        erodeSlider->setValue(newValue);
//                        tissueSegmentation->setErode(newValue);
//                        checkFlag = true;
//                    }
//                }
//                // /*
//                if (checkFlag) {
//                    auto temporaryTissueSegmentation = TissueSegmentation::New();
//                    temporaryTissueSegmentation->setInputData(m_image);
//                    temporaryTissueSegmentation->setThreshold(tissueSegmentation->getThreshold());
//                    temporaryTissueSegmentation->setErode(tissueSegmentation->getErode());
//                    temporaryTissueSegmentation->setDilate(tissueSegmentation->getDilate());

//                    auto someRenderer = SegmentationRenderer::New();
//                    someRenderer->setColor(1, Color(255.0f/255.0f, 127.0f/255.0f, 80.0f/255.0f));
//                    someRenderer->setInputData(temporaryTissueSegmentation->updateAndGetOutputData<Image>());
//                    someRenderer->setOpacity(0.4f); // <- necessary for the quick-fix temporary solution
//                    someRenderer->update();

//                    if (hasRenderer(tempTissueName)) {
//                        auto currRenderer = m_rendererList[tempTissueName];
//                        getView(0)->removeRenderer(currRenderer);
//                        m_rendererList.erase(tempTissueName);
//                    }
//                    insertRenderer(tempTissueName, someRenderer);
//                }
//                // */
//            });

//            auto currErodeValue = new QLabel;
//            currErodeValue->setText(QString::fromStdString(std::to_string(tissueSegmentation->getErode())));
//            currErodeValue->setFixedWidth(50);
//            QObject::connect(erodeSlider, &QSlider::valueChanged, [=](int newValue){currErodeValue->setText(QString::fromStdString(std::to_string(tissueSegmentation->getErode())));});

//            auto erodeWidget = new QWidget;
//            auto erodeSliderLayout = new QHBoxLayout;
//            erodeWidget->setLayout(erodeSliderLayout);
//            erodeSliderLayout->addWidget(erodeSlider);
//            erodeSliderLayout->addWidget(currErodeValue);

//            QList<QSlider *> fields;
//            QString labelThresh = "Threshold";
//            form.addRow(labelThresh, threshWidget);
//            fields << threshSlider;

//            QString labelDilate = "Dilation";
//            form.addRow(labelDilate, dilateWidget);
//            fields << dilateSlider;

//            QString labelErode = "Erosion";
//            form.addRow(labelErode, erodeWidget);
//            fields << erodeSlider;

//            // Add some standard buttons (Cancel/Ok) at the bottom of the dialog
//            QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
//                    Qt::Horizontal, &paramDialog);
//            buttonBox.button(QDialogButtonBox::Ok)->setText("Run");
//            form.addRow(&buttonBox);
//            QObject::connect(&buttonBox, SIGNAL(accepted()), &paramDialog, SLOT(accept()));
//            QObject::connect(&buttonBox, SIGNAL(rejected()), &paramDialog, SLOT(reject()));

//            // Show the dialog as modal
//            int ret = paramDialog.exec();

//            // should delete temporary segmentation when selecting is finished or cancelled
//            auto currRenderer = m_rendererList[tempTissueName];
//            getView(0)->removeRenderer(currRenderer);
//            m_rendererList.erase(tempTissueName);

//            std::cout << "Value chosen: " << ret << std::endl;
//            switch (ret) {
//                case 1:
//                    std::cout << "OK was pressed, should have updated params!" << std::endl;
//                    break;
//                case 0:
//                    std::cout << "Cancel was pressed." << std::endl;
//                    stopFlag = true;
//                    return false;
//                default:
//                    std::cout << "Default was pressed." << std::endl;
//                    return false;
//            }
//        }

//        std::cout << "Thresh: " << tissueSegmentation->getThreshold() << std::endl;
//        std::cout << "Dilate: " << tissueSegmentation->getDilate() << std::endl;
//        std::cout << "Erode:  " << tissueSegmentation->getErode() << std::endl;

//        // finally get resulting tissueMap to be used later on
//        auto tissue_seg = tissueSegmentation->updateAndGetOutputData<Image>();

//        auto someRenderer = SegmentationRenderer::New();
//        someRenderer->setColor(1, Color(255.0f/255.0f, 127.0f/255.0f, 80.0f/255.0f));
//        someRenderer->setInputData(tissue_seg);
//        someRenderer->setOpacity(0.4f); // <- necessary for the quick-fix temporary solution
//        someRenderer->update();

//        std::string currSegment = "tissue";

//        // TODO: should append some unique ID next to "tissue" (also really for all other results) such that multiple
//        //  runs with different hyperparamters may be ran, visualized and stored
//        /*
//        std::string origSegment = "tissue";
//        auto iter = 2;
//        while (hasRenderer(currSegment)) {
//            currSegment = origSegment + std::to_string(iter);
//            iter++;
//        }
//            */

//        m_rendererTypeList[currSegment] = "SegmentationRenderer";
//        createDynamicViewWidget(currSegment, modelName);
//        insertRenderer(currSegment, someRenderer);

//        availableResults[currSegment] = m_tissue;
//        exportComboBox->addItem(tr(currSegment.c_str()));

//        return true;
    }

} // End of namespace fast
