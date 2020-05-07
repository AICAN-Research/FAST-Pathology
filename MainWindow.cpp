#include "MainWindow.hpp"
#include <FAST/Importers/WholeSlideImageImporter.hpp>
#include <FAST/Visualization/ImagePyramidRenderer/ImagePyramidRenderer.hpp>
#include <QPushButton>
#include <QFileDialog>
#include <QFile>
#include <QSlider>
#include <FAST/Data/ImagePyramid.hpp>
#include <FAST/Data/Image.hpp>
#include <FAST/Algorithms/TissueSegmentation/TissueSegmentation.hpp>
#include <FAST/Algorithms/ImagePatch/PatchGenerator.hpp>
#include <FAST/Algorithms/ImagePatch/PatchStitcher.hpp>
#include <FAST/Algorithms/NeuralNetwork/NeuralNetwork.hpp>
#include <FAST/Algorithms/NeuralNetwork/SegmentationNetwork.hpp>
#include <FAST/Visualization/HeatmapRenderer/HeatmapRenderer.hpp>
#include <FAST/Algorithms/ImagePatch/ImageToBatchGenerator.hpp>
#include <FAST/Visualization/SegmentationRenderer/SegmentationRenderer.hpp>
#include <FAST/Visualization/SegmentationPyramidRenderer/SegmentationPyramidRenderer.hpp>
#include <FAST/Algorithms/NeuralNetwork/TensorToSegmentation.hpp>
#include <string>
#include <QtWidgets>
#include <QWidget>
#include <QFont>
#include <QBoxLayout>
#include <QToolBar>
#include <QPainter>
#include <iostream>
#include <algorithm>
#include <vector>
#include <stdio.h>      /* printf */
#include <math.h>       /* pow */
#include <istream>
#include <fstream>
#include <stdlib.h>
#include <QColorDialog>
#include <QDir>
#include <QLayoutItem>
#include <QObject>
#include <future> // wait for callback is finished
#include <FAST/Utility.hpp>
#include <QUrl>
#include <QScreen>
#include <QMessageBox>
#include <FAST/Exporters/ImageExporter.hpp>
#include <FAST/Exporters/ImageFileExporter.hpp>
#include <FAST/Algorithms/ScaleImage/ScaleImage.hpp>
#include <FAST/Data/Access/ImagePyramidAccess.hpp>
//#include "ExtractThumbnail.hpp"

using namespace std;

namespace fast {

MainWindow::MainWindow() {
    setTitle("fastPathology");
    enableMaximized(); // <- function from Window.cpp

    // get current working directory
    get_cwd();

    // create temporary tmp folder to store stuff, and create temporary file to keep history for visualization and
    // other stuff
    //create_tmp_folder_file();
    QTemporaryDir tmpDir;
    std::string tmpPath = tmpDir.path().toStdString();
    std::cout << "\n temporary path: " << tmpDir.path().toStdString() << "\n";

    // Create models folder platform assumes that this folder exists and contains all relevant models
    std::string dir_str;
    dir_str = cwd + "Models";
    createDirectories(dir_str);

    // changing color to the Qt background
    //mWidget->setStyleSheet("font-size: 16px; background: gray; color: white;");
    //mWidget->setStyleSheet("font-size: 16px; background: rgb(75, 74, 103); color: black;");
    mWidget->setStyleSheet("font-size: 16px; background: rgb(221, 209, 199); color: black;"); // Current favourite
    //mWidget->setStyleSheet("font-size: 16px; background: rgb(181, 211, 231); color: black;");
    //mWidget->setStyleSheet("font-size: 16px; background: (255, 125, 50); color: black;");
    mWidget->setStyleSheet("font-size: 16px");

    float OpenGL_background_color = 0.0f; //200.0f / 255.0f;

    // opacityTumorSlider->setStyleSheet("font-size: 16px"); // <- to set style sheet for a specific button/slider
    //mainestLayout->addLayout(topHorizontalLayout);

    superLayout = new QVBoxLayout();
    mWidget->setLayout(superLayout);

    // make overall Widget
    mainWidget = new QWidget();
    superLayout->insertWidget(1, mainWidget); //addWidget(mainWidget);

    // Create vertical GUI layout:
    mainLayout = new QHBoxLayout;
    //mWidget->setLayout(mainLayout);
    mainWidget->setLayout(mainLayout);

    createMainMenuWidget();

    auto view = createView();
    //view = mWidget->addView();
    //view->setLayout(mainLayout);

    //mainLayout->addLayout(menuLayout);
    //mainLayout->insertWidget(1, view);
    view->set2DMode();
    view->setBackgroundColor(Color(OpenGL_background_color, OpenGL_background_color, OpenGL_background_color)); // setting color to the background, around the WSI
    view->setAutoUpdateCamera(true);
    //view->setLayout(mainLayout);

    // create QSplitter for adjustable windows
    auto mainSplitter = new QSplitter(Qt::Horizontal);
    mainSplitter->addWidget(menuWidget);
    mainSplitter->addWidget(view);
    mainSplitter->setStretchFactor(1, 1);

    mainLayout->addWidget(mainSplitter);

    // create Menubar
    createMenubar();
}


void MainWindow::reportIssueUrl() {
    QDesktopServices::openUrl(QUrl("https://github.com/SINTEFMedtek/FAST-Pathology/issues", QUrl::TolerantMode));
}


void MainWindow::helpUrl() {
    QDesktopServices::openUrl(QUrl("https://github.com/SINTEFMedtek/FAST-Pathology", QUrl::TolerantMode));
}


void MainWindow::createMenubar() {

    // need to create a new QHBoxLayout for the menubar
    auto topFiller = new QMenuBar();
    topFiller->setMaximumHeight(30);

    //auto fileMenu = new QMenu();
    auto fileMenu = topFiller->addMenu(tr("&File"));
    //fileMenu->setFixedHeight(100);
    //fileMenu->setFixedWidth(100);
    QAction *createProjectAction;
    fileMenu->addAction("Create Project", this,  &MainWindow::createProject);
    fileMenu->addAction("Import WSI", this, &MainWindow::selectFile);
    fileMenu->addAction("Import Models", this, &MainWindow::addModels);
    fileMenu->addAction("Import Pipelines", this, &MainWindow::addPipelines);
    fileMenu->addSeparator();
    fileMenu->addAction("Quit", QApplication::quit);

    auto editMenu = topFiller->addMenu(tr("&Edit"));
    editMenu->addAction("Select Mode");  // TODO: Add function that changes GUI for research/clinical use
    editMenu->addAction("Info");

    auto pipelineMenu = topFiller->addMenu(tr("&Pipelines"));
    pipelineMenu->addAction("Import pipelines", this, &MainWindow::addPipelines);
    pipelineMenu->addAction("Run Pipeline");
    pipelineMenu->addAction("Pipeline Editor", this, &MainWindow::pipelineEditor);

    auto projectMenu = topFiller->addMenu(tr("&Projects"));
    projectMenu->addAction("Create Project", this, &MainWindow::createProject);
    projectMenu->addAction("Open Project", this, &MainWindow::openProject);
    projectMenu->addAction("Save Project", this, &MainWindow::saveProject);

    //auto deployMenu = new QMenu();
    auto deployMenu = topFiller->addMenu(tr("&Deploy"));
    //deployMenu->addMenu(tr("&Deploy"));
    //deployMenu->setFixedHeight(100);
    //deployMenu->setFixedWidth(100);
    deployMenu->addAction("Run Pipeline");
    deployMenu->addAction("Segment Tissue");
    deployMenu->addAction("Predict Tumor");
    deployMenu->addAction("Classify Grade");
    deployMenu->addSeparator();

    auto helpMenu = topFiller->addMenu(tr("&Help"));
    helpMenu->addAction("Contact support", helpUrl);
    helpMenu->addAction("Report issue", reportIssueUrl);
    helpMenu->addAction("Check for updates");  // TODO: Add function that checks if the current binary in usage is the most recent one
    helpMenu->addAction("About FastPathology");

    //topFiller->addMenu(fileMenu);
    //topFiller->addMenu(deployMenu);

    superLayout->insertWidget(0, topFiller);
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
    //mainLayout->insertWidget(0, menuWidget);
}


// TODO: Don't remember if this actually worked, or if I ended up using it
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
    tb->setFont(QFont("Times", 8)); //QFont::Bold));
    tb->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);  // adds text under icons


    //auto toolBar = new QToolBar;
    QPixmap openPix(QString::fromStdString(cwd + "/Icons/import-data-icon-19.png"));
    QPixmap processPix(QString::fromStdString(cwd + "/Icons/machine-learning-icon-8.png"));
    QPixmap viewPix(QString::fromStdString(cwd + "/Icons/visualize.png"));  //"/home/andre/Downloads/quick-view-icon-8.png");
    QPixmap resultPix(QString::fromStdString(cwd + "/Icons/Simpleicons_Business_bars-chart-up.svg.png"));
    QPixmap savePix(QString::fromStdString(cwd + "/Icons/save-time-icon-1.png"));

    //viewPix->

    QLabel myLabel;

    /*
    // icons
    QImage image(QString::fromStdString(cwd + "/Icons/import-data-icon-19.png"));
    QPainter p(&image);
    p.setPen(QPen(Qt::red));
    p.setFont(QFont("Times", 12, QFont::Bold));
    p.drawText(image.rect(), Qt::AlignCenter, "Text");
    myLabel.setPixmap(QPixmap::fromImage(image));
     */

    QPainter painter(&savePix);
    QFont font = painter.font();
    font.setPixelSize(4);
    //font.setBold(true);
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

    //QIcon icon = style()->standardIcon(QStyle::SP_DialogOkButton);
    //QPixmap pixmap = icon.pixmap(QSize(64, 64));

    QAction *process_action = tb->addAction(QIcon(processPix),tr("Process"));
    //QAction *process_action = tb->addAction(QIcon(myLabel), "Process");
    mapper->setMapping(process_action, 1);
    mapper->connect(process_action, SIGNAL(triggered(bool)), SLOT(map()));

    QAction *view_action = tb->addAction(QIcon(viewPix), "View");
    mapper->setMapping(view_action, 2);
    mapper->connect(view_action, SIGNAL(triggered(bool)), SLOT(map()));

    QAction *result_action = tb->addAction(QIcon(resultPix), "Stats"); //"Statistics");
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
    //menuWidget->setFixedWidth(300); //300);  // TODO: This was a good width for my screen, but need it to be adjustable (!)
    //menuWidget->set
    menuWidget->setMaximumWidth(700);
    menuWidget->setMinimumWidth(360);
    //tmpWidget->setStyleSheet("border:1px solid rgb(0, 255, 0); ");
    menuWidget->setLayout(dockLayout);
}


