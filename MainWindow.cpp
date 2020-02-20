#include "MainWindow.hpp"
#include <FAST/Importers/WholeSlideImageImporter.hpp>
#include <FAST/Visualization/ImagePyramidRenderer/ImagePyramidRenderer.hpp>
#include <QPushButton>
#include <QFileDialog>
#include <QFile>
#include <QSlider>
#include <QMainWindow>
#include <FAST/Data/ImagePyramid.hpp>
#include <FAST/Data/Image.hpp>
#include <FAST/Algorithms/TissueSegmentation/TissueSegmentation.hpp>
#include <FAST/Algorithms/ImagePatch/PatchGenerator.hpp>
#include <FAST/Algorithms/ImagePatch/PatchStitcher.hpp>
#include <FAST/Algorithms/NeuralNetwork/NeuralNetwork.hpp>
#include <FAST/Visualization/HeatmapRenderer/HeatmapRenderer.hpp>
#include <FAST/Algorithms/ImagePatch/ImageToBatchGenerator.hpp>
#include <FAST/Visualization/SegmentationRenderer/SegmentationRenderer.hpp>
#include <FAST/Algorithms/NeuralNetwork/TensorToSegmentation.hpp>
#include <string>
#include<QtWidgets>
#include<QWidget>
#include<QFont>
#include<QBoxLayout>
#include<QToolBar>
#include<QPainter>
#include<QFontMetrics>
#include <iostream>
#include<unistd.h>
#include <algorithm>
#include <vector>
#include <openslide/openslide.h>
#include <stdio.h>      /* printf */
#include <math.h>       /* pow */
#include <istream>
#include <fstream>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <QColorDialog>
#include <QDir>
#include<QLayoutItem>
#include <QObject>


class openslide_get_property_names;

using namespace std;

