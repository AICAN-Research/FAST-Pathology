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
    this->_application_name = "FastPathology";
    setTitle(this->_application_name);
    enableMaximized(); // <- function from Window.cpp

	cwd = QDir::homePath().toStdString() + "/fastpathology/";

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

    this->setupInterface();
    this->setupConnections();

    // Legacy stuff to remove.
    advancedMode = false;
}


void MainWindow::closeEvent (QCloseEvent *event)
{
    QMessageBox::StandardButton resBtn = QMessageBox::question(mWidget, QString::fromStdString(this->_application_name),
                                                                tr("Are you sure?\n"),
                                                                QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes,
                                                                QMessageBox::Yes);
    if (resBtn != QMessageBox::Yes) {
        event->ignore();
    } else {
        event->accept();
    }
}

void MainWindow::setupInterface()
{
    mWidget->setAcceptDrops(true); // to enable drag/drop events
    //mWidget->setMouseTracking(true);
    //mWidget->setFocusPolicy(Qt::StrongFocus);
    //mWidget->setFocusPolicy(Qt::ClickFocus);  //Qt::ClickFocus);

    // set window icon
    mWidget->setWindowIcon(QIcon(":/data/Icons/fastpathology_logo_large.png"));

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

    createMenubar();        // create Menubar
    createOpenGLWindow();   // create OpenGL window
}

void MainWindow::setupConnections()
{
    // @TODO. How to collect the closeEvent signal
    QObject::connect(mWidget, &WindowWidget::closeEvent, this, &MainWindow::closeEvent);
    // Overall
    QObject::connect(this->_side_panel_widget, &MainSidePanelWidget::newAppTitle, this, &MainWindow::updateAppTitleReceived);
    // @TODO. The drag and drop can be about anything? Images, models, pipelines, etc...?
    QObject::connect(mWidget, &WindowWidget::filesDropped, this->_side_panel_widget, &MainSidePanelWidget::filesDropped);
    QObject::connect(this->_side_panel_widget, &MainSidePanelWidget::addRendererToViewRequested, this, &MainWindow::addRendererToViewReceived);
    QObject::connect(this->_side_panel_widget, &MainSidePanelWidget::removeRendererFromViewRequested, this, &MainWindow::removeRendererFromViewReceived);
    QObject::connect(this->_side_panel_widget, &MainSidePanelWidget::changeWSIDisplayTriggered, this, &MainWindow::changeWSIDisplayReceived);
    QObject::connect(this->_side_panel_widget, &MainSidePanelWidget::resetDisplay, this, &MainWindow::resetDisplay);
    QObject::connect(this->_side_panel_widget, &MainSidePanelWidget::runPipelineEmitted, this, &MainWindow::runPipelineReceived);

    // Main menu actions
    QObject::connect(this->_file_menu_create_project_action, &QAction::triggered, this->_side_panel_widget, &MainSidePanelWidget::createProjectTriggered);
    QObject::connect(this->_file_menu_import_wsi_action, &QAction::triggered, this->_side_panel_widget, &MainSidePanelWidget::selectFilesTriggered);
    QObject::connect(this->_file_menu_add_model_action, &QAction::triggered, this->_side_panel_widget, &MainSidePanelWidget::addModelsTriggered);
    QObject::connect(this->_file_menu_add_pipeline_action, &QAction::triggered, this->_side_panel_widget, &MainSidePanelWidget::addPipelinesTriggered);

    QObject::connect(this->_project_menu_create_project_action, &QAction::triggered, this->_side_panel_widget, &MainSidePanelWidget::createProjectTriggered);
    QObject::connect(this->_project_menu_open_project_action, &QAction::triggered, this->_side_panel_widget, &MainSidePanelWidget::openProjectTriggered);
    QObject::connect(this->_project_menu_save_project_action, &QAction::triggered, this->_side_panel_widget, &MainSidePanelWidget::saveProjectTriggered);

    QObject::connect(this->_edit_menu_change_mode_action, &QAction::triggered, this->_side_panel_widget, &MainSidePanelWidget::setApplicationMode);
    QObject::connect(this->_edit_menu_download_testdata_action, &QAction::triggered, this->_side_panel_widget, &MainSidePanelWidget::downloadTestDataTriggered);

    QObject::connect(this->_pipeline_menu_import_action, &QAction::triggered, this->_side_panel_widget, &MainSidePanelWidget::addPipelinesTriggered);
    QObject::connect(this->_pipeline_menu_editor_action, &QAction::triggered, this->_side_panel_widget, &MainSidePanelWidget::editorPipelinesTriggered);

    QObject::connect(this->_help_menu_about_action, &QAction::triggered, this, &MainWindow::aboutProgram);
}

