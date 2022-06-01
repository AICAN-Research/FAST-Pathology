#include "MainWindow.hpp"
#include <FAST/Visualization/ImagePyramidRenderer/ImagePyramidRenderer.hpp>
#include <QPushButton>
#include <QFileDialog>
#include <QFile>
#include <FAST/Data/ImagePyramid.hpp>
#include <FAST/Data/Image.hpp>
#include <FAST/Visualization/BoundingBoxRenderer/BoundingBoxRenderer.hpp>
#include <string>
#include <QtWidgets>
#include <QWidget>
#include <QFont>
#include <QBoxLayout>
#include <QToolBar>
#include <QPainter>
#include <iostream>
#include <vector>
#include <stdio.h>      /* printf */
#include <istream>
#include <QColorDialog>
#include <QDir>
#include <QLayoutItem>
#include <QObject>
#include <future> // wait for callback is finished
#include <FAST/Utility.hpp>
#include <QUrl>
#include <QMessageBox>
#include <FAST/Data/Access/ImagePyramidAccess.hpp>
#include <FAST/Visualization/MultiViewWindow.hpp>
#include <QDragEnterEvent>
#include <QtNetwork>

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
    QObject::connect(this->_side_panel_widget, &MainSidePanelWidget::changeWSIDisplayTriggered, this, &MainWindow::changeWSIDisplayReceived);
    QObject::connect(this->_side_panel_widget, &MainSidePanelWidget::resetDisplay, this, &MainWindow::resetDisplay);

    // Main menu actions
    QObject::connect(this->_file_menu_create_project_action, &QAction::triggered, this->_side_panel_widget, &MainSidePanelWidget::createProjectTriggered);
    QObject::connect(this->_file_menu_import_wsi_action, &QAction::triggered, this->_side_panel_widget, &MainSidePanelWidget::selectFilesTriggered);
    QObject::connect(this->_file_menu_add_model_action, &QAction::triggered, this->_side_panel_widget, &MainSidePanelWidget::addModelsTriggered);
    QObject::connect(this->_file_menu_add_pipeline_action, &QAction::triggered, this, &MainWindow::addPipelines);

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

    this->_help_menu = topFiller->addMenu(tr("&Help"));
    this->_help_menu->addAction("Contact support", helpUrl);
    this->_help_menu->addAction("Report issue", reportIssueUrl);
    this->_help_menu->addAction("Check for updates");  // TODO: Add function that checks if the current binary in usage is the most recent one
    this->_help_menu_about_action = new QAction("About", this);
    this->_help_menu->addAction(this->_help_menu_about_action);

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
                getView(0)->removeAllRenderers();
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

void MainWindow::changeWSIDisplayReceived(std::string uid_name)
{
    auto view = getView(0);
    view->removeAllRenderers();
    auto img = DataManager::GetInstance()->getCurrentProject()->getImage(uid_name);
    DataManager::GetInstance()->setVisibleImageName(uid_name);

    auto renderer = ImagePyramidRenderer::create()
        ->connect(img->get_image_pyramid());
    view->addRenderer(renderer);

    auto results = DataManager::GetInstance()->getCurrentProject()->loadResults(uid_name);
    if(!results.empty()) {
        for(auto result : results) {
            view->addRenderer(result.renderer);
        }
        _side_panel_widget->getViewWidget()->setResults(results);
    }

    // update application name to contain current WSI
    if (advancedMode) {
        setTitle(this->_application_name + " (Research mode)" + " - " + splitCustom(uid_name, "/").back());
    } else {
        setTitle(this->_application_name + " - " + splitCustom(uid_name, "/").back());
    }

    // to render straight away (avoid waiting on all WSIs to be handled before rendering)
    QCoreApplication::processEvents(QEventLoop::AllEvents, 0);
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

            auto someButton = new QPushButton(mWidget);
            //someButton->setText(QString::fromStdString(currMetadata["task"]));
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

        // get metadata of current model

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

void MainWindow::resetDisplay(){
    getView(0)->removeAllRenderers();
    getView(0)->reinitialize();
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