namespace fast {

MainWindow::MainWindow() {
    setTitle("FastPathology");
    enableMaximized(); // <- function from Window.cpp

    // get current working directory
    get_cwd();

    // create temporary tmp folder to store stuff, and create temporary file to keep history for visualization and
    // other stuff
    create_tmp_folder_file();

    // check if Models folder exist, if not, create one -> platform assumes that this folder exists and contains all relevant models
    std::string dir_str;
    dir_str = cwd + "Models";
    const char * paths = dir_str.c_str();
    if (dirExists(paths) != 1) {
        int aa = mkdir(paths, 0777);
    }

    // changing color to the Qt background
    //mWidget->setStyleSheet("font-size: 16px; background: gray; color: white;");
    //mWidget->setStyleSheet("font-size: 16px; background: rgb(75, 74, 103); color: black;");
    mWidget->setStyleSheet("font-size: 16px; background: rgb(221, 209, 199); color: black;");
    //mWidget->setStyleSheet("font-size: 16px; background: rgb(181, 211, 231); color: black;");
    //mWidget->setStyleSheet("font-size: 16px; background: (255, 125, 50); color: black;");

    float OpenGL_background_color = 0.0f; //200.0f / 255.0f;

    // opacityTumorSlider->setStyleSheet("font-size: 16px"); // <- to set style sheet for a specific button/slider
    //mainestLayout->addLayout(topHorizontalLayout);

    // Create vertical GUI layout:
    mainLayout = new QHBoxLayout;
    mWidget->setLayout(mainLayout);

    createMainMenuWidget();

    auto view = createView();
    //view = mWidget->addView();

    //mainLayout->addLayout(menuLayout);
    mainLayout->insertWidget(1, view);
    view->set2DMode();
    view->setBackgroundColor(Color(OpenGL_background_color, OpenGL_background_color, OpenGL_background_color)); // setting color to the background, around the WSI
    view->setAutoUpdateCamera(true);
}


void MainWindow::createMainMenuWidget() {

    // create widgets for Menu layout
    createFileWidget();
    createProcessWidget(); // TODO: MAKE IT DYNAMIC DEPENDING ON WHICH MODELS ARE AVAILABLE IN MODELS FOLDER
    createViewWidget(); // TODO: MAKE IT DYNAMIC DEPENDING ON WHAT ANALYSIS HAVE BEEN DONE
    createStatsWidget();
    createExportWidget();
    createMenuWidget();

    // add widgets to meny layout
    mainLayout->insertWidget(0, menuWidget);
}


void clearLayout(QLayout *layout) {
    QLayoutItem *item;
    while((item = layout->takeAt(0))) {
        if (item->layout()) {
            clearLayout(item->layout());
            delete item->layout();
        }
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }
}


void MainWindow::createMenuWidget() {
    stackedWidget = new QStackedWidget;
    //stackedWidget->setStyleSheet("border:1px solid rgb(0, 255, 0); ");
    //stackedWidget->setFixedWidth(200);
    //stackedWidget->setStyleSheet("border:1px solid rgb(0, 0, 255); ");
    stackedWidget->insertWidget(0, fileWidget);
    stackedWidget->insertWidget(1, processWidget);
    stackedWidget->insertWidget(2, viewWidget); // TODO: Disable viewWidget at the start, as there is no images to visualize
    stackedWidget->insertWidget(3, statsWidget);
    stackedWidget->insertWidget(4, exportWidget);
    //stackedLayout->setSizeConstraint(QLayout::SetFixedSize);
    //stackedWidget->setLayout(mainLayout);

    int im_size = 40;

    auto mapper = new QSignalMapper;

    auto tb = new QToolBar();
    tb->setIconSize(QSize(im_size, im_size));


    //auto toolBar = new QToolBar;
    QPixmap openPix(QString::fromStdString(cwd + "/Icons/import-data-icon-19.png"));
    QPixmap processPix(QString::fromStdString(cwd + "/Icons/machine-learning-icon-8.png"));
    QPixmap viewPix(QString::fromStdString(cwd + "/Icons/visualize.png"));  //"/home/andre/Downloads/quick-view-icon-8.png");
    QPixmap resultPix(QString::fromStdString(cwd + "/Icons/Simpleicons_Business_bars-chart-up.svg.png"));
    QPixmap savePix(QString::fromStdString(cwd + "/Icons/save-time-icon-1.png"));

    QPainter painter(&savePix);
    QFont font = painter.font();
    font.setPixelSize(100);
    font.setBold(true);
    font.setFamily("Arial");
    painter.setFont(font);
    painter.setPen(*(new QColor(Qt::black)));
    //painter.drawText(QPoint(0, 500), "Read WSI");

    /*
    auto toolButton = new QToolButton(tb);
    toolButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toolButton->setStyleSheet("border: 0px; background-color: red;");
    auto act = new QAction();
    act->setIcon(QIcon("/home/andre/Downloads/import-data-icon-19.png"));
    //act->setText("some text");
    toolButton->setDefaultAction(act);
    mapper->connect(act, SIGNAL(clicked), SLOT(map()));
     */

    // /*
    QAction *file_action = tb->addAction(QIcon(openPix), "Import");
    mapper->setMapping(file_action, 0);
    mapper->connect(file_action, SIGNAL(triggered(bool)), SLOT(map()));

    tb->addSeparator();

    QAction *process_action = tb->addAction(QIcon(processPix),"Process");
    mapper->setMapping(process_action, 1);
    mapper->connect(process_action, SIGNAL(triggered(bool)), SLOT(map()));

    QAction *view_action = tb->addAction(QIcon(viewPix), "View");
    mapper->setMapping(view_action, 2);
    mapper->connect(view_action, SIGNAL(triggered(bool)), SLOT(map()));

    QAction *result_action = tb->addAction(QIcon(resultPix), "Statistics");
    mapper->setMapping(result_action, 3);
    mapper->connect(result_action, SIGNAL(triggered(bool)), SLOT(map()));

    QAction *save_action = tb->addAction(QIcon(savePix), "Export");
    mapper->setMapping(save_action, 4);
    mapper->connect(save_action, SIGNAL(triggered(bool)), SLOT(map()));
    // */


    //stackedWidget->connect(&mapper, SIGNAL(mapped(int)), SLOT(setCurrentIndex(int)));
    connect(mapper, SIGNAL(mapped(int)), stackedWidget, SLOT(setCurrentIndex(int)));


    auto dockLayout = new QVBoxLayout(); //or any other layout type you want
    dockLayout->setMenuBar(tb); // <-- the interesting part

    auto dockContent = new QWidget();
    dockContent->setLayout(dockLayout);

    //yourDockWidget->setWidget(dockContent);

    //QAction *quit = toolbar->addAction(QIcon(openPix), "Open File");

    //toolbar->addAction(QIcon(openPix), "Open File");
    //toolbar->addSeparator();


    /*
    auto pageComboBox = new QComboBox; // <- perhaps use toolbar instead?
    pageComboBox->setFixedWidth(100);
    pageComboBox->addItem(tr("File"));
    pageComboBox->addItem(tr("Process"));
    pageComboBox->addItem(tr("View"));
    pageComboBox->addItem(tr("Save"));
    connect(pageComboBox, SIGNAL(activated(int)), stackedWidget, SLOT(setCurrentIndex(int)));
    //pageComboBox->setCurrentIndex(0);
     */


    dockLayout = new QVBoxLayout;
    dockLayout->insertWidget(0, dockContent); //addWidget(dockContent);
    //tmpLayout->addWidget(pageComboBox);
    dockLayout->insertWidget(1, stackedWidget);

    menuWidget = new QWidget;
    menuWidget->setFixedWidth(300); //300);
    //tmpWidget->setStyleSheet("border:1px solid rgb(0, 255, 0); ");
    menuWidget->setLayout(dockLayout);
}


void MainWindow::createFileWidget() {

    fileLayout = new QVBoxLayout;

    fileWidget = new QWidget;
    fileWidget->setLayout(fileLayout);
    //fileWidget->setFixedWidth(200);

    auto selectFileButton = new QPushButton(mWidget);
    selectFileButton->setText("Select WSI");
    //selectFileButton->setFixedWidth(200);
    selectFileButton->setFixedHeight(50);
    selectFileButton->setStyleSheet("color: white; background-color: blue");
    QObject::connect(selectFileButton, &QPushButton::clicked, std::bind(&MainWindow::selectFile, this));

    auto addModelButton = new QPushButton(mWidget);
    addModelButton->setText("Import model");
    //selectFileButton->setFixedWidth(200);
    addModelButton->setFixedHeight(50);
    addModelButton->setStyleSheet("color: white; background-color: blue");
    QObject::connect(addModelButton, &QPushButton::clicked, std::bind(&MainWindow::addModels, this));

    auto quitButton = new QPushButton(mWidget);
    quitButton->setText("Quit");
    //quitButton->setFixedWidth(200);
    quitButton->setFixedHeight(50);
    quitButton->setStyleSheet("color: black; background-color: red"); //; border-style: outset; border-color: black; border-width: 3px");
    QObject::connect(quitButton, &QPushButton::clicked, std::bind(&Window::stop, this));

    auto smallTextWindow = new QTextEdit;
    smallTextWindow->setPlainText(tr("Hello, this is a prototype of the software I am developing as part of my "
                                     "PhD project. The software is made to be simple, but still contains multiple advanced "
                                     "options either for processing WSIs, deploying trained CNNs or visualizing WSIs and "
                                     "segments and stuff... Have fun! :)"));
    smallTextWindow->setReadOnly(true);

    auto bigEditor = new QTextEdit;
    bigEditor->setPlainText(tr("This widget takes up all the remaining space "
                               "in the top-level layout."));

    fileLayout->insertWidget(0, selectFileButton); //, Qt::AlignTop);
    fileLayout->insertWidget(1, addModelButton);
    fileLayout->insertWidget(2, smallTextWindow);
    fileLayout->insertWidget(3, bigEditor);
    fileLayout->insertWidget(4, quitButton, Qt::AlignTop);
}


void MainWindow::createViewWidget() {

    /*
    auto showHeatmapButton = new QPushButton(mWidget);
    showHeatmapButton->setText("Toggle grade map");
    //showHeatmapButton->setFixedWidth(200);
    showHeatmapButton->setFixedHeight(50);
    showHeatmapButton->setCheckable(true);
    showHeatmapButton->setChecked(true);
    //showHeatmapButton->setChecked(true);
    QObject::connect(showHeatmapButton, &QPushButton::clicked, std::bind(&MainWindow::showHeatmap, this));

    auto showTissueMaskButton = new QPushButton(mWidget);
    showTissueMaskButton->setText("Toggle tissue mask");
    //showTissueMaskButton->setFixedWidth(200);
    showTissueMaskButton->setFixedHeight(50);
    showTissueMaskButton->setCheckable(true);
    showTissueMaskButton->setChecked(true);
    QObject::connect(showTissueMaskButton, &QPushButton::clicked, std::bind(&MainWindow::toggleTissueMask, this));

    auto showBachMaskButton = new QPushButton(mWidget);
    showBachMaskButton->setText("Toggle bach map");
    //showBachMaskButton->setFixedWidth(200);
    showBachMaskButton->setFixedHeight(50);
    showBachMaskButton->setCheckable(true);
    showBachMaskButton->setChecked(true);
    QObject::connect(showBachMaskButton, &QPushButton::clicked, std::bind(&MainWindow::showBachMap, this));

    auto showImageButton = new QPushButton(mWidget);
    showImageButton->setText("Toggle WSI");
    //showImageButton->setFixedWidth(200);
    showImageButton->setFixedHeight(50);
    showImageButton->setCheckable(true);
    showImageButton->setChecked(true);
    QObject::connect(showImageButton, &QPushButton::clicked, std::bind(&MainWindow::showImage, this));

    auto showTumorButton = new QPushButton(mWidget);
    showTumorButton->setText("Toggle tumor map");
    //showTumorButton->setFixedWidth(200);
    showTumorButton->setFixedHeight(50);
    showTumorButton->setCheckable(true);
    showTumorButton->setChecked(true);
    QObject::connect(showTumorButton, &QPushButton::clicked, std::bind(&MainWindow::showTumorMask, this));

    auto hideBackgroundButton = new QPushButton(mWidget);
    hideBackgroundButton->setText("Toggle background class");
    //hideBackgroundButton->setFixedWidth(200);
    hideBackgroundButton->setFixedHeight(50);
    hideBackgroundButton->setCheckable(true);
    hideBackgroundButton->setChecked(true);
    QObject::connect(hideBackgroundButton, &QPushButton::clicked, std::bind(&MainWindow::hideBackgroundClass, this));

    auto opacityTissueSlider = new QSlider(Qt::Horizontal, mWidget);
    opacityTissueSlider->setFixedWidth(150);
    opacityTissueSlider->setMinimum(0);
    opacityTissueSlider->setMaximum(10);
    //opacityTissueSlider->setText("Tissue");
    opacityTissueSlider->setValue(4);
    opacityTissueSlider->setTickInterval(1);
    QObject::connect(opacityTissueSlider, &QSlider::valueChanged, std::bind(&MainWindow::opacityTissue, this, std::placeholders::_1));

    auto opacityHeatmapSlider = new QSlider(Qt::Horizontal, mWidget);
    opacityHeatmapSlider->setFixedWidth(150);
    opacityHeatmapSlider->setMaximum(0);
    opacityHeatmapSlider->setMaximum(10);
    opacityHeatmapSlider->setValue(6);
    opacityHeatmapSlider->setTickInterval(1);
    QObject::connect(opacityHeatmapSlider, &QSlider::valueChanged, std::bind(&MainWindow::opacityHeatmap, this, std::placeholders::_1));

    auto opacityTumorSlider = new QSlider(Qt::Horizontal, mWidget);
    opacityTumorSlider->setFixedWidth(150);
    opacityTumorSlider->setMaximum(0);
    opacityTumorSlider->setMaximum(10);
    opacityTumorSlider->setValue(6);
    opacityTumorSlider->setTickInterval(1);
    QObject::connect(opacityTumorSlider, &QSlider::valueChanged, std::bind(&MainWindow::opacityTumor, this, std::placeholders::_1));

    auto label_tumor = new QLabel();
    label_tumor->setText("Tumor: ");
    label_tumor->setFixedWidth(50);
    auto smallTextBox_tumor = new QHBoxLayout;
    smallTextBox_tumor->addWidget(label_tumor);
    smallTextBox_tumor->addWidget(opacityTumorSlider);
    auto smallTextBoxWidget_tumor = new QWidget;
    smallTextBoxWidget_tumor->setFixedHeight(50);
    smallTextBoxWidget_tumor->setLayout(smallTextBox_tumor);

    auto label_tissue = new QLabel();
    label_tissue->setText("Tissue: ");
    label_tissue->setFixedWidth(50);
    auto smallTextBox_tissue = new QHBoxLayout;
    smallTextBox_tissue->addWidget(label_tissue);
    smallTextBox_tissue->addWidget(opacityTissueSlider);
    auto smallTextBoxWidget_tissue = new QWidget;
    smallTextBoxWidget_tissue->setFixedHeight(50);
    smallTextBoxWidget_tissue->setLayout(smallTextBox_tissue);

    auto label_grade = new QLabel();
    label_grade->setText("Grade: ");
    label_grade->setFixedWidth(50);
    auto smallTextBox_grade = new QHBoxLayout;
    smallTextBox_grade->addWidget(label_grade);
    smallTextBox_grade->addWidget(opacityHeatmapSlider);
    auto smallTextBoxWidget_grade = new QWidget;
    smallTextBoxWidget_grade->setFixedHeight(50);
    smallTextBoxWidget_grade->setLayout(smallTextBox_grade);
    */


    /*
    // make QColorDialog for manually setting color to different classes
    auto colorSetWidget = new QColorDialog;
    colorSetWidget->setOption(QColorDialog::DontUseNativeDialog, true);
     */

    //viewLayout = new QVBoxLayout;
    //viewLayout->setAlignment(Qt::AlignTop);

    //viewWidget = new QWidget;
    //viewWidget->setLayout(viewLayout);
    //viewWidget->setFixedWidth(200);


    //// :)
    viewLayout = new QVBoxLayout;
    viewLayout->setAlignment(Qt::AlignTop);

    viewWidget = new QWidget;
    viewWidget->setLayout(viewLayout);
    //viewWidget->setFixedWidth(200);

    // create dynamic widget
    //createDynamicViewWidget(std::vector<bool>{true, true, true});


    // ComboBox in view section to set which image object to change
    // /*
    auto curr1PageWidget = new QWidget;
    auto curr2PageWidget = new QWidget;
    auto curr3PageWidget = new QWidget;
    auto curr4PageWidget = new QWidget;
    auto curr5PageWidget = new QWidget;
    // */

    auto wsiPageWidget = new QWidget;


    stackedLayout = new QStackedLayout;
    //stackedLayout->addWidget(curr1PageWidget);
    //stackedLayout->addWidget(curr1PageWidget);
    //stackedLayout->addWidget(dynamicViewWidget);
    //stackedLayout->addWidget(dynamicViewWidget);
    //stackedLayout->addWidget(dynamicViewWidget);

    auto stackedWidget = new QWidget;
    stackedWidget->setLayout(stackedLayout);

    //auto comboBoxLayout = new QVBoxLayout;
    //comboBoxLayout->addLayout(stackedLayout); //addLayout(stackedLayout);

    pageComboBox = new QComboBox;
    pageComboBox->setFixedWidth(150);
    //pageComboBox->addItem(tr("WSI"));
    //pageComboBox->addItem(tr("Tissue"));
    //pageComboBox->addItem(tr("Tumor"));
    //pageComboBox->addItem(tr("Grade"));
    //pageComboBox->addItem(tr("BACH"));
    connect(pageComboBox, SIGNAL(activated(int)), stackedLayout, SLOT(setCurrentIndex(int)));

    auto imageNameTexts = new QLabel();
    imageNameTexts->setText("Image: ");
    imageNameTexts->setFixedWidth(50);
    auto smallTextBox_imageName = new QHBoxLayout;
    smallTextBox_imageName->addWidget(imageNameTexts);
    smallTextBox_imageName->addWidget(pageComboBox);
    auto smallTextBoxWidget_imageName = new QWidget;
    smallTextBoxWidget_imageName->setFixedHeight(50);
    smallTextBoxWidget_imageName->setLayout(smallTextBox_imageName);

    viewLayout->insertWidget(0, smallTextBoxWidget_imageName);
    viewLayout->setAlignment(Qt::AlignTop);

    // add stuff to QComboBox
    //createDynamicViewWidget("WSI");
    //createDynamicViewWidget("tissue");
    //createDynamicViewWidget("tumor");
    //createDynamicViewWidget("tumor");

    //viewLayout->insertWidget(1, showImageButton);
    viewLayout->insertWidget(1, stackedWidget); //stackedWidget);//dynamicViewWidget);

    //viewLayout->addWidget(fixImageButton);


    // */

    /*
    auto fixTumorSegButton = new QPushButton(mWidget);
    fixTumorSegButton->setText("Fix tumor segment");
    fixTumorSegButton->setFixedWidth(200);
    fixTumorSegButton->setFixedHeight(50);
    QObject::connect(fixTumorSegButton, &QPushButton::clicked, std::bind(&MainWindow::fixTumorSegment, this));
    */

    /*
    auto saveTumorPredButton = new QPushButton(mWidget);
    saveTumorPredButton->setText("Save tumor pred");
    saveTumorPredButton->setFixedWidth(200);
    saveTumorPredButton->setFixedHeight(50);
    QObject::connect(saveTumorPredButton, &QPushButton::clicked, std::bind(&MainWindow::saveTumorPred, this));
    */

}


void MainWindow::createDynamicViewWidget(const std::string& someName, std::string modelName) {

    //std::string someName = "tumor";

    auto imageButton = new QPushButton(mWidget);
    imageButton->setText("Toggle image");
    //showHeatmapButton->setFixedWidth(200);
    imageButton->setFixedHeight(50);
    imageButton->setCheckable(true);
    imageButton->setChecked(true);
    //showHeatmapButton->setChecked(true);
    QObject::connect(imageButton, &QPushButton::clicked, std::bind(&MainWindow::toggleRenderer, this, someName));
    //imageButton->setDisabled(visible[0]);

    // for WSI this should be grayed out, shouldn't be able to change it
    auto opacitySlider = new QSlider(Qt::Horizontal, mWidget);
    opacitySlider->setFixedWidth(150);
    opacitySlider->setMinimum(0);
    opacitySlider->setMaximum(10);
    //opacityTissueSlider->setText("Tissue");
    opacitySlider->setValue(4);
    opacitySlider->setTickInterval(1);
    QObject::connect(opacitySlider, &QSlider::valueChanged, std::bind(&MainWindow::opacityRenderer, this, std::placeholders::_1, someName));

    auto label_tissue = new QLabel();
    std::string tmpSomeName = someName;
    tmpSomeName[0] = toupper(tmpSomeName[0]);
    label_tissue->setText(QString::fromStdString(tmpSomeName + ": "));
    label_tissue->setFixedWidth(50);
    auto smallTextBox_tissue = new QHBoxLayout;
    smallTextBox_tissue->addWidget(label_tissue);
    smallTextBox_tissue->addWidget(opacitySlider);
    auto smallTextBoxWidget_tissue = new QWidget;
    smallTextBoxWidget_tissue->setFixedHeight(50);
    smallTextBoxWidget_tissue->setLayout(smallTextBox_tissue);
    //smallTextBoxWidget_tissue->setDisabled(visible[1]);

    // make QColorDialog for manually setting color to different classes
    auto colorSetWidget = new QColorDialog;
    colorSetWidget->setOption(QColorDialog::DontUseNativeDialog, true);
    //colorSetWidget->setDisabled(visible[2]);

    // to be able to set which classes to show/hide
    channel_value = 0;

    auto toggleShowButton = new QPushButton(mWidget);
    toggleShowButton->setText("Toggle class");
    //showHeatmapButton->setFixedWidth(200);
    toggleShowButton->setFixedHeight(50);
    //toggleShowButton->setFixedWidth(100);
    toggleShowButton->setCheckable(true);
    toggleShowButton->setChecked(true);
    //showHeatmapButton->setChecked(true);
    QObject::connect(toggleShowButton, &QPushButton::clicked, std::bind(&MainWindow::hideChannel, this, someName));
    //imageButton->setDisabled(visible[0]);

    //    connect(pageComboBox, SIGNAL(activated(int)), stackedLayout, SLOT(setCurrentIndex(int)));

    /*
    auto pageWidget1 = new QLabel();
    pageWidget1->setText("background");
    auto pageWidget2 = new QLabel();
    pageWidget2->setText("tumor");
     */

    //auto currStackedLayout = new QStackedLayout;
    //currStackedLayout->insertWidget(0, pageWidget1);
    //currStackedLayout->insertWidget(1, pageWidget2);
    //currStackedLayout->insertWidget(0, )
    //currStackedLayout->insertWidget(0, );

    //std::cout << "\n hallo her er vi" << someClasses << "\n sup";
    //QList myList = QList::fromVector(QVector::fromStdVector(someVector));

    //QStringList someClasses = {"Background", "Tumor"};

    auto currComboBox = new QComboBox;
    //currComboBox->setFixedWidth(150);
    //currComboBox->deleteLater();
    //currComboBox->clear();
    //currComboBox->setEditable(false);

    if ((someName != "WSI") and (someName != "tissue")) {
        // get metadata of current model
        std::map<std::string, std::string> metadata = getModelMetadata(modelName);
        std::cout << "\n" << metadata["class_names"] << "\n classes \n";
        std::vector someVector = split(metadata["class_names"], ";");
        //QStringList myList;
        //QList<QString> someClasses;
        // clear vector first
        currentClassesInUse.clear();
        for (const auto & i : someVector){
            std::cout << "\n her er vi class" << i << "\n";
            currentClassesInUse.append(QString::fromStdString(i));
        }
        currComboBox->clear();
        currComboBox->update();
        currComboBox->insertItems(0, currentClassesInUse);
        //currComboBox->currentTextChanged()
    }
    connect(currComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateChannelValue(int)));