void MainWindow::updateAppTitleReceived(std::string title_suffix)
{
    this->setTitle(this->_application_name + title_suffix);
}

void MainWindow::createOpenGLWindow() {
	float OpenGL_background_color = 0.0f; //0.0f; //200.0f / 255.0f;
	view = createView();

	view->set2DMode();
	view->setBackgroundColor(Color(OpenGL_background_color, OpenGL_background_color, OpenGL_background_color)); // setting color to the background, around the WSI
	view->setAutoUpdateCamera(true);

	// create QSplitter for adjustable windows
	auto mainSplitter = new QSplitter(Qt::Horizontal);
	//mainSplitter->setHandleWidth(5);
	//mainSplitter->setFrameShadow(QFrame::Sunken);
	//mainSplitter->setStyleSheet("background-color: rgb(55, 100, 110);");
	mainSplitter->setHandleWidth(5);
	mainSplitter->setStyleSheet("QSplitter::handle { background-color: rgb(100, 100, 200); }; QMenuBar::handle { background-color: rgb(20, 100, 20); }");
    _side_panel_widget = new MainSidePanelWidget(this, mWidget); // create side panel with all user interactions
    mainSplitter->addWidget(_side_panel_widget);
	mainSplitter->addWidget(view);
	mainSplitter->setStretchFactor(1, 1);

    mainLayout->addWidget(mainSplitter);
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

void MainWindow::createMenubar()
{
    // need to create a new QHBoxLayout for the menubar
    auto topFiller = new QMenuBar(mWidget);
    //topFiller->setStyleSheet("QMenuBar::item:selected { background: white; }; QMenuBar::item:pressed {  background: white; };"
    //                         "border-bottom:2px solid rgba(25,25,120,75); "
    //                         "QMenu{background-color:palette(window);border:1px solid palette(shadow);}");
    //topFiller->setStyleSheet(qss);
    topFiller->setMaximumHeight(30);

    // File tab
    auto fileMenu = topFiller->addMenu(tr("&File"));
    //fileMenu->setFixedHeight(100);
    //fileMenu->setFixedWidth(100);
    //QAction *createProjectAction;
    this->_file_menu_create_project_action = new QAction("Create Project");
    fileMenu->addAction(this->_file_menu_create_project_action);
    this->_file_menu_import_wsi_action = new QAction("Import WSIs");
    fileMenu->addAction(this->_file_menu_import_wsi_action);
    this->_file_menu_add_model_action = new QAction("Add Models");
    fileMenu->addAction(this->_file_menu_add_model_action);
    this->_file_menu_add_pipeline_action = new QAction("Add Pipelines");
    fileMenu->addAction(this->_file_menu_add_pipeline_action);
    fileMenu->addSeparator();
    fileMenu->addAction("Quit", QApplication::quit);

    // Projects tab
    auto projectMenu = topFiller->addMenu(tr("&Projects"));
    this->_project_menu_create_project_action = new QAction("Create Project");
    projectMenu->addAction(this->_project_menu_create_project_action);
    this->_project_menu_open_project_action = new QAction("Open Project");
    projectMenu->addAction(this->_project_menu_open_project_action);
    this->_project_menu_save_project_action = new QAction("Save Project");
    projectMenu->addAction(this->_project_menu_save_project_action);
    projectMenu->addAction("Run for project", this, &MainWindow::runForProject);

    // Edit tab
    auto editMenu = topFiller->addMenu(tr("&Edit"));
    editMenu->addAction("Reset", this, &MainWindow::reset);
    this->_edit_menu_change_mode_action = new QAction("Change mode");
    editMenu->addAction(this->_edit_menu_change_mode_action);
    this->_edit_menu_download_testdata_action = new QAction("Download test data");
    editMenu->addAction(this->_edit_menu_download_testdata_action);

    // Pipelines tab
    this->_pipeline_menu = topFiller->addMenu(tr("&Pipelines"));
    this->_pipeline_menu_import_action = new QAction("Import pipelines");
    this->_pipeline_menu->addAction(this->_pipeline_menu_import_action);
    this->_pipeline_menu_editor_action = new QAction("Pipeline Editor");
    this->_pipeline_menu->addAction(this->_pipeline_menu_editor_action);
    runPipelineMenu = new QMenu("&Run Pipeline", mWidget);
    //runPipelineMenu->addAction("Grade classification");
    this->_pipeline_menu->addMenu(runPipelineMenu);
    loadPipelines(); // load pipelines that exists in the data/Pipelines directory

    // Deploy tab
    //auto this->_deploy_menu = new QMenu();
    this->_deploy_menu = topFiller->addMenu(tr("&Deploy"));
    //this->_deploy_menu->addMenu(tr("&Deploy"));
    //this->_deploy_menu->setFixedHeight(100);
    //this->_deploy_menu->setFixedWidth(100);
    this->_deploy_menu->addAction("Run Pipeline");
    this->_deploy_menu->addAction("Segment Tissue");
    this->_deploy_menu->addAction("Predict Tumor");
    this->_deploy_menu->addAction("Classify Grade");
    //this->_deploy_menu->addAction("MTL nuclei seg/detect", this, &MainWindow::MTL_test);
//    this->_deploy_menu->addAction("MIL bcgrade", this, &MainWindow::MIL_test);
//    this->_deploy_menu->addAction("Deep KMeans MTL", this, &MainWindow::Kmeans_MTL_test);
    this->_deploy_menu->addSeparator();

    this->_help_menu = topFiller->addMenu(tr("&Help"));
    this->_help_menu->addAction("Contact support", helpUrl);
    this->_help_menu->addAction("Report issue", reportIssueUrl);
    this->_help_menu->addAction("Check for updates");  // TODO: Add function that checks if the current binary in usage is the most recent one
    this->_help_menu_about_action = new QAction("About", this);
    this->_help_menu->addAction(this->_help_menu_about_action);

    //topFiller->addMenu(fileMenu);
    //topFiller->addMenu(this->_deploy_menu);

    superLayout->insertWidget(0, topFiller);
}

void MainWindow::reset()
{
    //first prompt warning, that it will delete all unsaved results, etc...
    if (! DataManager::GetInstance()->getCurrentProject()->isProjectEmpty())
    {
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
                removeAllRenderers();
                this->_side_panel_widget->resetInterface();
                break;
            case QMessageBox::No:
                1;
                break;
            default:
                break;
        }
    }

    // update application name to contain current WSI
    /*
    if (ProcessManager::GetInstance()->get_advanced_mode_status()) {
        setTitle(this->_application_name + " (Research mode)");
    } else {
        setTitle(this->_application_name);
    }*/

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

void MainWindow::changeWSIDisplayReceived(std::string uid_name, bool state)
{
    removeAllRenderers();
    if(state) {
        auto img = DataManager::GetInstance()->getCurrentProject()->getImage(uid_name);
        img->load_renderer();
        getView(0)->addRenderer(img->get_renderer("WSI"));
        DataManager::GetInstance()->setVisibleImageName(uid_name);
    }
    else
    {
        DataManager::GetInstance()->getCurrentProject()->getImage(uid_name)->unload_renderer();
        DataManager::GetInstance()->setVisibleImageName("");
    }
    getView(0)->reinitialize(); // Must call this after removing all renderers

    // update application name to contain current WSI
    if (advancedMode) {
        setTitle(this->_application_name + " (Research mode)" + " - " + splitCustom(uid_name, "/").back());
    } else {
        setTitle(this->_application_name + " - " + splitCustom(uid_name, "/").back());
    }

    // to render straight away (avoid waiting on all WSIs to be handled before rendering)
    QCoreApplication::processEvents(QEventLoop::AllEvents, 0);

    // Legacy to remove
    wsiFormat = metadata["openslide.vendor"]; // get WSI format
}

void MainWindow::updateChannelValue(int index) {
    channel_value = (uint) index;
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
//	for (const auto& item : wsiList) {
//		allFilesWidget->addItem(QString::fromStdString(item));
//	}
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
//		for (const auto& item : wsiList) {
//			selectedFilesWidget->addItem(QString::fromStdString(item));
//		}
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
//            QObject::connect(someButton, &QPushButton::clicked,
//                             std::bind(&MainWindow::pixelClassifier_wrapper, this, modelName));
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
//        QObject::connect(someButton, &QPushButton::clicked,
//                         std::bind(&MainWindow::pixelClassifier_wrapper, this, modelName));
        someButton->show();

        processLayout->insertWidget(processLayout->count(), someButton);

		progDialog.setValue(counter);
        QCoreApplication::processEvents(QEventLoop::AllEvents, 0);
		counter++;
    }
}

