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
#include <FAST/Algorithms/NeuralNetwork/TensorToSegmentation.hpp>
#include <FAST/Algorithms/NeuralNetwork/BoundingBoxNetwork.hpp>
#include <FAST/Visualization/BoundingBoxRenderer/BoundingBoxRenderer.hpp>
#include <FAST/Algorithms/Lambda/RunLambda.hpp>
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
#include <utility>
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
#include <FAST/Importers/HDF5TensorImporter.hpp>
#include <FAST/Exporters/HDF5TensorExporter.hpp>
#include <FAST/Importers/ImagePyramidPatchImporter.hpp>
#include <FAST/Exporters/ImagePyramidPatchExporter.hpp>
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
#include <QtNetwork>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkAccessManager>

using namespace std;

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
	std::string out;
	for (int i = 0; i < n; i++) {
		out.append(std::to_string(rand() % 10));
	}
	return out;
}

void MainWindow::receiveFileList(const QList<QString> &names) {
    selectFileDrag(names);
}


void MainWindow::createOpenGLWindow() {
    // initialize view
	view = createView();
    view->setSynchronizedRendering(false);
	view->set2DMode();
	view->setBackgroundColor(Color(OpenGL_background_color, OpenGL_background_color, OpenGL_background_color)); // setting color to the background, around the WSI
	view->setAutoUpdateCamera(true);

	// create QSplitter for adjustable windows
	mainSplitter = new QSplitter(Qt::Horizontal);
	mainSplitter->setHandleWidth(5);
	mainSplitter->setStyleSheet("QSplitter::handle { background-color: rgb(100, 100, 200); }; QMenuBar::handle { background-color: rgb(20, 100, 20); }");
	mainSplitter->addWidget(menuWidget);
	mainSplitter->addWidget(view);
	mainSplitter->setStretchFactor(1, 1);

    // finally, add widget to the main layout
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
                setTitle(applicationName + " (Research mode)" + " - " + splitCustom(filename, "/").back());
            } else {
                setTitle(applicationName + " - " + splitCustom(filename, "/").back());
            }
            break;
        case QMessageBox::No:
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

void MainWindow::downloadAndAddTestData() {
	// prompt
	auto mBox = new QMessageBox(mWidget);
	mBox->setIcon(QMessageBox::Warning);
	mBox->setText("This will download the test data, add the models, and open two WSIs.");
	mBox->setInformativeText("Are you sure you want to continue?");
	mBox->setDefaultButton(QMessageBox::Yes);
	mBox->setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
	int ret = mBox->exec();

	switch (ret) {
	case QMessageBox::Yes:
		// toggle and update text on button to show current mode (also default)
		break;
	case QMessageBox::No:
		// if "No", do nothing
		return;
	case QMessageBox::Cancel:
		// if "Cancel", do nothing
		return;
	default:
		break;
	}

	// progress bar
	auto progDialog = new QProgressDialog(mWidget);
	progDialog->setRange(0, 0);
	progDialog->setValue(0);
	progDialog->setVisible(true);
	progDialog->setModal(false);
	progDialog->setLabelText("Downloading test data...");
	progDialog->move(mWidget->width() - progDialog->width() * 1.1, progDialog->height() * 0.1);
	//m_pBar.show();

	QUrl url{"http://folk.ntnu.no/andpeder/FastPathology/test_data.zip"};

	QNetworkAccessManager* m_NetworkMngr = new QNetworkAccessManager(this);
	QNetworkReply *reply = m_NetworkMngr->get(QNetworkRequest(QUrl(url)));
	QEventLoop loop;

	QObject::connect(reply, &QNetworkReply::downloadProgress, [=](qint64 ist, qint64 max_) { //[=](qint64 ist, qint64 max) {
		progDialog->setRange(0, max_);
		progDialog->setValue(ist);
		//if (max < 0) hideProgress();
	});
	QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));

	loop.exec();
	QUrl aUrl(url);
	QFileInfo fileInfo = aUrl.path();

	QString downloadsFolder = QDir::homePath() + "/fastpathology/data";
	QFile file(downloadsFolder + "/" + fileInfo.fileName());
	file.open(QIODevice::WriteOnly);
	file.write(reply->readAll());
	file.close();
	delete reply;

	// unzip TODO: Is there something wrong with the include/import of this function? Might be a problem later on.
	extractZipFile((downloadsFolder + "/" + fileInfo.fileName()).toStdString(), downloadsFolder.toStdString());

	// OPTIONAL: Add Models to test if inference is working
	QList<QString> fileNames;
	QDirIterator it2(downloadsFolder + "/test_data/Models/", QDir::Files);
	while (it2.hasNext()) {
		auto tmp = it2.next();
		fileNames.push_back(tmp);
	}
	addModelsDrag(fileNames);

	auto mBox2 = new QMessageBox(mWidget);
	mBox2->setIcon(QMessageBox::Warning);
	mBox2->setText("Download is finished.");
	mBox2->setInformativeText("Do you wish to open the test WSIs?");
	mBox2->setDefaultButton(QMessageBox::Yes);
	mBox2->setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
	int ret2 = mBox2->exec();

	switch (ret2) {
	case QMessageBox::Yes:
		// toggle and update text on button to show current mode
		break;
	case QMessageBox::No:
		// if "No", do nothing (also default)
		return;
	case QMessageBox::Cancel:
		// if "Cancel", do nothing
		return;
	default:
		break;
	}

	// OPTIONAL: Add WSIs to project for visualization
	fileNames.clear();
	QDirIterator it(downloadsFolder + "/test_data/WSI/", QDir::Files);
	while (it.hasNext()) {
		auto tmp = it.next();
		fileNames.push_back(tmp);
	}
	selectFileDrag(fileNames);
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
	textBox->append("Author: André Pedersen");
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
	editMenu->addAction("Download test data", this, &MainWindow::downloadAndAddTestData);

    auto pipelineMenu = topFiller->addMenu(tr("&Pipelines"));
    pipelineMenu->addAction("Import pipelines", this, &MainWindow::addPipelines);
    pipelineMenu->addAction("Pipeline Editor", this, &MainWindow::customPipelineEditor);
    runPipelineMenu = new QMenu("&Run Pipeline", mWidget);
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
	//deployMenu->addAction("MTL nuclei seg/detect", this, &MainWindow::MTL_test);
    deployMenu->addAction("MIL bcgrade", this, &MainWindow::MIL_test);
    deployMenu->addAction("Deep KMeans MTL", this, &MainWindow::Kmeans_MTL_test);
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
        //runPipelineMenu->addAction(QString::fromStdString(splitCustom(currentFpl.toStdString(), ".")[0]), this, &MainWindow::lowresSegmenter);
        auto currentAction = runPipelineMenu->addAction(currentFpl); //QString::fromStdString(splitCustom(splitCustom(currentFpl.toStdString(), "/")[-1], ".")[0]));
        QObject::connect(currentAction, &QAction::triggered, std::bind(&MainWindow::runPipeline, this, cwd + "data/Pipelines/" + currentFpl.toStdString()));

        //auto currentAction = runPipelineMenu->addAction(QString::fromStdString(splitCustom(someFile, ".")[0]));
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
	QPixmap processPix(QString::fromStdString(":/data/Icons/process_icon_new_cropped_resized.png"));
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
        std::vector someVector = splitCustom(metadata["class_names"], ";");
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
		for (const auto & i : { 1 }) { //{ 0, 1 }) {  // TODO: Supports only binary images (where class of interest = 1)
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

            std::cout << "\nset color was pressed! (in SegmentationRenderer)" << std::endl;

			// TODO: Supports only binary images (where class of interest = 1)
			someRenderer->setColor(currComboBox->currentIndex() + 1, Color((float)(rgb.red() / 255.0f), (float)(rgb.green() / 255.0f), (float)(rgb.blue() / 255.0f)));
		});
	// TODO: Need to add proper options in SegmentationPyramidRenderer and SegmentationRenderer for toggling and setting which classes to show, do to this properly...
	} else if (m_rendererTypeList[someName] == "SegmentationPyramidRenderer") {
		// get metadata of current model
		std::map<std::string, std::string> metadata = getModelMetadata(modelName);
		std::vector someVector = splitCustom(metadata["class_names"], ";");
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
			auto someRenderer = std::dynamic_pointer_cast<SegmentationRenderer>(m_rendererList[someName]);
            std::cout << "\nset color was pressed!" << std::endl;
            someRenderer->setColor(currComboBox->currentIndex() + 1, Color((float)(rgb.red() / 255.0f), (float)(rgb.green() / 255.0f), (float)(rgb.blue() / 255.0f)));
			//someRenderer->setChannelColor(currComboBox->currentIndex(), Color((float)(rgb.red() / 255.0f), (float)(rgb.green() / 255.0f), (float)(rgb.blue() / 255.0f)));
		});
	} else if (m_rendererTypeList[someName] == "BoundingBoxRenderer") {
		// get metadata of current model
		std::map<std::string, std::string> metadata = getModelMetadata(modelName);
		std::vector someVector = splitCustom(metadata["class_names"], ";");
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
			//someRenderer->setLabelColor(currComboBox->currentIndex(), Color((float)(rgb.red() / 255.0f), (float)(rgb.green() / 255.0f), (float)(rgb.blue() / 255.0f)));
		});
	}
    /*
    else {
        simpleInfoPrompt("Invalid renderer used...");
	}
     */

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
		//colorSetWidget->setDisabled(true);
		//biggerTextBoxWidget_imageName->setDisabled(true);
	} else if (m_rendererTypeList[someName] == "BoundingBoxRenderer") {

	} else {
		colorSetWidget->setDisabled(false);
	}

	auto allBox = new QGroupBox(tr("Modify image"), mWidget);
	auto classBox = new QGroupBox(tr("Modify class"), mWidget);

    dynamicViewLayout = new QVBoxLayout;
    dynamicViewLayout->setAlignment(Qt::AlignTop);
    dynamicViewWidget->setLayout(dynamicViewLayout);

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

    auto backgroundLayout = new QVBoxLayout;

    scriptEditorWidget = new QDialog(mWidget);
    scriptEditorWidget->setWindowTitle("Script Editor");
    backgroundLayout->addWidget(scriptEditorWidget);

    scriptLayout = new QVBoxLayout;
    scriptEditorWidget->setLayout(scriptLayout);

    scriptEditor = new QPlainTextEdit;
    auto highlighter = new PipelineHighlighter(scriptEditor->document());
    const QFont fixedFont("UbuntuMono");
    scriptEditor->setFont(fixedFont);

    scriptLayout->insertWidget(1, scriptEditor);

    // center QDialog when opened
    QRect rect = scriptEditorWidget->geometry();
    QRect parentRect = mWidget->geometry();
    rect.moveTo(mWidget->mapToGlobal(QPoint(parentRect.x() + parentRect.width() - rect.width(), parentRect.y())));
	scriptEditorWidget->resize(600, 800);

    createActionsScript();
    scriptEditorWidget->show();
}

void MainWindow::createActionsScript() {

    auto menuBar = new QMenuBar(scriptEditorWidget);
    auto toolBar = new QToolBar();

    scriptLayout->setMenuBar(menuBar);

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
}