void MainWindow::createWSIScrollAreaWidget() {
    //auto scrollAreaDialog = new QDialog();
    //scrollAreaDialog->setGeometry(100, 100, 260, 260);

    // TODO: Need to substitute this with QListWidget or similar as it's quite slow and memory expensive for
    //        larger number of elements
    scrollArea = new QScrollArea();  //scrollAreaDialog);
    scrollArea->setAlignment(Qt::AlignTop);
    //scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    scrollArea->setWidgetResizable(true);
    scrollArea->setGeometry(10, 10, 200, 200);

    scrollList = new QListWidget();
    scrollList->setItemAlignment(Qt::AlignTop);
    scrollList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    scrollList->setResizeMode(QListView::Adjust);  // resizable adaptively
    scrollList->setGeometry(10, 10, 200, 200);
    //QObject::connect(scrollList, &QPushButton::clicked, std::bind(&MainWindow::selectFileInProject, this, 1));
    //QObject::connect(scrollList, &QListWidget::itemPressed, std::bind(&MainWindow::selectFileInProject, this, 1));  // this, SLOT(onListMailItemClicked(QListWidgetItem*)));
    //QObject::connect(scrollList,itemClicked(QListWidgetItem*), std::bind(&MainWindow::selectFileInProject, this, 1));
    //connect(ui->listMail, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onListMailItemClicked(QListWidgetItem*)));
    // QListWidget::itemPressed(QListWidgetItem *item)
    //QObject::connect(scrollList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(&MainWindow::selectFileInProject));

    scrollArea->setWidget(scrollList);

    scrollWidget = new QWidget();
    //scrollArea->setWidget(scrollWidget);

    scrollLayout = new QVBoxLayout();
    scrollWidget->setLayout(scrollLayout);

    //connect(scrollList, SIGNAL(activated(int)), scrollLayout, SLOT(setCurrentIndex(int)));

    /*
    for (int i = 0; i < 4; i++)
    {
        auto button = new QPushButton(); //QString( "%1" ).arg( i ) );
        //QPixmap pixmap("/home/andrep/workspace/FAST-Pathology/ImageExporterTest.png"); //"/home/andrep/Pictures/medium.png");
        QPixmap pixmap("/home/andrep/Pictures/medium.png");
        QIcon ButtonIcon(pixmap);
        button->setIcon(ButtonIcon);
        //button->setIconSize(pixmap.rect().size());
        button->setIconSize(QSize(200, 200));
        //button->setFixedSize(100, 100);
        scrollLayout->addWidget(button);
    }
     */

    //fileLayout->insertWidget(2, scrollArea);  //widget);
    fileLayout->addWidget(scrollArea);

    //scrollAreaDialog->show();
}


void MainWindow::createFileWidget() {

    fileLayout = new QVBoxLayout;
    fileLayout->setAlignment(Qt::AlignTop);

    fileWidget = new QWidget;
    fileWidget->setLayout(fileLayout);
    //fileWidget->setFixedWidth(200);

    auto createProjectButton = new QPushButton(fileWidget);
    createProjectButton->setText("Create Project");
    createProjectButton->setFixedHeight(50);
    createProjectButton->setStyleSheet("color: white; background-color: blue");
    QObject::connect(createProjectButton, &QPushButton::clicked, std::bind(&MainWindow::createProject, this));

    auto openProjectButton = new QPushButton(fileWidget);
    openProjectButton->setText("Open Project");
    openProjectButton->setFixedHeight(50);
    openProjectButton->setStyleSheet("color: white; background-color: blue");
    QObject::connect(openProjectButton, &QPushButton::clicked, std::bind(&MainWindow::openProject, this));

    auto selectFileButton = new QPushButton(fileWidget);
    selectFileButton->setText("Select WSI");
    //selectFileButton->setFixedWidth(200);
    selectFileButton->setFixedHeight(50);
    selectFileButton->setStyleSheet("color: white; background-color: blue");
    QObject::connect(selectFileButton, &QPushButton::clicked, std::bind(&MainWindow::selectFile, this));

    /*
    auto addModelButton = new QPushButton(fileWidget);
    addModelButton->setText("Import model");
    //selectFileButton->setFixedWidth(200);
    addModelButton->setFixedHeight(50);
    addModelButton->setStyleSheet("color: white; background-color: blue");
    QObject::connect(addModelButton, &QPushButton::clicked, std::bind(&MainWindow::addModels, this));

    auto quitButton = new QPushButton(fileWidget);
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
    */

    fileLayout->addWidget(createProjectButton);
    fileLayout->addWidget(openProjectButton);
    fileLayout->addWidget(selectFileButton); //, Qt::AlignTop);
    /*
    fileLayout->addWidget(addModelButton);
    fileLayout->addWidget(smallTextWindow);
    fileLayout->addWidget(bigEditor);
    fileLayout->addWidget(quitButton, Qt::AlignTop);
     */

    createWSIScrollAreaWidget();

}


void MainWindow::createViewWidget() {

    viewLayout = new QVBoxLayout;
    viewLayout->setAlignment(Qt::AlignTop);

    viewWidget = new QWidget;
    viewWidget->setLayout(viewLayout);
    //viewWidget->setFixedWidth(200);

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

    auto stackedWidget = new QWidget;
    stackedWidget->setLayout(stackedLayout);

    pageComboBox = new QComboBox;
    pageComboBox->setFixedWidth(150);
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

    viewLayout->insertWidget(1, stackedWidget);

}


