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
#include <FAST/Algorithms/NeuralNetwork/BoundingBoxNetwork.hpp>
#include <FAST/Visualization/BoundingBoxRenderer/BoundingBoxRenderer.hpp>
#include <FAST/Algorithms/Lambda/RunLambda.hpp>
#include <FAST/Algorithms/ScaleImage/ScaleImage.hpp>
#include <FAST/Data/BoundingBox.hpp>
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
#include <FAST/Importers/ImageImporter.hpp>
#include <FAST/Exporters/ImageExporter.hpp>
#include <FAST/Importers/ImageFileImporter.hpp>
#include <FAST/Exporters/ImageFileExporter.hpp>
#include <FAST/Algorithms/ScaleImage/ScaleImage.hpp>
#include <FAST/Data/Access/ImagePyramidAccess.hpp>
#include <QShortcut>
#include <FAST/Algorithms/BinaryThresholding/BinaryThresholding.hpp>
#include <FAST/Algorithms/ImageResizer/ImageResizer.hpp>
#include <FAST/PipelineEditor.hpp>
#include <FAST/Pipeline.hpp>
#include <FAST/Visualization/MultiViewWindow.hpp>
#include <FAST/Algorithms/NonMaximumSuppression/NonMaximumSuppression.hpp>
#include <FAST/Algorithms/Morphology/Dilation.hpp>
#include <FAST/Algorithms/Morphology/Erosion.hpp>
#include <QMimeData>
#include <QDragEnterEvent>
//#include <jkqtplotter/graphs/jkqtpbarchart.h>

using namespace std;

//inline void initMyResource() { Q_INIT_RESOURCE(qtres); }

namespace fast {

MainWindow::MainWindow() {
    applicationName = "FastPathology";
    advancedMode = false;
    setTitle(applicationName);
    enableMaximized(); // <- function from Window.cpp

    mWidget->setAcceptDrops(true); // to enable drag/drop events
    QObject::connect(mWidget, &WindowWidget::filesDropped, std::bind(&MainWindow::receiveFileList, this, std::placeholders::_1));  //this, SLOT(receiveFileList(QList<QString>))); //std::bind(&MainWindow::receiveFileList, this));

    //mWidget->setMouseTracking(true);
    //mWidget->setFocusPolicy(Qt::StrongFocus);
    //mWidget->setFocusPolicy(Qt::ClickFocus);  //Qt::ClickFocus);
    
	cwd = QDir::homePath().toStdString();
    cwd += "/fastpathology/";

	// set window icon
	mWidget->setWindowIcon(QIcon(":/data/Icons/fastpathology_logo_large.png"));

    // create temporary tmp folder to store stuff, and create temporary file to store history
    QTemporaryDir tmpDir;
	tmpDirPath = tmpDir.path().toStdString();
	std::cout << "Temporary path: " << tmpDirPath << std::endl;

    //printf("\n%d\n",__LINE__); // <- this is nice for debugging

    // Create models folder platform assumes that this folder exists and contains all relevant models (and pipelines, and temporary models)
    //createDirectories(dir_str);
	QDir().mkpath(QString::fromStdString(cwd) + "data/Models"); // <- mkpath creates the entire path recursively (convenient if subfolder dependency is missing)
    QDir().mkpath(QString::fromStdString(cwd) + "data/Pipelines");

	// create temporary project folder
	QString projectFolderName = QString::fromStdString(tmpDirPath) + "/project_" + QString::fromStdString(createRandomNumbers_(8));
	QDir().mkpath(projectFolderName);
	QString projectFileName = "/project.txt";
	QFile file(projectFolderName + projectFileName);
	if (file.open(QIODevice::ReadWrite)) {
		QTextStream stream(&file);
	}
	QDir().mkdir(projectFolderName + QString::fromStdString("/results"));
	QDir().mkdir(projectFolderName + QString::fromStdString("/pipelines"));
	QDir().mkdir(projectFolderName + QString::fromStdString("/thumbnails"));

    // changing color to the Qt background)
    //mWidget->setStyleSheet("font-size: 16px; background: rgb(221, 209, 199); color: black;"); // Current favourite
	mWidget->setStyle(QStyleFactory::create("Fusion")); // TODO: This has to be before setStyleSheet?
	/*
    const auto qss =
            "QToolButton:!hover{background-color: #00ff00;}"
            "QToolButton:hover{background-color: #ff0000;}"
            "font-size: 16px"
            "background: rgb(221, 209, 199)"
            "color: black"
            ;
	 */
    const auto qss = "font-size: 16px;"
                     "QMenuBar::item:selected { background: white; }; QMenuBar::item:pressed {  background: white; };"
                     "QMenu{background-color:palette(window);border:1px solid palette(shadow);};"
                     ;
	//mWidget->setStyleSheet("font-size: 16px; background: rgb(221, 209, 199); color: black;");
    mWidget->setStyleSheet(qss);

    superLayout = new QVBoxLayout;
    mWidget->setLayout(superLayout);

    // make overall Widget
    mainWidget = new QWidget(mWidget);
    superLayout->insertWidget(1, mainWidget);

    // Create vertical GUI layout:
    mainLayout = new QHBoxLayout;
    mainWidget->setLayout(mainLayout);

    createMainMenuWidget(); // create menu widget
    createMenubar();        // create Menubar
    createOpenGLWindow();   // create OpenGL window
}


std::string MainWindow::createRandomNumbers_(int n) {
	std::string out = "";
	for (int i = 0; i < n; i++) {
		out = out + std::to_string(i);
	}
	return out;
}


void MainWindow::receiveFileList(const QList<QString> &names) {
    selectFileDrag(names);
}


void MainWindow::createOpenGLWindow() {
	float OpenGL_background_color = 0.0f; //0.0f; //200.0f / 255.0f;
	view = createView();
	//view = mWidget->addView();
	//view->setLayout(mainLayout);

	//mainLayout->addLayout(menuLayout);
	//mainLayout->insertWidget(1, view);
	view->set2DMode();
	view->setBackgroundColor(Color(OpenGL_background_color, OpenGL_background_color, OpenGL_background_color)); // setting color to the background, around the WSI
	view->setAutoUpdateCamera(true);
	//view->setLayout(mainLayout);
	//view->setToolTip("hallo");

	// create QSplitter for adjustable windows
	auto mainSplitter = new QSplitter(Qt::Horizontal);
	//mainSplitter->setHandleWidth(5);
	//mainSplitter->setFrameShadow(QFrame::Sunken);
	//mainSplitter->setStyleSheet("background-color: rgb(55, 100, 110);");
	mainSplitter->setHandleWidth(5);
	mainSplitter->setStyleSheet("QSplitter::handle { background-color: rgb(100, 100, 200); }; QMenuBar::handle { background-color: rgb(20, 100, 20); }");
	mainSplitter->addWidget(menuWidget);
	mainSplitter->addWidget(view);
	mainSplitter->setStretchFactor(1, 1);

	/*
	//tb->setStyleSheet("QMenuBar::item:selected { background: white; }; QMenuBar::item:pressed {  background: white; };");
    //                         "border-bottom:2px solid rgba(25,25,120,75); "
    //                         "QMenu{background-color:palette(window);border:1px solid palette(shadow);}");
	*/

    mainLayout->addWidget(mainSplitter);
}


void MainWindow::setApplicationMode() {
    // prompt
    QMessageBox mBox;
    mBox.setIcon(QMessageBox::Warning);
    if (setModeButton->text() == "Clinical mode") {
        mBox.setText("This will set the application from clinical mode to advanced mode.");
    } else {
        mBox.setText("This will set the application from advanced mode to clinical mode.");
    }
    mBox.setInformativeText("Are you sure you want to change mode?");
    mBox.setDefaultButton(QMessageBox::No);
    mBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    int ret = mBox.exec();

    switch (ret) {
        case QMessageBox::Yes:
            // toggle and update text on button to show current mode
            advancedMode = !advancedMode; // toggle
            if (setModeButton->text() == "Clinical mode") {
                setModeButton->setText("Research mode");
            } else {
                setModeButton->setText("Clinical mode");
            }
            // also update title
            if (advancedMode) {
                setTitle(applicationName + " (Research mode)" + " - " + split(filename, "/").back());
            } else {
                setTitle(applicationName + " - " + split(filename, "/").back());
            }
            break;
        case QMessageBox::No:
            1; // if "No", do nothing (also default)
            break;
        default:
            break;
    }
}


void MainWindow::reportIssueUrl() {
    QDesktopServices::openUrl(QUrl("https://github.com/SINTEFMedtek/FAST-Pathology/issues", QUrl::TolerantMode));
}


void MainWindow::helpUrl() {
    QDesktopServices::openUrl(QUrl("https://github.com/SINTEFMedtek/FAST-Pathology", QUrl::TolerantMode));
}


void MainWindow::aboutProgram() {

	auto currLayout = new QVBoxLayout;

	QPixmap image(QString::fromStdString(":/data/Icons/fastpathology_logo_large.png"));

	auto label = new QLabel;
	label->setPixmap(image);
	label->setAlignment(Qt::AlignCenter);
	
	auto textBox = new QTextEdit;
	textBox->setEnabled(false);
	textBox->setText("<html><b>FastPathology v0.1.0</b</html>");
	textBox->append("");
	textBox->setAlignment(Qt::AlignCenter);
	textBox->append("Open-source platform for deep learning-based research and decision support in digital pathology.");
	textBox->append("");
	textBox->append("");
	textBox->setAlignment(Qt::AlignCenter);
	textBox->append("Author: Andre Pedersen"); // + QStringLiteral("�") + " Pedersen");
	textBox->setAlignment(Qt::AlignCenter);
	textBox->setStyleSheet("QTextEdit { border: none }");
	//textBox->setBaseSize(150, 200);

	currLayout->addWidget(label);
	currLayout->addWidget(textBox);


	auto dialog = new QDialog(mWidget);
	dialog->setWindowTitle("About");
	dialog->setLayout(currLayout);
	//dialog->setBaseSize(QSize(800, 800));

	dialog->show();
}


void MainWindow::createMenubar() {

    // need to create a new QHBoxLayout for the menubar
    auto topFiller = new QMenuBar(mWidget);
    //topFiller->setStyleSheet("QMenuBar::item:selected { background: white; }; QMenuBar::item:pressed {  background: white; };"
    //                         "border-bottom:2px solid rgba(25,25,120,75); "
    //                         "QMenu{background-color:palette(window);border:1px solid palette(shadow);}");
    //topFiller->setStyleSheet(qss);
    topFiller->setMaximumHeight(30);

    //auto fileMenu = new QMenu();
    auto fileMenu = topFiller->addMenu(tr("&File"));
    //fileMenu->setFixedHeight(100);
    //fileMenu->setFixedWidth(100);
    //QAction *createProjectAction;
    fileMenu->addAction("Create Project", this,  &MainWindow::createProject);
    fileMenu->addAction("Import WSIs", this, &MainWindow::selectFile);
    fileMenu->addAction("Add Models", this, &MainWindow::addModels);
    fileMenu->addAction("Add Pipelines", this, &MainWindow::addPipelines);
    fileMenu->addSeparator();
    fileMenu->addAction("Quit", QApplication::quit);

    auto editMenu = topFiller->addMenu(tr("&Edit"));
    editMenu->addAction("Reset", this, &MainWindow::reset);
    editMenu->addAction("Change mode", this, &MainWindow::setApplicationMode);

    auto pipelineMenu = topFiller->addMenu(tr("&Pipelines"));
    pipelineMenu->addAction("Import pipelines", this, &MainWindow::addPipelines);
    pipelineMenu->addAction("Pipeline Editor", this, &MainWindow::customPipelineEditor);
    runPipelineMenu = new QMenu("&Run Pipeline", mWidget);
    //runPipelineMenu->addAction("Tumor segmentation", this, &MainWindow::lowresSegmenter);
    //runPipelineMenu->addAction("Grade classification");
    pipelineMenu->addMenu(runPipelineMenu);

    auto projectMenu = topFiller->addMenu(tr("&Projects"));
    projectMenu->addAction("Create Project", this, &MainWindow::createProject);
    projectMenu->addAction("Open Project", this, &MainWindow::openProject);
    projectMenu->addAction("Save Project", this, &MainWindow::saveProject);
	projectMenu->addAction("Run for project", this, &MainWindow::runForProject);

    loadPipelines(); // load pipelines that exists in the data/Pipelines directory

    //auto deployMenu = new QMenu();
    auto deployMenu = topFiller->addMenu(tr("&Deploy"));
    //deployMenu->addMenu(tr("&Deploy"));
    //deployMenu->setFixedHeight(100);
    //deployMenu->setFixedWidth(100);
    deployMenu->addAction("Run Pipeline");
    deployMenu->addAction("Segment Tissue");
    deployMenu->addAction("Predict Tumor");
    deployMenu->addAction("Classify Grade");
    deployMenu->addAction("Lowres Tumor Segment", this, &MainWindow::lowresSegmenter);
    deployMenu->addSeparator();

    auto helpMenu = topFiller->addMenu(tr("&Help"));
    helpMenu->addAction("Contact support", helpUrl);
    helpMenu->addAction("Report issue", reportIssueUrl);
    helpMenu->addAction("Check for updates");  // TODO: Add function that checks if the current binary in usage is the most recent one
    helpMenu->addAction("About", this, &MainWindow::aboutProgram);

    //topFiller->addMenu(fileMenu);
    //topFiller->addMenu(deployMenu);

    superLayout->insertWidget(0, topFiller);
}


void MainWindow::loadPipelines() {
    QStringList pipelines = QDir(QString::fromStdString(cwd) + "data/Pipelines").entryList(QStringList() << "*.fpl" << "*.FPL",QDir::Files);
    foreach(QString currentFpl, pipelines) {
        //runPipelineMenu->addAction(QString::fromStdString(split(currentFpl.toStdString(), ".")[0]), this, &MainWindow::lowresSegmenter);
        auto currentAction = runPipelineMenu->addAction(currentFpl); //QString::fromStdString(split(split(currentFpl.toStdString(), "/")[-1], ".")[0]));
        QObject::connect(currentAction, &QAction::triggered, std::bind(&MainWindow::runPipeline, this, cwd + "data/Pipelines/" + currentFpl.toStdString()));

        //auto currentAction = runPipelineMenu->addAction(QString::fromStdString(split(someFile, ".")[0]));
        //QObject::connect(currentAction, &QAction::triggered, std::bind(&MainWindow::runPipeline, this, someFile));
    }
}


void MainWindow::reset() {
    //first prompt warning, that it will delete all unsaved results, etc...
    if (pageComboBox->count() > 0) {
        // prompt
        QMessageBox mBox;
        mBox.setIcon(QMessageBox::Warning);
        mBox.setText("This will delete all history.");
        mBox.setInformativeText("Are you sure you want to reset?");
        mBox.setDefaultButton(QMessageBox::No);
        mBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        int ret = mBox.exec();

        switch (ret) {
            case QMessageBox::Yes:
                wsiList.clear();
                savedList.clear();
                scrollList->clear();
                removeAllRenderers();
                pageComboBox->clear();
                exportComboBox->clear();
                break;
            case QMessageBox::No:
                1;
                break;
            default:
                break;
        }
    }

    // update application name to contain current WSI
    if (advancedMode) {
        setTitle(applicationName + " (Research mode)");
    } else {
        setTitle(applicationName);
    }

}


void MainWindow::createMainMenuWidget() {

    // create widgets for Menu layout
    createFileWidget();
    createProcessWidget();
    createViewWidget();
    createStatsWidget();
    createExportWidget(); // TODO: MAKE IT DYNAMIC DEPENDING ON WHAT RESULTS ARE AVAILABLE
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
    stackedWidget = new QStackedWidget(mWidget);
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

    auto mapper = new QSignalMapper(mWidget);

    auto spacerWidgetLeft = new QWidget(mWidget);
    spacerWidgetLeft->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    spacerWidgetLeft->setVisible(true);

    auto spacerWidgetRight = new QWidget(mWidget);
    spacerWidgetRight->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    spacerWidgetRight->setVisible(true);

    auto tb = new QToolBar(mWidget);
    //tb->setStyleSheet("QMenuBar::item:selected { background: white; }; QMenuBar::item:pressed {  background: white; };");
    //                         "border-bottom:2px solid rgba(25,25,120,75); "
    //                         "QMenu{background-color:palette(window);border:1px solid palette(shadow);}");
	//tb->setStyleSheet("{ background-color: rgb(100, 100, 200); }; QMenuBar::handle { background-color: rgb(20, 100, 20);");
    tb->setIconSize(QSize(im_size, im_size));
    //tb->setFixedWidth(200);
    //tb->setMovable(true);
    //tb->setMinimumSize(QSize(im_size, im_size));
    //tb->setBaseSize(QSize(im_size, im_size));
    tb->setFont(QFont("Times", 8)); //QFont::Bold));
    tb->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);  // adds text under icons

	//QResource::registerResource("qtres.qrc");

	/*
	std::cout << "Anything in Qt Resources: " << std::endl;
	QDirIterator it(":", QDir::NoFilter);
	while (it.hasNext()) {
		auto tmp = it.next();
		QDirIterator it2(tmp, QDir::NoFilter);
		std::cout << "elem: " << tmp.toStdString() << std::endl;
		while (it2.hasNext()) {
			std::cout << "elem: " << it2.next().toStdString() << std::endl;
		}
		//qDebug() << it.next();
	}
	 */
		
    //auto toolBar = new QToolBar;
    QPixmap openPix(QString::fromStdString(":/data/Icons/import_icon_new_cropped_resized.png"));
	QPixmap processPix(QStringLiteral(":/data/Icons/import_icon_new_cropped_resized.png"));
    QPixmap viewPix(QString::fromStdString(":/data/Icons/visualize_icon_new_cropped_resized.png"));
    QPixmap resultPix(QString::fromStdString(":/data/Icons/statistics_icon_new_cropped_resized.png"));
    QPixmap savePix(QString::fromStdString(":/data/Icons/export_icon_new_cropped_resized.png"));

    QPainter painter(&savePix);
    QFont font = painter.font();
    font.setPixelSize(4);
    //font.setBold(true);
    font.setFamily("Arial");
    painter.setFont(font);
    painter.setPen(*(new QColor(Qt::black)));
    //painter.drawText(QPoint(0, 500), "Read WSI");

    tb->addWidget(spacerWidgetLeft);

    auto actionGroup = new QActionGroup(tb);

    auto file_action = new QAction("Import", actionGroup);
    file_action->setIcon(QIcon(openPix));
    file_action->setCheckable(true);
    file_action->setChecked(true);
    tb->addAction(file_action);
    mapper->setMapping(file_action, 0);
    auto test2 = mapper->connect(file_action, SIGNAL(triggered(bool)), SLOT(map()));

    auto process_action = new QAction("Process", actionGroup);
    process_action->setIcon(QIcon(processPix));
    process_action->setCheckable(true);
    tb->addAction(process_action);
    mapper->setMapping(process_action, 1);
    mapper->connect(process_action, SIGNAL(triggered(bool)), SLOT(map()));

    auto view_action = new QAction("View", actionGroup);
    view_action->setIcon(QIcon(viewPix));
    view_action->setCheckable(true);
    tb->addAction(view_action);
    mapper->setMapping(view_action, 2);
    mapper->connect(view_action, SIGNAL(triggered(bool)), SLOT(map()));

    auto stats_action = new QAction("Stats", actionGroup);
    stats_action->setIcon(QIcon(resultPix));
    stats_action->setCheckable(true);
    tb->addAction(stats_action);
    mapper->setMapping(stats_action, 3);
    mapper->connect(stats_action, SIGNAL(triggered(bool)), SLOT(map()));

    auto save_action = new QAction("Export", actionGroup);
    save_action->setIcon(QIcon(savePix));
    save_action->setCheckable(true);
    tb->addAction(save_action);
    mapper->setMapping(save_action, 4);
    mapper->connect(save_action, SIGNAL(triggered(bool)), SLOT(map()));

    tb->addWidget(spacerWidgetRight);