bool MainWindow::saveScript() {
    std::cout << "Saving...: " << currScript.toStdString() << std::endl;
    if (currScript.isEmpty()) {
        return saveAsScript();
    } else {
        std::cout << "We are saving, not save as..." << std::endl;
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
}

void MainWindow::setCurrentFileScript(const QString &fileName) {

    currScript = fileName;
    scriptEditor->document()->setModified(false);
    scriptEditor->setWindowModified(false);

    QString shownName = currScript;
    if (currScript.isEmpty())
        shownName = "untitled.txt";
    scriptEditor->setWindowFilePath(shownName);
}

void MainWindow::updateChannelValue(int index) {
    channel_value = (uint) index;
}

void MainWindow::createProcessWidget() {

    processLayout = new QVBoxLayout;
    processLayout->setSpacing(6);
    processLayout->setAlignment(Qt::AlignTop);

    processWidget = new QWidget(mWidget);
    processWidget->setLayout(processLayout);

    auto segTissueButton = new QPushButton(mWidget);
    segTissueButton->setText("Segment Tissue");
    //segTissueButton->setFixedWidth(200);
    segTissueButton->setFixedHeight(50);
    QObject::connect(segTissueButton, &QPushButton::clicked, std::bind(&MainWindow::segmentTissue, this));

    // add tissue segmentation to widget (should always exist as built-in FAST)
    processLayout->insertWidget(0, segTissueButton);

    QDir directory(QString::fromStdString(cwd + "data/Models/"));
    QStringList paths = directory.entryList(QStringList() << "*.txt" << "*.TXT",QDir::Files);
    int counter = 1;
    foreach(QString currFile, paths) {

        // current model
        modelName = splitCustom(currFile.toStdString(), ".")[0];

        // get metadata of current model
        std::map<std::string, std::string> metadata = getModelMetadata(modelName);

        auto someButton = new QPushButton(mWidget);//(exportWidget);
        someButton->setText(QString::fromStdString(metadata["task"]));
        //predGradeButton->setFixedWidth(200);
        someButton->setFixedHeight(50);
        QObject::connect(someButton, &QPushButton::clicked, std::bind(&MainWindow::pixelClassifier_wrapper, this, modelName));
        someButton->show();

        processLayout->insertWidget(counter, someButton);

        counter++;
    }

    std::vector<string> modelPaths;
}

void MainWindow::createStatsWidget() {

    statsLayout = new QVBoxLayout;
    statsLayout->setAlignment(Qt::AlignTop);

    statsWidget = new QWidget(mWidget);
    statsWidget->setLayout(statsLayout);

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

    QStringList itemsInComboBox;
    for (int index = 0; index < pageComboBox->count(); index++) {
        std::cout << "some item: " << pageComboBox->itemText(index).toStdString() << std::endl;
        itemsInComboBox << pageComboBox->itemText(index);
    }

    auto saveThumbnailButton = new QPushButton(mWidget);
    saveThumbnailButton->setText("Save thumbnail");
    saveThumbnailButton->setFixedHeight(50);
    QObject::connect(saveThumbnailButton, &QPushButton::clicked, std::bind(&MainWindow::saveThumbnail, this));

    auto saveTissueButton = new QPushButton(mWidget);
    saveTissueButton->setText("Save tissue mask");
    saveTissueButton->setFixedHeight(50);
    QObject::connect(saveTissueButton, &QPushButton::clicked, std::bind(&MainWindow::saveTissueSegmentation, this));

    auto saveTumorButton = new QPushButton(mWidget);
    saveTumorButton->setText("Save tumor mask");
    saveTumorButton->setFixedHeight(50);
    QObject::connect(saveTumorButton, &QPushButton::clicked, std::bind(&MainWindow::saveTumor, this));

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
}

void MainWindow::createDynamicExportWidget(const std::string& someName) {
    /* Unimplemented at the moment */
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

	QCoreApplication::processEvents(QEventLoop::AllEvents, 0);

	auto counter = 0;
	for (const auto& currWSI : currentWSIs) {

		std::cout << "current WSI: " << currWSI << std::endl;
		auto access = m_image->getAccess(ACCESS_READ);
		auto input = access->getLevelAsImage(m_image->getNrOfLevels() - 1);
		if (m_runForProject) {
			auto someImporter = WholeSlideImageImporter::New();
            someImporter->setFilename(currWSI);
			auto currImage = someImporter->updateAndGetOutputData<ImagePyramid>();

			access = currImage->getAccess(ACCESS_READ);
			input = access->getLevelAsImage(currImage->getNrOfLevels() - 1);
		}

		// TODO: if only large image planes exist, should downsample the resulting thumbnails before export

		// attempt to save thumbnail to disk as .png
		ImageExporter::pointer exporter = ImageExporter::New();
		exporter->setFilename(projectFolderName.toStdString() + "/thumbnails/" + splitCustom(splitCustom(currWSI, "/").back(), ".")[0] + ".png");
		std::cout << "Name: " << projectFolderName.toStdString() + "/thumbnails/" + splitCustom(splitCustom(currWSI, "/").back(), ".")[0] + ".png" << std::endl;
		exporter->setInputData(input);
		exporter->update();

		// update progress bar
		progDialog.setValue(counter);
		QCoreApplication::processEvents(QEventLoop::AllEvents, 0);
        counter++;
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
    progDialog.setValue(0);
	progDialog.setVisible(true);
	progDialog.setModal(false);
	progDialog.setLabelText("Saving tissue segmentations...");
	//QRect screenrect = mWidget->screen()[0].geometry();
	progDialog.move(mWidget->width() - progDialog.width() * 1.1, progDialog.height() * 0.1);
	progDialog.show();

	QCoreApplication::processEvents(QEventLoop::AllEvents, 0);

	auto counter = 0;
	for (const auto& currWSI : currentWSIs) {

		// check if folder for current WSI exists, if not, create one
		QString wsiResultPath = (projectFolderName.toStdString() + "/results/" + splitCustom(splitCustom(currWSI, "/").back(), ".")[0] + "/").c_str();
		wsiResultPath = wsiResultPath.replace("//", "/");
		if (!QDir(wsiResultPath).exists()) {
			QDir().mkdir(wsiResultPath);
		}

        auto someImporter = WholeSlideImageImporter::New();
        someImporter->setFilename(currWSI);
        auto currImage = someImporter->updateAndGetOutputData<ImagePyramid>();

        auto tissueSegmentation = TissueSegmentation::New();
        tissueSegmentation->setInputData(currImage);

        QString outFile = (wsiResultPath.toStdString() + splitCustom(splitCustom(currWSI, "/").back(), ".")[0] + "_tissue.png").c_str();
        ImageExporter::pointer exporter = ImageExporter::New();
        exporter->setFilename(outFile.replace("//", "/").toStdString());
        exporter->setInputData(tissueSegmentation->updateAndGetOutputData<Image>());
        exporter->update();

		// update progress bar
		progDialog.setValue(counter);
		counter++;

		// to render straight away (avoid waiting on all WSIs to be handled before rendering)
		QCoreApplication::processEvents(QEventLoop::AllEvents, 0);
	}
}

void MainWindow::saveHeatmap() {

	// check if folder for current WSI exists, if not, create one
	QString wsiResultPath = (projectFolderName.toStdString() + "/results/" + splitCustom(splitCustom(filename, "/").back(), ".")[0] + "/").c_str();
	wsiResultPath = wsiResultPath.replace("//", "/");
	if (!QDir(wsiResultPath).exists()) {
		QDir().mkdir(wsiResultPath);
	}

	auto exporter = HDF5TensorExporter::New();
	exporter->setFilename(wsiResultPath.toStdString() + "tensor.h5");
	//exporter->setInputData(tensor);
	exporter->update();
}

void MainWindow::saveTumor() {
    // check if folder for current WSI exists, if not, create one
    QString wsiResultPath = (projectFolderName.toStdString() + "/results/" + splitCustom(splitCustom(filename, "/").back(), ".")[0] + "/").c_str();
    wsiResultPath = wsiResultPath.replace("//", "/");
    if (!QDir(wsiResultPath).exists()) {
        QDir().mkdir(wsiResultPath);
    }

    if (std::find(savedList.begin(), savedList.end(), "tumorSeg_lr") != savedList.end()) {
        simpleInfoPrompt("Result has already previously been saved.");
        return;
    }
    savedList.emplace_back("tumorSeg_lr");

    // attempt to save tissue mask to disk as .png
    QString outFile = (wsiResultPath.toStdString() + splitCustom(splitCustom(filename, "/").back(), ".")[0] + "_tumor_mask.png").c_str();

    ImageExporter::pointer exporter = ImageExporter::New();
    exporter->setFilename(outFile.replace("//", "/").toStdString());
    //exporter->setInputData(intensityScaler->updateAndGetOutputData<Image>());
    exporter->setInputData(m_tumorMap);
    exporter->update();

    simpleInfoPrompt("Tumor segmentation has been saved.");
}

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

    // TODO: Unable to read .zvi and .scn (Zeiss and Leica). I'm wondering if they are stored in some unexpected way (not image pyramids)
    auto fileNames = QFileDialog::getOpenFileNames(
        mWidget,
        tr("Select File(s)"), nullptr, tr("WSI Files (*.tiff *.tif *.svs *.ndpi *.bif *vms)"),  //*.zvi *.scn)"),
        nullptr, QFileDialog::DontUseNativeDialog
    );
	
	// return if the file dialog was cancelled without any files being selected
	if (fileNames.count() == 0) {
		return;
	}

	// for a new selection of wsi(s), should reset and update these QWidgets
	pageComboBox->clear();
	exportComboBox->clear();
    m_rendererList.clear();

    auto progDialog = QProgressDialog(mWidget);
    progDialog.setRange(0, fileNames.count()-1);
	//progDialog.setContentsMargins(0, 0, 0, 0);
    progDialog.setVisible(true);
    progDialog.setModal(false);
    progDialog.setLabelText("Loading WSIs...");
    //QRect screenrect = mWidget->screen()[0].geometry();
	progDialog.move(mWidget->width() - progDialog.width() * 1.1, progDialog.height() * 0.1);
    progDialog.show();

	QCoreApplication::processEvents(QEventLoop::AllEvents, 0);
	auto currentPosition = curr_pos;

    // need to handle scenario where a WSI is added, but there already exists N WSIs from before
    auto nb_wsis_in_list = wsiList.size();
    if (nb_wsis_in_list != 0)
        currentPosition = nb_wsis_in_list;

    if (m_doneFirstWSI) {
        // Stop any pipelines running in old view and delete it!
        currentView->stopPipeline();
        delete currentView;
    }

    // Get old view, and remove it from Widget
    currentView = getView(0);
    currentView->setSynchronizedRendering(false);  // Disable synchronized rendering
    mWidget->clearViews();

    auto tmpView = createView();
    tmpView->setSynchronizedRendering(false);
    tmpView->set2DMode();
    tmpView->setBackgroundColor(Color(OpenGL_background_color, OpenGL_background_color, OpenGL_background_color)); // setting color to the background, around the WSI
    tmpView->setAutoUpdateCamera(true);

    mainSplitter->replaceWidget(1, tmpView);
    mainSplitter->setStretchFactor(1, 1);

    mWidget->addView(tmpView); // Give new view to mWidget so it is used in the computation thread

    int counter = 0;
    for (QString& fileName : fileNames) {
        if (fileName == "")
            return;
        auto currFileName = fileName.toStdString();
        std::cout << "Selected file: " << currFileName << std::endl;
        wsiList.push_back(currFileName);

        // Import image from file using the ImageFileImporter
        auto importer = WholeSlideImageImporter::New();
        importer->setFilename(currFileName);
        auto currImage = importer->updateAndGetOutputData<ImagePyramid>();

        // for reading of multiple WSIs, only render first one
        if (counter == 0) { //fileNames.count()-1) {

            // current WSI (global)
            filename = currFileName;

            m_image = currImage;
            std::cout << "count:" << counter << std::endl;
            metadata = m_image->getMetadata(); // get metadata

            auto renderer = ImagePyramidRenderer::New();
            renderer->setSharpening(m_wsiSharpening);
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
            if (advancedMode) {
                setTitle(applicationName + " (Research mode)" + " - " + splitCustom(currFileName, "/").back());
            } else {
                setTitle(applicationName + " - " + splitCustom(currFileName, "/").back());
            }
        }
        counter ++;

        // Create thumbnail image
        // TODO: This is a little bit slow. Possible to speed it up? Bottleneck is probably the creation of thumbnails
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
		button->setToolTip(QString::fromStdString(splitCustom(currFileName, "/").back()));

        auto listItem = new QListWidgetItem;
        listItem->setSizeHint(QSize(width_val, height_val));
        QObject::connect(button, &QPushButton::clicked, std::bind(&MainWindow::selectFileInProject, this, currentPosition));
        scrollList->addItem(listItem);
        scrollList->setItemWidget(listItem, button);

        //curr_pos++; // this should change if we render the first WSI when loading
		currentPosition++;

        // update progress bar
        progDialog.setValue(counter);

        // to render straight away (avoid waiting on all WSIs to be handled before rendering)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 0);
    }

    // update flag only if first
    if (!m_doneFirstWSI)
        m_doneFirstWSI = true;  // then this should never happen again
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
                std::cout << "Results not saved yet. Just cancelled the switch!" << std::endl;
                // Save was clicked
                return;
            case QMessageBox::Discard:
                // Don't Save was clicked
                std::cout << "Discarded!" << std::endl;
                break;
            case QMessageBox::Cancel:
                // Cancel was clicked
                std::cout << "Cancelled!" << std::endl;
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

	QCoreApplication::processEvents(QEventLoop::AllEvents, 0);

    int counter = 0;
    for (const QString& fileName : fileNames) {

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

            auto renderer = ImagePyramidRenderer::New();
            renderer->setSharpening(m_wsiSharpening);
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
            //setTitle(applicationName + " - " + splitCustom(filename, "/").back());
            if (advancedMode) {
                setTitle(applicationName + " (Research mode)" + " - " + splitCustom(currFileName, "/").back());
            } else {
                setTitle(applicationName + " - " + splitCustom(currFileName, "/").back());
            }
        }
        counter ++;

        // TODO: This is a little bit slow. Possible to speed it up? Bottleneck is probably the creation of thumbnails
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

        progDialog.setValue(counter);
        QCoreApplication::processEvents(QEventLoop::AllEvents, 0);
    }
}