    auto imageNameTexts = new QLabel();
    imageNameTexts->setText("Class: ");
    imageNameTexts->setFixedWidth(50);
    auto smallTextBox_imageName = new QHBoxLayout;
    smallTextBox_imageName->addWidget(imageNameTexts);
    smallTextBox_imageName->addWidget(currComboBox);
    auto smallTextBoxWidget_imageName = new QWidget;
    smallTextBoxWidget_imageName->setFixedHeight(50);
    smallTextBoxWidget_imageName->setLayout(smallTextBox_imageName);
    auto biggerTextBox_imageName = new QVBoxLayout;
    biggerTextBox_imageName->addWidget(smallTextBoxWidget_imageName);
    biggerTextBox_imageName->addWidget(toggleShowButton);
    biggerTextBox_imageName->setAlignment(Qt::AlignTop);
    auto biggerTextBoxWidget_imageName = new QWidget;
    biggerTextBoxWidget_imageName->setLayout(biggerTextBox_imageName);

    // disable some features for specific types
    if (someName == "WSI") {
        opacitySlider->setDisabled(true);
        colorSetWidget->setDisabled(true);
        biggerTextBoxWidget_imageName->setDisabled(true);
    } else if (someName == "tissue") {
        biggerTextBoxWidget_imageName->setDisabled(true);
    }