void MainWindow::createDynamicViewWidget(const std::string& someName, std::string modelName) {

    //std::string someName = "tumor";

    dynamicViewWidget = new QWidget;

    auto imageButton = new QPushButton(dynamicViewWidget);
    imageButton->setText("Toggle image");
    //showHeatmapButton->setFixedWidth(200);
    imageButton->setFixedHeight(50);
    imageButton->setCheckable(true);
    imageButton->setChecked(true);
    //showHeatmapButton->setChecked(true);
    QObject::connect(imageButton, &QPushButton::clicked, std::bind(&MainWindow::toggleRenderer, this, someName));
    //imageButton->setDisabled(visible[0]);

    // for WSI this should be grayed out, shouldn't be able to change it
    auto opacitySlider = new QSlider(Qt::Horizontal, dynamicViewWidget);
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

    auto toggleShowButton = new QPushButton(dynamicViewWidget);
    toggleShowButton->setText("Toggle class");
    //showHeatmapButton->setFixedWidth(200);
    toggleShowButton->setFixedHeight(50);
    //toggleShowButton->setFixedWidth(100);
    toggleShowButton->setCheckable(true);
    toggleShowButton->setChecked(true);
    //showHeatmapButton->setChecked(true);
    QObject::connect(toggleShowButton, &QPushButton::clicked, std::bind(&MainWindow::hideChannel, this, someName));


    auto currComboBox = new QComboBox;

    if ((someName != "WSI") && (someName != "tissue") && (someName != "tumorSeg")) {
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
    } else if ((someName == "tissue") || (someName == "tumorSeg")) {
        biggerTextBoxWidget_imageName->setDisabled(true);
    }

    std::cout << "\n vi kom lengre 1 \n";


    dynamicViewLayout = new QVBoxLayout;
    dynamicViewLayout->setAlignment(Qt::AlignTop);

    dynamicViewWidget->setLayout(dynamicViewLayout);

    dynamicViewLayout->insertWidget(0, imageButton);
    //dynamicViewLayout->insertWidget(1, opacitySlider);
    dynamicViewLayout->insertWidget(1, smallTextBoxWidget_tissue);
    dynamicViewLayout->insertWidget(2, colorSetWidget);
    dynamicViewLayout->insertWidget(3, biggerTextBoxWidget_imageName);

    // add widget to QComboBox
    pageComboBox->addItem(tr(tmpSomeName.c_str()));
    stackedLayout->addWidget(dynamicViewWidget);

}


void MainWindow::pipelineEditor() {

    // get screen geometry (resolution/size)
    //QRect screenGeometry = QApplication::desktop()->screenGeometry();
    //QGuiApplication::screens();
    1;
    std::cout << "\n\n hallo";

    auto scriptEditorWidgetBackground = new QWidget(mWidget);
    auto backgroundLayout = new QVBoxLayout;

    scriptEditorWidget = new QDialog(mWidget);
    scriptEditorWidget->setWindowTitle("Script Editor");
    //scriptEditorWidget = new QWidget(mWidget);
    //scriptEditorWidget->setSizeGripEnabled(true);

    //QRect scr = QApplication::desktop()->screenGeometry();
    //move(scr.center() - rect().center() );

    backgroundLayout->addWidget(scriptEditorWidget);

    //scriptEditorWidget->setCentralWidget;
    //scriptEditorWidget->setWindowFlags(Qt::AlignHCenter);
    //scriptEditorWidget->setWindowFlags(Qt::Popup);
    //scriptEditorWidget->setBaseSize(500, 500);
    scriptEditorWidget->setMinimumWidth(800);
    scriptEditorWidget->setMinimumHeight(1000);  // TODO: Better to use baseSize, but not working?
    //QLayout *scriptLayout;

    scriptLayout = new QVBoxLayout; // mWidget);
    scriptEditorWidget->setLayout(scriptLayout);

    scriptEditor = new QPlainTextEdit;
    scriptLayout->insertWidget(1, scriptEditor);

    /*
    scriptLayout->setGeometry(QStyle::alignedRect(
            Qt::LeftToRight,
            Qt::AlignCenter,
            mWidget->size(),
            mWidget->QGuiApplication::primaryScreen()));
            //qApp->desktop()->availableGeometry()));
    */


    //QRect rec = QApplication::desktop()->screenGeometry();


    //QScreen *screen = QGuiApplication::primaryScreen();
    //QRect  screenGeometry = screen->geometry();

    //scriptEditorWidget->move(300, 300);
    /*
    scriptEditorWidget->move(mWidget->window()->frameGeometry().topLeft() +
                             mWidget->window()->rect().center() -
                             screenGeometry.center());
    */

    // center QDialog when opened
    QRect rect = scriptEditorWidget->geometry();
    QRect parentRect = mWidget->geometry();
    rect.moveTo(mWidget->mapToGlobal(QPoint(parentRect.x() + parentRect.width() - rect.width(), parentRect.y())));

    /*
    scriptEditorWidget->adjustSize();
    scriptEditorWidget->move(mWidget->window()->frameGeometry().topLeft() +
                             mWidget->window()->rect().center() -
                             screenGeometry.center());
    */

    /*
    QPoint dialogCenter = scriptEditorWidget->mapToGlobal(scriptEditorWidget->rect().center());
    QPoint parentWindowCenter = mWidget->window()->mapToGlobal(
            mWidget->window()->rect().center());
    move(parentWindowCenter - dialogCenter);
     */

    //scriptEditorWidget->move(QGuiApplication::screens()[0]->rect().center() - scriptEditorWidget->rect().center())

    // resizable window using QSplitter
    /*
    auto mainSplitter = new QSplitter(Qt::);
    mainSplitter->addWidget(menuWidget);
    mainSplitter->addWidget(view);
    mainSplitter->setStretchFactor(1, 1);

    mainLayout->addWidget(mainSplitter);
     */

    createActionsScript();

    // finally, show window
    //scriptEditorWidgetBackground->show();
    scriptEditorWidget->show();
    //scriptEditorWidget->close();
}


void MainWindow::createActionsScript() {

    auto menuBar = new QMenuBar(scriptEditorWidget); //scriptEditorWidget);
    //menuBar->setMinimumHeight(200);

    auto toolBar = new QToolBar();

    //QMenu *fileMenu = menuBar->addMenu(tr("&File"));
    //auto fileMenu = menuBar->addMenu(tr("&File"));
    //fileMenu->addAction("Create Project", this, &MainWindow::createProject);


    //scriptEditorWidget->layout()->setMenuBar(menuBar);
    //scriptLayout->setMenuBar(menuBar);
    //scriptEditorWidget->layout()->setMenuBar(menuBar);
    scriptLayout->setMenuBar(menuBar);

    //auto
    //topFiller->setMaximumHeight(30);

    //auto fileMenu = new QMenu();
    //auto fileMenu = topFiller->addMenu(tr("&File"));

    auto fileMenu = menuBar->addMenu(tr("&File"));
    auto fileToolBar = new QToolBar;

    const QIcon newIcon = QIcon::fromTheme("document-new", QIcon(":/images/new.png"));
    auto newAct = new QAction(newIcon, tr("&New"), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("Create a new file"));
    connect(newAct, &QAction::triggered, this, &MainWindow::newFileScript);
    fileMenu->addAction(newAct);

    const QIcon openIcon = QIcon::fromTheme("document-open", QIcon(":/images/open.png"));
    auto openAct = new QAction(openIcon, tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, &QAction::triggered, this, &MainWindow::openScript);
    fileMenu->addAction(openAct);
    fileToolBar->addAction(openAct);

    const QIcon saveIcon = QIcon::fromTheme("document-save", QIcon(":/images/save.png"));
    auto saveAct = new QAction(saveIcon, tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the document to disk"));
    connect(saveAct, &QAction::triggered, this, &MainWindow::saveScript);
    fileMenu->addAction(saveAct);
    fileToolBar->addAction(saveAct);

    const QIcon saveAsIcon = QIcon::fromTheme("document-save-as");
    QAction *saveAsAct = fileMenu->addAction(saveAsIcon, tr("Save &As..."), this, &MainWindow::saveAsScript);
    saveAsAct->setShortcuts(QKeySequence::SaveAs);
    saveAsAct->setStatusTip(tr("Save the document under a new name"));

    fileMenu->addSeparator();

    QMenu *editMenu = menuBar->addMenu(tr("&Edit"));
    //QToolBar *editToolBar = addToolBar(tr("Edit"));
    auto editToolBar = new QToolBar(tr("Edit"));

#ifndef QT_NO_CLIPBOARD
    const QIcon cutIcon = QIcon::fromTheme("edit-cut", QIcon(":/images/cut.png"));
    auto cutAct = new QAction(cutIcon, tr("Cu&t"), this);

    cutAct->setShortcuts(QKeySequence::Cut);
    cutAct->setStatusTip(tr("Cut the current selection's contents to the "
                            "clipboard"));
    connect(cutAct, &QAction::triggered, scriptEditor, &QPlainTextEdit::cut);
    editMenu->addAction(cutAct);
    editToolBar->addAction(cutAct);

    const QIcon copyIcon = QIcon::fromTheme("edit-copy", QIcon(":/images/copy.png"));
    auto copyAct = new QAction(copyIcon, tr("&Copy"), this);
    copyAct->setShortcuts(QKeySequence::Copy);
    copyAct->setStatusTip(tr("Copy the current selection's contents to the "
                             "clipboard"));
    connect(copyAct, &QAction::triggered, scriptEditor, &QPlainTextEdit::copy);
    editMenu->addAction(copyAct);
    editToolBar->addAction(copyAct);

    const QIcon pasteIcon = QIcon::fromTheme("edit-paste", QIcon(":/images/paste.png"));
    auto pasteAct = new QAction(pasteIcon, tr("&Paste"), this);
    pasteAct->setShortcuts(QKeySequence::Paste);
    pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current "
                              "selection"));
    connect(pasteAct, &QAction::triggered, scriptEditor, &QPlainTextEdit::paste);
    editMenu->addAction(pasteAct);
    editToolBar->addAction(pasteAct);

    menuBar->addSeparator();