void MainWindow::selectFileInProject(int pos) {

    // if you select a WSI and it's already open, do nothing
    std::cout << "CurrentPos: " << pos << std::endl;
    std::cout << "Length of wsiList: " << std::to_string(wsiList.size()) << std::endl;
    if (filename == wsiList[pos]) {
        simpleInfoPrompt("WSI is already open");
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
			std::cout << "Yes was pressed." << std::endl;
			saveTissueSegmentation(); // TODO: Need to generalize this. Check which results exists, and save all sequentially. Yes, should have a save current results method
			break;
		case QMessageBox::No:
			std::cout << "No was pressed." << std::endl;
			// remove results of current WSI
			removeAllRenderers();
			pageComboBox->clear();
			exportComboBox->clear();
			break;
		case QMessageBox::Cancel:
			std::cout << "Cancel was pressed." << std::endl;
			return;
		default:
			std::cout << "Default was pressed." << std::endl;
			break;
		}
	}

	// remove results of current WSI
	savedList.clear();
	pageComboBox->clear();
	exportComboBox->clear();
	m_rendererList.clear();
	m_rendererTypeList.clear();
	clearLayout(stackedLayout);

    // Stop any pipelines running in old view and delete it!
    currentView->stopPipeline();
    delete currentView;

    // Get old view, and remove it from Widget
    currentView = getView(0);
    currentView->setSynchronizedRendering(false);  // Disable synchronized rendering
    mWidget->clearViews();

    auto tmpView = createView();
    tmpView->setSynchronizedRendering(false);
    tmpView->set2DMode();
    tmpView->setBackgroundColor(Color(OpenGL_background_color, OpenGL_background_color, OpenGL_background_color)); // setting color to the background, around the WSI
    tmpView->setAutoUpdateCamera(true);

    mainSplitter->replaceWidget(1, tmpView);
    mainSplitter->setStretchFactor(1, 1);

    mWidget->addView(tmpView); // Give new view to mWidget so it is used in the computation thread

    removeAllRenderers();  // VERY IMPORTANT THAT THIS IS DONE AFTER!!!

	// add WSI to project list
	filename = wsiList[pos];

	//stopComputationThread();
	// Import image from file using the ImageFileImporter
	importer = WholeSlideImageImporter::New();
	std::cout << "Current filename: " << filename << std::endl;
	importer->setFilename(filename);
	m_image = importer->updateAndGetOutputData<ImagePyramid>();

	// get metadata
	metadata = m_image->getMetadata();

	auto renderer = ImagePyramidRenderer::New();
    renderer->setSharpening(m_wsiSharpening);
	renderer->setInputData(m_image);

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

	// check if any results exists for current WSI, if there are load them
	std::string wsiPath = splitCustom(splitCustom(filename, "/").back(), ".")[0];
	auto currentResultPath = projectFolderName.toStdString() + "/results/" + wsiPath.c_str();

	QDirIterator iter(QString::fromStdString(currentResultPath));
	std::cout << "Current result folder path: " << currentResultPath << std::endl;
	while (iter.hasNext()) {
		auto currentResult = iter.next().toStdString();

        auto tmp = splitCustom(currentResult, "/").back();
        if ((tmp == ".") || (tmp == ".."))
            continue;

        // check if current "file" is a directory, if directly, it will assume that there exists some high-res seg results to render, else do other stuff
        QFileInfo pathFileInfo(currentResult.c_str());
        if (pathFileInfo.isDir()){
            if (!QDir(currentResult.c_str()).isEmpty()) {
                loadHighres(QString::fromStdString(currentResult), QString::fromStdString(splitCustom(splitCustom(currentResult, "/").back(), wsiPath + "_").back()));
            } else {
                simpleInfoPrompt("Project directory containing high-resolution result was empty.");
            }
        } else {
            auto splits = splitCustom(splitCustom(currentResult.c_str(), "/").back(), ".");
            std::cout << "Current result path: " << currentResult << std::endl;

            auto str = splits.back();
            transform(str.begin(), str.end(), str.begin(), ::tolower);
            if (str == "png") {
                loadSegmentation(QString::fromStdString(currentResult), QString::fromStdString(
                        splitCustom(splitCustom(splitCustom(currentResult, "/").back(), ".")[0], wsiPath + "_").back()));
            } else if ((str == "h5") || (str == "hd5") || (str == "hdf5")) {
                std::cout << "heatmap chosen: " << str << std::endl;
                loadHeatmap(QString::fromStdString(currentResult), QString::fromStdString(
                        splitCustom(splitCustom(splitCustom(currentResult, "/").back(), ".")[0], wsiPath + "_").back()));
            } else {
                std::cout << "Unable to load result (format is not supported): " << currentResultPath << std::endl;
            }
        }
	}

    // update application name to contain current WSI
    if (advancedMode) {
        setTitle(applicationName + " (Research mode)" + " - " + splitCustom(filename, "/").back());
    } else {
        setTitle(applicationName + " - " + splitCustom(filename, "/").back());
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
                std::cout << "Saved!" << std::endl;
                saveProject();
                break;
            case QMessageBox::No:
                std::cout << "Removing WSIs from QListWidget!" << std::endl;
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
    projectFolderName = splitCustom(projectPath.toStdString(), "project.txt")[0].c_str();
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
        simpleInfoPrompt("There was found no valid WSI in the project file.");
    }

    auto progDialog = QProgressDialog(mWidget);
    progDialog.setRange(0, fileNames.count()-1);
    //progDialog.setContentsMargins(0, 0, 0, 0);
    progDialog.setVisible(true);
    progDialog.setModal(false);
    progDialog.setLabelText("Loading WSIs...");
    //QRect screenrect = mWidget->screen()[0].geometry();
    progDialog.move(mWidget->width() - progDialog.width() * 1.1, progDialog.height() * 0.1);
    progDialog.show();

    QCoreApplication::processEvents(QEventLoop::AllEvents, 0);

    int counter = 0;
    for (QString &fileName : fileNames) {

        if (fileName == "")
            return;
        filename = fileName.toStdString();
        wsiList.push_back(filename);

        // Import image from file using the ImageFileImporter
        auto someImporter = WholeSlideImageImporter::New();
        someImporter->setFilename(fileName.toStdString());
        m_image = someImporter->updateAndGetOutputData<ImagePyramid>();

        // for reading of multiple WSIs, only render last one
        if (counter == fileNames.count() - 1) {

            // get metadata
            metadata = m_image->getMetadata();

            auto renderer = ImagePyramidRenderer::New();
            renderer->setSharpening(m_wsiSharpening);
            renderer->setInputData(m_image);

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

			// check if any results exists for current WSI, if there are load them
			std::string wsiPath = splitCustom(splitCustom(filename, "/").back(), ".")[0];
			auto currentResultPath = projectFolderName.toStdString() + "/results/" + wsiPath.c_str();

			// update title to include name of current file
            if (advancedMode) {
                setTitle(applicationName + " (Research mode)" + " - " + splitCustom(filename, "/").back());
            } else {
                setTitle(applicationName + " - " + splitCustom(filename, "/").back());
            }

			std::cout << "Current WSI used: " << fileName.toStdString() << std::endl;

			QDirIterator iter(QString::fromStdString(currentResultPath));
			std::cout << "Current result folder path: " << currentResultPath << std::endl;
			while (iter.hasNext()) {
				auto currentResult = iter.next().toStdString();

				std::cout << "current file: " << currentResult << std::endl;

				auto tmp = splitCustom(currentResult, "/").back();

				if ((tmp == ".") || (tmp == ".."))
				    continue;

				// check if current "file" is a directory, if directly, it will assume that there exists some high-res seg results to render, else do other stuff
                QFileInfo pathFileInfo(currentResult.c_str());
                if (pathFileInfo.isDir()){
                    if (!QDir(currentResult.c_str()).isEmpty()) {
                        loadHighres(QString::fromStdString(currentResult), QString::fromStdString(splitCustom(splitCustom(currentResult, "/").back(), wsiPath + "_").back()));
                    } else {
                        simpleInfoPrompt("Project directory containing high-resolution result was empty.");
                    }
                } else {
                    auto splits = splitCustom(splitCustom(currentResult.c_str(), "/").back(), ".");
                    std::cout << "Current result path: " << currentResult << std::endl;

                    auto str = splits.back();
                    transform(str.begin(), str.end(), str.begin(), ::tolower);
                    if (str == "png") {
                        // @TODO: this splitception will be handled better in the future :p
                        loadSegmentation(QString::fromStdString(currentResult), QString::fromStdString(splitCustom(splitCustom(splitCustom(currentResult, "/").back(), splitCustom(splitCustom(fileName.toStdString(), "/").back(), ".")[0] + "_").back(), ".")[0]));
                    } else if ((str == "h5") || (str == "hd5") || (str == "hdf5")) {
                        loadHeatmap(QString::fromStdString(currentResult), QString::fromStdString(splitCustom(splitCustom(splitCustom(currentResult, "/").back(), ".")[0], wsiPath + "_").back()));
                    } else {
                        std::cout << "Unable to load result (format is not supported): " << currentResultPath << std::endl;
                    }
                }
			}
        }
        counter++;

        // check if thumbnail image exists for current WSI, if yes load it, else extract one
        std::string wsiPath = splitCustom(splitCustom(filename, "/").back(), ".")[0];
        QString fullPath = projectFolderName + "/thumbnails/" + wsiPath.c_str();
        fullPath = fullPath.replace("//", "/") + ".png";

        // try to convert FAST Image -> QImage
        QImage image;
        std::cout << "Path: " << fullPath.toStdString() << std::endl;
        if (QDir().exists(fullPath)) {
            std::cout << "Thumbnail exists! Load it project folder" << std::endl;
            image.load(fullPath);
        } else {
            std::cout << "Thumbnail does not exist! Creating one from the WSI" << std::endl;
            // get thumbnail image
            image = extractThumbnail();
        }

        // /*
        auto button = new QPushButton();
        auto m_NewPixMap = QPixmap::fromImage(image);
        QIcon ButtonIcon(m_NewPixMap);
        button->setIcon(ButtonIcon);
        //button->setCheckable(true);
        button->setAutoDefault(true);
        //button->setStyleSheet("border: none, padding: 0, background: none");
        //button->setIconSize(QSize(200, 200));
        button->setToolTip(QString::fromStdString(wsiPath));
        int height_val = 150;
        button->setIconSize(QSize(height_val, (int) std::round(
                (float) image.width() * (float) height_val / (float) image.height())));

        auto listItem = new QListWidgetItem;
        listItem->setToolTip(QString::fromStdString(wsiPath));
        listItem->setSizeHint(
                QSize((int) std::round((float) image.width() * (float) height_val / (float) image.height()),
                      height_val));
        QObject::connect(button, &QPushButton::clicked, std::bind(&MainWindow::selectFileInProject, this, curr_pos));
        scrollList->addItem(listItem);
        scrollList->setItemWidget(listItem, button);

        curr_pos++;

        // update progress bar
        progDialog.setValue(counter);
        QCoreApplication::processEvents(QEventLoop::AllEvents, 0);
    }
}

void MainWindow::simpleInfoPrompt(const QString& str) {
    // prompt if no valid WSI was found in project-file
    auto mBox = new QMessageBox(mWidget);
    mBox->setText(str);
    mBox->setIcon(QMessageBox::Information);
    mBox->setModal(false);
    QRect screenrect = mWidget->screen()[0].geometry();
    mBox->move(mWidget->width() - mBox->width() / 2, -mWidget->width() / 2 - mBox->width() / 2);
    mBox->show();
    QTimer::singleShot(3000, mBox, SLOT(accept()));
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
                    pixelData[i * 4 + (2-c)] = (unsigned char) data;  // TODO: NOTE (2-c) fixed BGR->RGB, but maybe there is a smarter solution?
					//pixelData[i * 4 + c] = (unsigned char)data;  // TODO: NOTE (2-c) fixed BGR->RGB, but maybe there is a smarter solution?
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
	});

	QObject::connect(applyButton, &QPushButton::clicked, [=]() {
		std::cout << "Apply was clicked." << std::endl;
		m_runForProjectWsis.clear();
		for (int row = 0; row < selectedFilesWidget->count(); row++) {
			m_runForProjectWsis.push_back(selectedFilesWidget->item(row)->text().toStdString());
		}
		projectDialog->accept();
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

        std::string someFile = splitCustom(fileName.toStdString(), "/").back();
        std::string oldLocation = splitCustom(fileName.toStdString(), someFile)[0];
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
        //runPipelineMenu->addAction(QString::fromStdString(splitCustom(someFile, ".")[0]), this, &MainWindow::runPipeline);
        //auto currentAction = new QAction(QString::fromStdString(splitCustom(someFile, ".")[0]));
        auto currentAction = runPipelineMenu->addAction(QString::fromStdString(splitCustom(someFile, ".")[0]));
        QObject::connect(currentAction, &QAction::triggered, std::bind(&MainWindow::runPipeline, this, someFile));

        //auto currentAction = runPipelineMenu->addAction(currentFpl); //QString::fromStdString(splitCustom(splitCustom(currentFpl.toStdString(), "/")[-1], ".")[0]));
        //QObject::connect(currentAction, &QAction::triggered, std::bind(&MainWindow::runPipeline, this, cwd + "data/Pipelines/" + currentFpl.toStdString()));
    }
}

