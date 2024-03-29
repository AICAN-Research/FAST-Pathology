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
    QDir().mkpath(QString::fromStdString(cwd) + "images");
    QDir().mkpath(QString::fromStdString(cwd) + "datahub");

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

    setupInterface();
    setupConnections();

    auto modelFiles = getDirectoryList(join(cwd, "models"), true, true);
    auto datahubFiles = getDirectoryList(join(cwd, "datahub"), true, true);
    if(modelFiles.empty() && datahubFiles.empty()) {
        auto reply = QMessageBox::question(nullptr,  "Download AI models?",
                                           "You have no AI models in your fastpathology folder. Our data hub contains publicly available AI models which can be downloaded. Do you wish to browse these now?");
        if(reply == QMessageBox::Yes) {
            showSplashMenu(false, true);
        } else {
            showSplashMenu(false, false);
        }
    } else {
        showSplashMenu(false, false);
    }
}

void MainWindow::showSplashMenu(bool allowClose, bool showDataHub) {
    // Start splash
    auto splash = new ProjectSplashWidget(cwd + "/projects/", allowClose);
    QObject::connect(splash, &ProjectSplashWidget::quitSignal, mWidget, &QWidget::close);
    QObject::connect(splash, &ProjectSplashWidget::refreshPipelines, _side_panel_widget, &MainSidePanelWidget::refreshPipelines);
    QObject::connect(splash, &ProjectSplashWidget::newProjectSignal, [=](QString name) {
        if(m_project)
            reset();
        std::cout << "Creating project with name " << name.toStdString() << std::endl;;
        m_project = std::make_shared<Project>(name.toStdString());
        emit updateProjectTitle();
    });
    QObject::connect(splash, &ProjectSplashWidget::openProjectSignal, [=](QString name) {
        if(m_project)
            reset();
        std::cout << "Opening project with name " << name.toStdString() << std::endl;;
        // TODO have some sort of progress bar here.
        m_project = std::make_shared<Project>(name.toStdString(), true); // <-- This takes time
        emit _side_panel_widget->loadProject();
        emit updateProjectTitle();
    });
    QObject::connect(splash, &ProjectSplashWidget::loadTestDataIntoProject, [=]() {
        if(m_project) {
            auto folder = join(cwd, "images");
            for(auto filename : getDirectoryList(folder)) {
                if(filename == "LICENSE.md")
                    continue;
                std::cout << "Loading " << filename << std::endl;
                m_project->includeImage(join(folder, filename));
            }
        }
        emit _side_panel_widget->loadProject();
        emit updateProjectTitle();
    });

    splash->show();
    if(showDataHub)
        splash->dataHub();
}
void MainWindow::showSplashMenuWithClose() {
    showSplashMenu(true);
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
    QApplication::setWindowIcon(QIcon(":/data/Icons/fastpathology_logo_large.png"));

    // changing color to the Qt background)
    QApplication::setStyle(QStyleFactory::create("Fusion")); // TODO: This has to be before setStyleSheet?
    const auto qss = "QMenuBar::item:selected { background: white; }; QMenuBar::item:pressed {  background: white; };" 
        "QMenu{background-color:palette(window);border:1px solid palette(shadow);};"  // This is needed for some strange reason..
        ;
    mWidget->setStyleSheet(qss);

    superLayout = new QVBoxLayout;
    auto oldLayout = mWidget->layout();
    delete oldLayout;
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
	view = createView();

	view->set2DMode();
	view->setBackgroundColor(Color(0.9, 0.9, 0.9)); // setting color to the background, around the WSI
	view->setAutoUpdateCamera(true);
	view->setScalebar(true);

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
	//mainSplitter->setSizes({(int)((float)mWidget->width()/4), (int)((float)mWidget->width()*3/4)});

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
    setTitle(_application_name + " - " + splitCustom(uid_name, "/").back());

    // to render straight away (avoid waiting on all WSIs to be handled before rendering)
    QCoreApplication::processEvents(QEventLoop::AllEvents, 0);
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