void MainWindow::loadHighres(QString path, QString name) {

}

void MainWindow::loadHeatmap(QString tissuePath, QString name) {

}

void MainWindow::loadSegmentation(QString tissuePath, QString name) {

}

void MainWindow::runPipeline(std::string path)
{
    auto progDialog = QProgressDialog(mWidget);
    progDialog.setRange(0, 1);
    progDialog.setVisible(true);
    progDialog.setModal(false);
    progDialog.setLabelText("Running pipeline across WSIs in project...");
    progDialog.move(mWidget->width() - progDialog.width() * 1.1, progDialog.height() * 0.1);
    progDialog.show();

    QCoreApplication::processEvents(QEventLoop::AllEvents, 0);

    auto visible_wsi_uid = DataManager::GetInstance()->getVisibleImageName();
    std::string dump_filepath = DataManager::GetInstance()->getCurrentProject()->getRootFolder() + "/results/" + visible_wsi_uid + "/";

    // if folder does not exist, create one
    if (!QDir(QString::fromStdString(dump_filepath)).exists())
    {
        QDir().mkdir(QString::fromStdString(dump_filepath));
    }

    // TODO: This now always saves result as TIFF, which is not correct. Need generic way of knowing which results that are exported and how to save these
    dump_filepath = dump_filepath + visible_wsi_uid + ".tiff";

    // TODO: Perhaps use corresponding .txt-file to feed arguments in the pipeline
    // pipeline requires some user-defined inputs, e.g. which WSI to use (and which model?)
    std::map<std::string, std::string> arguments;
    arguments["filename"] = DataManager::GetInstance()->get_visible_image()->get_filename();
    arguments["exportPath"] = dump_filepath;

    // parse fpl-file, and run pipeline with corresponding input arguments
    auto pipeline = Pipeline(path, arguments);
    pipeline.parse();

    // get and start running POs
    for (auto&& po : pipeline.getProcessObjects()) {
        if (po.second->getNrOfOutputPorts() == 0 && std::dynamic_pointer_cast<Renderer>(po.second) == nullptr) {
            // Process object has no output ports, must add to window to make sure it is updated.
            reportInfo() << "Process object " << po.first << " had no output ports defined in pipeline, therefore adding to window so it is updated." << reportEnd();
            addProcessObject(po.second);
        }
    }

    // load renderers, if any
    for (const auto& renderer : pipeline.getRenderers()) {
        auto currId = createRandomNumbers_(8);
        std::cout<<"Insert renderer\n";
//        insertRenderer("result_" + currId, renderer);
//        createDynamicViewWidget("result_" + currId, "result_" + currId);
    }

    // to render straight away (avoid waiting on all WSIs to be handled before rendering)
    QCoreApplication::processEvents(QEventLoop::AllEvents, 0);
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
//		createDynamicViewWidget(key, modelName);
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
//    m_rendererList.clear();
    getView(0)->removeAllRenderers();
    getView(0)->stopPipeline();
}