#endif // !QT_NO_CLIPBOARD

#ifndef QT_NO_CLIPBOARD
    cutAct->setEnabled(false);
    copyAct->setEnabled(false);
    connect(scriptEditor, &QPlainTextEdit::copyAvailable, cutAct, &QAction::setEnabled);
    connect(scriptEditor, &QPlainTextEdit::copyAvailable, copyAct, &QAction::setEnabled);
#endif // !QT_NO_CLIPBOARD

    /*
    auto fileMenu = menuBar->addMenu(tr("&File"));
    //fileMenu->addAction("&New", this,  &MainWindow::newFileScript);
    auto newAct = new QAction(tr("&New"), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("Create a new file"));
    connect(newAct, &QAction::triggered, this, &MainWindow::newFileScript);

    //fileMenu->addSeparator();
    fileMenu->addAction("Quit", QApplication::quit);

    scriptLayout->insertWidget(0, menuBar);
     */

    //scriptLayout->insertWidget(0, menuBar);
}


bool MainWindow::saveScript() {
    std::cout << "\nSaving...: " << currScript.toStdString() << "\n";
    if (currScript.isEmpty()) {
        return saveAsScript();
    } else {
        std::cout << "\nWe are saving, not save as...\n";
        return saveFileScript(currScript);
    }
}


bool MainWindow::saveAsScript() {
    QFileDialog dialog(scriptEditorWidget);
    //dialog.DontUseNativeDialog;
    dialog.setOption(QFileDialog::DontUseNativeDialog);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    if (dialog.exec() != QDialog::Accepted)
        return false;
    return saveFileScript(dialog.selectedFiles().first());
}


void MainWindow::createStatusBarScript() {
    //auto statusBar = new QStatusBar;
    statusBar->showMessage(tr("Ready"));
}


bool MainWindow::saveFileScript(const QString &fileName) {
    QString errorMessage;

    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    QSaveFile file(fileName);
    if (file.open(QFile::WriteOnly | QFile::Text)) {
        QTextStream out(&file);
        out << scriptEditor->toPlainText();
        if (!file.commit()) {
            errorMessage = tr("Cannot write file %1:\n%2.")
                    .arg(QDir::toNativeSeparators(fileName), file.errorString());
        }
    } else {
        errorMessage = tr("Cannot open file %1 for writing:\n%2.")
                .arg(QDir::toNativeSeparators(fileName), file.errorString());
    }
    QGuiApplication::restoreOverrideCursor();

    if (!errorMessage.isEmpty()) {
        QMessageBox::warning(scriptEditorWidget, tr("Application"), errorMessage);
        return false;
    }

    setCurrentFileScript(fileName);
    //statusBar->showMessage(tr("File saved"), 2000);
    return true;
}