void MainWindow::addModelsDrag(const QList<QString> &fileNames) {

	// if Models/ folder doesnt exist, create it
	QDir().mkpath(QDir::homePath() + "fastpathology/data/Models");

	auto progDialog = QProgressDialog(mWidget);
	progDialog.setRange(0, fileNames.count() - 1);
	progDialog.setVisible(true);
	progDialog.setModal(false);
	progDialog.setLabelText("Adding models...");
	QRect screenrect = mWidget->screen()[0].geometry();
	progDialog.move(mWidget->width() - progDialog.width() / 2, -mWidget->width() / 2 - progDialog.width() / 2);
	progDialog.show();

	QCoreApplication::processEvents(QEventLoop::AllEvents, 0);

	int counter = 0;
	// now iterate over all selected files and add selected files and corresponding ones to Models/
	for (const QString& fileName : fileNames) {
		std::cout << fileName.toStdString() << std::endl;

		if (fileName == "")
			return;

		std::string someFile = splitCustom(fileName.toStdString(), "/").back(); // TODO: Need to make this only split on last "/"
		std::string oldLocation = splitCustom(fileName.toStdString(), someFile)[0];
		std::string newLocation = cwd + "data/Models/";

		std::vector<string> names = splitCustom(someFile, ".");
		string fileNameNoFormat = names[0];
		string formatName = names[1];

		// copy selected file to Models folder
		// check if file already exists in new folder, if yes, print warning, and continue to next one
		string newPath = cwd + "data/Models/" + someFile;
		if (fileExists(newPath)) {
			std::cout << "file with the same name already exists in folder, didn't transfer... " << std::endl;
			progDialog.setValue(counter);
			counter++;
            QCoreApplication::processEvents(QEventLoop::AllEvents, 0);
		} else {
            //QFile::copy(fileName, QString::fromStdString(newPath));

            // check which corresponding model files that exist, except from the one that is chosen
            std::vector<std::string> allowedFileFormats{"txt", "pb", "h5", "mapping", "xml", "bin", "uff", "anchors", "onnx"};

            foreach(std::string currExtension, allowedFileFormats) {
                std::string oldPath = oldLocation + fileNameNoFormat + "." + currExtension;
                if (fileExists(oldPath)) {
                    QFile::copy(QString::fromStdString(oldPath), QString::fromStdString(
                            cwd + "data/Models/" + fileNameNoFormat + "." + currExtension));
                }
            }

            // when models are added, ProcessWidget should be updated by adding the new widget to ProcessWidget layout
            // current model
            modelName = fileNameNoFormat;

            // get metadata of current model
            std::map<std::string, std::string> currMetadata = getModelMetadata(modelName);

            auto someButton = new QPushButton(mWidget);
            someButton->setText(QString::fromStdString(currMetadata["task"]));
            //predGradeButton->setFixedWidth(200);
            someButton->setFixedHeight(50);
            QObject::connect(someButton, &QPushButton::clicked,
                             std::bind(&MainWindow::pixelClassifier_wrapper, this, modelName));
            someButton->show();

            processLayout->insertWidget(processLayout->count(), someButton);

            progDialog.setValue(counter);
            QCoreApplication::processEvents(QEventLoop::AllEvents, 0);
            counter++;
        }
	}
}

void MainWindow::addModels() {

    //QString fileName = QFileDialog::getOpenFileName(
    QStringList ls = QFileDialog::getOpenFileNames(
            mWidget,
            tr("Select Model"), nullptr,
            tr("Model Files (*.pb *.txt *.h5 *.xml *.mapping *.bin *.uff *.anchors *.onnx *.fpl"),
            nullptr, QFileDialog::DontUseNativeDialog
    ); // TODO: DontUseNativeDialog - this was necessary because I got wrong paths -> /run/user/1000/.../filename instead of actual path

	auto progDialog = QProgressDialog(mWidget);
	progDialog.setRange(0, ls.count() - 1);
	progDialog.setVisible(true);
	progDialog.setModal(false);
	progDialog.setLabelText("Adding models...");
	QRect screenrect = mWidget->screen()[0].geometry();
	progDialog.move(mWidget->width() - progDialog.width() / 2, -mWidget->width() / 2 - progDialog.width() / 2);
	progDialog.show();

	QCoreApplication::processEvents(QEventLoop::AllEvents, 0);

	int counter = 0;
    // now iterate over all selected files and add selected files and corresponding ones to Models/
    for (QString& fileName : ls) {

        if (fileName == "")
            return;

        std::string someFile = splitCustom(fileName.toStdString(), "/").back(); // TODO: Need to make this only split on last "/"
        std::string oldLocation = splitCustom(fileName.toStdString(), someFile)[0];
        std::string newLocation = cwd + "data/Models/";

        std::vector<string> names = splitCustom(someFile, ".");
        string fileNameNoFormat = names[0];
        string formatName = names[1];

        // copy selected file to Models folder
        // check if file already exists in new folder, if yes, print warning, and stop
        string newPath = cwd + "data/Models/" + someFile;
        if (fileExists(newPath)) {
            std::cout << "file with the same name already exists in folder, didn't transfer... " << std::endl;
			progDialog.setValue(counter);
			counter++;
            continue;
        }

        // check which corresponding model files that exist, except from the one that is chosen
        std::vector<std::string> allowedFileFormats{"txt", "pb", "h5", "mapping", "xml", "bin", "uff", "anchors", "onnx", "fpl"};

        std::cout << "Copy test" << std::endl;
        foreach(std::string currExtension, allowedFileFormats) {
            std::string oldPath = oldLocation + fileNameNoFormat + "." + currExtension;
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
                         std::bind(&MainWindow::pixelClassifier_wrapper, this, modelName));
        someButton->show();

        processLayout->insertWidget(processLayout->count(), someButton);

		progDialog.setValue(counter);
        QCoreApplication::processEvents(QEventLoop::AllEvents, 0);
		counter++;
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
        std::cout << "WSI format not set, uses default format: " << metadata["aperio.AppMag"] << std::endl;
        magnification_lvl = std::stof(metadata["aperio.AppMag"]);
    }
    return magnification_lvl;
}

