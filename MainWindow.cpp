#include "MainWindow.hpp"
#include <FAST/Importers/WholeSlideImageImporter.hpp>
#include <FAST/Visualization/ImagePyramidRenderer/ImagePyramidRenderer.hpp>
#include <QPushButton>
#include <QFileDialog>
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

using namespace std;

namespace fast {

MainWindow::MainWindow() {
    setTitle("FastPathology");
    enableMaximized(); // <- function from Window.cpp

    // changing color to the Qt background
    //mWidget->setStyleSheet("font-size: 16px; background: gray; color: white;");
    //mWidget->setStyleSheet("font-size: 16px; background: rgb(75, 74, 103); color: black;");
    mWidget->setStyleSheet("font-size: 16px; background: rgb(221, 209, 199); color: black;");
    //mWidget->setStyleSheet("font-size: 16px; background: rgb(181, 211, 231); color: black;");
    //mWidget->setStyleSheet("font-size: 16px; background: (255, 125, 50); color: black;");

    float OpenGL_background_color = 0.0f; //200.0f / 255.0f;


    // opacityTumorSlider->setStyleSheet("font-size: 16px"); // <- to set style sheet for a specific button/slider


    // create vertical menu layout
    //auto mainLayout = new QVBoxLayout;
    //mWidget->setLayout(mainLayout);

    //auto topHorizontalLayout = new QHBoxLayout;
    //mWidget->setLayout(topHorizontalLayout);

    /*
    auto fileMenu = new QMenuBar;
    fileMenu->addMenu(tr("&File"));
    fileMenu->setFixedHeight(100);
    fileMenu->setFixedWidth(200);
    fileMenu->addAction(newAct);
    */

    /*
    auto fileMenu = new QMenuBar();
    fileMenu->addMenu(tr("&File"));
    fileMenu->addAction(newAct);
    fileMenu->setFixedHeight(100);
    fileMenu->setFixedWidth(100);
    topHorizontalLayout->addWidget(fileMenu);

    auto deployMenu = new QMenuBar;
    deployMenu->addMenu(tr("&Deploy"));
    deployMenu->setFixedHeight(100);
    deployMenu->setFixedWidth(100);
    deployMenu->addAction("Segment Tissue");
    deployMenu->addAction("Predict tumor");
    deployMenu->addAction("Predict histological grade");
    topHorizontalLayout->addWidget(deployMenu);

    auto viewMenu = new QMenuBar;
    viewMenu->addMenu(tr("&View"));
    viewMenu->setFixedHeight(100);
    viewMenu->setFixedWidth(100);
    viewMenu->addAction("Choose method");
    viewMenu->addAction("Toggle");
    topHorizontalLayout->addWidget(viewMenu);
     */

    //mainestLayout->addLayout(topHorizontalLayout);

    // Create vertical GUI layout:
    auto mainLayout = new QHBoxLayout;
    mWidget->setLayout(mainLayout);

    auto fileLayout = new QVBoxLayout;
    //fileLayout->setAlignment(Qt::AlignTop);
    auto processLayout = new QVBoxLayout;
    auto viewLayout = new QVBoxLayout;
    auto statsLayout = new QVBoxLayout;
    auto saveLayout = new QVBoxLayout;

    processLayout->setSpacing(6); // 50
    processLayout->setAlignment(Qt::AlignTop);
    viewLayout->setAlignment(Qt::AlignTop);








    // ------------------------------------------------
    // --------------------- FILE ---------------------
    // ------------------------------------------------

    auto fileWidget = new QWidget;
    fileWidget->setLayout(fileLayout);
    //fileWidget->setFixedWidth(200);

    auto selectFileButton = new QPushButton(mWidget);
    selectFileButton->setText("Select WSI");
    //selectFileButton->setFixedWidth(200);
    selectFileButton->setFixedHeight(50);
    selectFileButton->setStyleSheet("color: white; background-color: blue");
    QObject::connect(selectFileButton, &QPushButton::clicked, std::bind(&MainWindow::selectFile, this));

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

    fileLayout->addWidget(selectFileButton); //, Qt::AlignTop);
    fileLayout->addWidget(smallTextWindow);
    fileLayout->addWidget(bigEditor);
    fileLayout->addWidget(quitButton, Qt::AlignTop);






    // -------------------------------------------------
    // -------------------- PROCESS --------------------
    // -------------------------------------------------

    auto processWidget = new QWidget;
    processWidget->setLayout(processLayout);
    //processWidget->setFixedWidth(200);

    auto segTissueButton = new QPushButton(mWidget);
    segTissueButton->setText("Segment Tissue");
    //segTissueButton->setFixedWidth(200);
    segTissueButton->setFixedHeight(50);
    QObject::connect(segTissueButton, &QPushButton::clicked, std::bind(&MainWindow::segmentTissue, this));

    auto predGradeButton = new QPushButton(mWidget);
    predGradeButton->setText("Predict histological grade");
    //predGradeButton->setFixedWidth(200);
    predGradeButton->setFixedHeight(50);
    QObject::connect(predGradeButton, &QPushButton::clicked, std::bind(&MainWindow::predictGrade, this));

    auto predTumorButton = new QPushButton(mWidget);
    predTumorButton->setText("Predict tumor");
    //predTumorButton->setFixedWidth(200);
    predTumorButton->setFixedHeight(50);
    QObject::connect(predTumorButton, &QPushButton::clicked, std::bind(&MainWindow::predictTumor, this));

    // /*
    auto predBachButton = new QPushButton(mWidget);
    predBachButton->setText("Predict BACH");
    //predBachButton->setFixedWidth(200);
    predBachButton->setFixedHeight(50);
    QObject::connect(predBachButton, &QPushButton::clicked, std::bind(&MainWindow::predictBACH, this));


    processLayout->addWidget(segTissueButton);
    processLayout->addWidget(predTumorButton);
    processLayout->addWidget(predGradeButton);
    processLayout->addWidget(predBachButton);








    // ------------------------------------------------
    // --------------------- VIEW ---------------------
    // ------------------------------------------------

    auto viewWidget = new QWidget;
    viewWidget->setLayout(viewLayout);
    //viewWidget->setFixedWidth(200);

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
    QObject::connect(showTissueMaskButton, &QPushButton::clicked, std::bind(&MainWindow::showTissueMask, this));

    ///*
    auto showBachMaskButton = new QPushButton(mWidget);
    showBachMaskButton->setText("Toggle bach map");
    //showBachMaskButton->setFixedWidth(200);
    showBachMaskButton->setFixedHeight(50);
    showBachMaskButton->setCheckable(true);
    showBachMaskButton->setChecked(true);
    QObject::connect(showBachMaskButton, &QPushButton::clicked, std::bind(&MainWindow::showBachMap, this));
    //*/

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

    auto fixImageButton = new QPushButton(mWidget);
    fixImageButton->setText("Reset View");
    //fixImageButton->setFixedWidth(200);
    fixImageButton->setFixedHeight(50);
    QObject::connect(fixImageButton, &QPushButton::clicked, std::bind(&MainWindow::fixImage, this));

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

    viewLayout->addWidget(showImageButton);
    viewLayout->addWidget(showTissueMaskButton);
    viewLayout->addWidget(showTumorButton);
    viewLayout->addWidget(showHeatmapButton);
    viewLayout->addWidget(showBachMaskButton);
    viewLayout->addWidget(smallTextBoxWidget_tissue);
    viewLayout->addWidget(smallTextBoxWidget_tumor);
    viewLayout->addWidget(smallTextBoxWidget_grade);
    viewLayout->addWidget(hideBackgroundButton);
    viewLayout->addWidget(fixImageButton);


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






    // ------------------------------------------------------
    // --------------------- Statistics ---------------------
    // ------------------------------------------------------

    auto statsWidget = new QWidget;
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

    /*
    auto m_ImageLabel = new QLabel;
    auto m_TextLabel = new QLabel;
    m_ImageLabel->setPixmap(pm);
    m_TextLabel->setText("Very nice histogram :)");
     */


    /*
    path2.moveTo(rect2.left() + (rect2.width() / 2), rect2.top());
    path2.lineTo(rect2.bottomLeft());
    path2.lineTo(rect2.bottomRight());
    path2.lineTo(rect2.left() + (rect2.width() / 2), rect2.top());
     */
    /*
    path2.moveTo(rect2.right(), rect2.top() - (rect2.height() / 2));
    path2.moveTo(rect2.topRight());
    path2.moveTo(rect2.bottomRight());
    path2.moveTo(rect2.right(), rect2.top() - (rect2.height() / 2));
    */

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

    //drawHist();


    /*
    auto set0 = new QBarSet("Jane");
    QBarSet *set1 = new QBarSet("John");
    QBarSet *set2 = new QBarSet("Axel");
    QBarSet *set3 = new QBarSet("Mary");
    QBarSet *set4 = new QBarSet("Samantha");

    *set0 << 1 << 2 << 3 << 4 << 5 << 6;
    *set1 << 5 << 0 << 0 << 4 << 0 << 7;
    *set2 << 3 << 5 << 8 << 13 << 8 << 5;
    *set3 << 5 << 6 << 7 << 3 << 4 << 5;
    *set4 << 9 << 7 << 5 << 3 << 1 << 2;

    QBarSeries *series = new QBarSeries();
    series->append(set0);
    series->append(set1);
    series->append(set2);
    series->append(set3);
    series->append(set4);

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Simple barchart example");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    QStringList categories;
    categories << "Jan" << "Feb" << "Mar" << "Apr" << "May" << "Jun";
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(0,15);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);
     */







    // --------------------------------------------------
    // --------------------- Export ---------------------
    // --------------------------------------------------

    auto exportWidget = new QWidget;
    exportWidget->setLayout(saveLayout);

    // make button that prints distribution of pixels of each class -> for histogram
    auto exportSegmentationButton = new QPushButton(mWidget);
    exportSegmentationButton->setText("Export segmentation");
    exportSegmentationButton->setFixedHeight(50);
    QObject::connect(exportSegmentationButton, &QPushButton::clicked, std::bind(&MainWindow::exportSeg, this));


    auto smallTextWindowExport = new QTextEdit;
    smallTextWindowExport->setPlainText(tr("Yes some info yes"));
    smallTextWindowExport->setReadOnly(true);

    saveLayout->addWidget(exportSegmentationButton);
    saveLayout->addWidget(smallTextWindowExport);







    // ------------------------------------------------
    // --------------------- MENU ---------------------
    // ------------------------------------------------

    auto stackedWidget = new QStackedWidget;
    //stackedWidget->setStyleSheet("border:1px solid rgb(0, 255, 0); ");
    //stackedWidget->setFixedWidth(200);
    //stackedWidget->setStyleSheet("border:1px solid rgb(0, 0, 255); ");
    stackedWidget->addWidget(fileWidget);
    stackedWidget->addWidget(processWidget);
    stackedWidget->addWidget(viewWidget);
    stackedWidget->addWidget(statsWidget);
    stackedWidget->addWidget(exportWidget);
    //stackedLayout->setSizeConstraint(QLayout::SetFixedSize);
    //stackedWidget->setLayout(mainLayout);

    //auto stackedWidget = new QWidget;
    //stackedWidget->setLayout(stackedLayout);

    int im_size = 40;

    auto mapper = new QSignalMapper;

    auto tb = new QToolBar();
    tb->setIconSize(QSize(im_size, im_size));



    // ----- CAN YOU PLEASE INSTALL filesystem library in FAST for C++ 17, please <3
    long size;
    char *buf;
    char *ptr;

    size = pathconf(".", _PC_PATH_MAX);

    if ((buf = (char *)malloc((size_t)size)) != NULL)
        ptr = getcwd(buf, (size_t)size);

    cwd = ptr;
    std::string delimiter = "cmake-build-release";
    std::string delimiter2 = "cmake-build-debug";

    // Search for the substring in string
    size_t pos = cwd.find(delimiter);

    if (pos != std::string::npos)
    {
        // If found then erase it from string
        cwd.erase(pos, delimiter.length());
    }

    // Search for the substring in string
    size_t pos2 = cwd.find(delimiter2);

    if (pos2 != std::string::npos)
    {
        // If found then erase it from string
        cwd.erase(pos2, delimiter2.length());
    }
    // ----- CAN YOU PLEASE INSTALL filesystem library in FAST for C++ 17, please <3




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


    auto tmpLayout = new QVBoxLayout;
    tmpLayout->addWidget(dockContent);
    //tmpLayout->addWidget(pageComboBox);
    tmpLayout->addWidget(stackedWidget);

    auto tmpWidget = new QWidget;
    tmpWidget->setFixedWidth(300);
    //tmpWidget->setStyleSheet("border:1px solid rgb(0, 255, 0); ");
    tmpWidget->setLayout(tmpLayout);


    mainLayout->addWidget(tmpWidget);

    /*
    auto menuLayout = new QVBoxLayout;

    auto menuBar = new QMenuBar;
    auto fileMenus = new QMenu(tr("&File"));
    auto exitAction = new QAction;
    exitAction = fileMenus->addAction(tr("&Exit"));
    auto processMenu = new QMenu(tr("&Process"));
    auto viewMenu = new QMenu(tr("&View"));
    auto saveMenu = new QMenu(tr("&Save"));

    menuBar->addMenu(fileMenus);
    menuBar->addMenu(processMenu);
    menuBar->addMenu(viewMenu);
    menuBar->addMenu(saveMenu);

    menuLayout->setMenuBar(menuBar);
    //menuLayout->addLayout(fileLayout);
    //menuLayout->addLayout(viewLayout);
    menuLayout->addLayout(stackedLayout);

    mainLayout->addLayout(menuLayout);
     */
    //setLayout(mainLayout);

    //stackedLayout->addItem(menuLayout);


    /*
    menuLayout->addWidget(selectFileButton);
    menuLayout->addWidget(segTissueButton);
    menuLayout->addWidget(predTumorButton);
    menuLayout->addWidget(predGradeButton);
    menuLayout->addWidget(predBachButton);
    //menuLayout->addWidget(fixTumorSegButton);
    //menuLayout->addWidget(saveTumorPredButton);
    menuLayout->addWidget(showHeatmapButton);
    menuLayout->addWidget(showTumorButton);
    menuLayout->addWidget(showBachMaskButton);
    menuLayout->addWidget(showTissueMaskButton);
    menuLayout->addWidget(showImageButton);
    menuLayout->addWidget(hideBackgroundButton);
    menuLayout->addWidget(fixImageButton);
    //menuLayout->addWidget(opacityTissueSlider);
    //menuLayout->addWidget(opacityHeatmapSlider);
    //menuLayout->addWidget(opacityTumorSlider);
    menuLayout->addLayout(smallTextBox_tissue);
    menuLayout->addLayout(smallTextBox_tumor);
    menuLayout->addLayout(smallTextBox_grade);
    menuLayout->addWidget(quitButton);
     */

    auto view = createView();

    //mainLayout->addLayout(menuLayout);
    mainLayout->addWidget(view);
    view->set2DMode();
    view->setBackgroundColor(Color(OpenGL_background_color, OpenGL_background_color, OpenGL_background_color)); // setting color to the background, around the WSI

    //mainestLayout->addLayout(mainLayout);



    //renderer->setSynchronizedRendering(true);

    segRenderer = SegmentationRenderer::New(); // <- fails here...
    segRenderer->setOpacity(0.4); // does something
    segRenderer->setColor(Segmentation::LABEL_FOREGROUND, Color(255.0/255.0, 127.0/255.0, 80.0/255.0));

    segTumorRenderer = SegmentationRenderer::New(); // <- fails here...
    segTumorRenderer->setOpacity(0.4); // does something
    segTumorRenderer->setColor(Segmentation::LABEL_FOREGROUND, Color(255.0/255.0f, 0.0f, 0.0f));
    segTumorRenderer->setColor(Segmentation::LABEL_BACKGROUND, Color(0.0f, 255.0f/255.0f, 0.0f));

    heatmapRenderer = HeatmapRenderer::New();
    heatmapRenderer->setChannelColor(0, Color::Blue());
    heatmapRenderer->setInterpolation(false);
    heatmapRenderer->setChannelColor(1, Color(50.0f/255.0f, 205.0f/255.0f, 50.0f/255.0f));
    heatmapRenderer->setChannelColor(2, Color(139.0f/255.0f, 0.0f, 0.0f));
    //heatmapRenderer->setImageImported(false);

    tumorRenderer = HeatmapRenderer::New();
    tumorRenderer->setChannelColor(0, Color::Blue());
    tumorRenderer->setInterpolation(false);
    //heatmapRenderer->setChannelColor(1, Color(50.0f/255.0f, 205.0f/255.0f, 50.0f/255.0f));
    tumorRenderer->setChannelColor(1, Color(139.0f/255.0f, 0.0f, 0.0f));
    //tumorRenderer->setChannelHidden(0, true);

    bachRenderer = HeatmapRenderer::New();
    bachRenderer->setChannelColor(0, Color::Blue());
    bachRenderer->setInterpolation(false);
    //heatmapRenderer->setChannelColor(1, Color(50.0f/255.0f, 205.0f/255.0f, 50.0f/255.0f));
    bachRenderer->setChannelColor(1, Color::Green());
    bachRenderer->setChannelColor(2, Color::Red());
    bachRenderer->setChannelColor(3, Color::Yellow());
    //tumorRenderer->setChannelHidden(0, true);

    //view->setAutoUpdateCamera(true);


    view->setAutoUpdateCamera(true);

    /*
    auto menuBar = new QMenuBar;

    auto fileMenus = new QMenu(tr("&File"));
    auto exitAction = new QAction;
    exitAction = fileMenus->addAction(tr("&Exit"));
    menuBar->addMenu(fileMenus);

    QObject::connect(exitAction, &QAction::triggered, std::bind(&QDialog::accept, this)); //this, &QDialog::accept);
     */

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

    //tumorRenderer->clearHeatmap();

    // Import image from file using the ImageFileImporter
    auto importer = WholeSlideImageImporter::New();
    importer->setFilename(fileName.toStdString());
    m_image = importer->updateAndGetOutputData<ImagePyramid>();

    renderer = ImagePyramidRenderer::New();
    renderer->setInputData(m_image);

    removeAllRenderers();
    insertRenderer("WSI", renderer);
    getView(0)->reinitialize(); // Must call this after removing all renderers
}

bool MainWindow::segmentTissue() {

    // Patch wise classification -> heatmap
    auto tissueSegmentation = TissueSegmentation::New();
    tissueSegmentation->setInputData(m_image);
    m_tissue = tissueSegmentation->updateAndGetOutputData<Image>();

    segRenderer->setInputData(m_tissue);
    segRenderer->setOpacity(0.4); // <- necessary for the quick-fix temporary solution

    insertRenderer("tissue", segRenderer);

    tissue_flag = true;
    showTissueMask();

    return true;
}

bool MainWindow::predictGrade() {
    int size = 512;

    //if (m_tumorMap)
       //tumorRenderer->clearHeatmap();

    // - if 512x512, 10x model is chosen
    auto generator = PatchGenerator::New();
    generator->setPatchSize(size, size);
    generator->setPatchLevel(2);
    generator->setInputData(0, m_image);
    if (m_tumorMap) {
        generator->setInputData(1, m_tumorMap);
        tissue_flag = false;  // <- by setting this to false, and running showTissueMask(), tissue mask is not being visualized at the start of running inference
        showTissueMask();
    }
    //if (m_tumorMap)
    //    generator->setInputData(1, m_tumorMap);

    auto network = NeuralNetwork::New();
    network->setInputNode(0, "input_1", NodeType::IMAGE, TensorShape({ 1, size, size, 3 }));
    network->setOutputNode(0, "dense_1/Softmax", NodeType::TENSOR, TensorShape({ 1, size, size, 3 }));

    //QString::fromStdString(cwd + "/Icons/import-data-icon-19.png");
    std::cout<<"japps2"<<"\n\n"<<cwd<<"\n\n";
    std::cout<<"jappsss"<<cwd + "/Models/bc_grade_model_3_grades_10x_" + std::to_string(size) + ".pb";
    network->load(cwd + "/Models/bc_grade_model_3_grades_10x_" + std::to_string(size) + ".pb");
    network->setInputConnection(generator->getOutputPort());
    network->setScaleFactor(1.0f/255.0f);

    //auto stitcher = PatchStitcher::New();
    //stitcher->setInputConnection(network->getOutputPort());
    //m_gradeMap = stitcher->updateAndGetOutputData<Tensor>();
    //heatmapRenderer->setInputConnection(stitcher->getOutputPort());
    //heatmapRenderer->setInputData(m_gradeMap); // <- this results in prediction/pw-rendering crashing/freezing

    auto stitcher = PatchStitcher::New();
    //stitcher->setInputData(network->getOutputPort());
    stitcher->setInputConnection(network->getOutputPort());

    auto port = stitcher->getOutputPort();
    //stitcher->update();
    //m_gradeMap = port->getNextFrame<Tensor>();

    //m_tumorMap = TensorToSegmentation()

    heatmapRenderer->setInputConnection(port);
    insertRenderer("grade", heatmapRenderer);

    return true;
}


bool MainWindow::predictTumor() {
    int size = 299;
    int res = 20;

    // - if 256x256, 10x model is chosen
    auto generator = PatchGenerator::New();
    generator->setPatchSize(size, size);
    generator->setPatchLevel(2);
    generator->setInputData(0, m_image);
    if (m_tissue) {
        generator->setInputData(1, m_tissue);
        tissue_flag = false;  // <- by setting this to false, and running showTissueMask(), tissue mask is not being visualized at the start of running inference
        showTissueMask();
    }

    auto batchgen = ImageToBatchGenerator::New();
    batchgen->setInputConnection(generator->getOutputPort());
    batchgen->setMaxBatchSize(8);

    auto network = NeuralNetwork::New();
    network->setInputNode(0, "input_1", NodeType::IMAGE, TensorShape({1, size, size, 3}));
    network->setOutputNode(0, "dense_1/Softmax", NodeType::IMAGE, TensorShape({1, size, size, 3}));
    network->load(cwd + "/Models/tumor_classify_model_" + std::to_string(res) + "x_" + std::to_string(size) + ".pb");
    network->setInputConnection(batchgen->getOutputPort());
    network->setScaleFactor(1.0f/255.0f);
    //network->setInferenceEngine("TensorFlowCPU");

    auto stitcher = PatchStitcher::New();
    //stitcher->setInputData(network->getOutputPort());
    stitcher->setInputConnection(network->getOutputPort());

    auto port = stitcher->getOutputPort();
    stitcher->update();
    m_tumorMap_tensor = port->getNextFrame<Tensor>();

    tumorRenderer->setInputConnection(port);

    auto tmp = TensorToSegmentation::New();
    tmp->setInputData(m_tumorMap_tensor);
    m_tumorMap = tmp->updateAndGetOutputData<Image>();

    //m_tumorMap = TensorToSegmentation()

    //tumorRenderer->setInputConnection(port);
    //tumorRenderer->setInputData(m_tumorMap);
    //tumorRenderer->setInputConnection(stitcher->getOutputPort());
    //m_tumorMap = stitcher->updateAndGetOutputData<Tensor>(); // Bad cast. Tried to cast from type Tensor to Image

    //m_tumorMap = stitcher->updateAndGetOutputData<Image>();
    //m_tumorMap = stitcher->updateAndGetOutputData<Tensor>();

    segTumorRenderer->setInputData(m_tumorMap);
    segTumorRenderer->setOpacity(0.8);

    insertRenderer("tumor", tumorRenderer);

    return true;
}


bool MainWindow::predictBACH() {

    auto generator = PatchGenerator::New();
    generator->setPatchSize(512, 512);
    generator->setPatchLevel(4);
    generator->setInputData(0, m_image);
    generator->setInputData(1, m_tissue);

    auto network = NeuralNetwork::New();
    network->setInputNode(0, "input_1", NodeType::IMAGE, TensorShape({1, 512, 512, 3}));
    network->setOutputNode(0, "sequential/dense_1/Softmax", NodeType::TENSOR, TensorShape({1, 512, 512, 4}));

    network->load(cwd + "/Models/inception_v3_bach_model.pb");
    network->setInputConnection(generator->getOutputPort());
    network->setScaleFactor(1.0f/255.0f);

    auto stitcher = PatchStitcher::New();
    stitcher->setInputConnection(network->getOutputPort());
    m_bachMap = stitcher->updateAndGetOutputData<Tensor>();
    bachRenderer->setInputConnection(stitcher->getOutputPort());

    return true;
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

    tumorRenderer->setInputData(m_tumorMap);

    return true;
}


bool MainWindow::showHeatmap() {
    if (!heatmapRenderer) {
        return false;
    }else{
        heatmapRenderer->setDisabled(!heatmapRenderer->isDisabled());
        return true;
    }
}

bool MainWindow::showBachMap() {
    if (!heatmapRenderer) {
        return false;
    }else{
        bachRenderer->setDisabled(!bachRenderer->isDisabled());
        return true;
    }
}

bool MainWindow::showTumorMask() {
    if (!tumorRenderer){
        return false;
    }else {
        tumorRenderer->setDisabled(!tumorRenderer->isDisabled());
        return true;
    }
}

bool MainWindow::showTissueMask() {
    if (!segRenderer){
        return false;
    }else {
        tissue_flag = !tissue_flag;
        segRenderer->setDisabled(tissue_flag);
        //segTumorRenderer->setDisabled(tissue_flag);
        return true;
    }
}

bool MainWindow::opacityTissue(int value) {
    //std::cout << (float)value/10.0f << std::endl;
    if (!segRenderer) {
        return false;
    }else{
        segRenderer->setOpacity((float) value / 10.0f); // does nothing?
        segRenderer->setModified(true);
        return true;
    }
}

bool MainWindow::opacityHeatmap(int value) {
    //std::cout << (float)value/10.0f << std::endl;
    if (!heatmapRenderer){
        return false;
    }else {
        heatmapRenderer->setMaxOpacity((float) value / 10.0f);
        return true;
    }
}

bool MainWindow::opacityTumor(int value) {
    //std::cout << (float)value/10.0f << std::endl;
    if (!tumorRenderer){
        return false;
    }else {
        tumorRenderer->setMaxOpacity((float) value / 10.0f);
        return true;
    }
}

bool MainWindow::showImage() {
    if(!hasRenderer("WSI")){
        return false;
    }else{
        auto renderer = getRenderer("WSI");
        renderer->setDisabled(!renderer->isDisabled());
        return true;
    }
}

bool MainWindow::hideBackgroundClass() {
    if (!tumorRenderer) {
        return false;
    }else if (tumorRenderer){
        std::cout<<background_flag<<"\n\n\n\n\n";
        background_flag = !background_flag;
        std::cout<<background_flag<<"\n\n\n\n\n";
        tumorRenderer->setChannelHidden(0, background_flag);
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
        getView(0)->reinitialize();
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

/*
bool MainWindow::onClicked() {
    if (!tumorRenderer){
        std::cout << btn << "\n\n\n\n";
        return false;
    }else {
        btn = !btn;
        std::cout << btn << "\n\n\n\n";
        tumorRenderer->setChannelHidden(0, btn);
        return true;
    }
}
 */

}