    //stackedWidget->connect(&mapper, SIGNAL(mapped(int)), SLOT(setCurrentIndex(int)));
    connect(mapper, SIGNAL(mapped(int)), stackedWidget, SLOT(setCurrentIndex(int)));

    auto dockLayout = new QVBoxLayout; //or any other layout type you want
    dockLayout->setMenuBar(tb); // <-- the interesting part

    auto dockContent = new QWidget(mWidget);
    dockContent->setLayout(dockLayout);

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

    menuWidget = new QWidget(mWidget);
    //menuWidget->setFixedWidth(300); //300);  // TODO: This was a good width for my screen, but need it to be adjustable (!)
    //menuWidget->set
    menuWidget->setMaximumWidth(700);
    menuWidget->setMinimumWidth(360);
    //tmpWidget->setStyleSheet("border:1px solid rgb(0, 255, 0); ");
    menuWidget->setLayout(dockLayout);

    // add button on the bottom of widget for toggling clinical/advanced mode
    setModeButton = new QPushButton(mWidget);
    setModeButton->setText("Clinical mode");
    setModeButton->setFixedHeight(50);
    setModeButton->setStyleSheet("color: white; background-color: gray");
    QObject::connect(setModeButton, &QPushButton::clicked, std::bind(&MainWindow::setApplicationMode, this));

    advancedMode = false; // true : advanced mode
    dockLayout->addWidget(setModeButton);
}


void MainWindow::createWSIScrollAreaWidget() {
    //auto scrollAreaDialog = new QDialog();
    //scrollAreaDialog->setGeometry(100, 100, 260, 260);

    // TODO: Need to substitute this with QListWidget or similar as it's quite slow and memory expensive for
    //        larger number of elements
    scrollArea = new QScrollArea(mWidget);  //scrollAreaDialog);
    scrollArea->setAlignment(Qt::AlignTop);
    //scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    scrollArea->setWidgetResizable(true);
    scrollArea->setGeometry(10, 10, 200, 200);

    scrollList = new QListWidget(mWidget);
    //scrollList->setStyleSheet("border: none, padding: 0, background: white, color: none");
    scrollList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    scrollList->setItemAlignment(Qt::AlignTop);
    scrollList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    scrollList->setResizeMode(QListView::Adjust);  // resizable adaptively
    scrollList->setGeometry(10, 10, 200, 200);
	//scrollList->setStyleSheet("*:hover {background:blue;}");  // setting color of widget when hovering
	//scrollList->setStyleSheet("QListWidget:item:selected:active {background: blue;}; QListWidget:item:selected:!active {background: gray};");
	//scrollList->setFocus();
    //scrollList->setViewMode(QListWidget::IconMode);
    //scrollList->setIconSize(QSize(100, 100));
    //scrollList->setFlow(QListView::TopToBottom);
    //scrollList->setResizeMode(QListWidget::Adjust);
    //QObject::connect(scrollList, &QPushButton::clicked, std::bind(&MainWindow::selectFileInProject, this, 1));
    //QObject::connect(scrollList, &QListWidget::itemPressed, std::bind(&MainWindow::selectFileInProject, this, 1));  // this, SLOT(onListMailItemClicked(QListWidgetItem*)));
    //QObject::connect(scrollList,itemClicked(QListWidgetItem*), std::bind(&MainWindow::selectFileInProject, this, 1));
    //connect(ui->listMail, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onListMailItemClicked(QListWidgetItem*)));
    // QListWidget::itemPressed(QListWidgetItem *item)
    //QObject::connect(scrollList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(&MainWindow::selectFileInProject));

	//scrollList->setSelectionMode(QListWidgetItem::NoSelection);
	//QObject::connect(scrollList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(itemDoubleClicked(QListWidgetItem*)));

    scrollArea->setWidget(scrollList);

    scrollWidget = new QWidget(mWidget);
    //scrollArea->setWidget(scrollWidget);

    scrollLayout = new QVBoxLayout;
    scrollWidget->setLayout(scrollLayout);

    //connect(scrollList, SIGNAL(activated(int)), scrollLayout, SLOT(setCurrentIndex(int)));

    //fileLayout->insertWidget(2, scrollArea);  //widget);
    fileLayout->addWidget(scrollArea);

    //scrollAreaDialog->show();
}


void MainWindow::createFileWidget() {

    fileLayout = new QVBoxLayout;
    fileLayout->setAlignment(Qt::AlignTop);

    fileWidget = new QWidget(mWidget);
    fileWidget->setLayout(fileLayout);
    //fileWidget->setFixedWidth(200);

    auto createProjectButton = new QPushButton(mWidget);
    createProjectButton->setText("Create Project");
    createProjectButton->setFixedHeight(50);
    createProjectButton->setStyleSheet("color: white; background-color: blue");
    QObject::connect(createProjectButton, &QPushButton::clicked, std::bind(&MainWindow::createProject, this));

    auto openProjectButton = new QPushButton(mWidget);
    openProjectButton->setText("Open Project");
    openProjectButton->setFixedHeight(50);
    //openProjectButton->setStyleSheet("color: white; background-color: blue");
    QObject::connect(openProjectButton, &QPushButton::clicked, std::bind(&MainWindow::openProject, this));

    auto selectFileButton = new QPushButton(mWidget);
    selectFileButton->setText("Import WSIs");
    //selectFileButton->setFixedWidth(200);
    selectFileButton->setFixedHeight(50);
    //selectFileButton->setStyleSheet("color: white; background-color: blue");
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
    auto opacitySlider = new QSlider(Qt::Horizontal, dynamicViewWidget);
    //opacitySlider->setFixedWidth(200);
    opacitySlider->setMinimum(0);
    opacitySlider->setMaximum(20);
    //opacityTissueSlider->setText("Tissue");
    opacitySlider->setValue(8);
    opacitySlider->setTickInterval(1);
    QObject::connect(opacitySlider, &QSlider::valueChanged, std::bind(&MainWindow::opacityRenderer, this, std::placeholders::_1, someName));

    auto label_tissue = new QLabel(mWidget);
    std::string tmpSomeName = someName;
    tmpSomeName[0] = toupper(tmpSomeName[0]);
    label_tissue->setText(QString::fromStdString(tmpSomeName + ": "));
    label_tissue->setFixedWidth(50);
    auto smallTextBox_tissue = new QHBoxLayout;
    smallTextBox_tissue->addWidget(label_tissue);
    smallTextBox_tissue->addWidget(opacitySlider);
    auto smallTextBoxWidget_tissue = new QWidget(mWidget);
    smallTextBoxWidget_tissue->setFixedHeight(50);
    smallTextBoxWidget_tissue->setLayout(smallTextBox_tissue);
    //smallTextBoxWidget_tissue->setDisabled(visible[1]);

    // make QColorDialog for manually setting color to different classes
    auto colorSetWidget = new QColorDialog;
    colorSetWidget->setOption(QColorDialog::DontUseNativeDialog, true);
    //colorSetWidget->setDisabled(visible[2]);

	auto colorButton = new QPushButton;
	colorButton->setFixedHeight(50);
	colorButton->setText("Set color");
	colorButton->setChecked(true);

	//auto colorSquare = new QLabel;
	//colorSquare->setStyleSheet("color: white; background-color: white");

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

	/*
    auto killInferenceButton = new QPushButton(mWidget);
    killInferenceButton->setText("Kill inference");
    killInferenceButton->setFixedHeight(50);
    QObject::connect(killInferenceButton, &QPushButton::clicked, std::bind(&MainWindow::killInference, this, someName));
    killInferenceButton->setStyleSheet("color: black; background-color: red");
	 */

    auto deleteButton = new QPushButton(mWidget);
    deleteButton->setText("Delete object");
    deleteButton->setFixedHeight(50);
    QObject::connect(deleteButton, &QPushButton::clicked, std::bind(&MainWindow::deleteViewObject, this, someName));
    deleteButton->setStyleSheet("color: black; background-color: red");  // "border: 1px solid red");


    auto currComboBox = new QComboBox;

    // additional options for setting which classes to show and colors
    if (m_rendererTypeList[someName] == "HeatmapRenderer") {
        // get metadata of current model
        std::map<std::string, std::string> metadata = getModelMetadata(modelName);
        std::vector someVector = split(metadata["class_names"], ";");
        // clear vector first
        currentClassesInUse.clear();
        for (const auto & i : someVector){
            currentClassesInUse.append(QString::fromStdString(i));
        }
        currComboBox->clear();
        currComboBox->update();
        currComboBox->insertItems(0, currentClassesInUse);

		QObject::connect(colorButton, &QPushButton::clicked, [=]() {
			auto rgb = colorSetWidget->getColor().toRgb();

			auto someRenderer = std::dynamic_pointer_cast<HeatmapRenderer>(m_rendererList[someName]);
			someRenderer->setChannelColor(currComboBox->currentIndex(), Color((float)(rgb.red() / 255.0f), (float)(rgb.green() / 255.0f), (float)(rgb.blue() / 255.0f)));
		});
	} else if(m_rendererTypeList[someName] == "SegmentationRenderer") {
		// clear vector first
		currentClassesInUse.clear();
		for (const auto & i : { 1 }) { //{ 0, 1 }) {  // @TODO: Supports only binary images (where class of interest = 1)
			currentClassesInUse.append(QString::number(i));
		}
		currComboBox->clear();
		currComboBox->update();
		currComboBox->insertItems(0, currentClassesInUse);

		QObject::connect(colorButton, &QPushButton::clicked, [=]() {
			auto rgb = colorSetWidget->getColor().toRgb();
			auto someRenderer = std::dynamic_pointer_cast<SegmentationRenderer>(m_rendererList[someName]);
			//auto currImage = someRenderer->updateAndGetOutputData<Image>();
			//currImage->
			//std::cout << "window: " << someRenderer->updateAndGetOutputData<Image>() << std::endl;
			//auto vals = someRenderer->getIntensityWindow();

			// @TODO: Supports only binary images (where class of interest = 1)
			someRenderer->setColor(currComboBox->currentIndex() + 1, Color((float)(rgb.red() / 255.0f), (float)(rgb.green() / 255.0f), (float)(rgb.blue() / 255.0f)));
		});
	// @TODO: Need to add proper options in SegmentationPyramidRenderer and SegmnetationRenderer for toggling and setting which classest to show, do to this properly...
	}
	else if (m_rendererTypeList[someName] == "SegmentationPyramidRenderer") {
		// get metadata of current model
		std::map<std::string, std::string> metadata = getModelMetadata(modelName);
		std::vector someVector = split(metadata["class_names"], ";");
		// clear vector first
		currentClassesInUse.clear();
		for (const auto & i : someVector) {
			currentClassesInUse.append(QString::fromStdString(i));
		}
		currComboBox->clear();
		currComboBox->update();
		currComboBox->insertItems(0, currentClassesInUse);

		QObject::connect(colorButton, &QPushButton::clicked, [=]() {
			auto rgb = colorSetWidget->getColor().toRgb();
			auto someRenderer = std::dynamic_pointer_cast<SegmentationPyramidRenderer>(m_rendererList[someName]);
			//someRenderer->setChannelColor(currComboBox->currentIndex(), Color((float)(rgb.red() / 255.0f), (float)(rgb.green() / 255.0f), (float)(rgb.blue() / 255.0f)));
		});
	} else if (m_rendererTypeList[someName] == "BoundingBoxRenderer") {
		// get metadata of current model
		std::map<std::string, std::string> metadata = getModelMetadata(modelName);
		std::vector someVector = split(metadata["class_names"], ";");
		// clear vector first
		currentClassesInUse.clear();
		for (const auto & i : someVector) {
			currentClassesInUse.append(QString::fromStdString(i));
		}
		currComboBox->clear();
		currComboBox->update();
		currComboBox->insertItems(0, currentClassesInUse);

		QObject::connect(colorButton, &QPushButton::clicked, [=]() {
			auto rgb = colorSetWidget->getColor().toRgb();
			auto someRenderer = std::dynamic_pointer_cast<BoundingBoxRenderer>(m_rendererList[someName]);
			someRenderer->setLabelColor(currComboBox->currentIndex(), Color((float)(rgb.red() / 255.0f), (float)(rgb.green() / 255.0f), (float)(rgb.blue() / 255.0f)));
		});
	} else {
		std::cout << "Invalid renderer used..." << std::endl;
	}

    connect(currComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateChannelValue(int)));

	//auto toggleCheckBox = new QCheckBox(mWidget);
	//toggleCheckBox->setChecked(true);

    auto imageNameTexts = new QLabel();
    imageNameTexts->setText("Class: ");
    imageNameTexts->setFixedWidth(50);
    auto smallTextBox_imageName = new QHBoxLayout;
    smallTextBox_imageName->addWidget(imageNameTexts);
    smallTextBox_imageName->addWidget(currComboBox);
	//smallTextBox_imageName->addWidget(toggleCheckBox);

    auto smallTextBoxWidget_imageName = new QWidget;
    smallTextBoxWidget_imageName->setFixedHeight(50);
    smallTextBoxWidget_imageName->setLayout(smallTextBox_imageName);
    auto biggerTextBox_imageName = new QVBoxLayout;
    biggerTextBox_imageName->addWidget(smallTextBoxWidget_imageName);
    //biggerTextBox_imageName->addWidget(toggleShowButton);
    biggerTextBox_imageName->setAlignment(Qt::AlignTop);

	auto smallToggleColorBox_layout = new QHBoxLayout;
	smallToggleColorBox_layout->addWidget(toggleShowButton);
	smallToggleColorBox_layout->addWidget(colorButton);
	auto smallToggleColorBox = new QWidget;
	smallToggleColorBox->setLayout(smallToggleColorBox_layout);
    auto biggerTextBoxWidget_imageName = new QWidget;
    biggerTextBoxWidget_imageName->setLayout(smallToggleColorBox_layout);

    // disable some features for specific renderer types
    if (m_rendererTypeList[someName] == "ImagePyramidRenderer") {
        opacitySlider->setDisabled(true);
        colorSetWidget->setDisabled(true);
        biggerTextBoxWidget_imageName->setDisabled(true);
	} else if (m_rendererTypeList[someName] == "SegmentationRenderer") {
		toggleShowButton->setDisabled(true);
	} else if (m_rendererTypeList[someName] == "SegmentationPyramidRenderer") {
		toggleShowButton->setDisabled(true);
		colorSetWidget->setDisabled(true);
		biggerTextBoxWidget_imageName->setDisabled(true);
	} else if (m_rendererTypeList[someName] == "BoundingBoxRenderer") {
		1;
	} else {
		colorSetWidget->setDisabled(false);
	}

	auto allBox = new QGroupBox(tr("Modify image"), mWidget);
	auto classBox = new QGroupBox(tr("Modify class"), mWidget);

    dynamicViewLayout = new QVBoxLayout;
    dynamicViewLayout->setAlignment(Qt::AlignTop);
    dynamicViewWidget->setLayout(dynamicViewLayout);
    //dynamicViewLayout->addWidget(imageButton);
    //dynamicViewLayout->addWidget(smallTextBoxWidget_tissue);
	//dynamicViewLayout->addWidget(smallTextBoxWidget_imageName);
    //dynamicViewLayout->addWidget(biggerTextBoxWidget_imageName);
    //dynamicViewLayout->addWidget(deleteButton);

	auto allViewLayout = new QVBoxLayout;
	allViewLayout->addWidget(imageButton);
	allViewLayout->addWidget(smallTextBoxWidget_tissue);

	allBox->setLayout(allViewLayout);

	auto classViewLayout = new QVBoxLayout;
	classViewLayout->addWidget(smallTextBoxWidget_imageName);
	classViewLayout->addWidget(biggerTextBoxWidget_imageName);

	classBox->setLayout(classViewLayout);

	dynamicViewLayout->addWidget(allBox);
	dynamicViewLayout->addWidget(classBox);
	dynamicViewLayout->addWidget(deleteButton);

    // add widget to QComboBox
    pageComboBox->addItem(tr(tmpSomeName.c_str()));
    stackedLayout->addWidget(dynamicViewWidget);
}


void MainWindow::customPipelineEditor() {

    //auto scriptEditorWidgetBackground = new QWidget(mWidget);
    auto backgroundLayout = new QVBoxLayout;

    scriptEditorWidget = new QDialog(mWidget);
    scriptEditorWidget->setWindowTitle("Script Editor");
    backgroundLayout->addWidget(scriptEditorWidget);

    //scriptEditorWidget->setMinimumWidth(800);
    //scriptEditorWidget->setMinimumHeight(1000);  // TODO: Better to use baseSize, but not working?

	//scriptEditorWidget->setBaseSize(800, 1000);

    scriptLayout = new QVBoxLayout;
    scriptEditorWidget->setLayout(scriptLayout);

    scriptEditor = new QPlainTextEdit;
    auto highlighter = new PipelineHighlighter(scriptEditor->document());
    const QFont fixedFont("UbuntuMono");
    scriptEditor->setFont(fixedFont);

	//auto scriptEditor = new PipelineEditor("C:/Users/andrp/fast/pipelines/wsi_patch_segmentation.fpl");
	
	// @TODO: This works, but I don't like the editor, and I just want to be able to start the editor without needing to 
	//auto scriptEditor = new PipelineEditor("C:/Users/andrp/fast/pipelines/wsi_patch_segmentation.fpl");
	//scriptEditor->show();

    scriptLayout->insertWidget(1, scriptEditor);

    // center QDialog when opened
    QRect rect = scriptEditorWidget->geometry();
    QRect parentRect = mWidget->geometry();
    rect.moveTo(mWidget->mapToGlobal(QPoint(parentRect.x() + parentRect.width() - rect.width(), parentRect.y())));

	scriptEditorWidget->resize(600, 800); //resize((int)(0.6 * scriptEditorWidget->width()), (int) (0.8 * scriptEditorWidget->height()));

    createActionsScript();
    scriptEditorWidget->show(); // finally slow editor
}


QColor MainWindow::changeColor() {
	auto colorSetWidget = new QColorDialog(mWidget);
	colorSetWidget->setOption(QColorDialog::DontUseNativeDialog, true);
	QColor color = colorSetWidget->getColor();
	return color;
}


void MainWindow::createActionsScript() {

    auto menuBar = new QMenuBar(scriptEditorWidget); //scriptEditorWidget);
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
                tr("Open File"), nullptr, tr("WSI Files (*.fpl *.txt)"),
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
    std::cout << "We updated the channel value!" << std::endl;
    channel_value = (uint) index;
}