    dynamicViewLayout = new QVBoxLayout;
    dynamicViewLayout->setAlignment(Qt::AlignTop);

    dynamicViewWidget = new QWidget;
    dynamicViewWidget->setLayout(dynamicViewLayout);

    dynamicViewLayout->insertWidget(0, imageButton);
    //dynamicViewLayout->insertWidget(1, opacitySlider);
    dynamicViewLayout->insertWidget(1, smallTextBoxWidget_tissue);
    dynamicViewLayout->insertWidget(2, colorSetWidget);
    dynamicViewLayout->insertWidget(3, biggerTextBoxWidget_imageName);

    // add widget to QComboBox
    pageComboBox->addItem(tr(tmpSomeName.c_str()));
    stackedLayout->addWidget(dynamicViewWidget);

    /*
    dynamicViewLayout->insertWidget(0, showImageButton);
    dynamicViewLayout->insertWidget(1, showTissueMaskButton);
    dynamicViewLayout->insertWidget(2, showTumorButton);
    dynamicViewLayout->insertWidget(3, showHeatmapButton);
    dynamicViewLayout->insertWidget(4, showBachMaskButton);
    dynamicViewLayout->insertWidget(5, smallTextBoxWidget_tissue);
    dynamicViewLayout->insertWidget(6, smallTextBoxWidget_tumor);
    dynamicViewLayout->insertWidget(7, smallTextBoxWidget_grade);
    dynamicViewLayout->insertWidget(8, colorSetWidget);
    dynamicViewLayout->insertWidget(9, hideBackgroundButton);
     */
}


void MainWindow::updateChannelValue(int index) {
    std::cout << "\n\n We updated the channel value! \n\n";
    channel_value = (uint) index;
}


void MainWindow::createStatsWidget() {

    statsLayout = new QVBoxLayout;

    statsWidget = new QWidget;
    statsWidget->setLayout(statsLayout);

    // make button that prints distribution of pixels of each class -> for histogram
    auto calcTissueHistButton = new QPushButton(mWidget);
    calcTissueHistButton->setText("Calculate tissue histogram");
    calcTissueHistButton->setFixedHeight(50);
    QObject::connect(calcTissueHistButton, &QPushButton::clicked, std::bind(&MainWindow::calcTissueHist, this));

    int barWidth = 9;
    int boxWidth = 250;
    int boxHeight = 250;
    QPixmap pm(boxWidth, boxHeight);
    pm.fill();

    int len = 5;

    //int x_vec[] = {1, 2, 3, 4, 5};
    std::string x_vec[] = {"a", "b", "c", "d", "e"};
    int y_vec[] = {12, 20, 43, 30, 5};

    double drawMinHeight = 0.1 * (double)(boxHeight);
    double drawMinWidth = 0.1 * (double)(boxWidth);

    double newWidth = (double)(boxWidth - drawMinWidth);
    double newHeight = (double)(boxHeight - drawMinHeight);

    double maxHeight = *std::max_element(y_vec,y_vec+len);
    double drawMaxHeight = (double)(maxHeight + maxHeight*0.1);

    auto painters = new QPainter(&pm);
    painters->setPen(QColor(140, 140, 210));

    for (int i = 0; i < len; i++) {
        std::cout<<std::to_string(i)<<"\n\n\n";
        qreal h = y_vec[i] * maxHeight;
        // draw level
        painters->fillRect(drawMinWidth / 2 + (double)(i + 1) * (double)(newWidth) / (double)(len + 1) - (double)((double)(barWidth)/(double)(2)),
                           newHeight,
                           barWidth,
                           - (double)(y_vec[i]) / (double)(drawMaxHeight) * (double)(newHeight),
                           Qt::blue);
        // clear the rest of the control
        //painters->fillRect(barWidth * i, 0, barWidth * (i + 1), maxHeight - h, Qt::black);
    }

    int lineWidth = 3;
    int space = drawMinHeight;

    //auto linePainter = new QPainter(&pm);
    painters->setPen(QPen(QColor(0, 0, 0), lineWidth));
    painters->drawLine(space, boxHeight - space, boxWidth - space, boxHeight - space);
    painters->drawLine(space - 1, boxHeight - space, space - 1, space);

    // add ticks on lines
    painters->setPen(QPen(QColor(0, 0, 0), 2));
    painters->setFont(QFont("times", 10));
    std::string stringIter;
    int xTextSpace = 18;
    int numTicks = 5;
    double tickSize = 10;
    for (int j = 0; j < numTicks; j++) {
        painters->drawLine(drawMinWidth / 2 + (double)(j + 1) * (double)(newWidth) / (double)(len + 1),
                           newHeight,
                           drawMinWidth / 2 + (double)(j + 1) * (double)(newWidth) / (double)(len + 1),
                           newHeight + tickSize / 2);
        painters->drawText(drawMinWidth / 2 + (double)(j + 1) * (double)(newWidth) / (double)(len + 1) - 4, newHeight + xTextSpace, QString::number(j + 1));

    }

    int yTextSpace = 22;
    for (int j = 0; j < numTicks; j++) {
        painters->drawLine(space - tickSize / 2,
                           drawMinHeight - drawMinHeight*0.5 + (double)(j + 1) * (double)(newHeight) / (double)(len + 1),
                           space,
                           drawMinHeight - drawMinHeight*0.5 + (double)(j + 1) * (double)(newHeight) / (double)(len + 1));

        painters->drawText(drawMinWidth - yTextSpace, drawMinHeight - drawMinHeight*0.5 + (double)(j + 1) * (double)(newHeight) / (double)(len + 1) + 4,
                           QString::number((int)(std::round((double)(numTicks - j) * (double)(maxHeight) / (double)(len + 1) + 4))));
        //painters->drawText(drawMinWidth - yTextSpace, drawMinHeight - drawMinHeight*0.5 + (double)(j + 1) * (double)(newHeight) / (double)(len + 1) + 4,
        //        QString::number((int)(y_vec[numTicks - j - 1])));

    }

    // draw arrow heads on end of axes
    int xArrowSize = 8;
    int yArrowSize = 16;
    QRectF rect = QRectF(space - xArrowSize + lineWidth, space - yArrowSize, xArrowSize, yArrowSize);

    QPainterPath path;
    path.moveTo(rect.left() + (rect.width() / 2), rect.top());
    path.lineTo(rect.bottomLeft());
    path.lineTo(rect.bottomRight());
    path.lineTo(rect.left() + (rect.width() / 2), rect.top());

    painters->fillPath(path, QBrush(QColor ("black")));

    QTransform t;
    t.translate(boxWidth, boxHeight - space - yArrowSize - xArrowSize);
    t.rotate(90);
    QPainterPath path2 = t.map(path);

    // draw title of histogram
    painters->setPen(QPen(QColor(0, 0, 0), 2));
    painters->setFont(QFont("times", 10));


    painters->fillPath(path2, QBrush(QColor ("black")));

    auto histBox = new QLabel;
    histBox->setPixmap(pm);

    auto smallTextWindowStats = new QTextEdit;
    smallTextWindowStats->setPlainText(tr("This is some window with text that displays some relevant"
                                          "information regarding the inference or analysis you just did"));
    smallTextWindowStats->setReadOnly(true);

    statsLayout->addWidget(calcTissueHistButton);
    statsLayout->addWidget(histBox);
    statsLayout->addWidget(smallTextWindowStats);
}

void MainWindow::createExportWidget() {

    exportLayout = new QVBoxLayout;

    exportWidget = new QWidget;
    exportWidget->setLayout(exportLayout);

    // make button that prints distribution of pixels of each class -> for histogram
    auto exportSegmentationButton = new QPushButton(mWidget);
    exportSegmentationButton->setText("Export segmentation");
    exportSegmentationButton->setFixedHeight(50);
    QObject::connect(exportSegmentationButton, &QPushButton::clicked, std::bind(&MainWindow::exportSeg, this));

    auto smallTextWindowExport = new QTextEdit;
    smallTextWindowExport->setPlainText(tr("Yes some info yes"));
    smallTextWindowExport->setReadOnly(true);

    exportLayout->addWidget(exportSegmentationButton);
    exportLayout->addWidget(smallTextWindowExport);
}

