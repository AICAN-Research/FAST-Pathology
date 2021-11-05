//
// Created by dbouget on 06.10.2021.
//

#include "MainSidePanelWidget.h"

namespace fast{
    MainSidePanelWidget::MainSidePanelWidget(QWidget *parent): QWidget(parent)
    {
        this->setUpInterface();
        this->setupConnections();
    }

    MainSidePanelWidget::~MainSidePanelWidget(){

    }

    void MainSidePanelWidget::setUpInterface(){
        this->_container_stacked_widget = new QStackedWidget(this);
        this->_project_widget = new ProjectWidget(this);
        this->_process_widget = new ProcessWidget(this);
        this->_view_widget = new ViewWidget(this);
        this->_view_widget->setVisible(false);
        this->_stats_widget = new StatsWidget(this);
        //stackedWidget->setStyleSheet("border:1px solid rgb(0, 255, 0); ");
        //stackedWidget->setFixedWidth(200);
        //stackedWidget->setStyleSheet("border:1px solid rgb(0, 0, 255); ");
        this->_container_stacked_widget->insertWidget(0, this->_project_widget);
        this->_container_stacked_widget->insertWidget(1, this->_process_widget);
        this->_container_stacked_widget->insertWidget(2, this->_view_widget); // @TODO.: Disable viewWidget at the start, as there is no images to visualize
        this->_container_stacked_widget->insertWidget(3, this->_stats_widget);
//        this->_container_stacked_widget->insertWidget(4, exportWidget);
        //stackedLayout->setSizeConstraint(QLayout::SetFixedSize);
        //stackedWidget->setLayout(mainLayout);

        int im_size = 40;
        auto mapper = new QSignalMapper(this);
        auto spacerWidgetLeft = new QWidget(this);
        spacerWidgetLeft->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        spacerWidgetLeft->setVisible(true);

        auto spacerWidgetRight = new QWidget(this);
        spacerWidgetRight->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        spacerWidgetRight->setVisible(true);

        auto tb = new QToolBar(this);
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
        connect(mapper, SIGNAL(mapped(int)), this->_container_stacked_widget, SLOT(setCurrentIndex(int)));

        auto dockLayout = new QVBoxLayout; //or any other layout type you want
        dockLayout->setMenuBar(tb); // <-- the interesting part

        auto dockContent = new QWidget(this);
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
        dockLayout->insertWidget(1, this->_container_stacked_widget);

        auto menuWidget = new QWidget(this);
        //menuWidget->setFixedWidth(300); //300);  // TODO: This was a good width for my screen, but need it to be adjustable (!)
        //menuWidget->set
        menuWidget->setMaximumWidth(700);
        menuWidget->setMinimumWidth(360);
        //tmpWidget->setStyleSheet("border:1px solid rgb(0, 255, 0); ");
        menuWidget->setLayout(dockLayout);

        // add button on the bottom of widget for toggling clinical/advanced mode
        this->_app_mode_pushbutton = new QPushButton(this);
        this->_app_mode_pushbutton->setText("Clinical mode");
        this->_app_mode_pushbutton->setFixedHeight(50);
        this->_app_mode_pushbutton->setStyleSheet("color: white; background-color: gray");

        dockLayout->addWidget(this->_app_mode_pushbutton);
        this->setLayout(dockLayout);
    }

    void MainSidePanelWidget::resetInterface()
    {
        this->_project_widget->resetInterface();
        this->_process_widget->resetInterface();
        this->_view_widget->resetInterface();
        this->_stats_widget->resetInterface();
    }

    void MainSidePanelWidget::setupConnections()
    {
//        QObject::connect(_project_widget, SIGNAL(&ProjectWidget::newImageDisplay), this, SIGNAL(&MainSidePanelWidget::newImageDisplay));
        QObject::connect(_project_widget, &ProjectWidget::newImageDisplay, this, &MainSidePanelWidget::newImageDisplay);
        QObject::connect(_project_widget, &ProjectWidget::resetDisplay, this, &MainSidePanelWidget::resetDisplay);
        QObject::connect(this->_process_widget, &ProcessWidget::processTriggered, this->_view_widget, &ViewWidget::processTriggerUpdate);
        QObject::connect(this->_process_widget, &ProcessWidget::addRendererToViewRequested, this, &MainSidePanelWidget::addRendererToViewRequested);
        QObject::connect(this->_app_mode_pushbutton, &QPushButton::clicked, this, &MainSidePanelWidget::setApplicationMode);
        QObject::connect(this, &MainSidePanelWidget::createProjectTriggered, this->_project_widget, &ProjectWidget::createProject);
        QObject::connect(this, &MainSidePanelWidget::openProjectTriggered, this->_project_widget, &ProjectWidget::openProject);
        QObject::connect(this, &MainSidePanelWidget::selectFilesTriggered, this->_project_widget, &ProjectWidget::selectFile);
        QObject::connect(this, &MainSidePanelWidget::saveProjectTriggered, this->_project_widget, &ProjectWidget::saveProject);
        QObject::connect(this, &MainSidePanelWidget::downloadTestDataTriggered, this->_project_widget, &ProjectWidget::downloadAndAddTestData);
        QObject::connect(this, &MainSidePanelWidget::filesDropped, this->_project_widget, &ProjectWidget::selectFileDrag);
        QObject::connect(this, &MainSidePanelWidget::addModelsTriggered, this->_process_widget, &ProcessWidget::addModels);
        QObject::connect(this, &MainSidePanelWidget::addPipelinesTriggered, this->_process_widget, &ProcessWidget::addPipelines);
        QObject::connect(this, &MainSidePanelWidget::editorPipelinesTriggered, this->_process_widget, &ProcessWidget::editorPipelinesReceived);
    }

    void MainSidePanelWidget::setApplicationMode() {
        QMessageBox mBox;
        mBox.setIcon(QMessageBox::Warning);
        if (this->_app_mode_pushbutton->text() == "Clinical mode") {
            mBox.setText("This will set the application from clinical mode to advanced mode.");
        } else {
            mBox.setText("This will set the application from advanced mode to clinical mode.");
        }
        mBox.setInformativeText("Are you sure you want to change mode?");
        mBox.setDefaultButton(QMessageBox::No);
        mBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        int ret = mBox.exec();

        // @TODO. Is the filename supposed to be the path of the displayed WSI, or project path, or...?
        auto filename = DataManager::GetInstance()->getVisibleImageName();
        switch (ret) {
            case QMessageBox::Yes:
                // toggle and update text on button to show current mode
                ProcessManager::GetInstance()->set_advanced_mode_status(!ProcessManager::GetInstance()->get_advanced_mode_status()); // toggle
                if (this->_app_mode_pushbutton->text() == "Clinical mode") {
                    this->_app_mode_pushbutton->setText("Research mode");
                } else {
                    this->_app_mode_pushbutton->setText("Clinical mode");
                }
                // also update title
                if (ProcessManager::GetInstance()->get_advanced_mode_status()) {
                    auto app_title_extension = std::string(" (Research mode)") + std::string(" - ") + splitCustom(filename, "/").back();
                    emit newAppTitle(app_title_extension);
                }
                else {
                    auto app_title_extension = " - " + splitCustom(filename, "/").back();
                    emit newAppTitle(app_title_extension);
                }
                break;
            case QMessageBox::No:
                1; // if "No", do nothing
                break;
            default:
                break;
        }
    }
}