void MainWindow::createProcessWidget() {

    processLayout = new QVBoxLayout;
    processLayout->setSpacing(6); // 50
    processLayout->setAlignment(Qt::AlignTop);

    processWidget = new QWidget(mWidget);  //mWidget);
    processWidget->setLayout(processLayout);
    //processWidget->setFixedWidth(200);

    auto segTissueButton = new QPushButton(mWidget); //(exportWidget);
    segTissueButton->setText("Segment Tissue");
    //segTissueButton->setFixedWidth(200);
    segTissueButton->setFixedHeight(50);
    QObject::connect(segTissueButton, &QPushButton::clicked, std::bind(&MainWindow::segmentTissue, this));

    // add tissue segmentation to widget (should always exist as built-in FAST)
    processLayout->insertWidget(0, segTissueButton);

    // check Models folder which models are available and make button for each model and add to widget
    //QDir directory = QFileDialog::getExistingDirectory(this, tr("select directory"));

    QDir directory(QString::fromStdString(cwd + "data/Models/"));
    QStringList paths = directory.entryList(QStringList() << "*.txt" << "*.TXT",QDir::Files);
    int counter=1;
    foreach(QString currFile, paths) {

        // current model
        modelName = split(currFile.toStdString(), ".")[0];

        // get metadata of current model
        std::map<std::string, std::string> metadata = getModelMetadata(modelName);

        auto someButton = new QPushButton(mWidget);//(exportWidget);
        someButton->setText(QString::fromStdString(metadata["task"]));
        //predGradeButton->setFixedWidth(200);
        someButton->setFixedHeight(50);
        QObject::connect(someButton, &QPushButton::clicked, std::bind(&MainWindow::pixelClassifier, this, modelName));
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


void MainWindow::createStatsWidget() {

    statsLayout = new QVBoxLayout;
    statsLayout->setAlignment(Qt::AlignTop); //|Qt::AlignHCenter);

    statsWidget = new QWidget(mWidget);
    statsWidget->setLayout(statsLayout);
    //statsWidget->setFixedWidth(150);

    // make button that prints distribution of pixels of each class -> for histogram
    auto calcTissueHistButton = new QPushButton(statsWidget);
    calcTissueHistButton->setText("Calculate tissue histogram");
    calcTissueHistButton->setFixedHeight(50);
    QObject::connect(calcTissueHistButton, &QPushButton::clicked, std::bind(&MainWindow::calcTissueHist, this));

    statsLayout->insertWidget(0, calcTissueHistButton);
}


void MainWindow::createExportWidget() {

    exportLayout = new QVBoxLayout;
    exportLayout->setAlignment(Qt::AlignTop);

    exportWidget = new QWidget(mWidget);
    exportWidget->setLayout(exportLayout);

    //auto wsiPageWidget = new QWidget;
    exportStackedLayout = new QStackedLayout;

    auto exportStackedWidget = new QWidget(mWidget);
    exportStackedWidget->setLayout(exportStackedLayout);

    exportComboBox = new QComboBox(mWidget);
    exportComboBox->setFixedWidth(150);
    connect(exportComboBox, SIGNAL(activated(int)), exportStackedLayout, SLOT(setCurrentIndex(int)));

    //exportComboBox->addItem(tr("Thumbnail"));
    //exportComboBox->addItem(tr("Tissue mask"));

    QStringList itemsInComboBox;
    for (int index = 0; index < pageComboBox->count(); index++) {
        std::cout << "some item: " << pageComboBox->itemText(index).toStdString() << std::endl;
        itemsInComboBox << pageComboBox->itemText(index);
    }

    auto saveThumbnailButton = new QPushButton(mWidget);
    saveThumbnailButton->setText("Save thumbnail");
    saveThumbnailButton->setFixedHeight(50);
    //saveThumbnailButton->setStyleSheet("color: white; background-color: blue");
    QObject::connect(saveThumbnailButton, &QPushButton::clicked, std::bind(&MainWindow::saveThumbnail, this));

    auto saveTissueButton = new QPushButton(mWidget);
    saveTissueButton->setText("Save tissue mask");
    saveTissueButton->setFixedHeight(50);
    //saveTissueButton->setStyleSheet("color: white; background-color: blue");
    QObject::connect(saveTissueButton, &QPushButton::clicked, std::bind(&MainWindow::saveTissueSegmentation, this));

    auto saveTumorButton = new QPushButton(mWidget);
    saveTumorButton->setText("Save tumor mask");
    saveTumorButton->setFixedHeight(50);
    //saveTumorButton->setStyleSheet("color: white; background-color: blue");
    QObject::connect(saveTumorButton, &QPushButton::clicked, std::bind(&MainWindow::saveTumor, this));

    auto saveGradeButton = new QPushButton(mWidget);
    saveGradeButton->setText("Save grade heatmap");
    saveGradeButton->setFixedHeight(50);
    //saveGradeButton->setStyleSheet("color: white; background-color: blue");
    //QObject::connect(saveGradeButton, &QPushButton::clicked, std::bind(&MainWindow::saveGrade, this));

    auto imageNameTexts = new QLabel(mWidget);
    imageNameTexts->setText("Results: ");
    imageNameTexts->setFixedWidth(75);
    auto smallTextBox_imageName = new QHBoxLayout;
    smallTextBox_imageName->addWidget(imageNameTexts);
    smallTextBox_imageName->addWidget(exportComboBox);
    auto smallTextBoxWidget_imageName = new QWidget(mWidget);
    smallTextBoxWidget_imageName->setFixedHeight(50);
    smallTextBoxWidget_imageName->setLayout(smallTextBox_imageName);

    //exportLayout->setAlignment(Qt::AlignTop);
    exportLayout->addWidget(smallTextBoxWidget_imageName);
    exportLayout->addWidget(saveThumbnailButton);
    exportLayout->addWidget(saveTissueButton);
    exportLayout->addWidget(saveTumorButton);
    exportLayout->addWidget(saveGradeButton);
}


void MainWindow::createDynamicExportWidget(const std::string& someName) {
    1;
}


void MainWindow::saveThumbnail() {

	std::vector<std::string> currentWSIs;
	if (m_runForProject) {
		currentWSIs = m_runForProjectWsis;
	} else {
		currentWSIs.push_back(wsiList[curr_pos]);
	}

	auto progDialog = QProgressDialog(mWidget);
	progDialog.setRange(0, (int)currentWSIs.size() - 1);
	//progDialog.setContentsMargins(0, 0, 0, 0);
	progDialog.setVisible(true);
	progDialog.setModal(false);
	progDialog.setLabelText("Saving thumbnails...");
	//QRect screenrect = mWidget->screen()[0].geometry();
	progDialog.move(mWidget->width() - progDialog.width() * 1.1, progDialog.height() * 0.1);
	progDialog.show();

	auto counter = 0;
	for (const auto& currWSI : currentWSIs) {

		std::cout << "current WSI: " << currWSI << std::endl;
		auto access = m_image->getAccess(ACCESS_READ);
		auto input = access->getLevelAsImage(m_image->getNrOfLevels() - 1);
		if (m_runForProject) {
			auto importer = WholeSlideImageImporter::New();
			importer->setFilename(currWSI);
			auto currImage = importer->updateAndGetOutputData<ImagePyramid>();

			access = currImage->getAccess(ACCESS_READ);
			input = access->getLevelAsImage(currImage->getNrOfLevels() - 1);
		}

		/*
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
					data = ((uchar *)inputData)[i * nrOfComponents + c]; // assumes TYPE_UINT8
					//pixelData[i * 4 + (2-c)] = (unsigned char) data;  // TODO: NOTE (2-c) fixed BGR->RGB, but maybe there is a smarter solution?
					pixelData[i * 4 + c] = (unsigned char)data;  // TODO: NOTE (2-c) fixed BGR->RGB, but maybe there is a smarter solution?
					pixelData[i * 4 + 3] = 255; // Alpha
				}
			}
		}
		 */

		auto intensityScaler = ScaleImage::New();
		intensityScaler->setInputData(input); //m_tissue);  // expects Image data type
		intensityScaler->setLowestValue(0.0f);
		intensityScaler->setHighestValue(1.0f);
		//intensityScaler->update();

		// attempt to save thumbnail to disk as .png
		ImageExporter::pointer exporter = ImageExporter::New();
		exporter->setFilename(projectFolderName.toStdString() + "/thumbnails/" + split(split(currWSI, "/").back(), ".")[0] + ".png");
		std::cout << "Name: " << projectFolderName.toStdString() + "/thumbnails/" + split(split(currWSI, "/").back(), ".")[0] + ".png" << std::endl;
		exporter->setInputData(intensityScaler->updateAndGetOutputData<Image>());
		exporter->update();

		/*
		auto mBox = new QMessageBox(mWidget);
		mBox->setText("Thumbnail has been saved.");
		mBox->setIcon(QMessageBox::Information);
		mBox->setModal(false);
		//mBox->show();
		QRect screenrect = mWidget->screen()[0].geometry();
		mBox->move(mWidget->width() - mBox->width() / 2, -mWidget->width() / 2 - mBox->width() / 2);
		mBox->show(); // Don't ask why I do multiple show()s here. I just do, and it works
		QTimer::singleShot(3000, mBox, SLOT(accept()));
		 */

		// update progress bar
		progDialog.setValue(counter);
		counter++;

		// to render straight away (avoid waiting on all WSIs to be handled before rendering)
		QCoreApplication::processEvents(QEventLoop::AllEvents, 0);
	}
}


void MainWindow::saveTissueSegmentation() {

	std::vector<std::string> currentWSIs;
	if (m_runForProject) {
		currentWSIs = m_runForProjectWsis;
	} else {
		currentWSIs.push_back(wsiList[curr_pos]);
	}

	auto progDialog = QProgressDialog(mWidget);
	progDialog.setRange(0, (int)currentWSIs.size() - 1);
	//progDialog.setContentsMargins(0, 0, 0, 0);
	progDialog.setVisible(true);
	progDialog.setModal(false);
	progDialog.setLabelText("Saving tissue segmentations...");
	//QRect screenrect = mWidget->screen()[0].geometry();
	progDialog.move(mWidget->width() - progDialog.width() * 1.1, progDialog.height() * 0.1);
	progDialog.show();

	auto counter = 0;
	for (const auto& currWSI : currentWSIs) {

		// check if folder for current WSI exists, if not, create one
		QString wsiResultPath = (projectFolderName.toStdString() + "/results/" + split(split(currWSI, "/").back(), ".")[0] + "/").c_str();
		wsiResultPath = wsiResultPath.replace("//", "/");
		if (!QDir(wsiResultPath).exists()) {
			QDir().mkdir(wsiResultPath);
		}

		/*
		if (std::find(savedList.begin(), savedList.end(), "tissue") != savedList.end()) {
			auto mBox = new QMessageBox(mWidget);
			mBox->setText("Result has already previously been saved.");
			mBox->setIcon(QMessageBox::Information);
			mBox->setModal(false);
			//mBox->show();
			QRect screenrect = mWidget->screen()[0].geometry();
			mBox->move(mWidget->width() - mBox->width() / 2, -mWidget->width() / 2 - mBox->width() / 2);
			mBox->show(); // Don't ask why I do multiple show()s here. I just do, and it works
			QTimer::singleShot(3000, mBox, SLOT(accept()));
			return;
		}
		savedList.emplace_back("tissue");
		 */

		if (m_runForProject) {
			auto importer = WholeSlideImageImporter::New();
			importer->setFilename(currWSI);
			auto currImage = importer->updateAndGetOutputData<ImagePyramid>();

			auto tissueSegmentation = TissueSegmentation::New();
			tissueSegmentation->setInputData(currImage);
			m_tissue = tissueSegmentation->updateAndGetOutputData<Image>();
		}

		auto intensityScaler = ScaleImage::New();
		intensityScaler->setInputData(m_tissue);
		intensityScaler->setLowestValue(0.0f);
		intensityScaler->setHighestValue(1.0f);

		QString outFile = (wsiResultPath.toStdString() + split(split(currWSI, "/").back(), ".")[0] + "_tissue_mask.png").c_str();
		ImageExporter::pointer exporter = ImageExporter::New();
		exporter->setFilename(outFile.replace("//", "/").toStdString());
		exporter->setInputData(intensityScaler->updateAndGetOutputData<Image>());
		exporter->update();

		/*
		auto mBox = new QMessageBox(mWidget);
		mBox->setText("Tissue segmentation has been saved.");
		mBox->setIcon(QMessageBox::Information);
		mBox->setModal(false);
		//mBox->show();
		QRect screenrect = mWidget->screen()[0].geometry();
		mBox->move(mWidget->width() - mBox->width() / 2, -mWidget->width() / 2 - mBox->width() / 2);
		mBox->show(); // Don't ask why I do multiple show()s here. I just do, and it works
		QTimer::singleShot(3000, mBox, SLOT(accept()));
		 */

		 // update progress bar
		progDialog.setValue(counter);
		counter++;

		// to render straight away (avoid waiting on all WSIs to be handled before rendering)
		QCoreApplication::processEvents(QEventLoop::AllEvents, 0);
	}
}


void MainWindow::displayMessage(QString message) {
    auto mBox = new QMessageBox(mWidget);
    mBox->setText(message);
    mBox->setIcon(QMessageBox::Information);
    mBox->setModal(false);
    //mBox->show();
    QRect screenrect = mWidget->screen()[0].geometry();
    mBox->move(mWidget->width() - mBox->width() / 2, - mWidget->width() / 2 - mBox->width() / 2);
    mBox->show(); // Don't ask why I do multiple show()s here. I just do, and it works
    QTimer::singleShot(3000, mBox, SLOT(accept()));
}


void MainWindow::saveResults(std::string result) {
    // check if folder for current WSI exists, if not, create one
    QString wsiResultPath = (projectFolderName.toStdString() + "/results/" + split(split(filename, "/").back(), ".")[0] + "/").c_str();
    wsiResultPath = wsiResultPath.replace("//", "/");
    if (!QDir(wsiResultPath).exists()) {
        QDir().mkdir(wsiResultPath);
    }

    if (std::find(savedList.begin(), savedList.end(), result) != savedList.end()) {
        displayMessage("Result has already previously been saved.");
        return;
    }
    savedList.emplace_back(result);

    // attempt to save tissue mask to disk as .pngfileN
    QString outFile = (wsiResultPath.toStdString() + split(split(filename, "/").back(), ".")[0] + "_tumor_mask.png").c_str();
    ImageExporter::pointer exporter = ImageExporter::New();
    exporter->setFilename(outFile.replace("//", "/").toStdString());
    //exporter->setInputData(intensityScaler->updateAndGetOutputData<Image>());
    exporter->setInputData(m_tumorMap);
    exporter->update();
}


void MainWindow::saveTumor() {
    // check if folder for current WSI exists, if not, create one
    QString wsiResultPath = (projectFolderName.toStdString() + "/results/" + split(split(filename, "/").back(), ".")[0] + "/").c_str();
    wsiResultPath = wsiResultPath.replace("//", "/");
    if (!QDir(wsiResultPath).exists()) {
        QDir().mkdir(wsiResultPath);
    }

    if (std::find(savedList.begin(), savedList.end(), "tumorSeg_lr") != savedList.end()) {
        auto mBox = new QMessageBox(mWidget);
        mBox->setText("Result has already previously been saved.");
        mBox->setIcon(QMessageBox::Information);
        mBox->setModal(false);
        //mBox->show();
        QRect screenrect = mWidget->screen()[0].geometry();
        mBox->move(mWidget->width() - mBox->width() / 2, - mWidget->width() / 2 - mBox->width() / 2);
        mBox->show(); // Don't ask why I do multiple show()s here. I just do, and it works
        QTimer::singleShot(3000, mBox, SLOT(accept()));
        return;
    }
    savedList.emplace_back("tumorSeg_lr");

    // attempt to save tissue mask to disk as .png
    QString outFile = (wsiResultPath.toStdString() + split(split(filename, "/").back(), ".")[0] + "_tumor_mask.png").c_str();
    ImageExporter::pointer exporter = ImageExporter::New();
    exporter->setFilename(outFile.replace("//", "/").toStdString());
    //exporter->setInputData(intensityScaler->updateAndGetOutputData<Image>());
    exporter->setInputData(m_tumorMap);
    exporter->update();

    auto mBox = new QMessageBox(mWidget);
    mBox->setText("Tumor segmentation has been saved.");
    mBox->setIcon(QMessageBox::Information);
    mBox->setModal(false);
    //mBox->show();
    QRect screenrect = mWidget->screen()[0].geometry();
    mBox->move(mWidget->width() - mBox->width() / 2, - mWidget->width() / 2 - mBox->width() / 2);
    mBox->show(); // Don't ask why I do multiple show()s here. I just do, and it works
    QTimer::singleShot(3000, mBox, SLOT(accept()));
}


void MainWindow::saveGrade() {
    // check if folder for current WSI exists, if not, create one
    QString wsiResultPath = (projectFolderName.toStdString() + "/results/" + split(split(filename, "/").back(), ".")[0] + "/").c_str();
    wsiResultPath = wsiResultPath.replace("//", "/");
    if (!QDir(wsiResultPath).exists()) {
        QDir().mkdir(wsiResultPath);
    }

    savedList.emplace_back("grade");

    // attempt to save tissue mask to disk as .png
    QString outFile = (wsiResultPath.toStdString() + split(split(filename, "/").back(), ".")[0] + "_grade_mask.png").c_str();
    ImageExporter::pointer exporter = ImageExporter::New();
    exporter->setFilename(outFile.replace("//", "/").toStdString());
    //exporter->setInputData(intensityScaler->updateAndGetOutputData<Image>());
    exporter->setInputData(m_gradeMap);
    exporter->update();

    auto mBox = new QMessageBox(mWidget);
    mBox->setText("Grade heatmap has been saved.");
    mBox->setIcon(QMessageBox::Information);
    mBox->setModal(false);
    //mBox->show();
    QRect screenrect = mWidget->screen()[0].geometry();
    mBox->move(mWidget->width() - mBox->width() / 2, - mWidget->width() / 2 - mBox->width() / 2);
    mBox->show(); // Don't ask why I do multiple show()s here. I just do, and it works
    QTimer::singleShot(3000, mBox, SLOT(accept()));
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


int MainWindow::mkdir(const char *path)
{
    return QDir().mkdir(path);
}


int MainWindow::rmdir(const char *path)
{
    return QDir(path).removeRecursively();
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
        mBox.setStyleSheet(mWidget->styleSheet());
        mBox.setText("There are unsaved results.");
        mBox.setInformativeText("Do you wish to save them?");
        mBox.setDefaultButton(QMessageBox::Save);
        mBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        int ret = mBox.exec();

        switch (ret) {
            case QMessageBox::Save:
                std::cout << "Results not saved yet. Just cancelled the switch!" << std::endl;
                // Save was clicked
                return;
            case QMessageBox::Discard:
                // Don't Save was clicked
				std::cout << "Discarded!" << std::endl;
                break;
            case QMessageBox::Cancel:
                // Cancel was clicked
                std::cout << "Cancelled!"  << std::endl;
                return;
            default:
                // should never be reached
                break;
        }
    }
    /*
    if (pageComboBox->count() != 0) { // if not empty, clear
        pageComboBox->clear();
        exportComboBox->clear();
    }
     */

    pageComboBox->clear();
    exportComboBox->clear();

    // TODO: Unable to read .zvi and .scn (Zeiss and Leica). I'm wondering if they are stored in some unexpected way (not image pyramids)
    auto fileNames = QFileDialog::getOpenFileNames(
        mWidget,
        tr("Select File(s)"), nullptr, tr("WSI Files (*.tiff *.tif *.svs *.ndpi *.bif *vms)"),  //*.zvi *.scn)"),
        nullptr, QFileDialog::DontUseNativeDialog
    );

    auto progDialog = QProgressDialog(mWidget);
    progDialog.setRange(0, fileNames.count()-1);
	//progDialog.setContentsMargins(0, 0, 0, 0);
    progDialog.setVisible(true);
    progDialog.setModal(false);
    progDialog.setLabelText("Loading WSIs...");
    //QRect screenrect = mWidget->screen()[0].geometry();
	progDialog.move(mWidget->width() - progDialog.width() * 1.1, progDialog.height() * 0.1);
    progDialog.show();

    int counter = 0;
    for (QString& fileName : fileNames) {

        if (fileName == "")
            return;
        //filename = fileName.toStdString();
        auto currFileName = fileName.toStdString();
        std::cout << "Selected file: " << currFileName << std::endl;
        wsiList.push_back(currFileName);

        // Import image from file using the ImageFileImporter
        auto importer = WholeSlideImageImporter::New();
        importer->setFilename(currFileName);
        auto currImage = importer->updateAndGetOutputData<ImagePyramid>();

        // for reading of multiple WSIs, only render last one
        if (counter == 0) { //fileNames.count()-1) {

            // current WSI (global)
            filename = currFileName;

            m_image = currImage;
            std::cout << "count:" << counter << std::endl;
            metadata = m_image->getMetadata(); // get metadata

            /*
            std::cout << "\nPrinting metadata: " << std::endl;
            for (auto elem : metadata) {
                std::cout << elem.first << " " << elem.second << "\n";
            }
             */

            auto renderer = ImagePyramidRenderer::New();
            renderer->setInputData(m_image);

            // TODO: Something here results in me not being able to run analysis on new images (after the first)
            removeAllRenderers();
            m_rendererTypeList["WSI"] = "ImagePyramidRenderer";
            insertRenderer("WSI", renderer);
            getView(0)->reinitialize(); // Must call this after removing all renderers

            wsiFormat = metadata["openslide.vendor"]; // get WSI format
            magn_lvl = getMagnificationLevel(); // get magnification level of current WSI

            // now make it possible to edit image in the View Widget
            createDynamicViewWidget("WSI", modelName);

            // update application name to contain current WSI
            //setTitle(applicationName + " - " + split(filename, "/").back());
            if (advancedMode) {
                setTitle(applicationName + " (Research mode)" + " - " + split(currFileName, "/").back());
            } else {
                setTitle(applicationName + " - " + split(currFileName, "/").back());
            }
        }
        counter ++;

        // @TODO: This is a little bit slow. Possible to speed it up? Bottleneck is probably the creation of thumbnails
        auto access = currImage->getAccess(ACCESS_READ);
        auto input = access->getLevelAsImage(currImage->getNrOfLevels() - 1);

        // try to convert to FAST Image -> QImage
        QImage image(input->getWidth(), input->getHeight(), QImage::Format_RGB32);

        // TODO have to do some type conversion here, assuming float for now
        unsigned char *pixelData = image.bits();

        ImageAccess::pointer new_access = input->getImageAccess(ACCESS_READ);
        void *inputData = new_access->get();
        uint nrOfComponents = input->getNrOfChannels();

        for (uint x = 0; x < (uint)input->getWidth(); x++) {
            for (uint y = 0; y < (uint)input->getHeight(); y++) {
                uint i = x + y * input->getWidth();
                for (uint c = 0; c < (uint)input->getNrOfChannels(); c++) {
                    float data;
                    data = ((uchar *) inputData)[i * nrOfComponents + c]; // assumes TYPE_UINT8
                    pixelData[i * 4 + (2-c)] = (unsigned char) data;  // TODO: NOTE (2-c) fixed BGR->RGB, but maybe there is a smarter solution?
                    pixelData[i * 4 + 3] = 255; // Alpha
                }
            }
        }

        auto button = new QPushButton(mWidget);
        auto m_NewPixMap = QPixmap::fromImage(image);
        QIcon ButtonIcon(m_NewPixMap);
        button->setIcon(ButtonIcon);
        int width_val = 100;
        int height_val = 150;
        button->setIconSize(QSize((int) std::round(0.9 * (float) image.width() * (float) height_val / (float) image.height()), (int) std::round(0.9f * (float) height_val)));
		button->setToolTip(QString::fromStdString(split(currFileName, "/").back()));

        auto listItem = new QListWidgetItem;
        listItem->setSizeHint(QSize(width_val, height_val));
        QObject::connect(button, &QPushButton::clicked, std::bind(&MainWindow::selectFileInProject, this, curr_pos));
        scrollList->addItem(listItem);
        scrollList->setItemWidget(listItem, button);

        curr_pos++; // this should change if we render the first WSI when loading

        // TODO: Importing multiple WSIs, results in QMessageBox flickering... (2speedy)
        /*
        auto mBox = new QMessageBox(mWidget);
        std::string path = "Finished reading: " + split(fileName.toStdString(), "/").back();
        mBox->setText(path.c_str());
        mBox->close();
        mBox->setIcon(QMessageBox::Information);
        mBox->setModal(false);
        QRect screenrect = mWidget->screen()[0].geometry();
        mBox->move(mWidget->width() - mBox->width() / 2, - mWidget->width() / 2 - mBox->width() / 2);
        mBox->show();
        QTimer::singleShot(3000, mBox, SLOT(accept()));
         */

        // update progress bar
        progDialog.setValue(counter);

        // to render straight away (avoid waiting on all WSIs to be handled before rendering)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 0);
    }

    // in the end, report when all WSIs are loaded in
}


void MainWindow::selectFileDrag(const QList<QString> &fileNames) {

    // check if view object list is empty, if not, prompt to save results or not, if not clear
    if (pageComboBox->count() > 1) {
        // prompt
        QMessageBox mBox;
        mBox.setIcon(QMessageBox::Warning);
        mBox.setStyleSheet(mWidget->styleSheet());
        mBox.setText("There are unsaved results.");
        mBox.setInformativeText("Do you wish to save them?");
        mBox.setDefaultButton(QMessageBox::Save);
        mBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        int ret = mBox.exec();

        switch (ret) {
            case QMessageBox::Save:
                std::cout << "\n Results not saved yet. Just cancelled the switch! \n";
                // Save was clicked
                return;
            case QMessageBox::Discard:
                // Don't Save was clicked
                std::cout << "\n Discarded! \n";
                break;
            case QMessageBox::Cancel:
                // Cancel was clicked
                std::cout << "\n Cancelled! \n";
                return;
            default:
                // should never be reached
                break;
        }
    }
    /*
    if (pageComboBox->count() != 0) { // if not empty, clear
        pageComboBox->clear();
        exportComboBox->clear();
    }
     */

    pageComboBox->clear();
    exportComboBox->clear();

    auto progDialog = QProgressDialog(mWidget);
    progDialog.setRange(0, fileNames.count()-1);
    progDialog.setVisible(true);
    progDialog.setModal(false);
    progDialog.setLabelText("Loading WSIs...");
    QRect screenrect = mWidget->screen()[0].geometry();
    progDialog.move(mWidget->width() - progDialog.width() / 2, - mWidget->width() / 2 - progDialog.width() / 2);
    progDialog.show();

    int counter = 0;
    for (const QString& fileName : fileNames) {

        if (fileName == "")
            return;
        //filename = fileName.toStdString();
        auto currFileName = fileName.toStdString();
        std::cout << "\nSelected file: " << currFileName << "\n";
        wsiList.push_back(currFileName);

        // Import image from file using the ImageFileImporter
        auto importer = WholeSlideImageImporter::New();
        importer->setFilename(currFileName);
        auto currImage = importer->updateAndGetOutputData<ImagePyramid>();

        // for reading of multiple WSIs, only render last one
        if (counter == 0) { //fileNames.count()-1) {

            // current WSI (global)
            filename = currFileName;

            m_image = currImage;
            std::cout << "\n count:" << counter;
            metadata = m_image->getMetadata(); // get metadata

            /*
            std::cout << "\nPrinting metadata: " << std::endl;
            for (auto elem : metadata) {
                std::cout << elem.first << " " << elem.second << "\n";
            }
             */

            auto renderer = ImagePyramidRenderer::New();
            renderer->setInputData(m_image);

            // TODO: Something here results in me not being able to run analysis on new images (after the first)
            removeAllRenderers();
            m_rendererTypeList["WSI"] = "ImagePyramidRenderer";
            insertRenderer("WSI", renderer);
            getView(0)->reinitialize(); // Must call this after removing all renderers

            wsiFormat = metadata["openslide.vendor"]; // get WSI format
            magn_lvl = getMagnificationLevel(); // get magnification level of current WSI

            // now make it possible to edit image in the View Widget
            createDynamicViewWidget("WSI", modelName);

            // update application name to contain current WSI
            //setTitle(applicationName + " - " + split(filename, "/").back());
            if (advancedMode) {
                setTitle(applicationName + " (Research mode)" + " - " + split(currFileName, "/").back());
            } else {
                setTitle(applicationName + " - " + split(currFileName, "/").back());
            }
        }
        counter ++;

        // @TODO: This is a little bit slow. Possible to speed it up? Bottleneck is probably the creation of thumbnails
        auto access = currImage->getAccess(ACCESS_READ);
        auto input = access->getLevelAsImage(currImage->getNrOfLevels() - 1);

        // try to convert to FAST Image -> QImage
        QImage image(input->getWidth(), input->getHeight(), QImage::Format_RGB32);

        // TODO have to do some type conversion here, assuming float for now
        unsigned char *pixelData = image.bits();

        ImageAccess::pointer new_access = input->getImageAccess(ACCESS_READ);
        void *inputData = new_access->get();
        uint nrOfComponents = input->getNrOfChannels();

        for (uint x = 0; x < (uint)input->getWidth(); x++) {
            for (uint y = 0; y < (uint)input->getHeight(); y++) {
                uint i = x + y * input->getWidth();
                for (uint c = 0; c < (uint)input->getNrOfChannels(); c++) {
                    float data;
                    data = ((uchar *) inputData)[i * nrOfComponents + c]; // assumes TYPE_UINT8
                    pixelData[i * 4 + (2-c)] = (unsigned char) data;  // TODO: NOTE (2-c) fixed BGR->RGB, but maybe there is a smarter solution?
                    pixelData[i * 4 + 3] = 255; // Alpha
                }
            }
        }

        auto button = new QPushButton(mWidget);
        auto m_NewPixMap = QPixmap::fromImage(image);
        QIcon ButtonIcon(m_NewPixMap);
        button->setIcon(ButtonIcon);
        int width_val = 100;
        int height_val = 150;
        button->setIconSize(QSize((int) std::round(0.9 * (float) image.width() * (float) height_val / (float) image.height()), (int) std::round(0.9f * (float) height_val)));

        auto listItem = new QListWidgetItem;
        listItem->setSizeHint(QSize(width_val, height_val));
        QObject::connect(button, &QPushButton::clicked, std::bind(&MainWindow::selectFileInProject, this, curr_pos));
        scrollList->addItem(listItem);
        scrollList->setItemWidget(listItem, button);

        curr_pos++; // this should change if we render the first WSI when loading

        // TODO: Importing multiple WSIs, results in QMessageBox flickering... (2speedy)
        /*
        auto mBox = new QMessageBox(mWidget);
        std::string path = "Finished reading: " + split(fileName.toStdString(), "/").back();
        mBox->setText(path.c_str());
        mBox->close();
        mBox->setIcon(QMessageBox::Information);
        mBox->setModal(false);
        QRect screenrect = mWidget->screen()[0].geometry();
        mBox->move(mWidget->width() - mBox->width() / 2, - mWidget->width() / 2 - mBox->width() / 2);
        mBox->show();
        QTimer::singleShot(3000, mBox, SLOT(accept()));
         */

        // update progress bar
        progDialog.setValue(counter);

        // to render straight away (avoid waiting on all WSIs to be handled before rendering)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 0);
    }

    // in the end, report when all WSIs are loaded in
}


void MainWindow::selectFileInProject(int pos) {

    // if you select a WSI and it's already open, do nothing
    std::cout << "CurrentPos: " << pos << std::endl;
    std::cout << "Length of wsiList: " << std::to_string(wsiList.size()) << std::endl;
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
    std::cout << "counts: " << (pageComboBox->count() - savedList.size()) << std::endl;
    if ((pageComboBox->count() - savedList.size()) > 1) {
		QMessageBox mBox;
        mBox.setIcon(QMessageBox::Warning);
        mBox.setText("This will remove results on current WSI.");
        mBox.setInformativeText("Do you want to save results?");
        mBox.setDefaultButton(QMessageBox::Yes);
        mBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        int ret = mBox.exec();
        switch (ret) {
            case QMessageBox::Yes:
                std::cout << "\nYes was pressed." << std::endl;
                saveTissueSegmentation(); // TODO: Need to generalize this. Check which results exists, and save all sequentially
                break;
            case QMessageBox::No:
                std::cout << "\nNo was pressed." << std::endl;
                // remove results of current WSI
                removeAllRenderers();
                pageComboBox->clear();
                exportComboBox->clear();
                break;
            case QMessageBox::Cancel:
                std::cout << "\nCancel was pressed." << std::endl;
                return;
            default:
                std::cout << "\nDefault was pressed." << std::endl;
                break;
        }
    }

    // remove results of current WSI
    savedList.clear();
    removeAllRenderers();
    pageComboBox->clear();
    exportComboBox->clear();
    m_rendererList.clear();
    m_rendererTypeList.clear();
	clearLayout(stackedLayout);


    // kill all NN pipelines and clear holders
    for (auto const& keyValue : m_neuralNetworkList) {
        auto neuralNetwork = keyValue.second;
        neuralNetwork->stopPipeline();
    }
    m_neuralNetworkList.clear(); // finally clear map

    // add WSI to project list
    filename = wsiList[pos];

    //stopComputationThread();
    // Import image from file using the ImageFileImporter
    importer = WholeSlideImageImporter::New();
    std::cout << "\nCurrent filename: " << filename << std::endl;
    importer->setFilename(filename);
    m_image = importer->updateAndGetOutputData<ImagePyramid>();

    // get metadata
    metadata = m_image->getMetadata();

    auto renderer = ImagePyramidRenderer::New();
    renderer->setInputData(m_image);

    // TODO: Something here results in me not being able to run analyzis on new images (after the first)
    removeAllRenderers();
    m_rendererTypeList["WSI"] = "ImagePyramidRenderer";
    insertRenderer("WSI", renderer);
    getView(0)->reinitialize(); // Must call this after removing all renderers

    //startComputationThread();

    // get WSI format
    wsiFormat = metadata["openslide.vendor"];

    // get magnification level of current WSI
    magn_lvl = getMagnificationLevel();

    // now make it possible to edit image in the View Widget
    createDynamicViewWidget("WSI", modelName);

    // check if tissue segmentation already exists, if yes import it and add to renderer
    std::string wsiPath = split(split(filename, "/").back(), ".")[0];
    QString tissuePath = projectFolderName + "/results/" + wsiPath.c_str() + "/" + wsiPath.c_str() + "_tissue_mask.png";
    tissuePath = tissuePath.replace("//", "/");  // FIXME: I doubt that this fix works for windows(?) / and \ used
    //tissuePath = "/home/andrep/test2.png";
    std::cout << "\nCurrent path: " << tissuePath.toStdString() << "\n";
    if (QDir().exists(tissuePath)) {
        loadTissue(tissuePath);
        std::cout << "len of saved list: " << savedList.size() << "\n";
    }

    // check if tumor segmentation already exists, if yes import it and add to renderer
    QString tumorPath = projectFolderName + "/results/" + wsiPath.c_str() + "/" + wsiPath.c_str() + "_tumor_mask.png";
    tumorPath = tumorPath.replace("//", "/");  // FIXME: I doubt that this fix works for windows(?) / and \ used
    //tissuePath = "/home/andrep/test2.png";
    std::cout << "\nCurrent path: " << tumorPath.toStdString() << "\n";
    if (QDir().exists(tumorPath)) {
        loadTumor(tumorPath);
        std::cout << "len of saved list: " << savedList.size() << "\n";
    }

    // update application name to contain current WSI
    //setTitle(applicationName + " - " + split(filename, "/").back());
    if (advancedMode) {
        setTitle(applicationName + " (Research mode)" + " - " + split(filename, "/").back());
    } else {
        setTitle(applicationName + " - " + split(filename, "/").back());
    }
}


void MainWindow::createProject() {

    // start by selecting where to create folder and give project name
    // should also handle if folder name already exist, prompt warning and option to change name
    QFileDialog dialog(mWidget);
    dialog.setFileMode(QFileDialog::AnyFile);

    projectFolderName = dialog.getExistingDirectory(
            mWidget, tr("Set Project Directory"),
            QCoreApplication::applicationDirPath(), QFileDialog::DontUseNativeDialog);

    std::cout << "Project dir: " << projectFolderName.toStdString() << std::endl;

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

    if (pageComboBox->count() > 0) {
        // prompt
        QMessageBox mBox;
        mBox.setIcon(QMessageBox::Warning);
        mBox.setText("Opening project will erase current edits.");
        mBox.setInformativeText("Do you still wish to open?");
        mBox.setDefaultButton(QMessageBox::No);
        mBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        int ret = mBox.exec();

        switch (ret) {
            case QMessageBox::Yes:
                wsiList.clear();
                savedList.clear();
                scrollList->clear();
                removeAllRenderers();
                pageComboBox->clear();
                exportComboBox->clear();
                curr_pos=0;
                break;
            case QMessageBox::No:
                break;
            default:
                break;
        }
    }

    curr_pos = 0; // reset

    // select project file
    QFileDialog dialog(mWidget);
    dialog.setFileMode(QFileDialog::ExistingFile);
    QString projectPath = dialog.getOpenFileName(
            mWidget,
            tr("Select Project File"), nullptr,
            tr("Project (*project.txt)"),
            nullptr, QFileDialog::DontUseNativeDialog
    );

    std::cout << projectPath.toStdString() << std::endl;
    projectFolderName = split(projectPath.toStdString(), "project.txt")[0].c_str();
    std::cout << projectFolderName.toStdString() << std::endl;

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
            m_rendererTypeList["WSI"] = "ImagePyramidRenderer";
            insertRenderer("WSI", renderer);
            getView(0)->reinitialize(); // Must call this after removing all renderers

            // get WSI format
            wsiFormat = metadata["openslide.vendor"];

            // get magnification level of current WSI
            magn_lvl = getMagnificationLevel();

            // now make it possible to edit image in the View Widget
            createDynamicViewWidget("WSI", modelName);

            // check if tissue segmentation already exists, if yes import it and add to renderer
            std::string wsiPath = split(split(filename, "/").back(), ".")[0];
            QString tissuePath = projectFolderName + "/results/" + wsiPath.c_str() + "/" + wsiPath.c_str() + "_tissue_mask.png";
            tissuePath = tissuePath.replace("//", "/");  // FIXME: I doubt that this fix works for windows(?) / and \ used
            //tissuePath = "/home/andrep/test2.png";
            std::cout << "\nCurrent path: " << tissuePath.toStdString() << "\n";
            if (QDir().exists(tissuePath)) {
                loadTissue(tissuePath);
            }

            // check if tumor segmentation already exists, if yes import it and add to renderer
            QString tumorPath = projectFolderName + "/results/" + wsiPath.c_str() + "/" + wsiPath.c_str() + "_tumor_mask.png";
            tumorPath = tumorPath.replace("//", "/");  // FIXME: I doubt that this fix works for windows(?) / and \ used
            //tissuePath = "/home/andrep/test2.png";
            std::cout << "\nCurrent path: " << tumorPath.toStdString() << "\n";
            if (QDir().exists(tumorPath)) {
                loadTumor(tumorPath);
            }
        }
        counter++;

        // check if thumbnail image exists for current WSI, if yes load it, else extract one
        std::string wsiPath = split(split(filename, "/").back(), ".")[0];
        QString fullPath = projectFolderName + "/thumbnails/" + wsiPath.c_str();
        fullPath = fullPath.replace("//", "/") + ".png";

        // try to convert FAST Image -> QImage
        QImage image;
        std::cout << "Path: " << fullPath.toStdString() << "\n";
        if (QDir().exists(fullPath)) {
            std::cout << "\n Thumbnail exists! Load it project folder \n";
            image.load(fullPath);
        } else {
            std::cout << "\n Thumbnail does not exist! Creating one from the WSI \n";
            // get thumbnail image
            image = extractThumbnail();
        }

        // /*
        auto button = new QPushButton();
        //QPixmap pixmap("/home/andrep/workspace/FAST-Pathology/ImageExporterTest.png");
        auto m_NewPixMap = QPixmap::fromImage(image);
        QIcon ButtonIcon(m_NewPixMap); //pixmap);
        button->setIcon(ButtonIcon);
        //button->setCheckable(true);
        button->setAutoDefault(true);
        //button->setStyleSheet("border: none, padding: 0, background: none");
        //button->setIconSize(QSize(200, 200));
        button->setToolTip(QString::fromStdString(wsiPath));
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
        listItem->setToolTip(QString::fromStdString(wsiPath));
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
        //mBox->show(); // TODO: Don't ask why I do multiple show()s here. I just do, and it works. -> However, seems like I don't need it anymore. So very mysterious...
        QRect screenrect = mWidget->screen()[0].geometry();
        mBox->move(mWidget->width() - mBox->width() / 2, -mWidget->width() / 2 - mBox->width() / 2);
        mBox->show();
        QTimer::singleShot(3000, mBox, SLOT(accept()));
    }
}