bool MainWindow::maybeSaveScript() {
    if (!scriptEditor->document()->isModified())
        return true;
    const QMessageBox::StandardButton ret = QMessageBox::warning(scriptEditorWidget, tr("Application"),
                                   tr("The document has been modified.\n"
                                      "Do you want to save your changes?"),
                                   QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    switch (ret) {
        case QMessageBox::Save:
            return saveScript();
        case QMessageBox::Cancel:
            return false;
        default:
            break;
    }
    return true;
}


void MainWindow::newFileScript() {
    if (maybeSaveScript()) {
        scriptEditor->clear();
        setCurrentFileScript(QString());
    }
}


void MainWindow::openScript() {
    if (maybeSaveScript()) {
        auto fileName = QFileDialog::getOpenFileName(
                mWidget,
                tr("Open File"), nullptr, tr("WSI Files (*.fpl)"),
                nullptr, QFileDialog::DontUseNativeDialog
        );
        if (!fileName.isEmpty())
            loadFileScript(fileName);
    }
}


void MainWindow::loadFileScript(const QString &fileName) {
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) { // handle read only files
        QMessageBox::warning(scriptEditorWidget, tr("Application"),
                             tr("Cannot read file %1:\n%2.")
                                     .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return;
    }

    QTextStream in(&file);
#ifndef QT_NO_CURSOR
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
#endif
    scriptEditor->setPlainText(in.readAll());
#ifndef QT_NO_CURSOR
    QGuiApplication::restoreOverrideCursor();
#endif

    setCurrentFileScript(fileName);
    //statusBar->showMessage(tr("File loaded"), 2000); // FIXME: Something crashed here...
}


void MainWindow::setCurrentFileScript(const QString &fileName) {

    currScript = fileName;
    scriptEditor->document()->setModified(false);
    //setWindowModified(false);
    scriptEditor->setWindowModified(false);

    QString shownName = currScript;
    if (currScript.isEmpty())
        shownName = "untitled.txt";
    scriptEditor->setWindowFilePath(shownName);
    //setWindowFilePath(shownName);
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
    auto calcTissueHistButton = new QPushButton(statsWidget);
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
    auto exportSegmentationButton = new QPushButton(exportWidget);
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

    auto segTissueButton = new QPushButton(exportWidget);
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

        auto someButton = new QPushButton(exportWidget);
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
    auto a = QDir(paths).exists();
    if (a == 1) {
        int bb = rmdir(paths);
    }
    int aa = mkdir(paths);  //mkdir(paths, 0777);

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

    // TODO FIX
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

int MainWindow::mkdir(const char *path)
{
    return QDir().mkdir(path);
}

int MainWindow::rmdir(const char *path)
{
    return QDir(path).removeRecursively();
}

void MainWindow::drawHist() {
    int aa = 2;
}


/*
void MainWindow::createActions() {
    newAct = new QAction(tr("&New"), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("Create a new file"));
    connect(newAct, &QAction::triggered, this, &MainWindow::selectFile);
}
 */


void MainWindow::selectFile() {

    // check if view object list is empty, if not, prompt to save results or not, if not clear
    if (pageComboBox->count() > 1) {
        // prompt
        QMessageBox mBox;
        mBox.setIcon(QMessageBox::Warning);
        mBox.setText("There are unsaved results.");
        mBox.setInformativeText("Do you wish to save them?");
        mBox.setDefaultButton(QMessageBox::Save);
        mBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        int ret = mBox.exec();

        switch (ret) {
            case QMessageBox::Save:
                std::cout << "\n Saved! \n";
                // Save was clicked
                break;
            case QMessageBox::Discard:
                // Don't Save was clicked
                std::cout << "\n Discarded! \n";
                break;
            case QMessageBox::Cancel:
                // Cancel was clicked
                std::cout << "\n Cancelled! \n";
                break;
            default:
                // should never be reached
                break;
        }
    }
    if (pageComboBox->count() != 0) { // if not empty, clear
        pageComboBox->clear(); // clear
    }

    auto fileNames = QFileDialog::getOpenFileNames(
            mWidget,
            tr("Select File(s)"), nullptr, tr("WSI Files (*.tiff *.tif *.svs *.ndpi)"),
            nullptr, QFileDialog::DontUseNativeDialog
            );

    int counter = 0;
    for (QString& fileName : fileNames) {

        if (fileName == "")
            return;
        filename = fileName.toStdString();

        wsiList.push_back(filename);

        // if new file is chosen, clear all cache of previous images
        //renderer->clearPyramid();
        //tumorRenderer->clearHeatmap();

        // Import image from file using the ImageFileImporter
        auto importer = WholeSlideImageImporter::New();
        importer->setFilename(fileName.toStdString());
        m_image = importer->updateAndGetOutputData<ImagePyramid>();

        // for reading of multiple WSIs, only render last one
        if (counter == fileNames.count()-1) {

            // get metadata
            metadata = m_image->getMetadata();

            auto renderer = ImagePyramidRenderer::New();
            renderer->setInputData(m_image);

            // TODO: Something here results in me not being able to run analysis on new images (after the first)
            removeAllRenderers();
            insertRenderer("WSI", renderer);
            getView(0)->reinitialize(); // Must call this after removing all renderers

            // get WSI format
            wsiFormat = metadata["openslide.vendor"];

            // get magnification level of current WSI
            magn_lvl = getMagnificationLevel();

            // now make it possible to edit image in the View Widget
            createDynamicViewWidget("WSI", modelName);
        }
        counter ++;

        auto access = m_image->getAccess(ACCESS_READ);
        auto input = access->getLevelAsImage(m_image->getNrOfLevels() - 1);

        // try to convert to FAST Image -> QImage
        QImage image(input->getWidth(), input->getHeight(), QImage::Format_RGB32);

        // TODO have to do some type conversion here, assuming float for now
        unsigned char *pixelData = image.bits();

        ImageAccess::pointer new_access = input->getImageAccess(ACCESS_READ);
        void *inputData = new_access->get();
        uint nrOfComponents = input->getNrOfChannels();

        for (uint x = 0; x < input->getWidth(); x++) {
            for (uint y = 0; y < input->getHeight(); y++) {
                uint i = x + y * input->getWidth();
                for (uint c = 0; c < input->getNrOfChannels(); c++) {
                    float data;
                    data = ((uchar *) inputData)[i * nrOfComponents + c]; // assumes TYPE_UINT8
                    pixelData[i * 4 + (2-c)] = (unsigned char) data;  // TODO: NOTE (2-c) fixed BGR->RGB, but maybe there is a smarter solution?
                    pixelData[i * 4 + 3] = 255; // Alpha
                }
            }
        }

        /*
        auto intensityScaler = ScaleImage::New();
        intensityScaler->setInputData(input); //m_tissue);  // expects Image data type
        intensityScaler->setLowestValue(0.0f);
        intensityScaler->setHighestValue(1.0f);
        //intensityScaler->update();
         */

        // attempt to save tissue mask to disk as .png
        /*
        ImageExporter::pointer exporter = ImageExporter::New();
        exporter->setFilename("/home/andrep/workspace/FAST-Pathology/ImageExporterTest.png");
        exporter->setInputData(input); //intensityScaler->updateAndGetOutputData<Image>());
        exporter->update();
         */

        // BGR -> RGB
        //auto channelConverter = new ImageChannelConverter::New();


        //QPixmap pixmap(exporter);

        // /*
        auto button = new QPushButton();
        //QPixmap pixmap("/home/andrep/workspace/FAST-Pathology/ImageExporterTest.png");
        auto m_NewPixMap = QPixmap::fromImage(image);
        QIcon ButtonIcon(m_NewPixMap); //pixmap);
        button->setIcon(ButtonIcon);
        //button->setIconSize(QSize(200, 200));
        int height_val = 150;
        button->setIconSize(QSize(height_val, (int) std::round((float) image.width() * (float) height_val / (float) image.height())));
        //QObject::connect(button, &QPushButton::clicked,std::bind(&MainWindow::patchClassifier, this, modelName));
        //scrollLayout->addWidget(button);
        //scrollList->addItem()
        //fileLayout->insertWidget(2, scrollArea);  //widget);
        // */
        //QObject::connect(button, &QListWidget::itemClicked, std::bind(&MainWindow::selectFileInProject, this));

        //QObject::connect(scrollList, &QPushButton::clicked, std::bind(&MainWindow::selectFileInProject, this, 1));
        //QObject::connect(scrollList, &QListWidget::itemPressed, std::bind(&MainWindow::selectFileInProject, this, 1));  // this, SLOT(onListMailItemClicked(QListWidgetItem*)));
        //QObject::connect(scrollList,itemClicked(QListWidgetItem*), std::bind(&MainWindow::selectFileInProject, this, 1));
        //connect(ui->listMail, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onListMailItemClicked(QListWidgetItem*)));
        // QListWidget::itemPressed(QListWidgetItem *item)
        //QObject::connect(scrollList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(&MainWindow::selectFileInProject));

        auto listItem = new QListWidgetItem;
        listItem->setSizeHint(
                QSize((int) std::round((float) image.width() * (float) height_val / (float) image.height()),
                      height_val));
        QObject::connect(button, &QPushButton::clicked, std::bind(&MainWindow::selectFileInProject, this, curr_pos));
        scrollList->addItem(listItem);
        scrollList->setItemWidget(listItem, button);

        curr_pos++;

        auto mBox = new QMessageBox(mWidget);
        std::string path = "Finished reading: " + split(fileName.toStdString(), "/").back();
        mBox->setText(path.c_str());
        mBox->setIcon(QMessageBox::Information);
        mBox->setModal(false);
        //mBox->show(); // TODO: Don't ask why I do multiple show()s here. I just do, and it works. Seems like I don't need it (anymore?)
        QRect screenrect = mWidget->screen()[0].geometry();
        mBox->move(mWidget->width() - mBox->width() / 2, - mWidget->width() / 2 - mBox->width() / 2);
        mBox->show();
        QTimer::singleShot(3000, mBox, SLOT(accept()));
    }
}


void MainWindow::selectFileInProject(int pos) {

    std::cout << "\n wsi selection worked!\n";

    // if you select a WSI and it's already open, do nothing
    if (filename == wsiList[pos]) {
        auto mBox = new QMessageBox(mWidget);
        mBox->setText("WSI is already open.");
        mBox->setIcon(QMessageBox::Information);
        mBox->setModal(false);
        mBox->show();
        QRect screenrect = mWidget->screen()[0].geometry();
        mBox->move(mWidget->width() - mBox->width() / 2, - mWidget->width() / 2 - mBox->width() / 2);
        mBox->show(); // Don't ask why I do multiple show()s here. I just do, and it works
        QTimer::singleShot(3000, mBox, SLOT(accept()));
        return;
    }

    // if there are any results created, prompt if you want to save results

    // add WSI to project list
    filename = wsiList[pos];

    // Import image from file using the ImageFileImporter
    auto importer = WholeSlideImageImporter::New();
    importer->setFilename(filename);
    m_image = importer->updateAndGetOutputData<ImagePyramid>();

    // get metadata
    metadata = m_image->getMetadata();

    auto renderer = ImagePyramidRenderer::New();
    renderer->setInputData(m_image);

    // TODO: Something here results in me not being able to run analyzis on new images (after the first)
    removeAllRenderers();
    insertRenderer("WSI", renderer);
    getView(0)->reinitialize(); // Must call this after removing all renderers

    // get WSI format
    wsiFormat = metadata["openslide.vendor"];

    // get magnification level of current WSI
    magn_lvl = getMagnificationLevel();

    // now make it possible to edit image in the View Widget
    createDynamicViewWidget("WSI", modelName);
}


// TODO: Make temporary directories using Qt's QTemporaryDir() instead of current solution

void MainWindow::createProject() {

    // start by selecting where to create folder and give project name
    // should also handle if folder name already exist, prompt warning and option to change name
    QFileDialog dialog(mWidget);
    dialog.setFileMode(QFileDialog::AnyFile);

    projectFolderName = dialog.getExistingDirectory(
            mWidget, tr("Set Project Directory"),
            QCoreApplication::applicationDirPath(), QFileDialog::DontUseNativeDialog);

    std::cout << "\nProject dir:\n" << projectFolderName.toStdString() << "\n";

    // create file for saving which WSIs exist in folder
    QString projectFileName = "/project.txt";
    QFile file(projectFolderName + projectFileName);
    if (file.open(QIODevice::ReadWrite)) {
        QTextStream stream(&file);
        //stream << "something" << endl;
    }

    // now create folders for saving results and such (prompt warning if name already exists)
    QDir().mkdir(projectFolderName + QString::fromStdString("/results"));
    QDir().mkdir(projectFolderName + QString::fromStdString("/pipelines"));
    QDir().mkdir(projectFolderName + QString::fromStdString("/thumbnails"));

    // check if any WSIs have been selected previously, and ask if you want to make a project and add these,
    // or make a new fresh one -> if no, need to clear all WSIs in the QListWidget
    if (pageComboBox->count() > 0) {
        // prompt
        QMessageBox mBox;
        mBox.setIcon(QMessageBox::Warning);
        mBox.setText("There are already WSIs that has been used.");
        mBox.setInformativeText("Do you wish to add them to the project?");
        mBox.setDefaultButton(QMessageBox::Yes);
        mBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        int ret = mBox.exec();

        switch (ret) {
            case QMessageBox::Yes:
                std::cout << "\n Saved! \n";
                saveProject();
                break;
            case QMessageBox::No:
                std::cout << "\n Removing WSIs from QListWidget! \n";
                scrollList->clear();
                break;
            default:
                break;
        }
    }
}


void MainWindow::openProject() {

    // clear wsiList if new project is made right after another one has been in use
    wsiList.clear();

    // select project file
    QFileDialog dialog(mWidget);
    dialog.setFileMode(QFileDialog::ExistingFile);
    QString projectPath = dialog.getOpenFileName(
            mWidget,
            tr("Select Project File"), nullptr,
            tr("Project (*project.txt)"),
            nullptr, QFileDialog::DontUseNativeDialog
    );

    std::cout << "\n " << projectPath.toStdString() << "\n";

    projectFolderName = split(projectPath.toStdString(), "project.txt")[0].c_str();

    std::cout << "\n " << projectFolderName.toStdString() << "\n";

    // check if all relevant files and folders are in selected folder directory
    // qDebug << "hallo";
    // if any of the folders does not exists, create them
    if (!QDir(projectFolderName + "/pipelines").exists()) {
        QDir().mkdir(projectFolderName + "/pipelines");
    }
    if (!QDir(projectFolderName + "/results").exists()) {
        QDir().mkdir(projectFolderName + "/results");
    }
    if (!QDir(projectFolderName + "/thumbnails").exists()) {
        QDir().mkdir(projectFolderName + "/thumbnails");
    }

    // now, parse project.txt-file and add if there are any WSIs in the project
    // create file for saving which WSIs exist in folder
    QList<QString> fileNames;
    QString projectFileName = "project.txt";
    QFile file(projectFolderName + projectFileName);
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            fileNames.push_back(line);
        }
    }

    if (fileNames.empty()) {
        // prompt if no valid WSI was found in project-file
        auto mBox = new QMessageBox(mWidget);
        mBox->setText("There was found no valid WSIs in the project-file.");
        mBox->setIcon(QMessageBox::Information);
        mBox->setModal(false);
        QRect screenrect = mWidget->screen()[0].geometry();
        mBox->move(mWidget->width() - mBox->width() / 2, -mWidget->width() / 2 - mBox->width() / 2);
        mBox->show();
        QTimer::singleShot(3000, mBox, SLOT(accept()));
    }

    int counter = 0;
    for (QString &fileName : fileNames) {

        if (fileName == "")
            return;
        filename = fileName.toStdString();
        wsiList.push_back(filename);

        // Import image from file using the ImageFileImporter
        auto importer = WholeSlideImageImporter::New();
        importer->setFilename(fileName.toStdString());
        m_image = importer->updateAndGetOutputData<ImagePyramid>();

        // for reading of multiple WSIs, only render last one
        if (counter == fileNames.count() - 1) {

            // get metadata
            metadata = m_image->getMetadata();

            auto renderer = ImagePyramidRenderer::New();
            renderer->setInputData(m_image);

            // TODO: Something here results in me not being able to run analysis on new images (after the first)
            removeAllRenderers();
            insertRenderer("WSI", renderer);
            getView(0)->reinitialize(); // Must call this after removing all renderers

            // get WSI format
            wsiFormat = metadata["openslide.vendor"];

            // get magnification level of current WSI
            magn_lvl = getMagnificationLevel();

            // now make it possible to edit image in the View Widget
            createDynamicViewWidget("WSI", modelName);
        }
        counter++;

        auto access = m_image->getAccess(ACCESS_READ);
        auto input = access->getLevelAsImage(m_image->getNrOfLevels() - 1);

        // try to convert to FAST Image -> QImage
        QImage image(input->getWidth(), input->getHeight(), QImage::Format_RGB32);

        // TODO have to do some type conversion here, assuming float for now
        unsigned char *pixelData = image.bits();

        ImageAccess::pointer new_access = input->getImageAccess(ACCESS_READ);
        void *inputData = new_access->get();
        uint nrOfComponents = input->getNrOfChannels();

        for (uint x = 0; x < input->getWidth(); x++) {
            for (uint y = 0; y < input->getHeight(); y++) {
                uint i = x + y * input->getWidth();
                for (uint c = 0; c < input->getNrOfChannels(); c++) {
                    float data;
                    data = ((uchar *) inputData)[i * nrOfComponents + c]; // assumes TYPE_UINT8
                    pixelData[i * 4 + c] = (unsigned char) data;
                    pixelData[i * 4 + 3] = 255; // Alpha
                }
            }
        }

        /*
        auto intensityScaler = ScaleImage::New();
        intensityScaler->setInputData(input); //m_tissue);  // expects Image data type
        intensityScaler->setLowestValue(0.0f);
        intensityScaler->setHighestValue(1.0f);
        //intensityScaler->update();
         */

        // attempt to save tissue mask to disk as .png
        /*
        ImageExporter::pointer exporter = ImageExporter::New();
        exporter->setFilename("/home/andrep/workspace/FAST-Pathology/ImageExporterTest.png");
        exporter->setInputData(input); //intensityScaler->updateAndGetOutputData<Image>());
        exporter->update();
         */

        //QPixmap pixmap(exporter);

        // /*
        auto button = new QPushButton();
        //QPixmap pixmap("/home/andrep/workspace/FAST-Pathology/ImageExporterTest.png");
        auto m_NewPixMap = QPixmap::fromImage(image);
        QIcon ButtonIcon(m_NewPixMap); //pixmap);
        button->setIcon(ButtonIcon);
        //button->setIconSize(QSize(200, 200));
        int height_val = 150;
        button->setIconSize(QSize(height_val, (int) std::round(
                (float) image.width() * (float) height_val / (float) image.height())));
        //QObject::connect(button, &QPushButton::clicked,std::bind(&MainWindow::patchClassifier, this, modelName));
        //scrollLayout->addWidget(button);
        //scrollList->addItem()
        //fileLayout->insertWidget(2, scrollArea);  //widget);
        // */
        //QObject::connect(button, &QListWidget::itemClicked, std::bind(&MainWindow::selectFileInProject, this));

        //QObject::connect(scrollList, &QPushButton::clicked, std::bind(&MainWindow::selectFileInProject, this, 1));
        //QObject::connect(scrollList, &QListWidget::itemPressed, std::bind(&MainWindow::selectFileInProject, this, 1));  // this, SLOT(onListMailItemClicked(QListWidgetItem*)));
        //QObject::connect(scrollList,itemClicked(QListWidgetItem*), std::bind(&MainWindow::selectFileInProject, this, 1));
        //connect(ui->listMail, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onListMailItemClicked(QListWidgetItem*)));
        // QListWidget::itemPressed(QListWidgetItem *item)
        //QObject::connect(scrollList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(&MainWindow::selectFileInProject));

        auto listItem = new QListWidgetItem;
        listItem->setSizeHint(
                QSize((int) std::round((float) image.width() * (float) height_val / (float) image.height()),
                      height_val));
        QObject::connect(button, &QPushButton::clicked, std::bind(&MainWindow::selectFileInProject, this, curr_pos));
        scrollList->addItem(listItem);
        scrollList->setItemWidget(listItem, button);

        curr_pos++;

        auto mBox = new QMessageBox(mWidget);
        std::string path = "Finished reading: " + split(fileName.toStdString(), "/").back();
        mBox->setText(path.c_str());
        mBox->setIcon(QMessageBox::Information);
        mBox->setModal(false);
        //mBox->show(); // TODO: Don't ask why I do multiple show()s here. I just do, and it works. However, seems like I don't need it anymore. So very mysterious...
        QRect screenrect = mWidget->screen()[0].geometry();
        mBox->move(mWidget->width() - mBox->width() / 2, -mWidget->width() / 2 - mBox->width() / 2);
        mBox->show();
        QTimer::singleShot(3000, mBox, SLOT(accept()));

    }
}