void MainWindow::createProcessWidget() {

    processLayout = new QVBoxLayout;
    processLayout->setSpacing(6); // 50
    processLayout->setAlignment(Qt::AlignTop);

    processWidget = new QWidget();  //mWidget);
    processWidget->setLayout(processLayout);
    //processWidget->setFixedWidth(200);

    auto segTissueButton = new QPushButton(mWidget);
    segTissueButton->setText("Segment Tissue");
    //segTissueButton->setFixedWidth(200);
    segTissueButton->setFixedHeight(50);
    QObject::connect(segTissueButton, &QPushButton::clicked, std::bind(&MainWindow::segmentTissue, this));

    // add tissue segmentation to widget (should always exist as built-in FAST)
    processLayout->insertWidget(0, segTissueButton);

    // check Models folder which models are available and make button for each model and add to widget
    //QDir directory = QFileDialog::getExistingDirectory(this, tr("select directory"));

    QDir directory(QString::fromStdString(cwd + "Models/"));
    QStringList paths = directory.entryList(QStringList() << "*.txt" << "*.TXT",QDir::Files);
    int counter=1;
    foreach(QString currFile, paths) {
        std::cout << "\n" << currFile.toStdString() << "\n halloeeen";

        // current model
        modelName = split(currFile.toStdString(), ".")[0];

        // get metadata of current model
        std::map<std::string, std::string> metadata = getModelMetadata(modelName);

        auto someButton = new QPushButton(mWidget);
        someButton->setText(QString::fromStdString(metadata["task"]));
        //predGradeButton->setFixedWidth(200);
        someButton->setFixedHeight(50);
        QObject::connect(someButton, &QPushButton::clicked, std::bind(&MainWindow::patchClassifier, this, modelName));
        someButton->show();

        processLayout->insertWidget(counter, someButton);

        counter++;
    }

    std::vector<string> modelPaths;

    /*
    auto predGradeButton = new QPushButton(mWidget);
    predGradeButton->setText("Predict histological grade");
    //predGradeButton->setFixedWidth(200);
    predGradeButton->setFixedHeight(50);
    //QObject::connect(predGradeButton, &QPushButton::clicked, std::bind(&MainWindow::predictGrade, this));
    //modelName = "grade"; //getModelMetadata();
    modelName = "bc_grade_model_3_grades_10x_512";
    QObject::connect(predGradeButton, &QPushButton::clicked, std::bind(&MainWindow::patchClassifier, this));

    auto predTumorButton = new QPushButton(mWidget);
    predTumorButton->setText("Predict tumor");
    //predTumorButton->setFixedWidth(200);
    predTumorButton->setFixedHeight(50);

    QObject::connect(predTumorButton, &QPushButton::clicked, std::bind(&MainWindow::predictTumor, this));

    //

    auto predBachButton = new QPushButton(mWidget);
    predBachButton->setText("Predict BACH");
    //predBachButton->setFixedWidth(200);
    predBachButton->setFixedHeight(50);

    QObject::connect(predBachButton, &QPushButton::clicked, std::bind(&MainWindow::predictBACH, this));

    processLayout->addWidget(predTumorButton);
    processLayout->addWidget(predGradeButton);
    processLayout->addWidget(predBachButton);

    */
}


void MainWindow::create_tmp_folder_file() {
    // create temporary tmp folder to store temporary stuff, checks if folder already exists, if yes, delete it
    // and make a new one -> temporary, only for current session
    std::string dir_str;
    dir_str = cwd + "tmp";
    const char * paths = dir_str.c_str();
    auto a = dirExists(paths);
    if (a == 1) {
        int bb = rmdir(paths);
    }
    int aa = mkdir(paths, 0777);

    // create temporary file for writing and parsing, showing what has been done and what is available
    // for the View widget
    auto tmp_file_path = dir_str + "/history.txt";
    if (fileExists(tmp_file_path)) {
        int cc = std::remove(tmp_file_path.c_str());
    }
    std::ofstream outfile(tmp_file_path);
    outfile.close();
}


void MainWindow::get_cwd() {
    long size;
    char *buf;
    char *ptr;

    size = pathconf(".", _PC_PATH_MAX);

    if ((buf = (char *) malloc((size_t) size)) != NULL)
        ptr = getcwd(buf, (size_t) size);

    cwd = ptr;
    std::string delimiter = "cmake-build-release";
    std::string delimiter2 = "cmake-build-debug";

    // Search for the substring in string
    size_t pos = cwd.find(delimiter);
    if (pos != std::string::npos) {
        // If found then erase it from string
        cwd.erase(pos, delimiter.length());
    }

    // Search for the substring in string
    size_t pos2 = cwd.find(delimiter2);

    if (pos2 != std::string::npos) {
    // If found then erase it from string
        cwd.erase(pos2, delimiter2.length());
    }
}


int MainWindow::dirExists(const char *path)
{
    struct stat info;

    if(stat( path, &info ) != 0)
        return 0;
    else if(info.st_mode & S_IFDIR)
        return 1;
    else
        return 0;
}


void MainWindow::drawHist() {
    int aa = 2;
}


void MainWindow::createActions() {
    newAct = new QAction(tr("&New"), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("Create a new file"));
    connect(newAct, &QAction::triggered, this, &MainWindow::selectFile);
}


void MainWindow::selectFile() {

    auto fileName = QFileDialog::getOpenFileName(
            mWidget,
            "Open File", nullptr, "WSI Files (*.tiff *.tif *.svs)"
            );

    if(fileName == "")
        return;
    filename = fileName.toStdString();

    // if new file is chosen, clear all cache of previous images
    //renderer->clearPyramid();
    //tumorRenderer->clearHeatmap();

    // Import image from file using the ImageFileImporter
    auto importer = WholeSlideImageImporter::New();
    importer->setFilename(fileName.toStdString());
    m_image = importer->updateAndGetOutputData<ImagePyramid>();
    //renderer->setInputData(m_image);

    auto renderer = ImagePyramidRenderer::New();
    renderer->setInputData(m_image);

    // TODO: Something here results in me not being able to run analyzis on new images (after the first)
    removeAllRenderers();
    insertRenderer("WSI", renderer);
    getView(0)->reinitialize(); // Must call this after removing all renderers

    wsiFormat = getWsiFormat();
    std::cout << "\n" << wsiFormat << "\nsjokolade";

    // get magnification level of current WSI
    magn_lvl = getMagnificationLevel();

    // now make it possible to edit image in the View Widget
    createDynamicViewWidget("WSI", modelName);

}


void MainWindow::addModels() {

    auto fileName = QFileDialog::getOpenFileName(
            mWidget,
            "Select Model", nullptr, "Model Files (*.pb *.txt)"
    );

    if(fileName == "")
        return;

    //qDebug().nospace() << "abc" << qPrintable(fileName) << "\ndef";
    std::string someFile = split(fileName.toStdString(), "/").back(); // TODO: Need to make this only split on last "/"
    std::string oldLocation = split(fileName.toStdString(), someFile)[0];

    std::string newLocation = cwd + "Models/";

    std::vector<string> names = split(someFile, ".");
    string fileNameNoFormat = names[0];
    string formatName = names[1];

    // copy selected file to Models folder
    // check if file already exists in new folder, if yes, print warning, and stop
    string newPath = cwd + "Models/" + someFile;
    if (fileExists(newPath)) {
        std::cout << "\n file with same name already exists in folder, didn't transfer... \n";
        return;
    }
    QFile::copy(fileName, QString::fromStdString(newPath));

    // check which corresponding model files that exist, except from the one that is chosen
    std::vector<string> allowedFileFormats{"txt", "pb", ""};

    if (formatName == "txt") {
        string oldPath = oldLocation + fileNameNoFormat + ".pb";
        if (fileExists(oldPath)) {
            QFile::copy(QString::fromStdString(oldPath), QString::fromStdString(cwd + "Models/" + fileNameNoFormat + ".pb"));
        } else {
            std::cout << "\n corresponding .pb file doesn't exist! \n";
            return;
        }
    } else {
        string oldPath = oldLocation + fileNameNoFormat + ".txt";
        if (fileExists(oldPath)) {
            QFile::copy(QString::fromStdString(oldPath), QString::fromStdString(cwd + "Models/" + fileNameNoFormat + ".txt"));
        } else {
            std::cout << "\n corresponding .txt file doesn't exist! \n";
            return;
        }
    }

    // when models are added, ProcessWidget should be updated by adding the new widget to ProcessWidget layout
    // current model
    modelName = fileNameNoFormat;

    // get metadata of current model
    std::map<std::string, std::string> metadata = getModelMetadata(modelName);

    auto someButton = new QPushButton(mWidget);
    someButton->setText(QString::fromStdString(metadata["task"]));
    //predGradeButton->setFixedWidth(200);
    someButton->setFixedHeight(50);
    QObject::connect(someButton, &QPushButton::clicked, std::bind(&MainWindow::patchClassifier, this, modelName));
    someButton->show();

    processLayout->insertWidget(processLayout->count(), someButton);

    //createMainMenuWidget();
    //if (processLayout){
    //    clearLayout(processLayout);
    //}
    //createProcessWidget();
    //mainLayout->removeWidget(menuWidget); // this doesnt work...

}


std::string MainWindow::getWsiFormat() {
    openslide_t* osr = openslide_open(filename.c_str());
    //const char tmp = openslide_detect_vendor(osr); // <- didn't work...
    return openslide_get_property_value(osr, "openslide.vendor");
}