QImage MainWindow::extractThumbnail() {
        auto access = m_image->getAccess(ACCESS_READ);
        auto input = access->getLevelAsImage(m_image->getNrOfLevels() - 1);

        // try to convert FAST Image -> QImage
        QImage image(input->getWidth(), input->getHeight(), QImage::Format_RGB32);

        // TODO have to do some type conversion here, assuming float for now
        unsigned char *pixelData = image.bits();

        ImageAccess::pointer new_access = input->getImageAccess(ACCESS_READ);
        void *inputData = new_access->get();
        uint nrOfComponents = input->getNrOfChannels();

        for (uint x = 0; x < (uint)input->getWidth(); x++) {
            for (uint y = 0; y < (uint)input->getHeight(); y++) {
                uint i = x + y * input->getWidth();
                for (uint c = 0; c < (uint)input->getNrOfChannels(); c++) {
                    float data;
                    data = ((uchar *) inputData)[i * nrOfComponents + c]; // assumes TYPE_UINT8
                    //pixelData[i * 4 + (2-c)] = (unsigned char) data;  // TODO: NOTE (2-c) fixed BGR->RGB, but maybe there is a smarter solution?
					pixelData[i * 4 + c] = (unsigned char)data;  // TODO: NOTE (2-c) fixed BGR->RGB, but maybe there is a smarter solution?
                }
                pixelData[i * 4 + 3] = 255; // Alpha
            }
        }
        return image;
};