bool MainWindow::segmentTissue() {
	if (wsiList.empty()) {
		std::cout << "Requires a WSI to be rendered in order to perform the analysis." << std::endl;
		return false;
	}

	// prompt if you want to run the analysis again, if it has already been ran
    if (hasRenderer("tissue")) {
        simpleInfoPrompt("Tissue segmentation on current WSI has already been performed.");
        return false;
    }

	// basic thresholding (with morph. post-proc.) based on euclidean distance from the color white
    auto tissueSegmentation = TissueSegmentation::New();
    tissueSegmentation->setInputData(m_image);

    stopFlag = false;
    if (advancedMode) {
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

        std::string tempTissueName = "temporaryTissue";

		QObject::connect(threshSlider, &QSlider::valueChanged, [=](int newValue) {
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
				someRenderer->setColor(1, Color(255.0f / 255.0f, 127.0f / 255.0f, 80.0f / 255.0f));
				someRenderer->setInputData(temporaryTissueSegmentation->updateAndGetOutputData<Image>());
				someRenderer->setOpacity(0.4f);
				someRenderer->update();

				if (hasRenderer(tempTissueName)) {
					auto currRenderer = m_rendererList[tempTissueName];
					getView(0)->removeRenderer(currRenderer);
					m_rendererList.erase(tempTissueName);
				}
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
        QObject::connect(dilateSlider, &QSlider::valueChanged, [=](int newValue) {
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
				someRenderer->setColor(1, Color(255.0f / 255.0f, 127.0f / 255.0f, 80.0f / 255.0f));
				someRenderer->setInputData(temporaryTissueSegmentation->updateAndGetOutputData<Image>());
				someRenderer->setOpacity(0.4f);
				someRenderer->update();

				if (hasRenderer(tempTissueName)) {
					auto currRenderer = m_rendererList[tempTissueName];
					getView(0)->removeRenderer(currRenderer);
					m_rendererList.erase(tempTissueName);
				}
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
        QObject::connect(erodeSlider, &QSlider::valueChanged, [=](int newValue) {
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
                someRenderer->setColor(1, Color(255.0f/255.0f, 127.0f/255.0f, 80.0f/255.0f));
                someRenderer->setInputData(temporaryTissueSegmentation->updateAndGetOutputData<Image>());
                someRenderer->setOpacity(0.4f); // <- necessary for the quick-fix temporary solution
                someRenderer->update();

                if (hasRenderer(tempTissueName)) {
					auto currRenderer = m_rendererList[tempTissueName];
					getView(0)->removeRenderer(currRenderer);
					m_rendererList.erase(tempTissueName);
                }
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

        // Add some standard buttons (Cancel/Ok) at the bottom of the dialog
        QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                Qt::Horizontal, &paramDialog);
        buttonBox.button(QDialogButtonBox::Ok)->setText("Run");
        form.addRow(&buttonBox);
        QObject::connect(&buttonBox, SIGNAL(accepted()), &paramDialog, SLOT(accept()));
        QObject::connect(&buttonBox, SIGNAL(rejected()), &paramDialog, SLOT(reject()));

        // Show the dialog as modal
        int ret = paramDialog.exec();

        // should delete temporary segmentation when selecting is finished or cancelled
        auto currRenderer = m_rendererList[tempTissueName];
        getView(0)->removeRenderer(currRenderer);
        m_rendererList.erase(tempTissueName);

        std::cout << "Value chosen: " << ret << std::endl;
        switch (ret) {
            case 1:
                std::cout << "OK was pressed, should have updated params!" << std::endl;
                break;
            case 0:
                std::cout << "Cancel was pressed." << std::endl;
                stopFlag = true;
                return false;
            default:
                std::cout << "Default was pressed." << std::endl;
                return false;
        }
    }

    std::cout << "Thresh: " << tissueSegmentation->getThreshold() << std::endl;
    std::cout << "Dilate: " << tissueSegmentation->getDilate() << std::endl;
    std::cout << "Erode:  " << tissueSegmentation->getErode() << std::endl;

    // finally get resulting tissueMap to be used later on
    m_tissue = tissueSegmentation->updateAndGetOutputData<Image>();

    auto someRenderer = SegmentationRenderer::New();
    someRenderer->setColor(1, Color(255.0f/255.0f, 127.0f/255.0f, 80.0f/255.0f));
    someRenderer->setInputData(m_tissue);
    someRenderer->setOpacity(0.4f); // <- necessary for the quick-fix temporary solution
    someRenderer->update();

	std::string currSegment = "tissue";

	// TODO: should append some unique ID next to "tissue" (also really for all other results) such that multiple
	//  runs with different hyperparamters may be ran, visualized and stored
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

    availableResults[currSegment] = m_tissue;
    exportComboBox->addItem(tr(currSegment.c_str()));

    return true;
}

void MainWindow::loadHighres(QString path, QString name) {
    if (!fileExists(path.toStdString()))
        return;

    auto someName = name.toStdString();
    std::cout << "High-res someName var: " << someName << std::endl;
    std::cout << "path: " << path.toStdString() + "/" << std::endl;

    auto someImporter = ImagePyramidPatchImporter::New();
    someImporter->setPath(path.toStdString() + "/");
    //someImporter->update();
    auto result = someImporter->updateAndGetOutputData<ImagePyramid>();

    auto someRenderer = SegmentationRenderer::New();
    someRenderer->setOpacity(0.5f);
    //someRenderer->setInputConnection(someImporter->getOutputPort()); //setInputConnection(importer->getOutputPort());
    someRenderer->setInputData(result);
    someRenderer->update();

    m_rendererTypeList[someName] = "SegmentationRenderer";
    insertRenderer(someName, someRenderer);
    createDynamicViewWidget(someName, modelName);
    savedList.emplace_back(someName);
}

void MainWindow::loadHeatmap(QString tissuePath, QString name) {
	if (!fileExists(tissuePath.toStdString()))
		return;

	auto someName = name.toStdString();
	std::cout << "Heatmap someName var: " << someName << std::endl;

	auto importer = HDF5TensorImporter::New();
	importer->setFilename(tissuePath.toStdString()); //"tensor.h5");
	importer->setDatasetName(someName);
	auto resultTensor = importer->updateAndGetOutputData<Tensor>();
	auto resultShape = resultTensor->getShape();
	//importer->update();
	//addProcessObject(importer);

	std::cout << "importer shape: " << std::endl;
	std::cout << resultShape.toString() << std::endl;

	// m_tumorMap->setSpacing((float) m_image->getFullHeight() / (float) input->getHeight(), (float) m_image->getFullWidth() / (float) input->getWidth(), 1.0f);

	auto someRenderer = HeatmapRenderer::New();
	//someRenderer->glPolygonOffset(512.0f, 512.0f);
	//someRenderer->setColor(1, Color(255.0f / 255.0f, 127.0f / 255.0f, 80.0f / 255.0f));
	someRenderer->setInputConnection(0, importer->getOutputPort());
	//someRenderer->setInputData(someImage);
	someRenderer->setMaxOpacity(0.6f);
	someRenderer->setInterpolation(false);
	//someRenderer->setOpacity(0.4f); // <- necessary for the quick-fix temporary solution
	someRenderer->update();

	m_rendererTypeList[someName] = "HeatmapRenderer";
	insertRenderer(someName, someRenderer);

	//hideTissueMask(false);

	// now make it possible to edit prediction in the View Widget
	createDynamicViewWidget(someName, modelName);

	std::cout << "Finished loading..." << std::endl;;
	savedList.emplace_back(someName);
}

void MainWindow::loadSegmentation(QString tissuePath, QString name) {

	if (!fileExists(tissuePath.toStdString()))
		return;

	auto someName = name.toStdString();

	auto reader = ImageFileImporter::New();
	reader->setFilename(tissuePath.toStdString());
	reader->setMainDevice(Host::getInstance());
	auto port = reader->getOutputPort();
	reader->update();
	auto someImage = port->getNextFrame<Image>();

	//auto wsi = getInputData<ImagePyramid>();
	auto access = m_image->getAccess(ACCESS_READ);
	auto input = access->getLevelAsImage(m_image->getNrOfLevels() - 1);
	auto currShape = someImage->getSize();

	std::cout << "Dimensions info (current): " << currShape[1] << ", " << currShape[0] << std::endl;
	std::cout << "Dimensions info (lowest): " << input->getHeight() << ", " << input->getWidth() << std::endl;
	std::cout << "Dimensions info (WSI): " << m_image->getFullHeight() << ", " << m_image->getFullWidth() << std::endl;

    // TODO: should store the corresponding model config files that contain all the information relevant for rendering
    //  and interaction with the software, e.g. number of classes, class names, class colors, etc...

	//someImage->setSpacing((float)m_image->getFullHeight() / (float)input->getHeight(), (float)m_image->getFullWidth() / (float)input->getWidth(), 1.0f);
	someImage->setSpacing((float)m_image->getFullHeight() / (float)currShape[1], (float)m_image->getFullWidth() / (float)currShape[0], 1.0f);

	auto someRenderer = SegmentationRenderer::New();
	someRenderer->setColor(1, Color(255.0f / 255.0f, 127.0f / 255.0f, 80.0f / 255.0f));
	someRenderer->setInputData(someImage);
	someRenderer->setOpacity(0.4f);
	someRenderer->update();

	m_rendererTypeList[someName] = "SegmentationRenderer";
	insertRenderer(someName, someRenderer);
	createDynamicViewWidget(someName, modelName);
	savedList.emplace_back(someName);
}

void MainWindow::runPipeline_wrapper(std::string path) {

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
    progDialog.setLabelText("Running pipeline across WSIs in project...");
    //QRect screenrect = mWidget->screen()[0].geometry();
    progDialog.move(mWidget->width() - progDialog.width() * 1.1, progDialog.height() * 0.1);
    progDialog.show();

    QCoreApplication::processEvents(QEventLoop::AllEvents, 0);

    auto counter = 0;
    for (const auto& currWSI : currentWSIs) {

        // if run for project is enabled, run the inference-export pipeline in a background thread, else don't
        if (m_runForProject) {
            std::atomic_bool stopped(false);
            std::thread inferenceThread([&, path]() {
                runPipeline(path);
            });
            inferenceThread.detach();
        }
        else {
            runPipeline(path);

            // now make it possible to edit prediction in the View Widget
            // createDynamicViewWidget(modelMetadata["name"], someModelName);
        }
    }
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
	progDialog.setLabelText("Running pipeline across WSIs in project...");
	//QRect screenrect = mWidget->screen()[0].geometry();
	progDialog.move(mWidget->width() - progDialog.width() * 1.1, progDialog.height() * 0.1);
	progDialog.show();

	QCoreApplication::processEvents(QEventLoop::AllEvents, 0);

    //std::map<std::string, std::string> modelMetadata = getModelMetadata(modelName);

	auto counter = 0;
	for (const auto& currWSI : currentWSIs) {

        // check if folder for current WSI exists, if not, create one
        QString projectFolderName = "C:/Users/andrp/workspace/test_projects/project3";
        QString wsiResultPath = (projectFolderName.toStdString() + "/results/" +
            splitCustom(splitCustom(currWSI, "/").back(), ".")[0]).c_str();
        wsiResultPath = wsiResultPath.replace("//", "/");
        if (!QDir(wsiResultPath).exists()) {
            QDir().mkdir(wsiResultPath);
        }
        auto currPath =
            wsiResultPath.toStdString() + "/" +
            splitCustom(wsiResultPath.toStdString(), "/").back() +
            "_" + "tubuleSegTest" + "/";
            //"_" + modelMetadata["name"] + "/";

		// TODO: Perhaps use corresponding .txt-file to feed arguments in the pipeline
		// pipeline requires some user-defined inputs, e.g. which WSI to use (and which model?)
		std::map<std::string, std::string> arguments;
		arguments["filename"] = filename;
        arguments["exportPath"] = currPath;
		//arguments["modelPath"] = path;

        std::cout << "exportPath: " << currPath << std::endl;

		// check if folder for current WSI exists, if not, create one
		/*
		QString wsiResultPath = (projectFolderName.toStdString() + "/results/" + splitCustom(splitCustom(filename, "/").back(), ".")[0] + "/").c_str();
		wsiResultPath = wsiResultPath.replace("//", "/");
		if (!QDir(wsiResultPath).exists()) {
			QDir().mkdir(wsiResultPath);
		}

		QString outFile = (wsiResultPath.toStdString() + splitCustom(splitCustom(filename, "/").back(), ".")[0] + "_heatmap.h5").c_str();

		arguments["outPath"] = outFile.replace("//", "/").toStdString();
		 */

		// parse fpl-file, and run pipeline with correspodning input arguments
		auto pipeline = Pipeline(path, arguments);
        pipeline.parse();

        std::cout << "Before update: " << std::endl;
        /*
        for (const auto& renderer : pipeline.getRenderers()) {
            auto currId = createRandomNumbers_(8);
            insertRenderer("result_" + currId, pipeline.getRenderers()[1]);
            createDynamicViewWidget("result_" + currId, "result_" + currId);
        }
         */

		// update progress bar
		progDialog.setValue(counter);
		counter++;

		// to render straight away (avoid waiting on all WSIs to be handled before rendering)
		QCoreApplication::processEvents(QEventLoop::AllEvents, 0);
	}
}

// Setting parameters for different methods
std::map<std::string, std::string> MainWindow::setParameterDialog(std::map<std::string, std::string> modelMetadata, int *successFlag) {
    QDialog paramDialog;
    paramDialog.setStyleSheet(mWidget->styleSheet()); // transfer style sheet from parent
    QFormLayout form(&paramDialog);
    form.addRow(new QLabel("Please, set the parameters for this analysis: "));

    std::cout << "Before update: " << std::endl;
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
	*successFlag = ret;
    std::cout << "Ret value: " << ret << std::endl;
	switch (ret) {
	case 1: //QMessageBox::Ok:
		std::cout << "OK was pressed: " << std::endl;
		for (auto const&[key, val] : modelMetadata) {
			modelMetadata[key] = fields.takeFirst()->text().toStdString(); //fields.takeAt(cnt)->text().toStdString();
		}
	case 0: //QMessageBox::Cancel:
		std::cout << "CANCEL was pressed: " << std::endl;
		stopFlag = true;
	default:
		std::cout << "Default..." << std::endl;
	}

    std::cout << "After update: " << std::endl;
    for (const auto &[k, v] : modelMetadata)
        std::cout << "m[" << k << "] = (" << v << ") " << std::endl;

    return modelMetadata;
}

void MainWindow::MIL_test() {

    std::string modelName = "custom_mil_model";

    // read model metadata (txtfile)
    std::map<std::string, std::string> modelMetadata = getModelMetadata(modelName);

    int patch_lvl_model = (int)(std::log(magn_lvl / (float)std::stoi(modelMetadata["magnification_level"])) / std::log(std::round(stof(metadata["openslide.level[1].downsample"]))));

    if (!hasRenderer("tissue")) {
        segmentTissue();
        hideTissueMask(true);
    }

    //auto generator = PatchGenerator::create(std::stoi(modelMetadata["input_img_size_x"]), std::stoi(modelMetadata["input_img_size_y"]), 1, patch_lvl_model, 0, );
    auto generator = PatchGenerator::New();
    generator->setPatchSize(std::stoi(modelMetadata["input_img_size_y"]), std::stoi(modelMetadata["input_img_size_x"]));
    generator->setPatchLevel(patch_lvl_model);
    generator->setInputData(0, m_image);
    if (m_tissue)
        generator->setInputData(1, m_tissue);
    //if (m_tumorMap)
    //    generator->setInputData(1, m_tumorMap);

    //auto batchgen = ImageToBatchGenerator::New();  // TODO: Can't use this with TensorRT (!)
    //batchgen->setInputConnection(generator->getOutputPort());
    //batchgen->setMaxBatchSize(1);
    //batchgen->setMaxBatchSize(std::stoi(modelMetadata["batch_process"])); // set 256 for testing stuff (tumor -> annotation, then grade from tumor segment)

    auto network = NeuralNetwork::New();
    network->setInferenceEngine("OpenVINO");
    // apparently this is needed if model has unspecified input size
    /*
    network->setInputNode(0, modelMetadata["input_node"], NodeType::IMAGE, TensorShape(
            { 1, 256, 256, 3 })); //{1, size, size, 3}

    network->setOutputNode(0, "conv2d_26/truediv", NodeType::TENSOR,
                           TensorShape({ 1, 256, 256, 3}));
    network->setOutputNode(1, "dense_1/Softmax", NodeType::TENSOR,
                           TensorShape({ 1, 2 }));
     */

    std::cout << "Current Inference Engine: " << network->getInferenceEngine() << std::endl;

    network->load(cwd + "data/Models/" + modelName + ".onnx");
    //network->load(cwd + "data/Models/" + modelName + "." + getModelFileExtension(network->getInferenceEngine()->getPreferredModelFormat())); //".uff");
    std::cout << "Current Inference Engine: " << network->getInferenceEngine() << std::endl;

    network->setInputConnection(generator->getOutputPort()); //generator->getOutputPort());
    vector scale_factor = splitCustom(modelMetadata["scale_factor"], "/"); // get scale factor from metadata
    network->setScaleFactor((float)std::stoi(scale_factor[0]) / (float)std::stoi(scale_factor[1]));   // 1.0f/255.0f

    auto stitcher1 = PatchStitcher::New();
    stitcher1->setInputConnection(network->getOutputPort(1));

    auto stitcher2 = PatchStitcher::New();
    stitcher2->setInputConnection(network->getOutputPort(0));

    auto someRenderer1 = HeatmapRenderer::New();
    someRenderer1->setMinConfidence(0.8);
    someRenderer1->setInterpolation(false);
    //someRenderer->setColor(0, Color(0.0f, 255.0f / 255.0f, 0.0f));
    //someRenderer1->setInputData(m_tumorMap);
    someRenderer1->setInputConnection(stitcher1->getOutputPort());

    auto someRenderer2 = HeatmapRenderer::New();
    //someRenderer2->setMinConfidence(0.5);
    //someRenderer2->setInterpolation(std::stoi(modelMetadata["interpolation"].c_str()));
    someRenderer2->setInterpolation(false);
    someRenderer2->setInputConnection(stitcher2->getOutputPort());
    //someRenderer2->setMaxOpacity(0.6f);

    //someRenderer2->setSynchronizedRendering(false);
    //someRenderer2->update();

    m_rendererTypeList["mil_grade"] = "HeatmapRenderer";
    insertRenderer("mil_grade", someRenderer1);

    m_rendererTypeList["mil_attention"] = "HeatmapRenderer";
    insertRenderer("mil_attention", someRenderer2);

    createDynamicViewWidget("mil_grade", modelName);
    createDynamicViewWidget("mil_attention", modelName);
}

void MainWindow::Kmeans_MTL_test() {
    std::string modelName = "feature_kmeans_model";

    // read model metadata (txtfile)
    std::map<std::string, std::string> modelMetadata = getModelMetadata(modelName);

    int patch_lvl_model = (int)(std::log(magn_lvl / (float)std::stoi(modelMetadata["magnification_level"])) / std::log(std::round(stof(metadata["openslide.level[1].downsample"]))));

    auto generator = PatchGenerator::New();
    generator->setPatchSize(std::stoi(modelMetadata["input_img_size_y"]), std::stoi(modelMetadata["input_img_size_x"]));
    generator->setPatchLevel(patch_lvl_model);
    generator->setInputData(0, m_image);

    auto tissueSegmentation = TissueSegmentation::New();
    tissueSegmentation->setInputData(m_image);
    tissueSegmentation->setThreshold(std::stoi(modelMetadata["tissue_threshold"]));

    generator->setInputConnection(1, tissueSegmentation->getOutputPort());

    auto network = NeuralNetwork::New();
    network->setInferenceEngine("TensorFlow");
    // apparently this is needed if model has unspecified input size
    network->setInputNode(0, modelMetadata["input_node"], NodeType::IMAGE, TensorShape(
            { 1, 256, 256, 3 }));

    network->setOutputNode(0, modelMetadata["output_node"], NodeType::TENSOR,
                           TensorShape({ 1, 8 }));

    std::cout << "Current Inference Engine: " << network->getInferenceEngine() << std::endl;

    network->load(cwd + "data/Models/" + modelName + "." + getModelFileExtension(network->getInferenceEngine()->getPreferredModelFormat())); //".uff");
    std::cout << "Current Inference Engine: " << network->getInferenceEngine() << std::endl;

    network->setInputConnection(generator->getOutputPort());
    vector scale_factor = splitCustom(modelMetadata["scale_factor"], "/"); // get scale factor from metadata
    network->setScaleFactor((float)std::stoi(scale_factor[0]) / (float)std::stoi(scale_factor[1]));   // 1.0f/255.0f

    auto stitcher1 = PatchStitcher::New();
    stitcher1->setInputConnection(network->getOutputPort(0));

    auto heatmap1 = HeatmapRenderer::New();
    heatmap1->setInterpolation(std::stoi(modelMetadata["interpolation"]));
    heatmap1->setInputConnection(stitcher1->getOutputPort());

    m_rendererTypeList["cluster_pw"] = "HeatmapRenderer";
    insertRenderer("cluster_pw", heatmap1);

    createDynamicViewWidget("cluster_pw", modelName);
}

void MainWindow::MTL_test() {

	std::string modelName = "model_nuclei_seg_detection_multitask";

	// read model metadata (txtfile)
	std::map<std::string, std::string> modelMetadata = getModelMetadata(modelName);

	int patch_lvl_model = (int)(std::log(magn_lvl / (float)std::stoi(modelMetadata["magnification_level"])) / std::log(std::round(stof(metadata["openslide.level[1].downsample"]))));

	if (!hasRenderer("tissue")) {
		segmentTissue();
		hideTissueMask(true);
	}

	auto generator = PatchGenerator::New();
	generator->setPatchSize(std::stoi(modelMetadata["input_img_size_y"]), std::stoi(modelMetadata["input_img_size_x"]));
	generator->setPatchLevel(patch_lvl_model);
	generator->setInputData(0, m_image);
	if (m_tissue)
		generator->setInputData(1, m_tissue);
	if (m_tumorMap)
		generator->setInputData(1, m_tumorMap);

	auto network = NeuralNetwork::New();
	network->setInferenceEngine("TensorFlow");
	// apparently this is needed if model has unspecified input size
	network->setInputNode(0, modelMetadata["input_node"], NodeType::IMAGE, TensorShape(
		{ 1, 256, 256, 3 })); //{1, size, size, 3}

	network->setOutputNode(0, "conv2d_26/truediv", NodeType::TENSOR,
		TensorShape({ 1, 256, 256, 3}));
	network->setOutputNode(1, "dense_1/Softmax", NodeType::TENSOR,
		TensorShape({ 1, 2 }));

	std::cout << "Current Inference Engine: " << network->getInferenceEngine() << std::endl;

	network->load(cwd + "data/Models/" + modelName + "." + getModelFileExtension(network->getInferenceEngine()->getPreferredModelFormat())); //".uff");
    std::cout << "Current Inference Engine: " << network->getInferenceEngine() << std::endl;

	network->setInputConnection(generator->getOutputPort());
	vector scale_factor = splitCustom(modelMetadata["scale_factor"], "/"); // get scale factor from metadata
	network->setScaleFactor((float)std::stoi(scale_factor[0]) / (float)std::stoi(scale_factor[1]));   // 1.0f/255.0f

	auto converter = TensorToSegmentation::New();
	converter->setInputConnection(network->getOutputPort(0));

	auto stitcher1 = PatchStitcher::New();
	stitcher1->setInputConnection(converter->getOutputPort(0));

	auto stitcher2 = PatchStitcher::New();
	stitcher2->setInputConnection(network->getOutputPort(1));

	auto someRenderer1 = SegmentationRenderer::New();
	someRenderer1->setOpacity(0.7f);
	//someRenderer->setColor(0, Color(0.0f, 255.0f / 255.0f, 0.0f));
	//someRenderer1->setInputData(m_tumorMap);
	someRenderer1->setInputConnection(stitcher1->getOutputPort());

	auto someRenderer2 = HeatmapRenderer::New();
	//someRenderer2->setInterpolation(std::stoi(modelMetadata["interpolation"].c_str()));
	someRenderer2->setInputConnection(stitcher2->getOutputPort());
	//someRenderer2->setMaxOpacity(0.6f);

	//someRenderer2->setSynchronizedRendering(false);
	//someRenderer2->update();

	m_rendererTypeList["nuclei_seg"] = "SegmentationRenderer";
	insertRenderer("nuclei_seg", someRenderer1);

	m_rendererTypeList["nuclei_detect"] = "HeatmapRenderer";
	insertRenderer("nuclei_detect", someRenderer2);

	createDynamicViewWidget("nuclei_seg", modelName);
	createDynamicViewWidget("nuclei_detect", modelName);
}

void MainWindow::pixelClassifier_wrapper(std::string someModelName) {

    std::cout << "Model name in wrapper: " << someModelName << std::endl;

    //connect(currComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateChannelValue(int)));

    // read model metadata (.txt file)
    std::map<std::string, std::string> modelMetadata = getModelMetadata(someModelName);

    // set parameters yourself (only enabled if advanced mode is ON)
    if (advancedMode) {
		auto successFlag = 1;
        modelMetadata = setParameterDialog(modelMetadata, &successFlag);
		if (successFlag != 1)
			return;
        for (const auto &[k, v] : modelMetadata)
            std::cout << "m[" << k << "] = (" << v << ") " << std::endl;
    }

    std::cout << "Final model metadata config sent to pixelClassifier:" << std::endl;
    for (const auto &[k, v] : modelMetadata)
        std::cout << "m[" << k << "] = (" << v << ") " << std::endl;

    // if run for project is enabled, run the inference-export pipeline in a background thread, else don't
    if (m_runForProject) {
        std::atomic_bool stopped(false);
        std::thread inferenceThread([&, someModelName, modelMetadata]() {
            pixelClassifier(someModelName, modelMetadata);
        });
        inferenceThread.detach();
    } else {
        pixelClassifier(someModelName, modelMetadata);

        // now make it possible to edit prediction in the View Widget
        //std::map<std::string, std::string> modelMetadata = getModelMetadata(someModelName);
        createDynamicViewWidget(modelMetadata["name"], someModelName);
    }
}

void MainWindow::pixelClassifier(std::string someModelName, std::map<std::string, std::string> modelMetadata) {

    std::cout << "Final model metadata config WITHIN to pixelClassifier:" << std::endl;
    for (const auto &[k, v] : modelMetadata)
        std::cout << "m[" << k << "] = (" << v << ") " << std::endl;

    // try {
    if (true) {
		// if no WSI is currently being rendered,
		if (wsiList.empty()) {
			std::cout << "Requires a WSI to be rendered in order to perform the analysis." << std::endl;
			return;
		}

		std::cout << "Current model: " << someModelName << std::endl;
		stopFlag = false;

		// for run-for-project
		std::vector<std::string> currentWSIs;
		if (m_runForProject) {
			currentWSIs = m_runForProjectWsis;
		}
		else {
			currentWSIs.push_back(filename);
		}

		// add current model name to map
		modelNames[someModelName] = someModelName;

		if (stopFlag) { // if "Cancel" is selected in advanced mode in parameter selection, don't run analysis
			return;
		}

		auto progDialog = QProgressDialog();
		progDialog.setRange(0, currentWSIs.size());
		//progDialog.setContentsMargins(0, 0, 0, 0);
		progDialog.setValue(0);
		progDialog.setVisible(true);
		progDialog.setModal(false);
		progDialog.setLabelText("Running inference...");
		//QRect screenrect = mWidget->screen()[0].geometry();
		progDialog.move(mWidget->width() - progDialog.width() * 1.1, progDialog.height() * 0.1);
		progDialog.show();

		QCoreApplication::processEvents(QEventLoop::AllEvents, 0);

		auto counter = 1;
		for (const auto &currWSI : currentWSIs) {

			std::cout << "current WSI: " << currWSI << std::endl;
			if (!hasRenderer(modelMetadata["name"])) { // only run analysis if it has not been ran previously on current WSI

				 // based on predicted magnification level of WSI, set magnificiation level for optimal input to model based on predicted resolution of WSI
				int patch_lvl_model = 0; // defaults to 0
				if (!modelMetadata["magnification_level"].empty()) {
					patch_lvl_model = (int)(
						std::log(magn_lvl / (float)std::stoi(modelMetadata["magnification_level"])) /
						std::log(std::round(stof(metadata["openslide.level[1].downsample"]))));
				}
				else {
					std::cout << "magnification_level was not properly defined in the model config file. Defaults to using image plane 0." << std::endl;
				}

				std::cout << "Curr patch level: " << patch_lvl_model << std::endl;

				// read current wsi
				std::cout << "current WSI: " << currWSI << std::endl;
				auto someImporter = WholeSlideImageImporter::New();
				someImporter->setFilename(currWSI);
				auto currImage = someImporter->updateAndGetOutputData<ImagePyramid>();

				if (!m_runForProject) {
					currImage = m_image;
				}

				auto access = currImage->getAccess(ACCESS_READ);

				ImageResizer::pointer resizer = ImageResizer::New();
				int currLvl;
				if (modelMetadata["resolution"] == "low") {
					//auto access = currImage->getAccess(ACCESS_READ);
					// TODO: Should automatically find best suitable magn.lvl. (NOTE: 2x the image size as for selecting lvl!)

					int levelCount = std::stoi(metadata["openslide.level-count"]);
					int inputWidth = std::stoi(modelMetadata["input_img_size_x"]);
					int inputHeight = std::stoi(modelMetadata["input_img_size_y"]);
					bool breakFlag = false;
					for (int i = 0; i < levelCount; i++) {
						if ((std::stoi(metadata["openslide.level[" + std::to_string(i) + "].width"]) <=
							inputWidth * 2) ||
							(std::stoi(metadata["openslide.level[" + std::to_string(i) + "].height"]) <=
								inputHeight * 2)) {
							currLvl = i - 1;
							breakFlag = true;
							break;
						}
					}
					if (!breakFlag)
						currLvl = levelCount - 1;

					std::cout << "Optimal patch level: " << std::to_string(currLvl) << std::endl;
					if (currLvl < 0) {
						std::cout << "Automatic chosen patch level for low_res is invalid: "
							<< std::to_string(currLvl)
							<< std::endl;
						return;
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
				QStringList tmpPaths = QDir(QString::fromStdString(Config::getLibraryPath())).entryList(
					QStringList(),
					QDir::Files);

				auto currOperatingSystem = QSysInfo::productType();
				auto currKernel = QSysInfo::kernelType();
				std::cout << "Current OS is: " << currOperatingSystem.toStdString() << std::endl;
				std::cout << "Current kernel is: " << currKernel.toStdString() << std::endl;
				if (currKernel == "linux") {
					foreach(QString filePath, tmpPaths) {
						if (filePath.toStdString().find("libInferenceEngine") != std::string::npos) {
							IEsList.push_back(
								splitCustom(splitCustom(filePath.toStdString(), "libInferenceEngine").back(),
									".so")[0]);
						}
					}
				}
				else if ((currKernel == "winnt") || (currKernel == "wince")) {
					foreach(QString filePath, tmpPaths) {
						if (filePath.toStdString().find("InferenceEngine") != std::string::npos) {
							IEsList.push_back(
								splitCustom(splitCustom(filePath.toStdString(), "InferenceEngine").back(),
									".dll")[0]);
						}
					}
				}
				else {
					std::cout
						<< "Current operating system is not using any of the supported kernels: linux and winnt. Current kernel is: "
						<< currKernel.toStdString() << std::endl;
				}

				// check which model formats exists, before choosing inference engine
				QDir directory(QString::fromStdString(cwd + "data/Models/"));
				QStringList models = directory.entryList(QStringList(), QDir::Files);

				std::list<std::string> acceptedModels;
				foreach(QString currentModel, models) {
					if (currentModel.toStdString().find(someModelName) != std::string::npos) {
						acceptedModels.push_back(
							"." + splitCustom(currentModel.toStdString(), someModelName + ".").back());
						std::cout
							<< "accepted models: ." +
							splitCustom(currentModel.toStdString(), someModelName + ".").back()
							<< std::endl;
					}
				}

				// init network
				auto network = NeuralNetwork::New(); // default, need special case for high_res segmentation
				if ((modelMetadata["problem"] == "segmentation") && (modelMetadata["resolution"] == "high")) {
					network = SegmentationNetwork::New();
				}
				else if ((modelMetadata["problem"] == "segmentation") &&
					(modelMetadata["resolution"] == "low")) {
					network = SegmentationNetwork::New();
				}

				bool checkFlag = true;

				std::cout << "Current available IEs: " << std::endl;
				foreach(std::string elem, IEsList) {
					std::cout << elem << ", " << std::endl;
				}

				std::cout << "Which model formats are available and that there exists an IE for: " << std::endl;
				foreach(std::string elem, acceptedModels) {
					std::cout << elem << ", " << std::endl;
				}

				std::string chosenIE;

				// /*
				// Now select best available IE based on which extensions exist for chosen model
				// TODO: Current optimization profile is: 0. Please ensure there are no enqueued operations pending in this context prior to switching profiles
				if ((std::find(acceptedModels.begin(), acceptedModels.end(), ".onnx") !=
                     acceptedModels.end()) &&
                    (std::find(IEsList.begin(), IEsList.end(), "TensorRT") != IEsList.end())) {
                    // @TODO: I don't think this works exactly how I want it to. TensorRT is still find as it is found in the lib/ directory, even though
                    //  it is not installed.
                    std::cout << "TensorRT (using ONNX) selected" << std::endl;
                    network->setInferenceEngine("TensorRT");
                    chosenIE = "onnx";
                }
                else if ((std::find(acceptedModels.begin(), acceptedModels.end(), ".uff") != acceptedModels.end()) &&
                         (std::find(IEsList.begin(), IEsList.end(), "TensorRT") != IEsList.end())) {
                    std::cout << "TensorRT selected (using UFF)" << std::endl;
                    network->setInferenceEngine("TensorRT");
                    chosenIE = "uff";
                }
                else if ((std::find(acceptedModels.begin(), acceptedModels.end(), ".onnx") !=
                    acceptedModels.end()) &&
                    (std::find(IEsList.begin(), IEsList.end(), "OpenVINO") != IEsList.end())) {
                    std::cout << "OpenVINO (using ONNX) selected" << std::endl;
                    network->setInferenceEngine("OpenVINO");
                    chosenIE = "onnx";
				}
                else if ((std::find(acceptedModels.begin(), acceptedModels.end(), ".xml") !=
                          acceptedModels.end()) &&
                         (std::find(IEsList.begin(), IEsList.end(), "OpenVINO") != IEsList.end())) {
                    std::cout << "OpenVINO (using IR) selected" << std::endl;
                    network->setInferenceEngine("OpenVINO");
                    chosenIE = "xml";
                }
				else if (std::find(acceptedModels.begin(), acceptedModels.end(), ".pb") !=
					acceptedModels.end() &&
					std::find(IEsList.begin(), IEsList.end(), "TensorFlow") != IEsList.end()) {
					std::cout << "TensorFlow selected" << std::endl;
					network->setInferenceEngine("TensorFlow");
				}
				/* else {
					std::cout << "Model does not exist in Models/ folder. Please add it using AddModels(). "
								 "It might also be that the model exists, but the Inference Engine does not. "
								 "Available IEs are: ";
					foreach(std::string elem, IEsList) {
						std::cout << elem << ", ";
					}
					checkFlag = false;
				}
				 */

				if (checkFlag) {
					std::cout << "Model was found." << std::endl;

					// TODO: Need to handle if model is in Models/, but inference engine is not available
					//Config::getLibraryPath();


					//if ((modelMetadata["problem"] == "segmentation") && (modelMetadata["resolution"] == "high")) {
					//	network->setInferenceEngine("OpenVINO");
					//}
					if (true) {
						// If model has CPU flag only, need to check if TensorFlowCPU is available, else run on OpenVINO, else use best available
						if (std::stoi(modelMetadata["cpu"]) == 1) {
							if (std::find(acceptedModels.begin(), acceptedModels.end(), ".pb") !=
								acceptedModels.end() &&
								std::find(IEsList.begin(), IEsList.end(), "TensorFlow") != IEsList.end()) {
                                std::cout << "GPU is disabled! (with TensorFlow)" << std::endl;
								network->setInferenceEngine("TensorFlow");
                                network->getInferenceEngine()->setDeviceType(InferenceDeviceType::CPU);  // Andre: 0 or 1 for personal Ubuntu Desktop
							}
							else if (std::find(acceptedModels.begin(), acceptedModels.end(), ".xml") !=
								acceptedModels.end() &&
								std::find(IEsList.begin(), IEsList.end(), "OpenVINO") != IEsList.end()) {
                                std::cout << "GPU is disabled! (with OpenVINO)" << std::endl;
								network->setInferenceEngine("OpenVINO");
								//network->getInferenceEngine()->setDeviceType(InferenceDeviceType::CPU);
                                network->getInferenceEngine()->setDeviceType(InferenceDeviceType::CPU);
							}
							else {
								std::cout
									<< "CPU only was selected, but was not able to find any CPU devices..."
									<< std::endl;
							}
						}

						// if stated in the model txt file, use the specified inference engine
						if (!((modelMetadata.count("IE") == 0) || modelMetadata["IE"] == "none")) {
							std::cout << "Preselected IE was used: " << modelMetadata["IE"] << std::endl;
							network->setInferenceEngine(modelMetadata["IE"]);
                            chosenIE = getModelFileExtension(network->getInferenceEngine()->getPreferredModelFormat());
						}

						const auto engine = network->getInferenceEngine()->getName();
						// IEs like TF and TensorRT need to be handled differently than IEs like OpenVINO
						if (engine.substr(0, 10) == "TensorFlow") {
							// apparently this is needed if model has unspecified input size
							network->setInputNode(0, modelMetadata["input_node"], NodeType::IMAGE, TensorShape(
								{ 1, std::stoi(modelMetadata["input_img_size_y"]),
								 std::stoi(modelMetadata["input_img_size_x"]),
								 std::stoi(modelMetadata["nb_channels"]) })); //{1, size, size, 3}

						    // TensorFlow needs to know what the output node is called
							if (modelMetadata["problem"] == "classification") {
								network->setOutputNode(0, modelMetadata["output_node"], NodeType::TENSOR,
									TensorShape(
										{ 1, std::stoi(modelMetadata["nb_classes"]) }));
							}
							else if (modelMetadata["problem"] == "segmentation") {
								network->setOutputNode(0, modelMetadata["output_node"], NodeType::TENSOR,
									TensorShape(
										{ 1, std::stoi(modelMetadata["input_img_size_y"]),
										 std::stoi(modelMetadata["input_img_size_x"]),
										 std::stoi(modelMetadata["nb_classes"]) }));
							}
							else if (modelMetadata["problem"] == "object_detection") {
								// FIXME: This is outdated for YOLOv3, as it has multiple output nodes -> need a way of handling this!
								network->setOutputNode(0, modelMetadata["output_node"], NodeType::TENSOR,
									TensorShape(
										{ 1, std::stoi(modelMetadata["nb_classes"]) }));
							}
						}
                        else if ((engine == "TensorRT") && (chosenIE == "uff")) {
                            // TensorRT needs to know everything about the input and output nodes
                            network->setInputNode(0, modelMetadata["input_node"], NodeType::IMAGE, TensorShape(
                                { 1, std::stoi(modelMetadata["nb_channels"]),
                                 std::stoi(modelMetadata["input_img_size_y"]),
                                 std::stoi(modelMetadata["input_img_size_y"]) })); //{1, size, size, 3}
                            network->setOutputNode(0, modelMetadata["output_node"], NodeType::TENSOR,
                                TensorShape({ 1, std::stoi(modelMetadata["nb_classes"]) }));
                        }

                        if ((engine != "TensorRT") && (engine != "OpenVINO")) {
                            chosenIE = getModelFileExtension(network->getInferenceEngine()->getPreferredModelFormat());
                        }
                        network->load(cwd + "data/Models/" + someModelName + "." + chosenIE);
					}

					auto generator = PatchGenerator::New();
					if (modelMetadata["resolution"] == "low") { // special case handling for low_res NN inference
						auto port = resizer->getOutputPort();
						resizer->update();
						network->setInputData(port->getNextFrame<Image>());
					} else {
						// whether or not to run tissue segmentation
						if (modelMetadata["tissue_threshold"] == "none") {
							std::cout
								<< "No tissue segmentation filtering will be applied before this analysis."
								<< std::endl;
						} else if (!modelMetadata["tissue_threshold"].empty()) {
                            std::cout << "Threshold was defined: " << modelMetadata["tissue_threshold"] << std::endl; 
							auto tissueSegmentation = TissueSegmentation::New();
							tissueSegmentation->setInputData(m_image);
							tissueSegmentation->setThreshold(std::stoi(modelMetadata["tissue_threshold"]));

							generator->setInputConnection(1, tissueSegmentation->getOutputPort());

							std::cout << "tissue_threshold was defined, so is performing thresholding as preprocessing step." << std::endl;
						}
						else {
							std::cout
								<< "The tissue_threshold has not been properly defined in the model config file, and thus the method will use any existing segmentation masks as filtering (if available)."
								<< std::endl;
							// TODO: This should be handled more generically. For pipelines that allow the user to use
							//   an already existing segmentation as mask for another method, they should be able to
							//   set this method themselves from the GUI (at least in advanced mode), or perhaps where
							//   results from previous runs may be used if available (instead through hard-coded variable
							//   names such as m_tissue and m_tumorMap.
							if (m_tissue) {
								generator->setInputData(1, m_tissue);
							} else if (m_tumorMap) {
								generator->setInputData(1, m_tumorMap);
							}
						}

						generator->setPatchSize(std::stoi(modelMetadata["input_img_size_y"]),
							std::stoi(modelMetadata["input_img_size_x"]));
						generator->setPatchLevel(patch_lvl_model);
						if (modelMetadata["mask_threshold"].empty()) {
							std::cout << "No mask_threshold variable exists. Defaults to 0.5." << std::endl;
						} else {
                            std::cout << "Setting mask_threshold to: " << modelMetadata["mask_threshold"] << std::endl;
							generator->setMaskThreshold(std::stof(modelMetadata["mask_threshold"]));
						}
						if (modelMetadata["patch_overlap"].empty()) {
							std::cout << "No patch_overlap variable exists. Defaults to 0." << std::endl;
						} else {
							generator->setOverlap(std::stof(modelMetadata["patch_overlap"]));
						}
						generator->setInputData(0, currImage);

						//auto batchgen = ImageToBatchGenerator::New();  // TODO: Can't use this with TensorRT (!)
						//batchgen->setInputConnection(generator->getOutputPort());
						//batchgen->setMaxBatchSize(std::stoi(modelMetadata["batch_process"])); // set 256 for testing stuff (tumor -> annotation, then grade from tumor segment)

						network->setInputConnection(generator->getOutputPort());
					}
					if (modelMetadata["scale_factor"].empty()) {
						std::cout << "scale_factor not defined. Defaults to using using no intensity normalization/scaling in preprocessing." << std::endl;
					}
					else {
						vector scale_factor = splitCustom(modelMetadata["scale_factor"], "/"); // get scale factor from metadata
						network->setScaleFactor(
							(float)std::stoi(scale_factor[0]) /
							(float)std::stoi(scale_factor[1]));   // 1.0f/255.0f
					}

					// define renderer from metadata
					if ((modelMetadata["problem"] == "classification") && (modelMetadata["resolution"] == "high")) {
						auto stitcher = PatchStitcher::New();
						stitcher->setInputConnection(network->getOutputPort());

						auto currentHeatmapName = modelMetadata["name"];
						std::cout << "currentHeatmapName: " << currentHeatmapName << ", currWSI: " << currWSI << std::endl;

						if (!m_runForProject) {
							m_patchStitcherList[modelMetadata["name"]] = stitcher;

							auto someRenderer = HeatmapRenderer::New();
							someRenderer->setInterpolation(std::stoi(modelMetadata["interpolation"]));
							someRenderer->setInputConnection(stitcher->getOutputPort());
							someRenderer->setMaxOpacity(0.6f);
							vector<string> colors = splitCustom(modelMetadata["class_colors"], ";");
							for (int i = 0; i < std::stoi(modelMetadata["nb_classes"]); i++) {
								vector<string> rgb = splitCustom(colors[i], ",");
								someRenderer->setChannelColor(i, Color((float)std::stoi(rgb[0]) / 255.0f,
									(float)std::stoi(rgb[1]) / 255.0f,
									(float)std::stoi(rgb[2]) / 255.0f));
							}

							m_rendererTypeList[modelMetadata["name"]] = "HeatmapRenderer";
							insertRenderer(modelMetadata["name"], someRenderer);
						}

						if (m_runForProject) {

							//auto start = std::chrono::high_resolution_clock::now();
							DataObject::pointer data;
							do {
								data = stitcher->updateAndGetOutputData<Tensor>();

							} while (!data->isLastFrame());
							// check if folder for current WSI exists, if not, create one
							QString wsiResultPath = (projectFolderName.toStdString() + "/results/" +
								splitCustom(splitCustom(currWSI, "/").back(), ".")[0]).c_str();
							wsiResultPath = wsiResultPath.replace("//", "/");
							if (!QDir(wsiResultPath).exists()) {
								QDir().mkdir(wsiResultPath);
							}

							auto exporter = HDF5TensorExporter::New();
							exporter->setFilename(wsiResultPath.toStdString() + "/" + splitCustom(wsiResultPath.toStdString(), "/").back() + "_" + currentHeatmapName + ".h5");
							exporter->setDatasetName(currentHeatmapName);
							exporter->setInputData(data);
							exporter->update();
						}

					}
					else if ((modelMetadata["problem"] == "segmentation") && (modelMetadata["resolution"] == "high")) {
						if (!m_runForProject) {
							auto stitcher = PatchStitcher::New();
							stitcher->setInputConnection(network->getOutputPort());
							auto port = stitcher->getOutputPort();

                            /*
                            auto start = std::chrono::high_resolution_clock::now();
                            DataObject::pointer data;
                            do {
                                data = stitcher->updateAndGetOutputData<DataObject>();
                            } while (!data->isLastFrame());
                             */

							auto someRenderer = SegmentationRenderer::New();
							someRenderer->setOpacity(0.7f, 1.0f);
							vector<string> colors = splitCustom(modelMetadata["class_colors"], ";");
							for (int i = 0; i < std::stoi(modelMetadata["nb_classes"]); i++) {
								vector<string> rgb = splitCustom(colors[i], ",");
								someRenderer->setColor(i, Color((float)std::stoi(rgb[0]) / 255.0f,
									(float)std::stoi(rgb[1]) / 255.0f,
									(float)std::stoi(rgb[2]) / 255.0f));
							}
							someRenderer->setInputConnection(stitcher->getOutputPort());

							m_rendererTypeList[modelMetadata["name"]] = "SegmentationRenderer";
							insertRenderer(modelMetadata["name"], someRenderer);
						}
						else {
							// check if folder for current WSI exists, if not, create one
							QString wsiResultPath = (projectFolderName.toStdString() + "/results/" +
								splitCustom(splitCustom(currWSI, "/").back(), ".")[0]).c_str();
							wsiResultPath = wsiResultPath.replace("//", "/");
							if (!QDir(wsiResultPath).exists()) {
								QDir().mkdir(wsiResultPath);
							}
							auto currPath =
								wsiResultPath.toStdString() + "/" +
								splitCustom(wsiResultPath.toStdString(), "/").back() +
								"_" + modelMetadata["name"] + "/";
							std::cout << "current high-res result path: " << currPath << std::endl;

							auto exporter = ImagePyramidPatchExporter::New();
							//exporter->setInputData(network->updateAndGetOutputData<Image>());
							exporter->setInputConnection(network->getOutputPort());
							exporter->setPath(currPath);

							//addProcessObject(exporter);  // TODO: Is this required when running the analysis without multi-threading? If included it seems like the segmentation is off-by-one (right-skewed)?

							auto port = network->getOutputPort();
							DataObject::pointer data;
							do {
								exporter->update();
								data = port->getNextFrame<DataObject>();
							} while (!data->isLastFrame());
						}
					}
					else if ((modelMetadata["problem"] == "object_detection") && (modelMetadata["resolution"] == "high")) {  // TODO: Perhaps use switch() instead of tons of if-statements?
					    // FIXME: Currently, need to do special handling for object detection as setThreshold and setAnchors only exist for BBNetwork and not NeuralNetwork

						auto currNetwork = BoundingBoxNetwork::New();
                        if (modelMetadata["pred_threshold"].empty()) {
                            std::cout << "No pred_threshold variable exists. Defaults to 0.1." << std::endl;
                        }
                        else {
                            currNetwork->setThreshold(std::stof(modelMetadata["nms_threshold"])); //0.01); // default: 0.5
                        }

						std::cout << "Current anchor file path: "
							<< cwd + "data/Models/" + someModelName + ".anchors"
							<< std::endl;

						// read anchors from corresponding anchor file
						std::vector<std::vector<Vector2f> > anchors;
						std::ifstream infile(cwd + "data/Models/" + someModelName + ".anchors");
						std::string anchorStr;
						while (std::getline(infile, anchorStr)) {
							std::vector<std::string> anchorVector = splitCustom(anchorStr, " ");
							anchorVector.resize(
								6); // for TinyYOLOv3 should only be 6 pairs, 3 for each level (2 levels)
							int cntr = 0;
							for (int i = 1; i < 3; i++) { // assumes TinyYOLOv3 (only two output layers)
								std::vector<Vector2f> levelAnchors;
								for (int j = 0; j < 3; j++) {
									auto currentPair = splitCustom(anchorVector[cntr], ",");
									levelAnchors.push_back(
										Vector2f(std::stoi(currentPair[0]), std::stoi(currentPair[1])));
									cntr++;
								}
								anchors.push_back(levelAnchors);
							}
						}
						currNetwork->setAnchors(anchors); // finally set anchors

						auto scale_factor = splitCustom(modelMetadata["scale_factor"],
							"/"); // get scale factor from metadata
						currNetwork->setScaleFactor((float)std::stoi(scale_factor[0]) /
							(float)std::stoi(scale_factor[1]));   // 1.0f/255.0f
						currNetwork->setInferenceEngine(
							"OpenVINO"); // FIXME: OpenVINO only currently, as I haven't generalized multiple output nodes case
						currNetwork->load(cwd + "data/Models/" + someModelName + "." + getModelFileExtension(
							currNetwork->getInferenceEngine()->getPreferredModelFormat())); //".uff");
						currNetwork->setInputConnection(generator->getOutputPort());

						auto nms = NonMaximumSuppression::New();
                        if (modelMetadata["nms_threshold"].empty()) {
                            std::cout << "No nms_threshold variable exists. Defaults to 0.5." << std::endl;
                        }
                        else {
                            nms->setThreshold(std::stof(modelMetadata["nms_threshold"]));
                        }
						nms->setInputConnection(currNetwork->getOutputPort());

						auto boxAccum = BoundingBoxSetAccumulator::New();
						boxAccum->setInputConnection(nms->getOutputPort());
						//boxAccum->setInputConnection(currNetwork->getOutputPort());

						auto boxRenderer = BoundingBoxRenderer::New();
						boxRenderer->setInputConnection(boxAccum->getOutputPort());

						m_rendererTypeList[modelMetadata["name"]] = "BoundingBoxRenderer";
						insertRenderer(modelMetadata["name"], boxRenderer);
					}
					else if ((modelMetadata["problem"] == "segmentation") &&
						(modelMetadata["resolution"] == "low")) {

						//auto converter = TensorToSegmentation::New();
						//converter->setInputConnection(network->getOutputPort());

						// resize back
						//auto access = currImage->getAccess(ACCESS_READ);
						auto input = access->getLevelAsImage(currLvl);

						ImageResizer::pointer resizer2 = ImageResizer::New();
						//resizer2->setInputData(converter->updateAndGetOutputData<Image>());
						resizer2->setInputConnection(network->getOutputPort());
						resizer2->setWidth(input->getWidth());
						resizer2->setHeight(input->getHeight());


						auto port2 = resizer2->getOutputPort();
						//m_tumorMap = port2->getNextFrame<Image>();
						resizer2->update();

						auto currMap = port2->getNextFrame<Image>();
						m_tumorMap = currMap;
						//auto currMap = m_tumorMap;

						if (!m_runForProject) {
							//m_tumorMap = currMap;

							currMap->setSpacing((float)currImage->getFullWidth() / (float)input->getWidth(),
								(float)currImage->getFullHeight() / (float)input->getHeight(),
								1.0f);

							auto someRenderer = SegmentationRenderer::New();
							someRenderer->setOpacity(0.4f);
							vector<string> colors = splitCustom(modelMetadata["class_colors"], ";");
							for (int i = 0; i < std::stoi(modelMetadata["nb_classes"]); i++) {
								vector<string> rgb = splitCustom(colors[i], ",");
								someRenderer->setColor(i, Color((float)std::stoi(rgb[0]) / 255.0f,
									(float)std::stoi(rgb[1]) / 255.0f,
									(float)std::stoi(rgb[2]) / 255.0f));
							}
							someRenderer->setInputData(currMap);
							//someRenderer->setInterpolation(false);
							someRenderer->update();

							m_rendererTypeList[modelMetadata["name"]] = "SegmentationRenderer";
							insertRenderer(modelMetadata["name"], someRenderer);
						}
						else {
							// save result
							QString wsiResultPath = (projectFolderName.toStdString() + "/results/" +
								splitCustom(splitCustom(currWSI, "/").back(), ".")[0]).c_str();
							wsiResultPath = wsiResultPath.replace("//", "/");
							std::cout << "current result path: " << wsiResultPath.toStdString() << std::endl;
							if (!QDir(wsiResultPath).exists()) {
								QDir().mkdir(wsiResultPath);
							}
							currMap->setSpacing(1.0f, 1.0f, 1.0f);

							auto exporter = ImageFileExporter::New();
							exporter->setFilename(
								wsiResultPath.toStdString() + "/" +
								splitCustom(splitCustom(currWSI, "/").back(), ".")[0] +
								"_" + modelMetadata["name"] + ".png");
							exporter->setInputData(currMap);
							exporter->update();
						}
					}
				}
			}
			// update progress bar
			// TODO: these are not updated in the main thread. Need to introduce signals such that the Qt-related stuff are properly updated in the main thread
			progDialog.setValue(counter);
			QCoreApplication::processEvents(QEventLoop::AllEvents, 0);
			counter++;
		}

		//emit inferenceFinished(someModelName);
		std::cout << "Inference thread is finished..." << std::endl;
    //} catch (const std::exception& e){
    //    simpleInfoPrompt("Something went wrong during inference.");
    };
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
        //key = v[0];
        //value = v[1];
        //metadata[key] = value;
        metadata.emplace(std::move(v[0]), std::move(v[1]));
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
std::vector<std::string> MainWindow::splitCustom(const std::string& s, const std::string& delimiter) {
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

const bool MainWindow::calcTissueHist() {
    std::cout << "Calculating histogram...";

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
        std::cout << std::to_string(i) << std::endl;
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

    statsLayout->insertWidget(1, smallTextWindowStats);
	
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

bool MainWindow::opacityRenderer(int value, const std::string& name) {
    if (m_rendererTypeList[name] == "ImagePyramidRenderer") {
        return false;
    }
    if (!hasRenderer(name)) {
        return false;
    }else{
        if (m_rendererTypeList[name] == "SegmentationRenderer") {
            auto someRenderer = std::dynamic_pointer_cast<SegmentationRenderer>(getRenderer(name));
            someRenderer->setOpacity((float) value / 20.0f);
        } else if (m_rendererTypeList[name] == "SegmentationRenderer") { // FIXME: Apparently, this doesn't change opacity
            auto someRenderer = std::dynamic_pointer_cast<SegmentationRenderer>(getRenderer(name));
            someRenderer->setOpacity((float) value / 20.0f);
        } else {
            auto someRenderer = std::dynamic_pointer_cast<HeatmapRenderer>(getRenderer(name));
            someRenderer->setMaxOpacity((float) value / 20.0f);
            //someRenderer->setMinConfidence(0.9);
        }
        return true;
    }
}

bool MainWindow::hideChannel(const std::string& name) {
    if (m_rendererTypeList[name] != "HeatmapRenderer") {
        return false;
    }
    if (!hasRenderer(name)) {
        return false;
    }else{
        background_flag = !background_flag;
        auto someRenderer = std::dynamic_pointer_cast<HeatmapRenderer>(getRenderer(name));
        someRenderer->setChannelHidden(channel_value, background_flag);
        return true;
    }
}

void MainWindow::deleteViewObject(std::string name) {
	if (m_rendererList.count(name) == 0)
		return;
    // need to remove from QComboBox, remove from renderer, remove potential saved results in project (need to check
    // if project is made first), and in the end, need to update ViewWidget
    //

	std::cout << "Current renderer: " << name << " , " << m_rendererList[name] << std::endl;

	//view->removeRenderer(m_rendererList[name]); //->resetRenderers();

	auto someRenderer = m_rendererList[name];
	getView(0)->removeRenderer(someRenderer);
	for (auto const&[key, val] : m_rendererList) {
		std::cout << "before: " << key << ": " << val << std::endl;
	}
	m_rendererList.erase(name);
	pageComboBox->removeItem(pageComboBox->findData(QString::fromStdString(name))); //pageComboBox->currentIndex());

	for (auto const& [key, val] : m_rendererList) {
		std::cout << "after: " << key << ": " << val << std::endl;
	}

	// need to update QComboBox as an element has been removed, to still keep the renderers mapped by index without any holes
	pageComboBox->clear();

	// then clear dynamic view widget layout, and add them again
	clearLayout(stackedLayout);
	for (auto const&[key, val] : m_rendererList) {
		modelName = "";
		if (modelNames.count(key) != 0) {
			modelName = modelNames[key];
		}
		createDynamicViewWidget(key, modelName);
	}

	// TODO: Should store QComboBox based on name, perhaps setTooltip instead, such that I dont need to think about indices.
	//	 But this requires that all results have unique names. Perhaps introduce this "make random unique name" for every object
	//	 that is rendered?

	// perhaps need to handle case when there is only one element left in the renderer
	std::cout << pageComboBox->count() << std::endl;
	if (pageComboBox->count() == 1) {
		pageComboBox->setCurrentIndex(0);
	}
	pageComboBox->update();
}

void MainWindow::insertRenderer(std::string name, std::shared_ptr<Renderer> renderer) {
    std::cout << "calling insert renderer" << std::endl;
    if (!hasRenderer(name)) {
        // Doesn't exist
        getView(0)->addRenderer(renderer);
        m_rendererList[name] = renderer;
        std::cout << "finished insert renderer" << std::endl;
    }
}

void MainWindow::removeAllRenderers() {
    m_rendererList.clear();
    getView(0)->removeAllRenderers();
}

bool MainWindow::hasRenderer(std::string name) {
    return m_rendererList.count(name) > 0;
}

std::shared_ptr<Renderer> MainWindow::getRenderer(std::string name) {
    if (!hasRenderer(name))
        throw Exception("Renderer with name " + name + " does not exist");
    return m_rendererList[name];
}

}