float MainWindow::getMagnificationLevel() {
    // read relevant metadata and store in list
    openslide_t* osr = openslide_open(filename.c_str());
    float magnification_lvl = 0.0f;
    if (wsiFormat == "generic-tiff") {
        int level_count = openslide_get_level_count(osr);
        std::vector<float> downsample_for_each_level;
        for (int i = 0; i < level_count; i++) {
            downsample_for_each_level.push_back(openslide_get_level_downsample(osr, i));
        }
        std::cout << "\n" << downsample_for_each_level[1] << "\n\n hallo";

        // assuming each 400X resolution correspond to 50000 cm, and thus 200x => 100000 cm ... (for .tiff format)
        // can predict which WSI resolution image is captured with, as some models is trained on 200x images
        std::vector<float> templateResVector;
        float resFractionValue = 50000;
        for (int i = 0; i < level_count; i++) {
            templateResVector.push_back(resFractionValue * ((float) i + 1));
        }

        auto resolution = atof(openslide_get_property_value(osr, "tiff.XResolution"));
        std::cout << resolution << "hallais";

        // find closest value => get magnification level
        auto i = min_element(begin(templateResVector), end(templateResVector), [=](int x, int y) {
            return abs(x - resolution) < abs(y - resolution);
        });
        float location = std::distance(begin(templateResVector), i);

        magnification_lvl = 40.0f / pow(2.0f, location);

        std::cout << magnification_lvl << "halloen";
    } else if (wsiFormat == "aperio"){
        magnification_lvl = atof(openslide_get_property_value(osr, "aperio.AppMag"));
    } else {  //"TODO: Make this more general, test different image formats to see how the magn_lvl metadata vary"
        magnification_lvl = atof(openslide_get_property_value(osr, "aperio.AppMag"));
    }
    return magnification_lvl;
}


std::vector<float> MainWindow::getDownsamplingLevels() {
    openslide_t* osr = openslide_open(filename.c_str());
    int level_count = openslide_get_level_count(osr);

    std::vector<float> downsample_for_each_level;
    for (int i=0; i<level_count; i++) {
        downsample_for_each_level.push_back(openslide_get_level_downsample(osr, i));
    }
    return downsample_for_each_level;
}


float MainWindow::getDownsamplingAtLevel(int value) {
    return getDownsamplingLevels()[value];
}


bool MainWindow::segmentTissue() {

    if (hasRenderer("tissue")) { // if analysis is ran again on same WSI, don't.
        std::cout << "Analysis on current WSI has already been performed... \n";
        return false;
    } else {
        std::cout << "I was here, should be here :)! \n";

        // Patch wise classification -> heatmap
        //auto tissueSegmentation = TissueSegmentation::New();
        auto tissueSegmentation = TissueSegmentation::New();
        tissueSegmentation->setInputData(m_image);
        m_tissue = tissueSegmentation->updateAndGetOutputData<Image>();

        auto someRenderer = SegmentationRenderer::New();
        someRenderer->setColor(Segmentation::LABEL_FOREGROUND, Color(255.0/255.0, 127.0/255.0, 80.0/255.0));
        someRenderer->setInputData(m_tissue);
        someRenderer->setOpacity(0.4); // <- necessary for the quick-fix temporary solution
        someRenderer->update();

        insertRenderer("tissue", someRenderer); // TODO: Seems like it hangs when I want to replace renderer if it already exists, or something around this (?)

        //hideTissueMask(false);

        // now make it possible to edit prediction in the View Widget
        createDynamicViewWidget("tissue", modelName);

        return true;
    }
}

bool MainWindow::patchClassifier(std::string modelName) {

    // read model metadata (txtfile)
    std::map<std::string, std::string> metadata = getModelMetadata(modelName);

    //if (hasRenderer(metadata["name"])) {
    //    removeRenderer(metadata["name"]);
    //}
    //auto tmpRenderer = getRenderer(metadata["name"]);

    // based on predicted magnification level of WSI, set magnificiation level for optimal input to model based on predicted resolution of WSI
    int patch_lvl_model = (int) (std::log(magn_lvl / (float)std::stoi(metadata["magnification_level"])) / std::log(std::round(getDownsamplingAtLevel(1))));

    // segment tissue
    segmentTissue(); // TODO: Something weird happens when I run this again

    auto generator = PatchGenerator::New();
    generator->setPatchSize(std::stoi(metadata["input_img_size_y"]), std::stoi(metadata["input_img_size_x"]));
    generator->setPatchLevel(patch_lvl_model);
    generator->setInputData(0, m_image);
    generator->setInputData(1, m_tissue);
    hideTissueMask(true);

    auto batchgen = ImageToBatchGenerator::New();
    batchgen->setInputConnection(generator->getOutputPort());
    batchgen->setMaxBatchSize(std::stoi(metadata["batch_process"]));

    auto network = NeuralNetwork::New();
    network->setInputNode(0, metadata["input_node"], NodeType::IMAGE, TensorShape({std::stoi(metadata["batch_size"]), std::stoi(metadata["input_img_size_y"]), std::stoi(metadata["input_img_size_x"]), std::stoi(metadata["nb_channels"])})); //{1, size, size, 3}
    network->setOutputNode(0, metadata["output_node"], NodeType::IMAGE, TensorShape({std::stoi(metadata["batch_size"]), std::stoi(metadata["input_img_size_y"]), std::stoi(metadata["input_img_size_y"]), std::stoi(metadata["nb_channels"])})); //{1, size, size, 2}
    network->load(cwd + "Models/" + modelName + ".pb");
    network->setInputConnection(batchgen->getOutputPort());
    vector scale_factor = split(metadata["scale_factor"], "/"); // get scale factor from metadata
    network->setScaleFactor((float)std::stoi(scale_factor[0]) / (float)std::stoi(scale_factor[1]));   // 1.0f/255.0f
    //network->setInferenceEngine("TensorFlowCPU");

    auto stitcher = PatchStitcher::New();
    stitcher->setInputConnection(network->getOutputPort());

    auto port = stitcher->getOutputPort();
    //port->getNextFrame();
    //stitcher->update();
    //port->getNextFrame<Tensor>();

    // define renderer from metadata
    auto heatmapRenderer = HeatmapRenderer::New();
    vector<string> colors = split(metadata["class_colors"], ";");
    for (int i = 0; i < std::stoi(metadata["nb_classes"]); i++) {
        vector<string> rgb = split(colors[i], ",");
        heatmapRenderer->setChannelColor(i, Color((float)std::stoi(rgb[0]) / 255.0f, (float)std::stoi(rgb[1]) / 255.0f, (float)std::stoi(rgb[2]) / 255.0f));
    }
    heatmapRenderer->setInterpolation(std::stoi(metadata["interpolation"].c_str()));
    heatmapRenderer->setInputConnection(port);
    //heatmapRenderer->update();

    insertRenderer(metadata["name"], heatmapRenderer);

    // now make it possible to edit prediction in the View Widget
    createDynamicViewWidget(metadata["name"], modelName);
}


bool MainWindow::predictGrade() {

    // define renderer
    auto heatmapRenderer = HeatmapRenderer::New();
    heatmapRenderer->setChannelColor(0, Color::Blue());
    heatmapRenderer->setInterpolation(false);
    heatmapRenderer->setChannelColor(1, Color(50.0f/255.0f, 205.0f/255.0f, 50.0f/255.0f));
    heatmapRenderer->setChannelColor(2, Color(139.0f/255.0f, 0.0f, 0.0f));

    // model chosen
    std::string modelName = "bc_grade_model_3_grades_10x_512";

    // read model metadata (txtfile)
    std::map<std::string, std::string> metadata = getModelMetadata(modelName);

    // based on predicted magnification level of WSI, set magnificiation level for optimal input to model based on predicted resolution of WSI
    int patch_lvl_model = (int) (std::log(magn_lvl / (float)std::stoi(metadata["magnification_level"])) / std::log(std::round(getDownsamplingAtLevel(1))));

    // segment tissue
    segmentTissue();

    auto generator = PatchGenerator::New();
    generator->setPatchSize(std::stoi(metadata["input_img_size_y"]), std::stoi(metadata["input_img_size_x"]));
    generator->setPatchLevel(patch_lvl_model);
    generator->setInputData(0, m_image);
    generator->setInputData(1, m_tissue);
    hideTissueMask(true);

    auto batchgen = ImageToBatchGenerator::New();
    batchgen->setInputConnection(generator->getOutputPort());
    batchgen->setMaxBatchSize(std::stoi(metadata["batch_process"]));

    auto network = NeuralNetwork::New();
    network->setInputNode(0, metadata["input_node"], NodeType::IMAGE, TensorShape({std::stoi(metadata["batch_size"]), std::stoi(metadata["input_img_size_y"]), std::stoi(metadata["input_img_size_x"]), std::stoi(metadata["nb_channels"])})); //{1, size, size, 3}
    network->setOutputNode(0, metadata["output_node"], NodeType::IMAGE, TensorShape({std::stoi(metadata["batch_size"]), std::stoi(metadata["input_img_size_y"]), std::stoi(metadata["input_img_size_y"]), std::stoi(metadata["nb_channels"])})); //{1, size, size, 2}
    network->load(cwd + "Models/" + modelName + ".pb");
    network->setInputConnection(batchgen->getOutputPort());
    vector scale_factor = split(metadata["scale_factor"], "/"); // get scale factor from metadata
    network->setScaleFactor((float)std::stoi(scale_factor[0]) / (float)std::stoi(scale_factor[1]));   // 1.0f/255.0f
    //network->setInferenceEngine("TensorFlowCPU");

    auto stitcher = PatchStitcher::New();
    stitcher->setInputConnection(network->getOutputPort());

    auto port = stitcher->getOutputPort();
    //stitcher->update();
    //m_gradeMap = port->getNextFrame<Tensor>();

    /*
    heatmapRenderer = HeatmapRenderer::New();  // TODO: Fix *Cannot make QOpenGLContext current in a different thread*
     */
    vector<string> colors = split(metadata["class_colors"], ";");
    for (int i = 0; i < std::stoi(metadata["nb_classes"]); i++) {
        vector<string> rgb = split(colors[i], ",");
        heatmapRenderer->setChannelColor(i, Color((float)std::stoi(rgb[0]) / 255.0f, (float)std::stoi(rgb[1]) / 255.0f, (float)std::stoi(rgb[2]) / 255.0f));
    }
    heatmapRenderer->setInterpolation(std::stoi(metadata["interpolation"].c_str()));
    heatmapRenderer->setInputConnection(port);
    insertRenderer("grade", heatmapRenderer);

    //auto view = mWidget->getViews()[0];
    //view->addRenderer(heatmapRenderer);

    return true;
}