void MainWindow::saveProject() {
    // create file for saving which WSIs exist in folder
    QString projectFileName = "/project.txt";
    QFile file(projectFolderName + projectFileName);
    file.resize(0);  // clear it and then write

    if (file.open(QIODevice::ReadWrite)) {
        foreach(std::string currPath, wsiList) {
            QTextStream stream(&file);
            stream << currPath.c_str() << endl;
        }
    }
}


void MainWindow::runForProject() {

	auto projectLayout = new QVBoxLayout;
	auto projectDialog = new QDialog(mWidget);
	projectDialog->setWindowTitle("Run for project");

	auto applyButton = new QPushButton(projectDialog);
	applyButton->setText("Apply");
	applyButton->setFixedWidth(100);
	applyButton->setFixedHeight(35);
	applyButton->setStyleSheet("color: white; background-color: blue");

	auto cancelButton = new QPushButton(projectDialog);
	cancelButton->setText("Cancel");
	cancelButton->setFixedWidth(100);
	cancelButton->setStyleSheet("color: white; background-color: red");
	cancelButton->setFixedHeight(35);

	auto enableToggle = new QPushButton(projectDialog);
	//enableToggle->setCheckable(true);
	if (m_runForProject) {
		enableToggle->setText("Enabled");
		//enableToggle->setChecked(true);
	} else {
		enableToggle->setText("Disabled");
		//enableToggle->setChecked(false);
	}
	enableToggle->setFixedWidth(100);
	enableToggle->setStyleSheet("color: white; background-color: gray");
	enableToggle->setFixedHeight(35);

	auto topButtonsLayout = new QHBoxLayout;
	topButtonsLayout->setAlignment(Qt::AlignLeft);
	topButtonsLayout->addWidget(applyButton);
	topButtonsLayout->addWidget(cancelButton);
	topButtonsLayout->addWidget(enableToggle);

	auto topButtonsWidget = new QWidget(projectDialog);
	topButtonsWidget->setLayout(topButtonsLayout);

	// start dialog asking which WSIs to use which exists in the current Project
	projectDialog->setLayout(projectLayout);

	projectLayout->addWidget(topButtonsWidget);

	auto wsiDialogLayout = new QHBoxLayout;
	auto wsiDialog = new QWidget(projectDialog);
	projectLayout->addWidget(wsiDialog);
	wsiDialog->setLayout(wsiDialogLayout);

	auto allFilesWidget = new QListWidget(projectDialog);
	allFilesWidget->setSelectionMode(QAbstractItemView::MultiSelection);
	for (const auto& item : wsiList) {
		allFilesWidget->addItem(QString::fromStdString(item));
	}
	allFilesWidget->setMinimumWidth(allFilesWidget->sizeHintForColumn(0));

	auto buttonsWidget = new QWidget(projectDialog);
	auto buttonsLayout = new QVBoxLayout;
	buttonsWidget->setLayout(buttonsLayout);
	buttonsLayout->setAlignment(Qt::AlignVCenter);

	auto fewRemoveButton = new QPushButton(projectDialog);
	fewRemoveButton->setText(" < ");  // make it bold (style stuff)

	auto allRemoveButton = new QPushButton(projectDialog);
	allRemoveButton->setText(" << ");  // make it bold

	auto fewSelectButton = new QPushButton(projectDialog);
	fewSelectButton->setText(" > ");  // make it bold (style stuff)

	auto allSelectButton = new QPushButton(projectDialog);
	allSelectButton->setText(" >> ");  // make it bold

	buttonsLayout->addWidget(fewSelectButton);
	buttonsLayout->addWidget(allSelectButton);
	buttonsLayout->addWidget(allRemoveButton);
	buttonsLayout->addWidget(fewRemoveButton);

	auto selectedFilesWidget = new QListWidget(projectDialog);
	selectedFilesWidget->setSelectionMode(QAbstractItemView::MultiSelection);
	for (const auto& item : m_runForProjectWsis) {
		selectedFilesWidget->addItem(QString::fromStdString(item));
	}
	selectedFilesWidget->setMinimumWidth(selectedFilesWidget->sizeHintForColumn(0));

	wsiDialogLayout->addWidget(allFilesWidget);
	wsiDialogLayout->addWidget(buttonsWidget);
	wsiDialogLayout->addWidget(selectedFilesWidget);

	// introduce events -> signals and slots, connect and stuff
	QObject::connect(allSelectButton, &QPushButton::clicked, [=]() {
		selectedFilesWidget->clear();
		for (const auto& item : wsiList) {
			selectedFilesWidget->addItem(QString::fromStdString(item));
		}
		selectedFilesWidget->setMinimumWidth(selectedFilesWidget->sizeHintForColumn(0));
	});

	QObject::connect(fewSelectButton, &QPushButton::clicked, [=]() {
		std::cout << "few select button was pressed" << std::endl;
		auto currSelectedItems = allFilesWidget->selectedItems();

		std::vector<std::string> currItemsAlreadySelected;
		for (int row = 0; row < selectedFilesWidget->count(); row++) {
			currItemsAlreadySelected.push_back(selectedFilesWidget->item(row)->text().toStdString());
		}

		for (const auto& currItem : currSelectedItems) {
			std::cout << "current item: " << currItem->text().toStdString() << std::endl;
			if (std::find(currItemsAlreadySelected.begin(), currItemsAlreadySelected.end(), currItem->text().toStdString()) != currItemsAlreadySelected.end()) {
				1;
			} else {
				selectedFilesWidget->addItem(currItem->text());
			}
		}
		// adjust Widget
		selectedFilesWidget->setMinimumWidth(selectedFilesWidget->sizeHintForColumn(0));
	});

	QObject::connect(allRemoveButton, &QPushButton::clicked, [=]() {
		selectedFilesWidget->clear();
		selectedFilesWidget->setMinimumWidth(selectedFilesWidget->sizeHintForColumn(0));
	});

	QObject::connect(fewRemoveButton, &QPushButton::clicked, [=]() {
		auto currSelectedItems = selectedFilesWidget->selectedItems();

		for (const auto& currItem : currSelectedItems) {
			delete selectedFilesWidget->takeItem(selectedFilesWidget->row(currItem));
		}

		selectedFilesWidget->setMinimumWidth(selectedFilesWidget->sizeHintForColumn(0));
	});

	QObject::connect(cancelButton, &QPushButton::clicked, [=]() {
		std::cout << "Cancel was clicked." << std::endl;
		projectDialog->close();
		return;
	});

	QObject::connect(applyButton, &QPushButton::clicked, [=]() {
		std::cout << "Apply was clicked." << std::endl;
		m_runForProjectWsis.clear();
		for (int row = 0; row < selectedFilesWidget->count(); row++) {
			m_runForProjectWsis.push_back(selectedFilesWidget->item(row)->text().toStdString());
		}
		projectDialog->accept();
		return;
	});

	QObject::connect(enableToggle, &QPushButton::clicked, [=]() {
		m_runForProject = !m_runForProject;
		if (m_runForProject) {
			enableToggle->setText("Enabled");
			enableToggle->setChecked(true);
		}
		else {
			enableToggle->setText("Disabled");
			enableToggle->setChecked(false);
		}
		std::cout << "Run for project was " << enableToggle->text().toStdString() << std::endl;
	});

	projectDialog->exec();

	// in dialog, should be able to select which WSIs to use, and then if apply is selected,
	// when selecting method in Process Widget it will run it for the selected WSIs sequentially
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

        std::string someFile = split(fileName.toStdString(), "/").back();
        std::string oldLocation = split(fileName.toStdString(), someFile)[0];
        std::string newLocation = cwd + "data/Pipelines/";
        std::string newPath = cwd + "data/Pipelines/" + someFile;
        if (fileExists(newPath)) {
			std::cout << fileName.toStdString() << " : " << "File with the same name already exists in folder, didn't transfer..." << std::endl;
            continue;
        } else {
            if (fileExists(fileName.toStdString())) {
                QFile::copy(fileName, QString::fromStdString(newPath));
            }
        }
        // should update runPipelineMenu as new pipelines are being added
        //runPipelineMenu->addAction(QString::fromStdString(split(someFile, ".")[0]), this, &MainWindow::runPipeline);
        //auto currentAction = new QAction(QString::fromStdString(split(someFile, ".")[0]));
        auto currentAction = runPipelineMenu->addAction(QString::fromStdString(split(someFile, ".")[0]));
        QObject::connect(currentAction, &QAction::triggered, std::bind(&MainWindow::runPipeline, this, someFile));

        //auto currentAction = runPipelineMenu->addAction(currentFpl); //QString::fromStdString(split(split(currentFpl.toStdString(), "/")[-1], ".")[0]));
        //QObject::connect(currentAction, &QAction::triggered, std::bind(&MainWindow::runPipeline, this, cwd + "data/Pipelines/" + currentFpl.toStdString()));
    }
}


void MainWindow::createPipeline() {
    1;
}


void MainWindow::runForProject_apply(std::string method) {
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

	auto progDialog = QProgressDialog(mWidget);
	//std::cout << "\nNum files: " << ls.count() << std::endl;
	progDialog.setRange(0, ls.count() - 1);
	progDialog.setVisible(true);
	progDialog.setModal(false);
	progDialog.setLabelText("Adding models...");
	QRect screenrect = mWidget->screen()[0].geometry();
	progDialog.move(mWidget->width() - progDialog.width() / 2, -mWidget->width() / 2 - progDialog.width() / 2);
	progDialog.show();

	int counter = 0;
    // now iterate over all selected files and add selected files and corresponding ones to Models/
    for (QString& fileName : ls) {

        if (fileName == "")
            return;

        std::cout << "orig filename: " << fileName.toStdString() << std::endl;

        //qDebug().nospace() << "abc" << qPrintable(fileName) << "\ndef";
        std::string someFile = split(fileName.toStdString(),
                                     "/").back(); // TODO: Need to make this only split on last "/"
        std::cout << "Path: " << someFile << std::endl;
        std::string oldLocation = split(fileName.toStdString(), someFile)[0];

        std::string newLocation = cwd + "data/Models/";

        std::cout << "Old cwd: " << cwd << std::endl;

        std::vector<string> names = split(someFile, ".");
        string fileNameNoFormat = names[0];
        string formatName = names[1];

        // copy selected file to Models folder
        // check if file already exists in new folder, if yes, print warning, and stop
        string newPath = cwd + "data/Models/" + someFile;
        if (fileExists(newPath)) {
            std::cout << "file with same name already exists in folder, didn't transfer... " << std::endl;
			progDialog.setValue(counter);
			counter++;
            continue;
        }
        //QFile::copy(fileName, QString::fromStdString(newPath));

        // check which corresponding model files that exist, except from the one that is chosen
        std::vector<std::string> allowedFileFormats{"txt", "pb", "h5", "mapping", "xml", "bin", "uff"};

        std::cout << "Copy test" << std::endl;
        foreach(std::string currExtension, allowedFileFormats) {
            std::string oldPath = oldLocation + fileNameNoFormat + "." + currExtension;
            std::cout << currExtension << oldPath << std::endl;
            if (fileExists(oldPath)) {
                QFile::copy(QString::fromStdString(oldPath), QString::fromStdString(cwd + "data/Models/" + fileNameNoFormat + "." + currExtension));
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
                         std::bind(&MainWindow::pixelClassifier, this, modelName));
        someButton->show();

        processLayout->insertWidget(processLayout->count(), someButton);

        //createMainMenuWidget();
        //if (processLayout){
        //    clearLayout(processLayout);
        //}
        //createProcessWidget();
        //mainLayout->removeWidget(menuWidget); // this doesnt work...

		progDialog.setValue(counter);
		counter++;

		// to render straight away (avoid waiting on all WSIs to be handled before rendering)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 0);
    }
}


float MainWindow::getMagnificationLevel() {

    float magnification_lvl = 0.0f;

    // TODO: Should check which formats are supported by OpenSlide, and do this for all (in a generalized matter...)
    if ((wsiFormat == "generic-tiff") || (wsiFormat == "philips") || (wsiFormat == "ventana")) {

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
        } else if ((wsiFormat == "phillips") || (wsiFormat == "ventata")) {
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
    } else if (wsiFormat == "hamamatsu") {
		std::cout << "Vendor: hamamatsu" << std::endl;
        magnification_lvl = std::stof(metadata["openslide.objective-power"]);
		std::cout << "Magn lvl: " << magnification_lvl << std::endl;
    } else {  //"TODO: Make this more general, test different image formats to see how the magn_lvl metadata vary"
        std::cout << "\n aperio test" << metadata["aperio.AppMag"] << "\n apps";
        magnification_lvl = std::stof(metadata["aperio.AppMag"]);
    }
    return magnification_lvl;
}


bool MainWindow::segmentTissue() {

	// if no WSI is currently being rendered, 
	if (wsiList.empty()) {
		std::cout << "Requires a WSI to be rendered in order to perform the analysis." << std::endl;
		return false;
	}

	// prompt if you want to run the analysis again, if it has already been ran
    if (hasRenderer("tissue")) {
        std::cout << "Analysis on current WSI has already been performed. Delete the rendered object from the View widget if you want to run the method again." << std::endl;
        return false;
    } else {

        // Patch wise classification -> heatmap
        auto tissueSegmentation = TissueSegmentation::New();
        tissueSegmentation->setInputData(m_image);
        //tissueSegmentation->setThreshold(85);
        //tissueSegmentation->setErode(9);
        //tissueSegmentation->setDilate(9);

        // need custom slider
        /*
        QSlider *slider(new QSlider);
        slider->setRange(0,100);
        slider->show();

        QObject::connect(slider, &QSlider::valueChanged, slider, [slider](int value){
            const int step = 2;
            int offset = value % step;
            if( offset != 0)
                slider->setValue(value - offset);
        });
         */

        stopFlag = false;
        if (advancedMode) { // FIXME: Turned off for testing. Something wrong
            // option for setting parameters
            QDialog paramDialog;
            paramDialog.setStyleSheet(mWidget->styleSheet()); // transfer style sheet from parent
            QFormLayout form(&paramDialog);
            form.addRow(new QLabel("Please, set the parameters for this analysis: "));

            // threshold : for WSI this should be grayed out, shouldn't be able to change it
            auto threshSlider = new QSlider(Qt::Horizontal, dynamicViewWidget);
            threshSlider->setFixedWidth(150);
            threshSlider->setMinimum(0);
            threshSlider->setMaximum(255);
            threshSlider->setValue(tissueSegmentation->getThreshold());
            threshSlider->setTickInterval(1);
            QObject::connect(threshSlider, &QSlider::valueChanged, [=](int newValue){tissueSegmentation->setThreshold(newValue);});

            auto currValue = new QLabel;
            currValue->setText(QString::fromStdString(std::to_string(tissueSegmentation->getThreshold())));
            currValue->setFixedWidth(50);
            QObject::connect(threshSlider, &QSlider::valueChanged, [=](int newValue){currValue->setText(QString::fromStdString(std::to_string(tissueSegmentation->getThreshold())));});

			// threshold
            auto threshWidget = new QWidget;
            auto sliderLayout = new QHBoxLayout;
            threshWidget->setLayout(sliderLayout);
            sliderLayout->addWidget(threshSlider);
            sliderLayout->addWidget(currValue);

			QObject::connect(threshSlider, &QSlider::valueChanged, [=](int newValue) {
				std::cout << "\n" << newValue << "\n";
				const int step = 2;
				threshSlider->setValue(newValue);
				tissueSegmentation->setThreshold(newValue);
				auto checkFlag = true;

				if (checkFlag) {
					auto temporaryTissueSegmentation = TissueSegmentation::New();
					temporaryTissueSegmentation->setInputData(m_image);
					temporaryTissueSegmentation->setThreshold(tissueSegmentation->getThreshold());
					temporaryTissueSegmentation->setErode(tissueSegmentation->getErode());
					temporaryTissueSegmentation->setDilate(tissueSegmentation->getDilate());

					auto someRenderer = SegmentationRenderer::New();
					someRenderer->setColor(Segmentation::LABEL_FOREGROUND, Color(255.0f / 255.0f, 127.0f / 255.0f, 80.0f / 255.0f));
					someRenderer->setInputData(temporaryTissueSegmentation->updateAndGetOutputData<Image>());
					someRenderer->setOpacity(0.4f); // <- necessary for the quick-fix temporary solution
					someRenderer->update();

					std::string tempTissueName = "temporaryTissue";
					if (hasRenderer(tempTissueName)) {
						1;

						auto someRenderer = m_rendererList[tempTissueName];
						getView(0)->removeRenderer(someRenderer);
						m_rendererList.erase(tempTissueName);

						//auto renderer = m_rendererList[tempTissueName];
						//getView(0)->removeRenderer(renderer);  // FIXME: This crashed. Can you remove renderer that is still computing?
						//auto renderer = m_rendererList["WSI"];
						//std::cout << "\nView: " << getView(0) << "\n";
						//getView(0)->removeRenderer(renderer);
						//view->removeRenderer(renderer);
						//m_rendererList.erase(tempTissueName); // I added this, as it seemed like it should have been done

						//removeRenderer(tempTissueName);
						//m_rendererList.erase(tempTissueName);
						//m_rendererTypeList.erase(tempTissueName);
						//m_rendererList.erase(tempTissueName);
					}
					//m_rendererTypeList[tempTissueName] = "SegmentationRenderer";
					insertRenderer(tempTissueName, someRenderer);
				}
			});

            // dilation
            auto dilateSlider = new QSlider(Qt::Horizontal, dynamicViewWidget);
            dilateSlider->setFixedWidth(150);
            dilateSlider->setMinimum(1);
            dilateSlider->setMaximum(28);
            dilateSlider->setValue(tissueSegmentation->getDilate());
            dilateSlider->setSingleStep(2);
            //QObject::connect(dilateSlider, &QSlider::valueChanged, [=](int newValue){tissueSegmentation->setDilate(newValue);});
            QObject::connect(dilateSlider, &QSlider::valueChanged, [=](int newValue) {
                std::cout << "\n" << newValue << "\n";
                const int step = 2;
				bool checkFlag = false;
                if (newValue < 3) {
                    dilateSlider->setValue(0);
                    tissueSegmentation->setDilate(0);
					checkFlag = true;
                } else {
                    if (newValue % 2 != 0) {
                        dilateSlider->setValue(newValue);
                        tissueSegmentation->setDilate(newValue);
						checkFlag = true;
                    }
                }

				if (checkFlag) {
					auto temporaryTissueSegmentation = TissueSegmentation::New();
					temporaryTissueSegmentation->setInputData(m_image);
					temporaryTissueSegmentation->setThreshold(tissueSegmentation->getThreshold());
					temporaryTissueSegmentation->setErode(tissueSegmentation->getErode());
					temporaryTissueSegmentation->setDilate(tissueSegmentation->getDilate());

					auto someRenderer = SegmentationRenderer::New();
					someRenderer->setColor(Segmentation::LABEL_FOREGROUND, Color(255.0f / 255.0f, 127.0f / 255.0f, 80.0f / 255.0f));
					someRenderer->setInputData(temporaryTissueSegmentation->updateAndGetOutputData<Image>());
					someRenderer->setOpacity(0.4f); // <- necessary for the quick-fix temporary solution
					someRenderer->update();

					std::string tempTissueName = "temporaryTissue";
					if (hasRenderer(tempTissueName)) {
						1;

						auto someRenderer = m_rendererList[tempTissueName];
						getView(0)->removeRenderer(someRenderer);
						m_rendererList.erase(tempTissueName);

						//auto renderer = m_rendererList[tempTissueName];
						//getView(0)->removeRenderer(renderer);  // FIXME: This crashed. Can you remove renderer that is still computing?
						//auto renderer = m_rendererList["WSI"];
						//std::cout << "\nView: " << getView(0) << "\n";
						//getView(0)->removeRenderer(renderer);
						//view->removeRenderer(renderer);
						//m_rendererList.erase(tempTissueName); // I added this, as it seemed like it should have been done

						//removeRenderer(tempTissueName);
						//m_rendererList.erase(tempTissueName);
						//m_rendererTypeList.erase(tempTissueName);
						//m_rendererList.erase(tempTissueName);
					}
					//m_rendererTypeList[tempTissueName] = "SegmentationRenderer";
					insertRenderer(tempTissueName, someRenderer);
				}
            });

            auto currDilateValue = new QLabel;
            currDilateValue->setText(QString::fromStdString(std::to_string(tissueSegmentation->getDilate())));
            currDilateValue->setFixedWidth(50);
            QObject::connect(dilateSlider, &QSlider::valueChanged, [=](int newValue){currDilateValue->setText(QString::fromStdString(std::to_string(tissueSegmentation->getDilate())));});

            auto dilateWidget = new QWidget;
            auto dilateSliderLayout = new QHBoxLayout;
            dilateWidget->setLayout(dilateSliderLayout);
            dilateSliderLayout->addWidget(dilateSlider);
            dilateSliderLayout->addWidget(currDilateValue);

            // erosion
            auto erodeSlider = new QSlider(Qt::Horizontal, dynamicViewWidget);
            erodeSlider->setFixedWidth(150);
            erodeSlider->setMinimum(1);
            erodeSlider->setMaximum(28);
            erodeSlider->setValue(tissueSegmentation->getErode());
            erodeSlider->setSingleStep(2);
            //QObject::connect(erodeSlider, &QSlider::valueChanged, [=](int newValue){tissueSegmentation->setErode(newValue);});
            QObject::connect(erodeSlider, &QSlider::valueChanged, [=](int newValue) {
                std::cout << "\n" << newValue << "\n";
                bool checkFlag = false;
                if (newValue < 3) {
                    erodeSlider->setValue(0);
                    tissueSegmentation->setErode(0);
                    checkFlag = true;
                } else {
                    if (newValue % 2 != 0) {
                        erodeSlider->setValue(newValue);
                        tissueSegmentation->setErode(newValue);
                        checkFlag = true;
                    }
                }
                // /*
                if (checkFlag) {
                    auto temporaryTissueSegmentation = TissueSegmentation::New();
                    temporaryTissueSegmentation->setInputData(m_image);
                    temporaryTissueSegmentation->setThreshold(tissueSegmentation->getThreshold());
                    temporaryTissueSegmentation->setErode(tissueSegmentation->getErode());
                    temporaryTissueSegmentation->setDilate(tissueSegmentation->getDilate());

                    auto someRenderer = SegmentationRenderer::New();
                    someRenderer->setColor(Segmentation::LABEL_FOREGROUND, Color(255.0f/255.0f, 127.0f/255.0f, 80.0f/255.0f));
                    someRenderer->setInputData(temporaryTissueSegmentation->updateAndGetOutputData<Image>());
                    someRenderer->setOpacity(0.4f); // <- necessary for the quick-fix temporary solution
                    someRenderer->update();

                    std::string tempTissueName = "temporaryTissue";
                    if (hasRenderer(tempTissueName)) {

						auto someRenderer = m_rendererList[tempTissueName];
						getView(0)->removeRenderer(someRenderer);
						m_rendererList.erase(tempTissueName);

                        //auto renderer = m_rendererList[tempTissueName];
                        //getView(0)->removeRenderer(renderer);  // FIXME: This crashed. Can you remove renderer that is still computing?
                        //auto renderer = m_rendererList["WSI"];
                        //std::cout << "\nView: " << getView(0) << "\n";
                        //getView(0)->removeRenderer(renderer);
                        //view->removeRenderer(renderer);
                        //m_rendererList.erase(tempTissueName); // I added this, as it seemed like it should have been done

                        //removeRenderer(tempTissueName);
                        //m_rendererList.erase(tempTissueName);
                        //m_rendererTypeList.erase(tempTissueName);
                        //m_rendererList.erase(tempTissueName);
                    }
                    //m_rendererTypeList[tempTissueName] = "SegmentationRenderer";
                    insertRenderer(tempTissueName, someRenderer);
                }
                // */
            });

            auto currErodeValue = new QLabel;
            currErodeValue->setText(QString::fromStdString(std::to_string(tissueSegmentation->getErode())));
            currErodeValue->setFixedWidth(50);
            QObject::connect(erodeSlider, &QSlider::valueChanged, [=](int newValue){currErodeValue->setText(QString::fromStdString(std::to_string(tissueSegmentation->getErode())));});

            auto erodeWidget = new QWidget;
            auto erodeSliderLayout = new QHBoxLayout;
            erodeWidget->setLayout(erodeSliderLayout);
            erodeSliderLayout->addWidget(erodeSlider);
            erodeSliderLayout->addWidget(currErodeValue);

            QList<QSlider *> fields;
            QString labelThresh = "Threshold";
            form.addRow(labelThresh, threshWidget);
            fields << threshSlider;

            QString labelDilate = "Dilation";
            form.addRow(labelDilate, dilateWidget);
            fields << dilateSlider;

            QString labelErode = "Erosion";
            form.addRow(labelErode, erodeWidget);
            fields << erodeSlider;

            /*
            //QList<QLineEdit *> fields;
            //QList<QVariant *> fields;
            //QVariant<T> fields;
            QList<QSlider *> fields;
            //auto lineEdit = new QLineEdit(&paramDialog);
            //lineEdit->setText(QString::number(tissueSegmentation->thresh()));
            QString label = "Threshold";
            //form.addRow(label, lineEdit);
            form.addRow(label, sliderWidget);
            fields << threshSlider; //lineEdit;
             */

            /*
            auto lineEdit2 = new QLineEdit(&paramDialog);
            lineEdit2->setText(QString::number(tissueSegmentation->dilate()));
            label = "Dilation";
            form.addRow(label, lineEdit2);
            fields << lineEdit2;

            auto lineEdit3 = new QLineEdit(&paramDialog);
            lineEdit3->setText(QString::number(tissueSegmentation->erode()));
            label = "Erosion";
            form.addRow(label, lineEdit3);
            fields << lineEdit3;
             */

            // Add some standard buttons (Cancel/Ok) at the bottom of the dialog
            QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                    Qt::Horizontal, &paramDialog);
            buttonBox.button(QDialogButtonBox::Ok)->setText("Run");
            form.addRow(&buttonBox);
            QObject::connect(&buttonBox, SIGNAL(accepted()), &paramDialog, SLOT(accept()));
            QObject::connect(&buttonBox, SIGNAL(rejected()), &paramDialog, SLOT(reject()));

            // Show the dialog as modal
            int ret = paramDialog.exec();
            std::cout << "Value chosen: " << ret << std::endl;
            switch (ret) {
                case 1:
                    std::cout << "\nOK was pressed, should have updated params!\n";
                    //tissueSegmentation->setThreshold(fields.takeFirst()->value());
                    //tissueSegmentation->setDilate(fields.takeFirst()->value());
                    //tissueSegmentation->setErode(fields.takeFirst()->value());
                    /*
                    tissueSegmentation->setThreshold(std::stoi(fields.takeFirst()->text().toStdString()));
                    tissueSegmentation->setDilate(std::stoi(fields.takeFirst()->text().toStdString()));
                    tissueSegmentation->setErode(std::stoi(fields.takeFirst()->text().toStdString()));
                     */
                    break;
                case 0:
                    std::cout << "\nCancel was pressed.\n";
                    stopFlag = true;
                    break;
                default:
                    std::cout << "\nDefault was pressed.\n";
                    break;
            }

			/*
			std::string tempTissueName = "temporaryTissue";
			auto someRenderer = m_rendererList[tempTissueName];
			getView(0)->removeRenderer(someRenderer);
			m_rendererList.erase(tempTissueName);
			 */

        }



		// update ViewWidget
		/*
		for (auto const&[key, val] : ) {
			createDynamicViewWidget("WSI", modelName);
			pageComboBox->removeItem();
		}
		 */

		std::cout << "Current stopFlag: " << stopFlag << std::endl;

        //if (stopFlag)
        //    return false;

        std::cout << "Thresh: " << tissueSegmentation->getThreshold() << std::endl;
        std::cout << "Dilate: " << tissueSegmentation->getDilate() << std::endl;
        std::cout << "Erode:  " << tissueSegmentation->getErode() << std::endl;

        //m_tissue = tissueSegmentation->updateAndGetOutputData<Image>();
        // finally get resulting tissueMap to be used later on
        m_tissue = tissueSegmentation->updateAndGetOutputData<Image>();

        auto someRenderer = SegmentationRenderer::New();
        someRenderer->setColor(Segmentation::LABEL_FOREGROUND, Color(255.0f/255.0f, 127.0f/255.0f, 80.0f/255.0f));
        someRenderer->setInputData(m_tissue);
        //someRenderer->setInputData(tissueSegmentation->updateAndGetOutputData<Image>());
        someRenderer->setOpacity(0.4f); // <- necessary for the quick-fix temporary solution
        //someRenderer->setBackgroundLabel(5);
        someRenderer->update();

		std::string currSegment = "tissue";
		/*
        std::string origSegment = "tissue";
        auto iter = 2;
        while (hasRenderer(currSegment)) {
            currSegment = origSegment + std::to_string(iter);
            iter++;
        }
		 */

		m_rendererTypeList[currSegment] = "SegmentationRenderer";
        createDynamicViewWidget(currSegment, modelName);
        insertRenderer(currSegment, someRenderer);

        // add final segmentation to result list to be accessible if wanted for exporting
        availableResults[currSegment] = m_tissue;
        exportComboBox->addItem(tr(currSegment.c_str()));

		std::cout << "TissueSegmenter adds result using dynamic view widget" << std::endl;

        return true;
    }
}


