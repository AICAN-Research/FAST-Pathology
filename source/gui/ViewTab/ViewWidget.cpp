#include "ViewWidget.h"
#include <FAST/Importers/WholeSlideImageImporter.hpp>
#include <FAST/Visualization/ImagePyramidRenderer/ImagePyramidRenderer.hpp>
#include <FAST/Data/ImagePyramid.hpp>
#include <FAST/Visualization/SegmentationRenderer/SegmentationRenderer.hpp>
#include <FAST/Visualization/HeatmapRenderer/HeatmapRenderer.hpp>
#include <QCheckBox>

namespace fast {
    ViewWidget::ViewWidget(QWidget *parent): QWidget(parent){
        setupInterface();
        setupConnections();
    }

    ViewWidget::~ViewWidget(){

    }

    void ViewWidget::setupInterface()
    {
        _main_layout = new QVBoxLayout(this);
        _main_layout->setAlignment(Qt::AlignTop);

        _stacked_layout = new QStackedLayout;

        _stacked_widget= new QWidget(this);
        _stacked_widget->setLayout(_stacked_layout);

        _page_combobox = new QComboBox(this);

        _main_layout->addWidget(_page_combobox);
        _main_layout->addWidget(_stacked_widget);
        setVisible(false);
        hide();
    }

    void ViewWidget::resetInterface()
    {
        _page_combobox->clear();
        // Clear stacked layout
        auto layout = _stacked_layout;
        QLayoutItem *item;
        while((item = layout->takeAt(0))) {
            if (item->widget())
                delete item->widget();
            delete item;
        }
        // end clear
        setVisible(false);
        hide();
    }

    void ViewWidget::setupConnections()
    {
        QObject::connect(_page_combobox, SIGNAL(activated(int)), _stacked_layout, SLOT(setCurrentIndex(int)));
    }

    void ViewWidget::writeRendererAttributes(Result result) {
        if(result.renderer->getNameOfClass() == "ImagePyramidRenderer")
            return;
        auto project = DataManager::GetInstance()->getCurrentProject();
        const std::string saveFolder = join(project->getRootFolder(), "results", result.WSI_uid, result.pipelineName, result.name);
        std::ofstream file(join(saveFolder, "renderer.attributes.txt"), std::iostream::out);
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

                auto label = new QLabel();
                label->setText("Classes:");
                layout->addWidget(label);

                for(int i = 1; i < result.classNames.size(); ++i) { // Assuming first class is background here
                    auto className = result.classNames[i];
                    auto button = new QPushButton();
                    button->setStyleSheet("text-align: left; padding: 10%;");
                    button->setText(QString::fromStdString(className));
                    QPixmap pixmap(64,64);
                    Color color = segRenderer->getColor(i);
                    pixmap.fill(QColor(color.getRedValue()*255, color.getGreenValue()*255, color.getBlueValue()*255));
                    button->setIcon(QIcon(pixmap));
                    layout->addWidget(button);

                    auto colorDialog = new QColorDialog();
                    colorDialog->setOption(QColorDialog::DontUseNativeDialog, true);

                    QObject::connect(button, &QPushButton::clicked, colorDialog, &QColorDialog::show);
                    QObject::connect(colorDialog, &QColorDialog::colorSelected, [i, button, segRenderer, result, this](QColor color) {
                        QPixmap pixmap(64,64);
                        pixmap.fill(color);
                        button->setIcon(QIcon(pixmap));
                        segRenderer->setColor(i, Color(color.red()/255.0f, color.green()/255.0f, color.blue()/255.0f));
                        writeRendererAttributes(result);
                    });
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

                auto label = new QLabel();
                label->setText("Classes:");
                layout->addWidget(label);

                for(int i = 0; i < result.classNames.size(); ++i) {
                    auto classLayout = new QHBoxLayout();
                    layout->addLayout(classLayout);

                    auto checkbox = new QCheckBox();
                    checkbox->setChecked(!heatmapRenderer->getChannelHidden(i));
                    classLayout->addWidget(checkbox);
                    QObject::connect(checkbox, &QCheckBox::stateChanged, [=]() {
                        heatmapRenderer->setChannelHidden(i, !checkbox->isChecked());
                        writeRendererAttributes(result);
                    });

                    auto className = result.classNames[i];
                    auto button = new QPushButton();
                    button->setStyleSheet("text-align: left; padding: 10%;");
                    button->setText(QString::fromStdString(className));
                    QPixmap pixmap(64,64);
                    Color color = heatmapRenderer->getChannelColor(i);
                    pixmap.fill(QColor(color.getRedValue()*255, color.getGreenValue()*255, color.getBlueValue()*255));
                    button->setIcon(QIcon(pixmap));
                    classLayout->addWidget(button);

                    auto colorDialog = new QColorDialog();
                    colorDialog->setOption(QColorDialog::DontUseNativeDialog, true);

                    QObject::connect(button, &QPushButton::clicked, colorDialog, &QColorDialog::show);
                    QObject::connect(colorDialog, &QColorDialog::colorSelected, [i, button, heatmapRenderer, result, this](QColor color) {
                        QPixmap pixmap(64,64);
                        pixmap.fill(color);
                        button->setIcon(QIcon(pixmap));
                        heatmapRenderer->setChannelColor(i, Color(color.red()/255.0f, color.green()/255.0f, color.blue()/255.0f));
                        writeRendererAttributes(result);
                    });
                }
            }
        }
        _page_combobox->adjustSize();
    }
} // End of namespace fast