bool MainWindow::predictTumor() {

    // define renderer
    auto tumorRenderer = HeatmapRenderer::New();
    tumorRenderer->setChannelColor(0, Color::Blue());
    tumorRenderer->setInterpolation(false);
    //heatmapRenderer->setChannelColor(1, Color(50.0f/255.0f, 205.0f/255.0f, 50.0f/255.0f));
    tumorRenderer->setChannelColor(1, Color(139.0f/255.0f, 0.0f, 0.0f));

    // model chosen
    std::string modelName = "tumor_classify_model_20x_299";

    // read model metadata (txtfile)
    std::map<std::string, std::string> metadata = getModelMetadata(modelName);

    // based on predicted magnification level of WSI, set magnificiation level for optimal input to model based on predicted resolution of WSI
    int patch_lvl_model = (int) (std::log(magn_lvl / (float)std::stoi(metadata["magnification_level"])) / std::log(std::round(getDownsamplingAtLevel(1))));

    // segment tissue
    segmentTissue();

    auto generator = PatchGenerator::New();
    generator->setPatchSize(std::stoi(metadata["input_img_size_y"]), std::stoi(metadata["input_img_size_x"]));
    generator->setPatchLevel(patch_lvl_model);
    generator->setInputData(0, m_image);
    generator->setInputData(1, m_tissue);
    hideTissueMask(true);

    auto batchgen = ImageToBatchGenerator::New();
    batchgen->setInputConnection(generator->getOutputPort());
    batchgen->setMaxBatchSize(std::stoi(metadata["batch_process"]));

    auto network = NeuralNetwork::New();
    network->setInputNode(0, metadata["input_node"], NodeType::IMAGE, TensorShape({std::stoi(metadata["batch_size"]), std::stoi(metadata["input_img_size_y"]), std::stoi(metadata["input_img_size_x"]), std::stoi(metadata["nb_channels"])})); //{1, size, size, 3}
    network->setOutputNode(0, metadata["output_node"], NodeType::IMAGE, TensorShape({std::stoi(metadata["batch_size"]), std::stoi(metadata["input_img_size_y"]), std::stoi(metadata["input_img_size_y"]), std::stoi(metadata["nb_channels"])})); //{1, size, size, 2}
    network->load(cwd + "Models/" + modelName + ".pb");
    network->setInputConnection(batchgen->getOutputPort());
    vector scale_factor = split(metadata["scale_factor"], "/"); // get scale factor from metadata
    network->setScaleFactor((float)std::stoi(scale_factor[0]) / (float)std::stoi(scale_factor[1]));   // 1.0f/255.0f
    //network->setInferenceEngine("TensorFlowCPU");

    auto stitcher = PatchStitcher::New();
    stitcher->setInputConnection(network->getOutputPort());

    auto port = stitcher->getOutputPort();
    stitcher->update();
    m_tumorMap_tensor = port->getNextFrame<Tensor>();

    /*
    heatmapRenderer = HeatmapRenderer::New();  // TODO: Fix *Cannot make QOpenGLContext current in a different thread*
     */
    vector<string> colors = split(metadata["class_colors"], ";");
    for (int i = 0; i < std::stoi(metadata["nb_classes"]); i++) {
        vector<string> rgb = split(colors[i], ",");
        tumorRenderer->setChannelColor(i, Color((float)std::stoi(rgb[0]) / 255.0f, (float)std::stoi(rgb[1]) / 255.0f, (float)std::stoi(rgb[2]) / 255.0f));
    }
    tumorRenderer->setInterpolation(std::stoi(metadata["interpolation"].c_str()));
    tumorRenderer->setInputConnection(port);

    auto tmp = TensorToSegmentation::New();
    tmp->setInputData(m_tumorMap_tensor);

    m_tumorMap = tmp->updateAndGetOutputData<Image>();

    auto segTumorRenderer = SegmentationRenderer::New();
    segTumorRenderer->setOpacity(0.4);
    segTumorRenderer->setColor(Segmentation::LABEL_FOREGROUND, Color(255.0/255.0f, 0.0f, 0.0f));
    segTumorRenderer->setColor(Segmentation::LABEL_BACKGROUND, Color(0.0f, 255.0f/255.0f, 0.0f));
    segTumorRenderer->setInputData(m_tumorMap);

    insertRenderer("tumor", tumorRenderer);
    insertRenderer("tumorSeg", segTumorRenderer);

    //auto view = mWidget->getViews()[0]; // TODO: Apparently, adding renderer here isn't sufficient for tumorRenderer(?). Also affects heatmapRenderer for whatever reason?
    //view->addRenderer(tumorRenderer);

    return true;
}


bool MainWindow::predictBACH() {

    // define renderer
    auto bachRenderer = HeatmapRenderer::New();
    bachRenderer->setChannelColor(0, Color::Blue());
    bachRenderer->setInterpolation(false);
    //heatmapRenderer->setChannelColor(1, Color(50.0f/255.0f, 205.0f/255.0f, 50.0f/255.0f));
    bachRenderer->setChannelColor(1, Color::Green());
    bachRenderer->setChannelColor(2, Color::Red());
    bachRenderer->setChannelColor(3, Color::Yellow());

    // model chosen
    std::string modelName = "inception_v3_bach_model";

    // read model metadata (txtfile)
    std::map<std::string, std::string> metadata = getModelMetadata(modelName);

    // based on predicted magnification level of WSI, set magnificiation level for optimal input to model based on predicted resolution of WSI
    int patch_lvl_model = (int) (std::log(magn_lvl / (float)std::stoi(metadata["magnification_level"])) / std::log(std::round(getDownsamplingAtLevel(1))));

    // segment tissue
    segmentTissue();

    auto generator = PatchGenerator::New();
    generator->setPatchSize(std::stoi(metadata["input_img_size_y"]), std::stoi(metadata["input_img_size_x"]));
    generator->setPatchLevel(patch_lvl_model);
    generator->setInputData(0, m_image);
    generator->setInputData(1, m_tissue);
    hideTissueMask(true);

    auto batchgen = ImageToBatchGenerator::New();
    batchgen->setInputConnection(generator->getOutputPort());
    batchgen->setMaxBatchSize(std::stoi(metadata["batch_process"]));

    auto network = NeuralNetwork::New();
    network->setInputNode(0, metadata["input_node"], NodeType::IMAGE, TensorShape({std::stoi(metadata["batch_size"]), std::stoi(metadata["input_img_size_y"]), std::stoi(metadata["input_img_size_x"]), std::stoi(metadata["nb_channels"])})); //{1, size, size, 3}
    network->setOutputNode(0, metadata["output_node"], NodeType::IMAGE, TensorShape({std::stoi(metadata["batch_size"]), std::stoi(metadata["input_img_size_y"]), std::stoi(metadata["input_img_size_y"]), std::stoi(metadata["nb_classes"])})); //{1, size, size, 2}
    network->load(cwd + "Models/" + modelName + ".pb");
    network->setInputConnection(batchgen->getOutputPort());
    vector scale_factor = split(metadata["scale_factor"], "/"); // get scale factor from metadata
    network->setScaleFactor((float)std::stoi(scale_factor[0]) / (float)std::stoi(scale_factor[1]));   // 1.0f/255.0f
    //network->setInferenceEngine("TensorFlowCPU");

    /*
    heatmapRenderer = HeatmapRenderer::New();  // TODO: Fix *Cannot make QOpenGLContext current in a different thread*
     */
    vector<string> colors = split(metadata["class_colors"], ";");
    for (int i = 0; i < std::stoi(metadata["nb_classes"]); i++) {
        vector<string> rgb = split(colors[i], ",");
        bachRenderer->setChannelColor(i, Color((float)std::stoi(rgb[0]) / 255.0f, (float)std::stoi(rgb[1]) / 255.0f, (float)std::stoi(rgb[2]) / 255.0f));
    }
    bachRenderer->setInterpolation(std::stoi(metadata["interpolation"].c_str()));

    auto stitcher = PatchStitcher::New();
    stitcher->setInputConnection(network->getOutputPort());
    m_bachMap = stitcher->updateAndGetOutputData<Tensor>();
    bachRenderer->setInputConnection(stitcher->getOutputPort());

    //auto view = mWidget->getViews()[0]; // TODO: Apparently, adding renderer here isn't sufficient for tumorRenderer(?). Also affects heatmapRenderer for whatever reason?
    //view->addRenderer(tumorRenderer);

    insertRenderer("bach", bachRenderer);

    return true;

}