void MainWindow::loadTissue(QString tissuePath) {
    //DeviceManager* deviceManager = DeviceManager::getInstance();
    //OpenCLDevice::pointer device = deviceManager->getOneOpenCLDevice();

	if (!fileExists(tissuePath.toStdString()))
		return;

    ImageImporter::pointer reader = ImageImporter::New();
    reader->setGrayscale(false);
    reader->setFilename(tissuePath.toStdString());
    //reader->setMainDevice(device);
    reader->setMainDevice(Host::getInstance());
    DataChannel::pointer port = reader->getOutputPort();
    reader->update();
    Image::pointer someImage = port->getNextFrame<Image>();

    //auto wsi = getInputData<ImagePyramid>();
    auto access = m_image->getAccess(ACCESS_READ);
    auto input = access->getLevelAsImage(m_image->getNrOfLevels()-1);

    std::cout << "Dimensions info: " << input->getHeight() << ", " << input->getWidth() << std::endl;
    std::cout << "Dimensions info: " << m_image->getFullHeight() << ", " << m_image->getFullWidth() << std::endl;

    someImage->setSpacing((float) m_image->getFullHeight() / (float) input->getHeight(), (float) m_image->getFullWidth() / (float) input->getWidth(), 1.0f);

    /*
    auto intensityScaler = ScaleImage::New();
    intensityScaler->setInputData(m_tissue); //m_tissue);  // expects Image data type
    intensityScaler->setLowestValue(0.0f);
    intensityScaler->setHighestValue(1.0f);
    //intensityScaler->update();
     */

    // /*
    auto thresholder = BinaryThresholding::New();
    thresholder->setLowerThreshold(0.5f);
    thresholder->setInputData(someImage);
    // */

    auto output = Segmentation::New();
    //output->createFromImage(input);
    //output->createFromImage(someImage);  // is it initialized or converted?
    output->create(someImage->getSize(), TYPE_UINT8, 3);
    //output->getOpenCLImageAccess(someImage);

    auto thresholder2 = BinaryThresholding::New();
    thresholder2->setLowerThreshold(0.5f);
    thresholder2->setInputData(output);

    //output->getOpenCLImageAccess(reader->updateAndGetOutputData<Image>());

    //m_tissue = reader->updateAndGetOutputData<Image>();

    auto someRenderer = SegmentationRenderer::New();
    someRenderer->setColor(Segmentation::LABEL_FOREGROUND, Color(255.0f/255.0f, 127.0f/255.0f, 80.0f/255.0f));
    //someRenderer->setInputData(reader->updateAndGetOutputData<Image>());
    //someRenderer->setInputData(output);  // someImage
    someRenderer->setInputConnection(0, thresholder->getOutputPort());
    //someRenderer->setInputData(someImage);
    someRenderer->setOpacity(0.4f); // <- necessary for the quick-fix temporary solution
    someRenderer->update();

    m_rendererTypeList["tissue"] = "SegmentationRenderer";
    insertRenderer("tissue", someRenderer);

    //hideTissueMask(false);

    // now make it possible to edit prediction in the View Widget
    createDynamicViewWidget("tissue", modelName);

    std::cout << "\nfinished loading..." << std::endl;;
    savedList.emplace_back("tissue");
}


void MainWindow::runPipeline(std::string path) {

	std::vector<std::string> currentWSIs;
	if (m_runForProject) {
		currentWSIs = m_runForProjectWsis;
	}
	else {
		currentWSIs.push_back(wsiList[curr_pos]);
	}

	auto progDialog = QProgressDialog(mWidget);
	progDialog.setRange(0, (int)currentWSIs.size() - 1);
	//progDialog.setContentsMargins(0, 0, 0, 0);
	progDialog.setVisible(true);
	progDialog.setModal(false);
	progDialog.setLabelText("Saving grade heatmaps...");
	//QRect screenrect = mWidget->screen()[0].geometry();
	progDialog.move(mWidget->width() - progDialog.width() * 1.1, progDialog.height() * 0.1);
	progDialog.show();

	auto counter = 0;
	for (const auto& currWSI : currentWSIs) {

		// TODO: Perhaps use corresponding .txt-file to feed arguments in the pipeline
		// pipeline requires some user-defined inputs, e.g. which WSI to use (and which model?)
		std::map<std::string, std::string> arguments;
		arguments["filename"] = filename;
		arguments["modelPath"] = path;

		// check if folder for current WSI exists, if not, create one
		QString wsiResultPath = (projectFolderName.toStdString() + "/results/" + split(split(currWSI, "/").back(), ".")[0] + "/").c_str();
		wsiResultPath = wsiResultPath.replace("//", "/");
		if (!QDir(wsiResultPath).exists()) {
			QDir().mkdir(wsiResultPath);
		}

		QString outFile = (wsiResultPath.toStdString() + split(split(currWSI, "/").back(), ".")[0] + "_heatmap.h5").c_str();

		arguments["outPath"] = outFile.replace("//", "/").toStdString();

		// parse fpl-file, and run pipeline with correspodning input arguments
		auto pipeline = Pipeline(path, arguments);
		//pipeline.parsePipelineFile();

		// add renderer
		//insertRenderer("testPipeline", pipeline.getRenderers()[1]); // only render the NN-results (not the WSI)
		//createDynamicViewWidget("testPipeline", "testPipeline"); // modelMetadata["name"], modelName);

		// update progress bar
		progDialog.setValue(counter);
		counter++;

		// to render straight away (avoid waiting on all WSIs to be handled before rendering)
		QCoreApplication::processEvents(QEventLoop::AllEvents, 0);
	}
}


void MainWindow::loadTumor(QString tumorPath) {
    //DeviceManager* deviceManager = DeviceManager::getInstance();
    //OpenCLDevice::pointer device = deviceManager->getOneOpenCLDevice();

    ImageImporter::pointer reader = ImageImporter::New();
    reader->setGrayscale(false);
    reader->setFilename(tumorPath.toStdString());
    //reader->setMainDevice(device);
    reader->setMainDevice(Host::getInstance());
    DataChannel::pointer port = reader->getOutputPort();
    reader->update();
    Image::pointer someImage = port->getNextFrame<Image>();

    //auto wsi = getInputData<ImagePyramid>();
    auto access = m_image->getAccess(ACCESS_READ);
    auto input = access->getLevelAsImage(5); //m_image->getNrOfLevels()-1);

    someImage->setSpacing((float) m_image->getFullHeight() / (float) input->getHeight(), (float) m_image->getFullWidth() / (float) input->getWidth(), 1.0f);

    /*
    auto intensityScaler = ScaleImage::New();
    intensityScaler->setInputData(m_tissue); //m_tissue);  // expects Image data type
    intensityScaler->setLowestValue(0.0f);
    intensityScaler->setHighestValue(1.0f);
    //intensityScaler->update();
     */

    // /*
    auto thresholder = BinaryThresholding::New();
    thresholder->setLowerThreshold(0.5f);
    thresholder->setInputData(someImage);
    // */

    auto output = Segmentation::New();
    //output->createFromImage(input);
    //output->createFromImage(someImage);  // is it initialized or converted?
    output->create(someImage->getSize(), TYPE_UINT8, 3);
    //output->getOpenCLImageAccess(someImage);

    auto thresholder2 = BinaryThresholding::New();
    thresholder2->setLowerThreshold(0.5f);
    thresholder2->setInputData(output);

    //output->getOpenCLImageAccess(reader->updateAndGetOutputData<Image>());

    std::cout << "\nLoading tumor mask... \n";

    //m_tissue = reader->updateAndGetOutputData<Image>();

    std::cout << "\nupdate... \n";

    auto someRenderer = SegmentationRenderer::New();
    someRenderer->setColor(Segmentation::LABEL_FOREGROUND, Color(0.0f, 0.0f, 255.0f/255.0f));
    //someRenderer->setInputData(reader->updateAndGetOutputData<Image>());
    //someRenderer->setInputData(output);  // someImage
    someRenderer->setInputConnection(0, thresholder->getOutputPort());
    //someRenderer->setInputData(someImage);
    someRenderer->setOpacity(0.4f); // <- necessary for the quick-fix temporary solution
    someRenderer->update();

    m_rendererTypeList["tumorSeg_lr"] = "SegmentationRenderer";
    insertRenderer("tumorSeg_lr", someRenderer);

    //hideTissueMask(false);

    // now make it possible to edit prediction in the View Widget
    createDynamicViewWidget("tumorSeg_lr", modelName);

    std::cout << "finished loading..." << std::endl;;
    savedList.emplace_back("tumorSeg_lr");
}


