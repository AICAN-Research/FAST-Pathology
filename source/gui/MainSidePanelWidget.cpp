#include "MainSidePanelWidget.h"
#include "MainWindow.hpp"

namespace fast{
    MainSidePanelWidget::MainSidePanelWidget(MainWindow* mainWindow, QWidget *parent): QWidget(parent)
    {
        m_mainWindow = mainWindow;
        setUpInterface();
        setupConnections();
    }

    MainSidePanelWidget::~MainSidePanelWidget(){

    }

    void MainSidePanelWidget::setUpInterface(){
        _container_stacked_widget = new QStackedWidget(this);
        _project_widget = new ProjectWidget(m_mainWindow, this);
        _process_widget = new ProcessWidget(m_mainWindow, this);
        _view_widget = new ViewWidget(m_mainWindow, this);
        //_stats_widget = new StatsWidget(this);
        //_export_widget = new ExportWidget(this);

        //stackedWidget->setStyleSheet("border:1px solid rgb(0, 255, 0); ");
        //stackedWidget->setFixedWidth(200);
        //stackedWidget->setStyleSheet("border:1px solid rgb(0, 0, 255); ");
        _container_stacked_widget->insertWidget(0, _project_widget);
        _container_stacked_widget->insertWidget(1, _process_widget);
        _container_stacked_widget->insertWidget(2, _view_widget);
        //_container_stacked_widget->insertWidget(3, _stats_widget);
        //_container_stacked_widget->insertWidget(4, _export_widget);
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
        tb->setFont(QFont("Ubuntu", 8)); //QFont::Bold));
        //tb->setStyleSheet("font-size: 20px");
        tb->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);  // adds text under icons

        //auto toolBar = new QToolBar;
        QPixmap menuIcon(QString::fromStdString(":/data/Icons/menu_icon.png"));
        QPixmap openPix(QString::fromStdString(":/data/Icons/import_icon_new_cropped_resized.png"));
        QPixmap processPix(QString::fromStdString(":/data/Icons/process_icon_new_cropped_resized.png"));
        QPixmap viewPix(QString::fromStdString(":/data/Icons/visualize_icon_new_cropped_resized.png"));
        //QPixmap resultPix(QString::fromStdString(":/data/Icons/statistics_icon_new_cropped_resized.png"));
        //QPixmap savePix(QString::fromStdString(":/data/Icons/export_icon_new_cropped_resized.png"));

        QPainter painter(&menuIcon);
        QFont font("Ubuntu", 4);
        painter.setFont(font);
        painter.setPen(*(new QColor(Qt::black)));
        //painter.drawText(QPoint(0, 500), "Read WSI");

        tb->addWidget(spacerWidgetLeft);

        auto actionGroup = new QActionGroup(tb);

        auto menu_action = new QAction("Menu", actionGroup);
        menu_action->setIcon(QIcon(menuIcon));
        menu_action->setCheckable(false);
        menu_action->setChecked(false);
        tb->addAction(menu_action);
        connect(menu_action, &QAction::triggered, this, &MainSidePanelWidget::showMenu);

        auto file_action = new QAction("Images", actionGroup);
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

        auto view_action = new QAction("Display", actionGroup);
        view_action->setIcon(QIcon(viewPix));
        view_action->setCheckable(true);
        tb->addAction(view_action);
        mapper->setMapping(view_action, 2);
        mapper->connect(view_action, SIGNAL(triggered(bool)), SLOT(map()));

        /*
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
         */

        tb->addWidget(spacerWidgetRight);

        //stackedWidget->connect(&mapper, SIGNAL(mapped(int)), SLOT(setCurrentIndex(int)));
        connect(mapper, SIGNAL(mapped(int)), _container_stacked_widget, SLOT(setCurrentIndex(int)));

        auto dockLayout = new QVBoxLayout; //or any other layout type you want
        dockLayout->setMenuBar(tb); // <-- the interesting part

        auto dockContent = new QWidget(this);
        dockContent->setLayout(dockLayout);

        dockLayout = new QVBoxLayout;
        dockLayout->insertWidget(0, dockContent); //addWidget(dockContent);
        //tmpLayout->addWidget(pageComboBox);
        dockLayout->insertWidget(1, _container_stacked_widget);

        setLayout(dockLayout);
    }

    void MainSidePanelWidget::resetInterface()
    {
        _project_widget->resetInterface();
        _process_widget->resetInterface();
        _view_widget->resetInterface();
        //_stats_widget->resetInterface();
        //_export_widget->resetInterface();
    }

    void MainSidePanelWidget::setupConnections()
    {
        QObject::connect(_project_widget, &ProjectWidget::changeWSIDisplayTriggered, this, &MainSidePanelWidget::changeWSIDisplayTriggered);
        QObject::connect(_project_widget, &ProjectWidget::resetDisplay, this, &MainSidePanelWidget::resetDisplay);
        QObject::connect(this, &MainSidePanelWidget::loadProject, _project_widget, &ProjectWidget::loadProject);
        QObject::connect(this, &MainSidePanelWidget::filesDropped, _project_widget, &ProjectWidget::selectFileDrag);
        QObject::connect(this, &MainSidePanelWidget::refreshPipelines, [=]() {
            _process_widget->refreshPipelines();
        });
        QObject::connect(_process_widget, &ProcessWidget::pipelineFinished, m_mainWindow, &MainWindow::changeWSIDisplayReceived);
        connect(m_mainWindow, &MainWindow::updateProjectTitle, _project_widget, &ProjectWidget::updateTitle);
    }

    ViewWidget *MainSidePanelWidget::getViewWidget() {
        return _view_widget;
    }
}
