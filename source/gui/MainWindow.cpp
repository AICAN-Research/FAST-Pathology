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
#include "source/gui/SplashWidget.hpp"
#include "source/logic/Project.h"

using namespace std;

namespace fast {

MainWindow::MainWindow() {
    _application_name = "FastPathology";
    setTitle(_application_name);
    enableMaximized(); // <- function from Window.cpp

	cwd = QDir::homePath().toStdString() + "/fastpathology/";

    // Create cwd if it doesn't exist
	QDir().mkpath(QString::fromStdString(cwd) + "models"); // <- mkpath creates the entire path recursively (convenient if subfolder dependency is missing)
    QDir().mkpath(QString::fromStdString(cwd) + "pipelines");
    QDir().mkpath(QString::fromStdString(cwd) + "projects");

    // Copy pipelines if they don't exist in pipelines and models folder
    auto dataPath = QCoreApplication::applicationDirPath().toStdString() + "/../data/";
    if(!isDir(dataPath)) {
        // For dev on windows
		dataPath = QCoreApplication::applicationDirPath().toStdString() + "/../../data/";
    }
    std::cout << "Data path was: " << dataPath << std::endl;
    for(std::string folder : {"pipelines"}) {
        for(auto filename : getDirectoryList(join(dataPath, folder))) {
            if(!isFile(join(cwd, folder, filename))) {
                std::cout << "File not found, copying to fastpathology folder: " << join(cwd, folder, filename) << std::endl;
                QFile::copy(QString::fromStdString(join(dataPath, folder, filename)), QString::fromStdString(join(cwd, folder, filename)));
            } else {
                //std::cout << "File already exists: " << join(cwd, folder, filename) << std::endl;
            }
        }
    }
    auto modelFiles = getDirectoryList(join(cwd, "models"), true, true);
    if(modelFiles.empty()) {
        auto reply = QMessageBox::question(nullptr,  "Download AI models?",
             "You have no AI models in your model folder. "
             "Do you wish to download some models now? (~450 MB)");
        if(reply == QMessageBox::Yes) {
            downloadZipFile("http://fast.eriksmistad.no/download/fastpathology-models-v1.0.0.zip", join(cwd, "models"));
        }
    }

    setupInterface();
    setupConnections();

    // Legacy stuff to remove.
    advancedMode = false;
    showSplashMenu(false);
}

void MainWindow::showSplashMenu(bool allowClose) {
    // Start splash
    auto splash = new ProjectSplashWidget(cwd + "/projects/", allowClose);
    connect(splash, &ProjectSplashWidget::quitSignal, mWidget, &QWidget::close);
    connect(splash, &ProjectSplashWidget::newProjectSignal, [=](QString name) {
        if(m_project)
            reset();
        std::cout << "Creating project with name " << name.toStdString() << std::endl;;
        m_project = std::make_shared<Project>(name.toStdString());
        emit updateProjectTitle();
    });
    connect(splash, &ProjectSplashWidget::openProjectSignal, [=](QString name) {
        if(m_project)
            reset();
        std::cout << "Opening project with name " << name.toStdString() << std::endl;;
        // TODO have some sort of progress bar here.
        m_project = std::make_shared<Project>(name.toStdString(), true); // <-- This takes time
        emit _side_panel_widget->loadProject();
        emit updateProjectTitle();
    });
    splash->show();
}
void MainWindow::showSplashMenuWithClose() {
    showSplashMenu(true);
}

void MainWindow::downloadZipFile(std::string URL, std::string destination) {
    createDirectories(destination);
    std::cout << "Progress: " << std::endl;
    QNetworkAccessManager manager;
    QUrl url(QString::fromStdString(URL));
    QNetworkRequest request(url);
    auto timer = new QElapsedTimer;
    timer->start();
    auto reply = manager.get(request);
    int step = 5;
    int progress = step;
    auto progressDialog = new QProgressDialog("Downloading AI models, please wait..", "Stop", 0, 101);
    progressDialog->setWindowTitle("Dowmloading AI models");
    progressDialog->setAutoClose(true);
    progressDialog->show();
    QObject::connect(reply, &QNetworkReply::downloadProgress, [&](quint64 current, quint64 max) {
        int percent = ((float)current / max) * 100;
        float speed = ((float)timer->elapsed() / 1000.0f)/percent;
        uint64_t remaining = speed * (100 - percent);
        if(percent >= progress) {
            std::cout << percent << "% - ETA ~" << (int)std::ceil((float)remaining / 60) << " minutes. " << std::endl;;
            progress += step;
        }
        progressDialog->setValue(percent);
    });
    auto tempLocation = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/data.zip";
    QFile file(tempLocation);
    if(!file.open(QIODevice::WriteOnly)) {
        throw Exception("Could not write to " + tempLocation.toStdString());
    }
    QObject::connect(reply, &QNetworkReply::readyRead, [&reply, &file]() {
        file.write(reply->read(reply->bytesAvailable()));
    });
    QObject::connect(&manager, &QNetworkAccessManager::finished, [&]() {
        std::cout << "Finished downloading file. Processing.." << std::endl;
        file.close();
        std::cout << "Unzipping the data file to: " << destination << std::endl;
        try {
            extractZipFile(file.fileName().toStdString(), destination);
        } catch(Exception & e) {
            std::cout << "ERROR: Zip extraction failed." << std::endl;
        }

        file.remove();
        std::cout << "Done." << std::endl;
        progressDialog->setValue(101);
    });

    auto eventLoop = new QEventLoop(&manager);

    // Make sure to quit the event loop when download is finished
    QObject::connect(&manager, &QNetworkAccessManager::finished, eventLoop, &QEventLoop::quit);

    // Wait for it to finish
    eventLoop->exec();
}


void MainWindow::closeEvent (QCloseEvent *event)
{
    QMessageBox::StandardButton resBtn = QMessageBox::question(mWidget, QString::fromStdString(_application_name),
                                                                tr("Are you sure you wish to quit?\n"),
                                                                QMessageBox::No | QMessageBox::Yes,
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
    const auto qss = "font-size: 18px;"
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

    createOpenGLWindow();   // create OpenGL window
}

void MainWindow::setupConnections()
{
    // Overall
    QObject::connect(_side_panel_widget, &MainSidePanelWidget::newAppTitle, this, &MainWindow::updateAppTitleReceived);
    // @TODO. The drag and drop can be about anything? Images, models, pipelines, etc...?
    QObject::connect(mWidget, &WindowWidget::filesDropped, _side_panel_widget, &MainSidePanelWidget::filesDropped);
    QObject::connect(_side_panel_widget, &MainSidePanelWidget::changeWSIDisplayTriggered, this, &MainWindow::changeWSIDisplayReceived);
    QObject::connect(_side_panel_widget, &MainSidePanelWidget::resetDisplay, this, &MainWindow::resetDisplay);
    QObject::connect(_side_panel_widget, &MainSidePanelWidget::showMenu, this, &MainWindow::showSplashMenuWithClose);
}

void MainWindow::updateAppTitleReceived(std::string title_suffix)
{
    setTitle(_application_name + title_suffix);
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

void MainWindow::reset()
{
    _side_panel_widget->resetInterface();
}

void MainWindow::changeWSIDisplayReceived(std::string uid_name)
{
    auto view = getView(0);
    view->removeAllRenderers();
    auto img = getCurrentProject()->getImage(uid_name);
    m_currentVisibleWSI = uid_name;

    auto renderer = ImagePyramidRenderer::create()
        ->connect(img->get_image_pyramid());
    view->addRenderer(renderer);

    auto results = getCurrentProject()->loadResults(uid_name);
    if(!results.empty()) {
        for(auto result : results) {
            view->addRenderer(result.renderer);
        }
        _side_panel_widget->getViewWidget()->setResults(results);
    }

    // update application name to contain current WSI
    if (advancedMode) {
        setTitle(_application_name + " (Research mode)" + " - " + splitCustom(uid_name, "/").back());
    } else {
        setTitle(_application_name + " - " + splitCustom(uid_name, "/").back());
    }

    // to render straight away (avoid waiting on all WSIs to be handled before rendering)
    QCoreApplication::processEvents(QEventLoop::AllEvents, 0);
}

void MainWindow::addPipelines() {

    QStringList ls = QFileDialog::getOpenFileNames(
            mWidget,
            tr("Select pipeline to add"), nullptr,
            tr("Pipeline Files (*.fpl)"),
            nullptr, QFileDialog::DontUseNativeDialog
    );

    // now iterate over all selected files and add selected files and corresponding ones to Pipelines/
    for (QString& fileName : ls) {

        if (fileName == "")
            continue;

        std::string someFile = getFileName(fileName.toStdString());
        std::string oldLocation = split(fileName.toStdString(), someFile)[0];
        std::string newLocation = join(cwd, "pipelines");
        std::string newPath = join(newLocation,  someFile);
        if (fileExists(newPath)) {
            auto reply = QMessageBox::warning(mWidget, "Error", "A pipeline with the filename " + QString::fromStdString(someFile) + " already exists.");
        } else {
            if (fileExists(fileName.toStdString())) {
                QFile::copy(fileName, QString::fromStdString(newPath));
            }
        }
        // TODO Update process widget?
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

        progDialog.setValue(counter);
        QCoreApplication::processEvents(QEventLoop::AllEvents, 0);
        counter++;
    }
}



void MainWindow::resetDisplay(){
    getView(0)->removeAllRenderers();
}

std::shared_ptr<ComputationThread> MainWindow::getComputationThread() {
    return Window::getComputationThread();
}

std::string MainWindow::getRootFolder() const {
    return cwd;
}

std::shared_ptr<Project> MainWindow::getCurrentProject() const {
    return m_project;
}

std::shared_ptr<WholeSlideImage> MainWindow::getCurrentWSI() const {
    return getCurrentProject()->getImage(getCurrentWSIUID());
}

std::string MainWindow::getCurrentWSIUID() const {
    return m_currentVisibleWSI;
}

}