bool MainWindow::lowresSegmenter() {

    modelName = "/home/andrep/workspace/FAST-Pathology/Models_old_020420/low_res_tumor_unet.pb";
    //modelName = "C:/Users/andrep/workspace/FAST-Pathology/data/Models/low_res_tumor_unet.xml";

    // read model metadata (txtfile)
    //std::map<std::string, std::string> modelMetadata = getModelMetadata(modelName);

    if (true) { //(!hasRenderer(modelMetadata["name"])) { // only run analysis if it has not been ran previously on current WSI
        auto access = m_image->getAccess(ACCESS_READ);
        //auto input = access->getLevelAsImage(m_image->getNrOfLevels()-1);
        auto input = access->getLevelAsImage(5); //m_image->getNrOfLevels()-1); // FIXME: Should automatically find best suitable magn.lvl.

        // resize
        ImageResizer::pointer resizer = ImageResizer::New();
        resizer->setInputData(input);
        resizer->setWidth(1024);
        resizer->setHeight(1024);
        auto port = resizer->getOutputPort();
        resizer->update();
        //Image::pointer resized = port->getNextFrame<Image>();

        auto network = NeuralNetwork::New();
        network->setInferenceEngine("TensorFlowCPU");
        network->setInputNode(0, "input_1", NodeType::IMAGE, {1, 1024, 1024, 3});
        network->setOutputNode(0, "conv2d_34/truediv", NodeType::TENSOR, {1, 1024, 1024, 2});
        network->load(modelName);
        network->setInputData(port->getNextFrame<Image>());
        //vector scale_factor = split(modelMetadata["scale_factor"], "/"); // get scale factor from metadata
        network->setScaleFactor(1.0f/255.0f);   // 1.0f/255.0f

        auto converter = TensorToSegmentation::New();
        converter->setInputConnection(network->getOutputPort());

        // resize back
        ImageResizer::pointer resizer2 = ImageResizer::New();
        resizer2->setInputData(converter->updateAndGetOutputData<Image>());
        resizer2->setWidth(input->getWidth());
        resizer2->setHeight(input->getHeight());
        auto port2 = resizer2->getOutputPort();
        resizer2->update();

        m_tumorMap = port2->getNextFrame<Image>();
        m_tumorMap->setSpacing((float) m_image->getFullHeight() / (float) input->getHeight(), (float) m_image->getFullWidth() / (float) input->getWidth(), 1.0f);

        auto segTumorRenderer = SegmentationRenderer::New();
        segTumorRenderer->setOpacity(0.4f);
        segTumorRenderer->setColor(Segmentation::LABEL_FOREGROUND, Color(255.0 / 255.0f, 0.0f, 0.0f));
        segTumorRenderer->setColor(Segmentation::LABEL_BACKGROUND, Color(0.0f, 255.0f / 255.0f, 0.0f));
        segTumorRenderer->setInputData(m_tumorMap);
        //segTumorRenderer->setInterpolation(false);
        segTumorRenderer->update();

        insertRenderer("tumorSeg_lr", segTumorRenderer);

        createDynamicViewWidget("tumorSeg_lr", modelName);
    }
    return true;
}


// Setting parameters for different methods
std::map<std::string, std::string> MainWindow::setParameterDialog(std::map<std::string, std::string> modelMetadata) {
    QDialog paramDialog;
    paramDialog.setStyleSheet(mWidget->styleSheet()); // transfer style sheet from parent
    QFormLayout form(&paramDialog);
    form.addRow(new QLabel("Please, set the parameters for this analysis: "));

    std::cout << "\nBefore update: " << std::endl;
    for (const auto &[k, v] : modelMetadata)
        std::cout << "m[" << k << "] = (" << v << ") " << std::endl;

    QList<QLineEdit *> fields;
    for (auto const& [key, val] : modelMetadata) {
        std::cout << key << ":" << val << std::endl;
        auto lineEdit = new QLineEdit(&paramDialog);
        lineEdit->setText(QString::fromStdString(val));
        QString label = QString::fromStdString(key); //QString::fromStdString(val).arg(QString::fromStdString(key));
        form.addRow(label, lineEdit);

        fields << lineEdit;
    }

    // Add some standard buttons (Cancel/Ok) at the bottom of the dialog
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
            Qt::Horizontal, &paramDialog);
    buttonBox.button(QDialogButtonBox::Ok)->setText("Run");
    form.addRow(&buttonBox);
    QObject::connect(&buttonBox, SIGNAL(accepted()), &paramDialog, SLOT(accept()));
    QObject::connect(&buttonBox, SIGNAL(rejected()), &paramDialog, SLOT(reject()));

    // Show the dialog as modal
    int ret = paramDialog.exec();
    std::cout << "Ret value: " << ret << std::endl;
    switch (ret) {
        case 1: //QMessageBox::Ok:
            std::cout << "OK was pressed: " << std::endl;
            for (auto const& [key, val] : modelMetadata) {
                modelMetadata[key] = fields.takeFirst()->text().toStdString(); //fields.takeAt(cnt)->text().toStdString();
            }
            break;
        case 0: //QMessageBox::Cancel:
            std::cout << "CANCEL was pressed: " << std::endl;
            stopFlag = true;
            break;
        default:
            std::cout << "Default..." << std::endl;
            break;
    }

    std::cout << "After update: " << std::endl;
    for (const auto &[k, v] : modelMetadata)
        std::cout << "m[" << k << "] = (" << v << ") " << std::endl;

    return modelMetadata;
}


// TODO: Integrate SegmentationClassifier inside this or make functions for each task?
//  perhaps make a new class instead?
bool MainWindow::pixelClassifier(std::string modelName) {

	// if no WSI is currently being rendered, 
	if (wsiList.empty()) {
		std::cout << "Requires a WSI to be rendered in order to perform the analysis." << std::endl;
		return false;
	}

	std::cout << "Current model: " << modelName << std::endl;

    // read model metadata (txtfile)
    std::map<std::string, std::string> modelMetadata = getModelMetadata(modelName);

    stopFlag = false;

    if (!hasRenderer(modelMetadata["name"])) { // only run analysis if it has not been ran previously on current WSI

		// add current model name to map
		modelNames[modelName] = modelName;

        // set parameters yourself (only enabled if advanced mode is ON
        if (advancedMode) {
            modelMetadata = setParameterDialog(modelMetadata);
            for (const auto &[k, v] : modelMetadata)
                std::cout << "m[" << k << "] = (" << v << ") " << std::endl;
        }
        if (stopFlag) { // if "Cancel" is selected in advanced mode in parameter selection, don't run analysis
            return false;
        }

		// segment tissue if not already ran, but hide it
		if (!hasRenderer("tissue")) {
			segmentTissue();
			hideTissueMask(true);
		}

        //auto tmpRenderer = getRenderer(metadata["name"]);

        // based on predicted magnification level of WSI, set magnificiation level for optimal input to model based on predicted resolution of WSI
        int patch_lvl_model = (int) (std::log(magn_lvl / (float)std::stoi(modelMetadata["magnification_level"])) / std::log(std::round(stof(metadata["openslide.level[1].downsample"]))));

		std::cout << "Curr patch level: " << patch_lvl_model << std::endl;

        ImageResizer::pointer resizer = ImageResizer::New();
        int currLvl;
        if (modelMetadata["resolution"] == "low") {
            auto access = m_image->getAccess(ACCESS_READ);
            // TODO: Should automatically find best suitable magn.lvl. (NOTE: 2x the image size as for selecting lvl!)

            int levelCount = std::stoi(metadata["openslide.level-count"]);
            int inputWidth = std::stoi(modelMetadata["input_img_size_x"]);
            int inputHeight = std::stoi(modelMetadata["input_img_size_y"]);
            bool breakFlag = false;
            for (int i=0; i<levelCount; i++) {
                if ((std::stoi(metadata["openslide.level[" + std::to_string(i) + "].width"]) <= inputWidth * 2) ||
                    (std::stoi(metadata["openslide.level[" + std::to_string(i) + "].height"]) <= inputHeight * 2)) {
                    currLvl = i - 1;
                    breakFlag = true;
                    break;
                }
            }
            if (!breakFlag)
                currLvl = levelCount - 1;

            std::cout << "Optimal patch level: " << std::to_string(currLvl) << std::endl;
            if (currLvl < 0) {
                std::cout << "Automatic chosen patch level for low_res is invalid: " <<std::to_string(currLvl) << std::endl;
                return false;
            }
            auto input = access->getLevelAsImage(currLvl);

            // resize
            //ImageResizer::pointer resizer = ImageResizer::New();
            resizer->setInputData(input);
            resizer->setWidth(inputWidth);
            resizer->setHeight(inputHeight);
        }

        // get available IEs as a list
        std::list<std::string> IEsList;
        QStringList tmpPaths = QDir(QString::fromStdString(Config::getLibraryPath())).entryList(QStringList(), QDir::Files);

		auto currOperatingSystem = QSysInfo::productType();
		auto currKernel = QSysInfo::kernelType();
		std::cout << "Current OS is: " << currOperatingSystem.toStdString() << std::endl;
		std::cout << "Current kernel is: " << currKernel.toStdString() << std::endl;
		if (currKernel == "linux") {
			foreach(QString filePath, tmpPaths) {
				if (filePath.toStdString().find("libInferenceEngine") != std::string::npos) {
					IEsList.push_back(split(split(filePath.toStdString(), "libInferenceEngine").back(), ".so")[0]);
				}
			}
		} else if ((currKernel == "winnt") || (currKernel == "wince")) {
			foreach(QString filePath, tmpPaths) {
				if (filePath.toStdString().find("InferenceEngine") != std::string::npos) {
					IEsList.push_back(split(split(filePath.toStdString(), "InferenceEngine").back(), ".dll")[0]);
				}
			}
		}
		else {
			std::cout << "Current operating system is not using any of the supported kernels: linux and winnt. Current kernel is: " << currKernel.toStdString() << std::endl;
		}

        // check which model formats exists, before choosing inference engine
        QDir directory(QString::fromStdString(cwd + "data/Models/"));
        QStringList models = directory.entryList(QStringList(), QDir::Files);

        std::list<std::string> acceptedModels;
        foreach(QString currentModel, models) {
            if (currentModel.toStdString().find(modelName) != std::string::npos) {
                acceptedModels.push_back("." + split(currentModel.toStdString(), modelName + ".").back());
				std::cout << "accepted models: ." + split(currentModel.toStdString(), modelName + ".").back() << std::endl;
            }
        }

        // init network
        auto network = NeuralNetwork::New(); // default, need special case for high_res segmentation
        if ((modelMetadata["problem"] == "segmentation") && (modelMetadata["resolution"] == "high")) {
            network = SegmentationNetwork::New();
        } else if ((modelMetadata["problem"] == "object_detection") && (modelMetadata["resolution"] == "high")) {
            1;
            //network = BoundingBoxNetwork::New();  // FIXME: This cannot be done, because the stuff right below is not available in NeuralNetwork
            //network->loadAttributes();
            //network->setThreshold(0.3); // default value: 0.3
            //network->setAnchors(getAnchorMetadata("tiny_yolo_anchors_pannuke"));
        } else if ((modelMetadata["problem"] == "segmentation") && (modelMetadata["resolution"] == "low")) {
            network = SegmentationNetwork::New();
        }
        //network->setInferenceEngine("TensorRT"); //"TensorRT");

        bool checkFlag = true;

		std::cout << "Current available IEs: " << std::endl;
		foreach(std::string elem, IEsList) {
			std::cout << elem << ", ";
		}

		std::cout << "Which model formats are available and that there exists an IE for: " << std::endl;
		foreach(std::string elem, acceptedModels) {
			std::cout << elem << ", ";
		}

        // /*
        // Now select best available IE based on which extensions exist for chosen model
        // TODO: Current optimization profile is: 0. Please ensure there are no enqueued operations pending in this context prior to switching profiles
        if ((std::find(acceptedModels.begin(), acceptedModels.end(), ".uff") != acceptedModels.end()) && (std::find(IEsList.begin(), IEsList.end(), "TensorRT") != IEsList.end())) {
			std::cout << "TensorRT selected" << std::endl;
			network->setInferenceEngine("TensorRT");
        } else if (std::find(acceptedModels.begin(), acceptedModels.end(), ".pb") != acceptedModels.end() && std::find(IEsList.begin(), IEsList.end(), "TensorFlowCUDA") != IEsList.end()) {
			std::cout << "TensorFlowCUDA selected" << std::endl;
			network->setInferenceEngine("TensorFlowCUDA");
            /*
            if (std::find(acceptedModels.begin(), acceptedModels.end(), ".xml") != acceptedModels.end() && std::find(IEsList.begin(), IEsList.end(), "OpenVINO") != IEsList.end()) {
                network->setInferenceEngine("OpenVINO");
            }
             */
        } else if (std::find(acceptedModels.begin(), acceptedModels.end(), ".xml") != acceptedModels.end() && std::find(IEsList.begin(), IEsList.end(), "OpenVINO") != IEsList.end()) {
			std::cout << "OpenVINO selected" << std::endl;
			network->setInferenceEngine("OpenVINO");
        } else if (std::find(acceptedModels.begin(), acceptedModels.end(), ".pb") != acceptedModels.end() && std::find(IEsList.begin(), IEsList.end(), "TensorFlowCPU") != IEsList.end()) {
			std::cout << "TensorFlowCPU selected" << std::endl;
			network->setInferenceEngine("TensorFlowCPU");
        } 
		/* else {
            std::cout << "\nModel does not exist in Models/ folder. Please add it using AddModels(). "
                         "It might also be that the model exists, but the Inference Engine does not. "
                         "Available IEs are: ";
            foreach(std::string elem, IEsList) {
                std::cout << elem << ", ";
            }
            std::cout << "\n";
            checkFlag = false;
        }
         */

        if (checkFlag) {
            std::cout << "Model was found." << std::endl;

            // TODO: Need to handle if model is in Models/, but inference engine is not available
            //Config::getLibraryPath();

            // If model has CPU flag only, need to check if TensorFlowCPU is available, else run on OpenVINO, else use best available
            if (std::stoi(modelMetadata["cpu"]) == 1) {
                if (std::find(acceptedModels.begin(), acceptedModels.end(), ".pb") != acceptedModels.end() && std::find(IEsList.begin(), IEsList.end(), "TensorFlowCPU") != IEsList.end()) {
                    network->setInferenceEngine("TensorFlowCPU");
                } else if (std::find(acceptedModels.begin(), acceptedModels.end(), ".xml") != acceptedModels.end() && std::find(IEsList.begin(), IEsList.end(), "OpenVINO") != IEsList.end()) {
                    network->setInferenceEngine("OpenVINO");
					network->getInferenceEngine()->setDeviceType(InferenceDeviceType::CPU);
				}
				else {
					std::cout << "CPU only was selected, but was not able to find any CPU devices..." << std::endl;
				}
                // else continue -> will use default one (one that is available)
            }

            //network->setInferenceEngine("TensorFlowCPU");
            //network->setInferenceEngine("TensorFlowCUDA");
            //network->setInferenceEngine("OpenVINO"); // default
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
                } else if (modelMetadata["problem"] == "object_detection") {
                    // FIXME: This is outdated for YoloV3, as it has multiple output nodes -> need a way of handling this!
                    network->setOutputNode(0, modelMetadata["output_node"], NodeType::TENSOR,
                                                 TensorShape({1, std::stoi(modelMetadata["nb_classes"])}));
                }
            } else if (engine == "TensorRT") {
                // TensorRT needs to know everything about the input and output nodes
                network->setInputNode(0, modelMetadata["input_node"], NodeType::IMAGE, TensorShape(
                        {1, std::stoi(modelMetadata["nb_channels"]), std::stoi(modelMetadata["input_img_size_y"]),
                         std::stoi(modelMetadata["input_img_size_y"])})); //{1, size, size, 3}
                network->setOutputNode(0, modelMetadata["output_node"], NodeType::TENSOR,
                                       TensorShape({1, std::stoi(modelMetadata["nb_classes"])}));
            }

            //network->setInferenceEngine("OpenVINO"); // force it to use a specific IE -> only for testing
            //network->setInferenceEngine("TensorRT");
            network->load(cwd + "data/Models/" + modelName + "." + network->getInferenceEngine()->getDefaultFileExtension()); //".uff");

            if (modelMetadata["resolution"] == "low") { // special case handling for low_res NN inference
                auto port = resizer->getOutputPort();
                resizer->update();
                network->setInputData(port->getNextFrame<Image>());
            } else {
                // for tissue, apply erosion to filter some of the tissue
                /*
                auto erosion = Erosion::New();
                erosion->setInputData(m_tissue);
                erosion->setStructuringElementSize(9);
                 */

                auto generator = PatchGenerator::New();
                generator->setPatchSize(std::stoi(modelMetadata["input_img_size_y"]), std::stoi(modelMetadata["input_img_size_x"]));
                generator->setPatchLevel(patch_lvl_model);
                generator->setInputData(0, m_image);
                if (m_tissue) {
                    generator->setInputData(1, m_tissue); //erosion->updateAndGetOutputData<Image>());
                }
                if (m_tumorMap) {
                    /*
                    // dilate tumorMap a little to reduce risk of loosing nuclei within the area of interest
                    auto dilation = Dilation::New(); // first two, initial closing (instead of opening) to increase sensitivity in detection
                    dilation->setInputData(m_tumorMap);
                    dilation->setStructuringElementSize(25);

                    generator->setInputData(1, dilation->updateAndGetOutputData<Image>());
                     */
                    generator->setInputData(1, m_tumorMap);
                }

                //auto batchgen = ImageToBatchGenerator::New();  // TODO: Can't use this with TensorRT (!)
                //batchgen->setInputConnection(generator->getOutputPort());
                //batchgen->setMaxBatchSize(std::stoi(modelMetadata["batch_process"])); // set 256 for testing stuff (tumor -> annotation, then grade from tumor segment)

                network->setInputConnection(generator->getOutputPort());
            }
            vector scale_factor = split(modelMetadata["scale_factor"], "/"); // get scale factor from metadata
            network->setScaleFactor((float) std::stoi(scale_factor[0]) / (float) std::stoi(scale_factor[1]));   // 1.0f/255.0f

            // define renderer from metadata
            if ((modelMetadata["problem"] == "classification") && (modelMetadata["resolution"] == "high")) {
                auto stitcher = PatchStitcher::New();
                stitcher->setInputConnection(network->getOutputPort());

                // add model to metadata list
                //m_modelMetadataList[modelMetadata["name"]] = modelMetadata;

                //const std::map<std::string, std::string> &testModelMetadata = modelMetadata;

                // testing RunLambda in FAST for saving heatmaps
                /*
                auto lambda = RunLambdaOnLastFrame::New();
                lambda->setInputConnection(stitcher->getOutputPort());
                lambda->setLambda([this, modelMetadata](DataObject::pointer data) {
                    std::cout << "Last frame detected, processing..." << std::endl;
                    auto tensor = std::dynamic_pointer_cast<Tensor>(data);
                    // TODO do stuff with tensor here
                    // need to get metadata again, as its happening on another thread
                    //std::map<std::string, std::string> modelMetadata = getModelMetadata(modelName); // TODO: Then edits in advanced mode is lost...

                    // TODO: Make modelMetadata accessible in current thread?

                    auto converter = TensorToSegmentation::New();
                    converter->setInputData(tensor);
                    //converter->setBackgroundLabel(200); //std::stoi(modelMetadata.at("nb_classes")) + 1); //200); // FIXME: This will not work in general, for instance if a model has the classes {0, 1, 3}, should set to a unique class
                    //converter->setThreshold(0.333f); // FIXME: This should be possible to set, but modelMetadata is not accessible in this thread
                    printf("\n%d\n",__LINE__); // <- this is nice for debugging

                    //auto currModelMetadata = m_modelMetadataList[]

                    //auto port = intensityScaler->getOutputPort();
                    //intensityScaler->update();

                    //Image::pointer result = port->getNextFrame<Image>();
                    //m_gradeMap = port->getNextFrame<Image>();

                    //m_tumorHeatmap = converter->updateAndGetOutputData<Image>();
                    auto currSegmentation = converter->updateAndGetOutputData<Image>();
                    //m_gradeMap = intensityScaler->updateAndGetOutputData<Image>();

                    // attempt simple post-processing

                    auto finalRenderer = SegmentationRenderer::New();
                    finalRenderer->setOpacity(0.4);
                    //tmpRenderer->setFillArea(true);
                    //tmpRenderer->setColor(0, Color(255.0, 0.0f, 0.0f));
                    //tmpRenderer->setColor(0, Color(0.0f, 0.0f, 255.0 / 255.0f));
                    //tmpRenderer->setColor(1, Color(255.0 / 255.0f, 0.0f, 0.0f));
                    //tmpRenderer->setColor(2, Color(255.0 / 255.0f, 127.0f, 90.0f));
                    //tmpRenderer->setColor(Segmentation::LABEL_FOREGROUND, Color(255.0 / 255.0f, 0.0f, 0.0f));
                    //tmpRenderer->setColor(Segmentation::LABEL_BACKGROUND, Color(30.0f, 255.0f / 255.0f, 127.0f));
                    //tmpRenderer->setColor(0, Color(1.0f, 0.0f, 0.0f));
                    //tmpRenderer->setColor(1, Color(0.0f, 1.0f, 0.0f));
                    //tmpRenderer->setColor(2, Color(0.0f, 0.0f, 1.0f));
                    //finalRenderer->setChannelHidden(0, false);
                    finalRenderer->setInterpolation(false); // false : no minecraft
                    finalRenderer->setInputData(currSegmentation);
                    //finalRenderer->setInputData(erosion->updateAndGetOutputData<Image>());
                    //finalRenderer->update();
                    //finalRenderer->setSynchronizedRendering(true);

                    for (const auto &[k, v] : modelMetadata)
                        std::cout << "m[" << k << "] = (" << v << ") " << std::endl;

                    m_rendererTypeList[modelMetadata.at("name") + "_final"] = "SegmentationRenderer";
                    insertRenderer(modelMetadata.at("name") + "_final", finalRenderer);

                    //std::cout << "\nCount: " << modelMetadata.size() << std::endl;

                    // TODO: Creation of Qt-stuff outside main thread?
                    createDynamicViewWidget(modelMetadata.at("name") + "_final", modelMetadata.at("model_name"));

                    // add final segmentation to result list to be accessible if wanted for exporting
                    //availableResults[modelMetadata.at("name") + "_final"] = currSegmentation;
                    //exportComboBox->addItem(tr((modelMetadata.at("name") + "_final").c_str()));
                });
                 */

                auto port = stitcher->getOutputPort();

                m_patchStitcherList[modelMetadata["name"]] = stitcher; // add stitcher to global list to be accessible later on

                auto someRenderer = HeatmapRenderer::New();
                someRenderer->setInterpolation(std::stoi(modelMetadata["interpolation"].c_str()));
                someRenderer->setInputConnection(stitcher->getOutputPort());
                //someRenderer->setInputConnection(lambda->getOutputPort());
                someRenderer->setMaxOpacity(0.6f);
                //heatmapRenderer->update();
                vector<string> colors = split(modelMetadata["class_colors"], ";");
                for (int i = 0; i < std::stoi(modelMetadata["nb_classes"]); i++) {
                    vector<string> rgb = split(colors[i], ",");
                    someRenderer->setChannelColor(i, Color((float) std::stoi(rgb[0]) / 255.0f,
                                                           (float) std::stoi(rgb[1]) / 255.0f,
                                                           (float) std::stoi(rgb[2]) / 255.0f));
                }

                m_rendererTypeList[modelMetadata["name"]] = "HeatmapRenderer";
                insertRenderer(modelMetadata["name"], someRenderer);
                //m_neuralNetworkList[modelMetadata["name"]] = network;

            } else if ((modelMetadata["problem"] == "segmentation") && (modelMetadata["resolution"] == "high")) {
                auto stitcher = PatchStitcher::New();
                stitcher->setInputConnection(network->getOutputPort());
                auto port = stitcher->getOutputPort();

                auto someRenderer = SegmentationPyramidRenderer::New();
                someRenderer->setOpacity(0.7f);
                someRenderer->setInputConnection(stitcher->getOutputPort());

                m_rendererTypeList[modelMetadata["name"]] = "SegmentationPyramidRenderer";
                insertRenderer(modelMetadata["name"], someRenderer);
            } else if ((modelMetadata["problem"] == "object_detection") && (modelMetadata["resolution"] == "high")) {  // TODO: Perhaps use switch() instead of tons of if-statements?

                // FIXME: Currently, need to do special handling for object detection as setThreshold and setAnchors only exist for BBNetwork and not NeuralNetwork
                auto generator = PatchGenerator::New();
                generator->setPatchSize(std::stoi(modelMetadata["input_img_size_y"]), std::stoi(modelMetadata["input_img_size_x"]));
                generator->setPatchLevel(patch_lvl_model);
                generator->setInputData(0, m_image);
                generator->setInputData(1, m_tissue);
                if (m_tumorMap)
                    generator->setInputData(1, m_tumorMap);

                auto currNetwork = BoundingBoxNetwork::New();
                currNetwork->setThreshold(0.1f); //0.01); // default: 0.5

                // read anchors from corresponding anchor file
                std::vector<std::vector<Vector2f> > anchors;
                std::ifstream infile(cwd + "data/Models/" + modelName + ".anchors");
                std::string anchorStr;
                while (std::getline(infile, anchorStr)) {
                    std::vector<std::string> anchorVector = split(anchorStr, " ");
                    anchorVector.resize(6); // for TinyYOLOv3 should only be 6 pairs, 3 for each level (2 levels)
                    int cntr = 0;
                    for (int i = 1; i < 3; i++) { // assumes TinyYOLOv3 (only two output layers)
                        std::vector<Vector2f> levelAnchors;
                        for (int j = 0; j < 3; j++) {
                            auto currentPair = split(anchorVector[cntr], ",");
                            levelAnchors.push_back(Vector2f(std::stoi(currentPair[0]), std::stoi(currentPair[1])));
                            cntr++;
                        }
                        anchors.push_back(levelAnchors);
                    }
                }
                currNetwork->setAnchors(anchors); // finally set anchors

                vector scale_factor = split(modelMetadata["scale_factor"], "/"); // get scale factor from metadata
                currNetwork->setScaleFactor((float) std::stoi(scale_factor[0]) / (float) std::stoi(scale_factor[1]));   // 1.0f/255.0f
                currNetwork->setInferenceEngine("OpenVINO"); // FIXME: OpenVINO only currently, as I haven't generalized multiple output nodes case
                currNetwork->load(cwd + "data/Models/" + modelName + "." + currNetwork->getInferenceEngine()->getDefaultFileExtension()); //".uff");
                currNetwork->setInputConnection(generator->getOutputPort());

                // FIXME: Bug when using NMS - ERROR [140237963507456] Terminated with unhandled exception: Size must be > 0, got: -49380162997889393559076864.000000 -96258.851562
                //auto nms = NonMaximumSuppression::New();
                //nms->setThreshold(0);
                //nms->setInputConnection(currNetwork->getOutputPort());
                
                auto boxAccum = BoundingBoxSetAccumulator::New();
                //boxAccum->setInputConnection(nms->getOutputPort());
                boxAccum->setInputConnection(currNetwork->getOutputPort());

                auto boxRenderer = BoundingBoxRenderer::New();
                boxRenderer->setInputConnection(boxAccum->getOutputPort());

                m_rendererTypeList[modelMetadata["name"]] = "BoundingBoxRenderer";
                insertRenderer(modelMetadata["name"], boxRenderer);
            } else if ((modelMetadata["problem"] == "segmentation") && (modelMetadata["resolution"] == "low")) {

                //auto converter = TensorToSegmentation::New();
                //converter->setInputConnection(network->getOutputPort());

                // resize back
                auto access = m_image->getAccess(ACCESS_READ);
                auto input = access->getLevelAsImage(currLvl);
                ImageResizer::pointer resizer2 = ImageResizer::New();
                //resizer2->setInputData(converter->updateAndGetOutputData<Image>());
                resizer2->setInputConnection(network->getOutputPort());
                resizer2->setWidth(input->getWidth());
                resizer2->setHeight(input->getHeight());
                auto port2 = resizer2->getOutputPort();
                resizer2->update();

                m_tumorMap = port2->getNextFrame<Image>();
                m_tumorMap->setSpacing((float) m_image->getFullHeight() / (float) input->getHeight(), (float) m_image->getFullWidth() / (float) input->getWidth(), 1.0f);

                auto someRenderer = SegmentationRenderer::New();
                someRenderer->setOpacity(0.4f);
                vector<string> colors = split(modelMetadata["class_colors"], ";");
                for (int i = 0; i < std::stoi(modelMetadata["nb_classes"]); i++) {
                    vector<string> rgb = split(colors[i], ",");
                    someRenderer->setColor(i, Color((float) std::stoi(rgb[0]) / 255.0f,
                                                           (float) std::stoi(rgb[1]) / 255.0f,
                                                           (float) std::stoi(rgb[2]) / 255.0f));
                }

                //someRenderer->setColor(Segmentation::LABEL_BACKGROUND, Color(0.0f, 255.0f / 255.0f, 0.0f));
                someRenderer->setInputData(m_tumorMap);
                //someRenderer->setInterpolation(false);
                someRenderer->update();

                m_rendererTypeList[modelMetadata["name"]] = "SegmentationRenderer";
                insertRenderer(modelMetadata["name"], someRenderer);

                // should kill network to free memory when finished
                //network->stopPipeline(); // TODO: Does nothing? Or at least I cannot see a different using "nvidia-smi"

                // add final segmentation to result list to be accessible if wanted for exporting
                //availableResults[modelMetadata["name"]] = m_tumorMap;
                //exportComboBox->addItem(tr(modelMetadata["name"].c_str()));
            }

            // now make it possible to edit prediction in the View Widget
            createDynamicViewWidget(modelMetadata["name"], modelName);

            // TODO: Save stitched heatmap as variable to be used later
            /*
            m_futureData = std::async(std::launch::async, [&, port]
             {
                // Wait for stitcher to finish before returning the data
                auto resultData = port->getNextFrame<Tensor>();
                while(true) {
                    if(resultData->isLastFrame())
                        break;
                    std::cout << "Not last frame, sleeping for 1 second" << std::endl;
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }
                std::cout << "Data is finished" << std::endl;
                // TODO Add the stuff you want to do here!
                auto converter = TensorToSegmentation::New();
                converter->setInputData(port->getNextFrame<Tensor>());

                m_gradeMap = converter->updateAndGetOutputData<Image>();

                saveGrade();
                //saveTissueSegmentation();

                return resultData;
            });
             */


            if (false) { //(modelMetadata["name"] == "tumor") {

                auto stitcher = PatchStitcher::New();
                stitcher->setInputConnection(network->getOutputPort());

                auto port = stitcher->getOutputPort();

                //  if (false) {
                stitcher->update();
                m_tumorMap_tensor = port->getNextFrame<Tensor>();

                auto tmp = TensorToSegmentation::New();
                tmp->setInputData(m_tumorMap_tensor);

                m_tumorMap = tmp->updateAndGetOutputData<Image>();

                // wait for process is finished, then save prediction as map
                /*
                std::promise<void> promise;
                std::future<void> future = promise.get_future();//std::future<int> out = std::async(heatmapRenderer); //ThreadPool.RegisterWaitForSingleObject();
                request();
                future.get();
                 */

                auto segTumorRenderer = SegmentationRenderer::New();
                segTumorRenderer->setOpacity(0.4f);
                segTumorRenderer->setColor(Segmentation::LABEL_FOREGROUND, Color(255.0 / 255.0f, 0.0f, 0.0f));
                segTumorRenderer->setColor(Segmentation::LABEL_BACKGROUND, Color(0.0f, 255.0f / 255.0f, 0.0f));
                segTumorRenderer->setInputData(m_tumorMap);
                //segTumorRenderer->setInterpolation(false);
                segTumorRenderer->update();

                m_rendererTypeList["tumorSeg"] = "SegmentationRenderer";
                insertRenderer("tumorSeg", segTumorRenderer);

                createDynamicViewWidget("tumorSeg", modelName);
            }
        }
    }
	return true;
}