void MainWindow::saveProject() {

    // create file for saving which WSIs exist in folder
    QString projectFileName = "/project.txt";
    QFile file(projectFolderName + projectFileName);
    file.resize(0);  // clear it and then write
    if (file.open(QIODevice::ReadWrite)) {
        foreach(std::string currPath, wsiList) {
            QTextStream stream(&file);
            std::cout << "\nLine: " << currPath << "\n";
            stream << currPath.c_str() << endl;
            //stream << "something" << endl;
        }
    }
}


void MainWindow::addPipelines() {

    QStringList ls = QFileDialog::getOpenFileNames(
            mWidget,
            tr("Select Pipeline"), nullptr,
            tr("Pipeline Files (*.fpl)"),
            nullptr, QFileDialog::DontUseNativeDialog
    );

    // now iterate over all selected files and add selected files and corresponding ones to Pipelines/
    for (QString& fileName : ls) {

        if (fileName == "")
            continue;

        std::cout << "\n testing... " << fileName.toStdString();

        std::string someFile = split(fileName.toStdString(), "/").back();
        std::string oldLocation = split(fileName.toStdString(), someFile)[0];
        std::string newLocation = cwd + "Pipelines/";
        std::string newPath = cwd + "Pipelines/" + someFile;
        std::cout << "\n testing2... " << newPath;
        if (fileExists(newPath)) {
            std::cout << "\n" << fileName.toStdString() << "\n: File with the same name already exists in folder, didn't transfer... \n";
            continue;
        } else {
            if (fileExists(fileName.toStdString())) {
                QFile::copy(fileName, QString::fromStdString(newPath));
            }
        }
    }
}