std::map<std::string, std::string> MainWindow::getModelMetadata(std::string modelName) {
    // parse corresponding txt file for relevant information regarding model
    std::ifstream infile(cwd + "Models/" + modelName + ".txt");
    std::string key, value, str;
    std::string delimiter = ":";
    std::map<std::string, std::string> metadata;
    while (std::getline(infile, str))
    {
        vector<string> v = split (str, delimiter);
        key = v[0];
        value = v[1];
        metadata[key] = value;
    }
    return metadata;
}

// for string delimiter
vector<string> MainWindow::split (string s, string delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    string token;
    vector<string> res;

    while ((pos_end = s.find (delimiter, pos_start)) != string::npos) {
        token = s.substr (pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back (token);
    }

    res.push_back (s.substr (pos_start));
    return res;
}

bool MainWindow::calcTissueHist() {
    std::cout<<"it worked!\n\n\n\n";
    std::cout<<"tissue button hist\n\n\n\n\n\n";
    if (m_tissue) {
        std::cout<<std::to_string(m_tissue->calculateMinimumIntensity())<<"\n\n";
        std::cout<<std::to_string(m_tissue->calculateMaximumIntensity())<<"\n\n\n";
        std::cout<<std::to_string(m_tissue->getDimensions())<<"\n\n\n";
        std::cout<<std::to_string(m_tissue->getNrOfVoxels())<<"\n\n\n";
        //std::cout<<std::to_string(std::unique(m_tissue.begin(), m_tissue.end()))<<"\n\n\n";
        std::cout<<std::to_string(m_tissue.unique())<<"\n\n\n";
    }
    return true;
}


bool MainWindow::saveTumorPred() {
    std::cout<<"it worked!\n\n\n\n";

    return true;
}


bool MainWindow::exportSeg() {
    std::cout<<"Data has been exported...\n\n";

    return true;
}


bool MainWindow::fixTumorSegment() {
    std::cout<<"upsups...\n\n\n\n";

    //tumorRenderer->setInputData(m_tumorMap);

    return true;
}


bool MainWindow::showHeatmap() {
    if (!hasRenderer("grade")) {
        return false;
    }else{
        auto heatmapRenderer = getRenderer("grade");
        heatmapRenderer->setDisabled(!heatmapRenderer->isDisabled());
        return true;
    }
}

bool MainWindow::showBachMap() {
    if (!hasRenderer("bach")) {
        return false;
    }else{
        auto bachRenderer = getRenderer("bach");
        bachRenderer->setDisabled(!bachRenderer->isDisabled());
        return true;
    }
}

bool MainWindow::showTumorMask() {
    if (!hasRenderer("tumor")){
        return false;
    }else {
        auto tumorRenderer = getRenderer("tumor");
        tumorRenderer->setDisabled(!tumorRenderer->isDisabled());
        return true;
    }
}

bool MainWindow::toggleTissueMask() {
    if (!hasRenderer("tissue")) { //segRenderer) {
        return false;
    } else {
        auto segRenderer = getRenderer("tissue");
        segRenderer->setDisabled(!segRenderer->isDisabled());
        return true;
    }
}


bool MainWindow::toggleRenderer(std::string name) {
    if (!hasRenderer(name)) {
        return false;
    } else {
        auto someRenderer = getRenderer(name);
        someRenderer->setDisabled(!someRenderer->isDisabled());
        return true;
    }
}


bool MainWindow::hideTissueMask(bool flag) {
    if (!hasRenderer("tissue")){
        return false;
    }else {
        auto segRenderer = getRenderer("tissue");
        segRenderer->setDisabled(flag);
        return true;
    }
}

bool MainWindow::opacityTissue(int value) {
    //std::cout << (float)value/10.0f << std::endl;
    if (!hasRenderer("tissue")) {
        return false;
    }else{
        auto segRenderer = std::dynamic_pointer_cast<SegmentationRenderer>(getRenderer("tissue"));
        segRenderer->setOpacity((float) value / 10.0f); // TODO: Some functionalities in SegmentationRenderer does not exist in Renderer (?)
        segRenderer->setModified(true);
        return true;
    }
}

bool MainWindow::opacityRenderer(int value, const std::string& someName) {
    if (someName == "WSI") { // TODO: make getRenderer more generic, to automatically recognize renderer type, such that I only need to set cases on renderer type not some random names I set myself...
        return false;
    }
    if (!hasRenderer(someName)) {
        return false;
    }else{
        if (someName == "tissue") { // TODO: Make this more generic -> should recognize which renderer type automatically
            auto someRenderer = std::dynamic_pointer_cast<SegmentationRenderer>(getRenderer(someName));
            someRenderer->setOpacity((float) value / 10.0f);
            someRenderer->setModified(true);
        } else {
            auto someRenderer = std::dynamic_pointer_cast<HeatmapRenderer>(getRenderer(someName));
            someRenderer->setMaxOpacity((float) value / 10.0f);
        }
        return true;
    }
}


bool MainWindow::hideChannel(const std::string& someName) {
    if ((someName == "WSI") or (someName == "tissue")){
        return false;
    }
    if (!hasRenderer(someName)) {
        return false;
    }else{
        std::cout<<background_flag<<"\n\n\n\n\n";
        background_flag = !background_flag;
        std::cout<<background_flag<<"\n\n\n\n\n"<<":)\n";
        std::cout << channel_value << "\n";
        std::cout << someName << "\n";
        auto someRenderer = std::dynamic_pointer_cast<HeatmapRenderer>(getRenderer(someName));
        std::cout << (unsigned int)channel_value << "\n";
        someRenderer->setChannelHidden(channel_value, background_flag);
        std::cout << "\n vi kom hiit :) \n";
        return true;
    }
}

bool MainWindow::opacityHeatmap(int value) {
    //std::cout << (float)value/10.0f << std::endl;
    if (!hasRenderer("grade")){
        return false;
    }else {
        auto heatmapRenderer = getRenderer("grade");
        //heatmapRenderer->setMaxOpacity((float) value / 10.0f);
        return true;
    }
}

bool MainWindow::opacityTumor(int value) {
    //std::cout << (float)value/10.0f << std::endl;
    if (!hasRenderer("tumor")){
        return false;
    }else {
        auto tumorRenderer = getRenderer("tumor");
        //tumorRenderer->setMaxOpacity((float) value / 10.0f); // TODO: Some functionalities in heatmapRenderer does not exist in Renderer (?)
        return true;
    }
}

bool MainWindow::showImage() {
    if (!hasRenderer("WSI")){
        return false;
    }else{
        auto renderer = getRenderer("WSI");
        renderer->setDisabled(!renderer->isDisabled());
        return true;
    }
}

bool MainWindow::hideBackgroundClass(std::string someName) {
    if (!hasRenderer(someName)) {
        return false;
    }else{
        std::cout<<background_flag<<"\n\n\n\n\n";
        background_flag = !background_flag;
        std::cout<<background_flag<<"\n\n\n\n\n";
        auto tumorRenderer = std::dynamic_pointer_cast<HeatmapRenderer>(getRenderer(someName));
        tumorRenderer->setChannelHidden(0, background_flag); // TODO: Some functionalities in heatmapRenderer does not exist in Renderer (?)
        return true;
    }
    /*
    if (!tumorRenderer && !heatmapRenderer) {
        return false;
    }else if (tumorRenderer && !heatmapRenderer) {
        background_flag = !background_flag;
        std::cout<<background_flag<<"\n\n\n\n\n";
        tumorRenderer->setChannelHidden(0, background_flag);
        return true;
    } else if (!tumorRenderer && heatmapRenderer) {
        heatmapRenderer->setChannelHidden(3, true);
        return true;
    } else {
        heatmapRenderer->setChannelHidden(3, true);
        tumorRenderer->setChannelHidden(0, true);
        return true;
    }
     */
}

bool MainWindow::fixImage() {
    if (!hasRenderer("WSI")){
        return false;
    }else{
        //stopComputationThread();
        //getView(0)->reinitialize();
        //startComputationThread();
        return false;
    }
}


void MainWindow::removeRenderer(std::string name) {
    if(m_rendererList.count(name) == 0)
        return;
    auto renderer = m_rendererList[name];
    getView(0)->removeRenderer(renderer);
}

void MainWindow::insertRenderer(std::string name, SharedPointer<Renderer> renderer) {
    std::cout << "calling insert renderer" << std::endl;
    if(!hasRenderer(name)) {
        // Doesn't exist
        getView(0)->addRenderer(renderer);
    }
    m_rendererList[name] = renderer;
    std::cout << "finished insert renderer" << std::endl;
}

void MainWindow::removeAllRenderers() {
    m_rendererList.clear();
    getView(0)->removeAllRenderers();
}

bool MainWindow::hasRenderer(std::string name) {
    return m_rendererList.count(name) > 0;
}

SharedPointer<Renderer> MainWindow::getRenderer(std::string name) {
    if(!hasRenderer(name))
        throw Exception("Renderer with name " + name + " does not exist");
    return m_rendererList[name];
}


//bool MainWindow::stopInference() {
//}


}