std::map<std::string, std::string> MainWindow::getModelMetadata(std::string modelName) {
    // parse corresponding txt file for relevant information regarding model
    std::ifstream infile(cwd + "data/Models/" + modelName + ".txt");
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

std::vector<std::vector<Vector2f> > MainWindow::getAnchorMetadata(std::string anchorFileName) {
    std::ifstream infile(cwd + "data/Anchors/" + anchorFileName + ".txt");
    std::string key, value, str;
    std::string delimiter = ":";
    std::vector<std::vector<Vector2f> > currMetadata;
    while (std::getline(infile, str))
    {
        vector<string> v = split (str, delimiter);
        key = v[0];
        value = v[1];
        //currMetadata[key] = value;
    }
    return currMetadata;
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
    std::cout<<"\nCalculating histogram...\n";

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
    histBox->setAlignment(Qt::AlignHCenter);

    statsLayout->insertWidget(2, histBox); // finally, add histogram to Widget

    // add some text box that explains in words the result from the analysis or something similar...
    auto smallTextWindowStats = new QTextEdit;
    smallTextWindowStats->setPlainText(tr("This is some window with text that displays some relevant "
                                          "information regarding the inference or analysis you just did."));
    smallTextWindowStats->setReadOnly(true);
    smallTextWindowStats->show();
    smallTextWindowStats->setFixedHeight(smallTextWindowStats->document()->size().toSize().height() + 3);

    statsLayout->insertWidget(1, smallTextWindowStats); // FIXME: AlignTop doesn't work for setting element under hist? -> pushed to the bottom...
	
	return true;
}


bool MainWindow::toggleRenderer(std::string name) {
	std::cout << "Current name: " << name << std::endl;
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


bool MainWindow::opacityRenderer(int value, const std::string& someName) {
    if (m_rendererTypeList[someName] == "ImagePyramidRenderer") {
        return false;
    }
    if (!hasRenderer(someName)) {
        return false;
    }else{
        if (m_rendererTypeList[someName] == "SegmentationRenderer") {
            auto someRenderer = std::dynamic_pointer_cast<SegmentationRenderer>(getRenderer(someName));
            someRenderer->setOpacity((float) value / 20.0f);
            someRenderer->setModified(true);
        } else if (m_rendererTypeList[someName] == "SegmentationPyramidRenderer") { // FIXME: Apparently, this doesn't change opacity
            auto someRenderer = std::dynamic_pointer_cast<SegmentationPyramidRenderer>(getRenderer(someName));
            someRenderer->setOpacity((float) value / 20.0f);
            someRenderer->setModified(true);
        } else {
            auto someRenderer = std::dynamic_pointer_cast<HeatmapRenderer>(getRenderer(someName));
            someRenderer->setMaxOpacity((float) value / 20.0f);
            //someRenderer->setMinConfidence(0.9);
        }
        return true;
    }
}


bool MainWindow::hideChannel(const std::string& someName) {
    if (m_rendererTypeList[someName] != "HeatmapRenderer") {
        return false;
    }
    if (!hasRenderer(someName)) {
        return false;
    }else{
        background_flag = !background_flag;
        auto someRenderer = std::dynamic_pointer_cast<HeatmapRenderer>(getRenderer(someName));
        someRenderer->setChannelHidden(channel_value, background_flag);
        return true;
    }
}


void MainWindow::deleteViewObject(std::string someName) {
	if (m_rendererList.count(someName) == 0)
		return;
    std::cout << "\nCurrent renderer: " << someName << "\n";
    // need to remove from QComboBox, remove from renderer, remove potential saved results in project (need to check
    // if project is made first), and in the end, need to update ViewWidget
    //
    //removeRendererObject(someName); // FIXME: This produces bug... Why?

	std::cout << "\nCurrent renderer: " << m_rendererList[someName] << "\n";

	//view->removeRenderer(m_rendererList[someName]); //->resetRenderers();

	auto someRenderer = m_rendererList[someName];
	getView(0)->removeRenderer(someRenderer);
	for (auto const&[key, val] : m_rendererList) {
		std::cout << "before: " << key << ": " << val << std::endl;
	}
	m_rendererList.erase(someName);
	pageComboBox->removeItem(pageComboBox->findData(QString::fromStdString(someName))); //pageComboBox->currentIndex());


	for (auto const& [key, val] : m_rendererList) {
		std::cout << "after: " << key << ": " << val << std::endl;
	}

	// need to update QComboBox as an element has been removed, to still keep the renderers mapped by index without any holes
	pageComboBox->clear();
	//for (auto const&[key, val] : m_rendererList) {
	//	pageComboBox->addItem(tr(key.c_str()));
	//}

	// then clear dynamic view widget layout, and add them again
	clearLayout(stackedLayout);
	for (auto const&[key, val] : m_rendererList) {
		modelName = "";
		if (modelNames.count(key) != 0) {
			modelName = modelNames[key];
		}
		createDynamicViewWidget(key, modelName);
	}

	// TODO TOMORROW ANDREEE:
	//@TODO: Should store QComboBox based on name, perhaps setTooltip instead, such that I dont need to think about indeces.
	//	but this requires that all results have unique names. Perhaps introduce this "make random unique name" for every object
	//	that is rendered?

	// perhaps need to handle case when there is only one element left in the renderer
	std::cout << pageComboBox->count() << std::endl;
	if (pageComboBox->count() == 1) {
		pageComboBox->setCurrentIndex(0);
	}
	pageComboBox->update();

	std::cout << "\nGot further after removing..." << std::endl;

    // TODO: Need to remove thumbnail
    //pageComboBox->removeItem(pageComboBox->currentIndex());
    //scrollList->removeItemWidget(scrollList->selectedItems());  // TODO: Apparently, this doesnt work as expected
    //auto listItems = scrollList->selectedItems();

    /*
    foreach (QListWidgetItem *NAME, scrollList->selectedItems()) {
        delete scrollList->takeItem(scrollList->row(NAME));
    }

    QModelIndexList selectedList = scrollList->selectionModel()->selectedIndexes(); // take the list of selected indexes
    std::sort(selectedList.begin(),selectedList.end(),[](const QModelIndex& a, const QModelIndex& b)->bool{return a.row()>b.row();}); // sort from bottom to top
    for(const QModelIndex& singleIndex : selectedList)
        scrollList->model()->removeRow(singleIndex.row()); // remove each row

    QListWidgetItem *it = scrollList->takeItem(scrollList->currentRow());
    delete it;
      */
    //scrollList->removeItemWidget()
}


// FIXME: This does not work as intended... It seems to stop inference, but makes it not possible to run next inference
void MainWindow::killInference(std::string someName) {
	if (m_patchStitcherList.count(someName) != 0) {
		auto patchStitcher = m_patchStitcherList[someName];
		patchStitcher->stopPipeline(); // need to stop patchStitcher as well? FIXME: Doesn't help...
	}
    //auto neuralNetwork = m_neuralNetworkList[someName];
	//neuralNetwork->stopPipeline(); // TODO: Fix? I'm guessing the patchStitcher is then still waiting...
}


void MainWindow::removeRendererObject(std::string name) {
    if(m_rendererList.count(name) == 0)
        return;
    //return;  // TODO: added this for now, as it seems like its not possible to remove renderers...
    std::cout << "Renderer to be deleted: " << name << "\n";
    //auto renderer = std::dynamic_pointer_cast<ImagePyramidRenderer>(getRenderer(name));
    //auto renderer = m_rendererList[name];

	//m_rendererList[name]->setDisabled()

    //auto renderer = std::dynamic_pointer_cast<ImagePyramidRenderer>(m_rendererList[name]);
    //auto renderer2 = std::shared_ptr<Renderer>(std::dynamic_pointer_cast<Renderer>(renderer));
    //renderer->stopPipeline();
    //renderer->setDisabled(true);
    //renderer2->stopPipeline();
    //renderer2->reset();
    //m_rendererList.erase(name); // I added this, as it seemed like it should have been done
    //getView(0)->removeRenderer(m_rendererList[name]);  // FIXME: This crashed. Can you remove renderer that is still computing?
}


void MainWindow::insertRenderer(std::string name, std::shared_ptr<Renderer> renderer) {
    std::cout << "calling insert renderer" << std::endl;
    if(!hasRenderer(name)) {
        renderer->setSynchronizedRendering(false);
        // Doesn't exist
        getView(0)->addRenderer(renderer);
        //view->addRenderer(renderer);
        //renderer->setSynchronizedRendering(false);
        m_rendererList[name] = renderer;
        std::cout << "finished insert renderer" << std::endl;
    }
    //renderer->setSynchronizedRendering(false);
    //m_rendererList[name] = renderer;
    //std::cout << "finished insert renderer" << std::endl;
}


void MainWindow::removeAllRenderers() {
    m_rendererList.clear();
    getView(0)->removeAllRenderers();
}


bool MainWindow::hasRenderer(std::string name) {
    return m_rendererList.count(name) > 0;
}


std::shared_ptr<Renderer> MainWindow::getRenderer(std::string name) {
    if(!hasRenderer(name))
        throw Exception("Renderer with name " + name + " does not exist");
    return m_rendererList[name];
}



}