void MainWindow::createPipeline() {
    1;
}


void MainWindow::addModels() {

    //QString fileName = QFileDialog::getOpenFileName(
    QStringList ls = QFileDialog::getOpenFileNames(
            mWidget,
            tr("Select Model"), nullptr,
            tr("Model Files (*.pb *.txt *.h5 *.xml *.mapping *.bin *.uff)"),
            nullptr, QFileDialog::DontUseNativeDialog
    ); // TODO: DontUseNativeDialog - this was necessary because I got wrong paths -> /run/user/1000/.../filename instead of actual path

    // now iterate over all selected files and add selected files and corresponding ones to Models/
    for (QString& fileName : ls) {

        if (fileName == "")
            return;

        std::cout << "\norig filename: " << fileName.toStdString();

        //qDebug().nospace() << "abc" << qPrintable(fileName) << "\ndef";
        std::string someFile = split(fileName.toStdString(),
                                     "/").back(); // TODO: Need to make this only split on last "/"
        std::cout << "\nPath: " << someFile;
        std::string oldLocation = split(fileName.toStdString(), someFile)[0];

        std::string newLocation = cwd + "Models/";

        std::cout << "Old cwd: \n" << cwd;

        std::vector<string> names = split(someFile, ".");
        string fileNameNoFormat = names[0];
        string formatName = names[1];

        // copy selected file to Models folder
        // check if file already exists in new folder, if yes, print warning, and stop
        string newPath = cwd + "Models/" + someFile;
        if (fileExists(newPath)) {
            std::cout << "\n file with same name already exists in folder, didn't transfer... \n";
            continue;
        }
        //QFile::copy(fileName, QString::fromStdString(newPath));

        // check which corresponding model files that exist, except from the one that is chosen
        std::vector<std::string> allowedFileFormats{"txt", "pb", "h5", "mapping", "xml", "bin", "uff"};

        std::cout << "\n Copy test \n";
        foreach(std::string currExtension, allowedFileFormats) {
            std::string oldPath = oldLocation + fileNameNoFormat + "." + currExtension;
            std::cout << currExtension << "\n\n" << oldPath << "\n\n";
            if (fileExists(oldPath)) {
                QFile::copy(QString::fromStdString(oldPath),QString::fromStdString(cwd + "Models/" + fileNameNoFormat + "." + currExtension));
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
        QObject::connect(someButton, &QPushButton::clicked,
                         std::bind(&MainWindow::patchClassifier, this, modelName));
        someButton->show();

        processLayout->insertWidget(processLayout->count(), someButton);

        //createMainMenuWidget();
        //if (processLayout){
        //    clearLayout(processLayout);
        //}
        //createProcessWidget();
        //mainLayout->removeWidget(menuWidget); // this doesnt work...
    }
}


float MainWindow::getMagnificationLevel() {

    float magnification_lvl = 0.0f;

    if ((wsiFormat == "generic-tiff") or (wsiFormat == "philips")) {

        int level_count = m_image->getNrOfLevels(); //(int)stof(metadata["openslide.level-count"]);

        // assuming each 400X resolution correspond to 50000 cm, and thus 200x => 100000 cm ... (for .tiff format)
        // can predict which WSI resolution image is captured with, as some models is trained on 200x images
        std::vector<float> templateResVector;
        float resFractionValue = 50000;
        for (int i = 0; i < level_count; i++) {
            templateResVector.push_back(resFractionValue * ((float) i + 1));
        }

        auto resolution = 1;  // needed to initialize outside of if statement -> set some silly dummy value
        //auto resolution = std::stof(m_image->getMetadata("tiff.XResolution")); //(int)stof(metadata["tiff.XResolution"]);

        std::cout << "\ntesting" << wsiFormat << "\n\n";
        if (wsiFormat == "generic-tiff") {
            resolution = std::stof(m_image->getMetadata("tiff.XResolution")); //(int)stof(metadata["tiff.XResolution"]);
        } else if (wsiFormat == "phillips") {
            resolution = std::stof(metadata["mpp-x"]);
        }

        // find closest value => get magnification level
        auto i = min_element(begin(templateResVector), end(templateResVector), [=](int x, int y) {
            return abs(x - resolution) < abs(y - resolution);
        });
        float location = std::distance(begin(templateResVector), i);

        magnification_lvl = 40.0f / pow(2.0f, location);

    } else if (wsiFormat == "aperio") {
        magnification_lvl = std::stof(metadata["aperio.AppMag"]); //atof(openslide_get_property_value(osr, "aperio.AppMag"));
    } else {  //"TODO: Make this more general, test different image formats to see how the magn_lvl metadata vary"
        std::cout << "\n aperio test" << metadata["aperio.AppMag"] << "\n apps";
        magnification_lvl = std::stof(metadata["aperio.AppMag"]);
    }
    return magnification_lvl;
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


bool MainWindow::imageSegmenter(std::string modelName) {
    // read model metadata (txtfile)
    std::map<std::string, std::string> modelMetadata = getModelMetadata(modelName);

    if (!hasRenderer(modelMetadata["name"])) { // only run analysis if it has not been ran previously on current WSI
        1;
    }

    return true;
}


// TODO: Integrate SegmentationClassifier inside this or make functions for each task?
bool MainWindow::patchClassifier(std::string modelName) {

    // read model metadata (txtfile)
    std::map<std::string, std::string> modelMetadata = getModelMetadata(modelName);

    if (!hasRenderer(modelMetadata["name"])) { // only run analysis if it has not been ran previously on current WSI

        //auto tmpRenderer = getRenderer(metadata["name"]);

        // based on predicted magnification level of WSI, set magnificiation level for optimal input to model based on predicted resolution of WSI
        int patch_lvl_model = (int) (std::log(magn_lvl / (float)std::stoi(modelMetadata["magnification_level"])) / std::log(std::round(stof(metadata["openslide.level[1].downsample"]))));

        // segment tissue
        segmentTissue(); // TODO: Something weird happens when I run this again
        hideTissueMask(true);

        auto generator = PatchGenerator::New();
        generator->setPatchSize(std::stoi(modelMetadata["input_img_size_y"]), std::stoi(modelMetadata["input_img_size_x"]));
        generator->setPatchLevel(patch_lvl_model);
        generator->setInputData(0, m_image);
        generator->setInputData(1, m_tissue);

        //auto batchgen = ImageToBatchGenerator::New();  // TODO: Can't use this with TensorRT (!)
        //batchgen->setInputConnection(generator->getOutputPort());
        //batchgen->setMaxBatchSize(std::stoi(modelMetadata["batch_process"])); // set 256 for testing stuff (tumor -> annotation, then grade from tumor segment)

        // get available IEs as a list
        std::list<std::string> IEsList;
        QStringList tmpPaths = QDir(QString::fromStdString(Config::getLibraryPath())).entryList(QStringList(), QDir::Files);
        foreach(QString filePath, tmpPaths) {
            if (filePath.toStdString().find("libInferenceEngine") != std::string::npos) {
                IEsList.push_back(split(split(filePath.toStdString(), "libInferenceEngine").back(), ".so")[0]);
            }
        }

        // check which model formats exists, before choosing inference engine
        QDir directory(QString::fromStdString(cwd + "Models/"));
        QStringList models = directory.entryList(QStringList(), QDir::Files);

        std::list<std::string> acceptedModels;
        foreach(QString currentModel, models) {
            if (currentModel.toStdString().find(modelName) != std::string::npos) {
                acceptedModels.push_back("." + split(currentModel.toStdString(), modelName + ".").back());
            }
        }

        // init network
        auto network = NeuralNetwork::New();
        if ((modelMetadata["problem"] == "segmentation") && (modelMetadata["resolution"] == "high")) {
            network = SegmentationNetwork::New();
        } else if ((modelMetadata["problem"] == "classification") && (modelMetadata["resolution"] == "high")) {
            network = NeuralNetwork::New();
        } else {
            std::cout << "\nSomething is wrong in the .txt-file... Please check that it is written correctly.\n";
        }
        //network->setInferenceEngine("TensorRT"); //"TensorRT");

        bool checkFlag = true;
        // /*
        // Now select best available IE based on which extensions exist for chosen model
        // TODO: Current optimization profile is: 0. Please ensure there are no enqueued operations pending in this context prior to switching profiles
        if (std::find(acceptedModels.begin(), acceptedModels.end(), ".uff") != acceptedModels.end() && std::find(IEsList.begin(), IEsList.end(), "TensorRT") != IEsList.end()) {
            network->setInferenceEngine("TensorRT");
        } else if (std::find(acceptedModels.begin(), acceptedModels.end(), ".pb") != acceptedModels.end() && std::find(IEsList.begin(), IEsList.end(), "TensorFlowCUDA") != IEsList.end()) {
            network->setInferenceEngine("TensorFlowCUDA");
            if (std::find(acceptedModels.begin(), acceptedModels.end(), ".xml") != acceptedModels.end() && std::find(IEsList.begin(), IEsList.end(), "OpenVINO") != IEsList.end()) {
                network->setInferenceEngine("OpenVINO");
            }
        } else if (std::find(acceptedModels.begin(), acceptedModels.end(), ".xml") != acceptedModels.end() && std::find(IEsList.begin(), IEsList.end(), "OpenVINO") != IEsList.end()) {
            network->setInferenceEngine("OpenVINO");
        } else if (std::find(acceptedModels.begin(), acceptedModels.end(), ".pb") != acceptedModels.end() && std::find(IEsList.begin(), IEsList.end(), "TensorFlowCPU") != IEsList.end()) {
            network->setInferenceEngine("TensorFlowCPU");
        } else {
            std::cout << "\nModel does not exist in Models/ folder. Please add it using AddModels(). "
                         "It might also be that the model exists, but the Inference Engine does not. "
                         "Available IEs are: ";
            foreach(std::string elem, IEsList) {
                std::cout << elem << ", ";
            }
            std::cout << "\n";
            checkFlag = false;
        }
        // */

        if (checkFlag) {
            std::cout << "\nModel was found.";

            // TODO: Need to handle if model is in Models/, but inference engine is not available
            Config::getLibraryPath();

            const auto engine = network->getInferenceEngine()->getName();
            // IEs like TF and TensorRT need to be handled differently than IEs like OpenVINO
            if (engine.substr(0, 10) == "TensorFlow") {
                // apparently this is needed if model has unspecified input size
                network->setInputNode(0, modelMetadata["input_node"], NodeType::IMAGE, TensorShape(
                        {1, std::stoi(modelMetadata["input_img_size_y"]), std::stoi(modelMetadata["input_img_size_x"]),
                         std::stoi(modelMetadata["nb_channels"])})); //{1, size, size, 3}
                // TensorFlow needs to know what the output node is called
                if (modelMetadata["problem"] == "classification") {
                    network->setOutputNode(0, modelMetadata["output_node"], NodeType::TENSOR,
                                           TensorShape({1, std::stoi(modelMetadata["nb_classes"])}));
                } else if (modelMetadata["problem"] == "segmentation") {
                    network->setOutputNode(0, modelMetadata["output_node"], NodeType::TENSOR,
                                           TensorShape({1, std::stoi(modelMetadata["input_img_size_y"]), std::stoi(modelMetadata["input_img_size_x"]),
                                                        std::stoi(modelMetadata["nb_classes"])}));
                }
            } else if (engine == "TensorRT") {
                // TensorRT needs to know everything about the input and output nodes
                network->setInputNode(0, modelMetadata["input_node"], NodeType::IMAGE, TensorShape(
                        {1, std::stoi(modelMetadata["nb_channels"]), std::stoi(modelMetadata["input_img_size_y"]),
                         std::stoi(modelMetadata["input_img_size_y"])})); //{1, size, size, 3}
                network->setOutputNode(0, modelMetadata["output_node"], NodeType::TENSOR,
                                       TensorShape({1, std::stoi(modelMetadata["nb_classes"])}));
            }
            network->load(cwd + "Models/" + modelName + "." +
                          network->getInferenceEngine()->getDefaultFileExtension()); //".uff");
            network->setInputConnection(generator->getOutputPort());
            vector scale_factor = split(modelMetadata["scale_factor"], "/"); // get scale factor from metadata
            network->setScaleFactor(
                    (float) std::stoi(scale_factor[0]) / (float) std::stoi(scale_factor[1]));   // 1.0f/255.0f

            auto stitcher = PatchStitcher::New();
            stitcher->setInputConnection(network->getOutputPort());

            auto port = stitcher->getOutputPort();
            //port->getNextFrame();
            //stitcher->update();
            //port->getNextFrame<Tensor>();


            // define renderer from metadata
            //auto someRenderer = HeatmapRenderer::New();
            if ((modelMetadata["problem"] == "classification") && (modelMetadata["resolution"] == "high")) {
                auto someRenderer = HeatmapRenderer::New();
                someRenderer->setInterpolation(std::stoi(modelMetadata["interpolation"].c_str()));
                someRenderer->setInputConnection(port);
                //heatmapRenderer->update();
                vector<string> colors = split(modelMetadata["class_colors"], ";");
                for (int i = 0; i < std::stoi(modelMetadata["nb_classes"]); i++) {
                    vector<string> rgb = split(colors[i], ",");
                    someRenderer->setChannelColor(i, Color((float) std::stoi(rgb[0]) / 255.0f,
                                                           (float) std::stoi(rgb[1]) / 255.0f,
                                                           (float) std::stoi(rgb[2]) / 255.0f));
                }

                insertRenderer(modelMetadata["name"], someRenderer);

            } else if ((modelMetadata["problem"] == "segmentation") && (modelMetadata["resolution"] == "high")) {
                auto someRenderer = SegmentationPyramidRenderer::New();
                someRenderer->setInputConnection(port);

                insertRenderer(modelMetadata["name"], someRenderer);
            }

            // now make it possible to edit prediction in the View Widget
            createDynamicViewWidget(modelMetadata["name"], modelName);

            //std::future<void> fut = std::async(stitcher, network->getOutputPort());

            // test
            //usleep(5);

            //if (modelMetadata["name"] == "tumor") {
            if (false) {
                stitcher->update();
                m_tumorMap_tensor = port->getNextFrame<Tensor>();

                auto tmp = TensorToSegmentation::New();
                tmp->setInputData(m_tumorMap_tensor);

                m_tumorMap = tmp->updateAndGetOutputData<Image>();

                // wait for process is finished, then same prediction as map
                /*
                std::promise<void> promise;
                std::future<void> future = promise.get_future();//std::future<int> out = std::async(heatmapRenderer); //ThreadPool.RegisterWaitForSingleObject();
                request();
                future.get();
                 */

                auto segTumorRenderer = SegmentationRenderer::New();
                segTumorRenderer->setOpacity(0.4);
                segTumorRenderer->setColor(Segmentation::LABEL_FOREGROUND, Color(255.0 / 255.0f, 0.0f, 0.0f));
                segTumorRenderer->setColor(Segmentation::LABEL_BACKGROUND, Color(0.0f, 255.0f / 255.0f, 0.0f));
                segTumorRenderer->setInputData(m_tumorMap);
                //segTumorRenderer->setInterpolation(false);
                segTumorRenderer->update();

                insertRenderer("tumorSeg", segTumorRenderer);

                createDynamicViewWidget("tumorSeg", modelName);
            }

        }
    }
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
        if ((someName == "tissue") && (someName == "tumorSeg")) { // TODO: Make this more generic -> should recognize which renderer type automatically
            auto someRenderer = std::dynamic_pointer_cast<SegmentationRenderer>(getRenderer(someName));
            someRenderer->setOpacity((float) value / 10.0f);
            someRenderer->setModified(true);
        } else {
            auto someRenderer = std::dynamic_pointer_cast<HeatmapRenderer>(getRenderer(someName));
            someRenderer->setMaxOpacity((float) value / 10.0f);
            //someRenderer->setMinConfidence(0.9);
        }
        return true;
    }
}


bool MainWindow::hideChannel(const std::string& someName) {
    if ((someName == "WSI") || (someName == "tissue")){
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