bool MainWindow::hasRenderer(std::string name) {
    return m_rendererList.count(name) > 0;
}

std::shared_ptr<Renderer> MainWindow::getRenderer(std::string name) {
    if (!hasRenderer(name))
        throw Exception("Renderer with name " + name + " does not exist");
    return m_rendererList[name];
}

void MainWindow::resetDisplay(){
    getView(0)->removeAllRenderers();
    getView(0)->reinitialize();
}

void MainWindow::addRendererToViewReceived(const std::string& name)
{
    if(DataManager::GetInstance()->get_visible_image()->has_renderer(name))
    {
        std::cout << "Inserting renderer with uid: " << name << std::endl;
        getView(0)->addRenderer(DataManager::GetInstance()->get_visible_image()->get_renderer(name));
    }
}

void MainWindow::removeRendererFromViewReceived(const std::string& name)
{
    if(DataManager::GetInstance()->get_visible_image()->has_renderer(name))
    {
        std::cout << "Removing renderer with uid: " << name << std::endl;
        getView(0)->removeRenderer(DataManager::GetInstance()->get_visible_image()->get_renderer(name));
    }
}

void MainWindow::runPipelineReceived(QString pipeline_uid)
{
}

std::shared_ptr<ComputationThread> MainWindow::getComputationThread() {
    return Window::getComputationThread();
}

std::string MainWindow::getRootFolder() const {
    return cwd;
}

std::shared_ptr<Project> MainWindow::getCurrentProject() {
    return DataManager::GetInstance()->getCurrentProject();
}

}
