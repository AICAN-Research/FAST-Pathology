#include "MainWindow.hpp"
#include <FAST/Importers/WholeSlideImageImporter.hpp>
#include <FAST/Visualization/ImagePyramidRenderer/ImagePyramidRenderer.hpp>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QCheckBox>
#include <QSlider>
#include <QMenuBar>
#include <QMainWindow>
#include <FAST/Algorithms/NeuralNetwork/SegmentationNetwork.hpp>
#include <FAST/Data/ImagePyramid.hpp>
#include <FAST/Data/Image.hpp>
//#include <FAST/Algorithms/TissueSegmentation/TissueSegmentation.hpp>
#include "TissueSegmentationOtsu/TissueSegmentationOtsu.hpp"
#include <FAST/Algorithms/ImagePatch/PatchGenerator.hpp>
#include <FAST/Algorithms/ImagePatch/PatchStitcher.hpp>
#include <FAST/Algorithms/NeuralNetwork/NeuralNetwork.hpp>
#include <FAST/Visualization/HeatmapRenderer/HeatmapRenderer.hpp>
#include <FAST/Visualization/ImagePyramidRenderer/ImagePyramidRenderer.hpp>
#include <FAST/Algorithms/ImagePatch/ImageToBatchGenerator.hpp>
#include <FAST/Visualization/SegmentationRenderer/SegmentationRenderer.hpp>

#include <typeinfo>
#include <string>

/*
#include <FAST/Data/ImagePyramid.hpp>
#include <FAST/Algorithms/TissueSegmentation/TissueSegmentation.hpp> // <- correct?
#include <FAST/Algorithms/Morphology/Dilation.hpp>
#include <FAST/Algorithms/Morphology/Erosion.hpp>
#include <FAST/Algorithms/GaussianSmoothingFilter/GaussianSmoothingFilter.hpp>
 */

namespace fast {

MainWindow::MainWindow() {
    setTitle("FAST-Pathology");
    enableMaximized(); // <- function from Window.cpp

    // changing color to the Qt background
    mWidget->setStyleSheet("background: gray; color: white;");

    // create vertical menu layout
    auto mainestLayout = new QVBoxLayout;
    mWidget->setLayout(mainestLayout);

    auto topHorizontalLayout = new QHBoxLayout;
    mWidget->setLayout(topHorizontalLayout);

    auto fileMenu = new QMenuBar;
    fileMenu->addMenu(tr("&File"));
    fileMenu->setFixedHeight(100);
    fileMenu->setFixedWidth(200);
    fileMenu->addAction("Import WSI");
    topHorizontalLayout->addWidget(fileMenu);

    auto deployMenu = new QMenuBar;
    deployMenu->addMenu(tr("&Deploy"));
    deployMenu->setFixedHeight(100);
    deployMenu->setFixedWidth(200);
    deployMenu->addAction("Segment Tissue");
    deployMenu->addAction("Predict tumor");
    deployMenu->addAction("Predict histological grade");
    topHorizontalLayout->addWidget(deployMenu);

    auto viewMenu = new QMenuBar;
    viewMenu->addMenu(tr("&View"));
    viewMenu->setFixedHeight(100);
    viewMenu->setFixedWidth(200);
    viewMenu->addAction("Choose method");
    viewMenu->addAction("Toggle");
    topHorizontalLayout->addWidget(viewMenu);


    mainestLayout->addLayout(topHorizontalLayout);


    // Create vertical GUI layout:
    auto mainLayout = new QHBoxLayout;
    mWidget->setLayout(mainLayout);

    auto selectFileButton = new QPushButton(mWidget);
    selectFileButton->setText("Select file");
    selectFileButton->setFixedWidth(200);
    selectFileButton->setFixedHeight(50);
    QObject::connect(selectFileButton, &QPushButton::clicked, std::bind(&MainWindow::selectFile, this));

    auto showHeatmapButton = new QPushButton(mWidget);
    showHeatmapButton->setText("Toggle grade map");
    showHeatmapButton->setFixedWidth(200);
    showHeatmapButton->setFixedHeight(50);
    showHeatmapButton->setCheckable(true);
    showHeatmapButton->setChecked(true);
    //showHeatmapButton->setChecked(true);
    QObject::connect(showHeatmapButton, &QPushButton::clicked, std::bind(&MainWindow::showHeatmap, this));

    auto showTissueMaskButton = new QPushButton(mWidget);
    showTissueMaskButton->setText("Toggle tissue mask");
    showTissueMaskButton->setFixedWidth(200);
    showTissueMaskButton->setFixedHeight(50);
    showTissueMaskButton->setCheckable(true);
    showTissueMaskButton->setChecked(true);
    QObject::connect(showTissueMaskButton, &QPushButton::clicked, std::bind(&MainWindow::showTissueMask, this));

    auto showBachMaskButton = new QPushButton(mWidget);
    showBachMaskButton->setText("Toggle bach map");
    showBachMaskButton->setFixedWidth(200);
    showBachMaskButton->setFixedHeight(50);
    showBachMaskButton->setCheckable(true);
    showBachMaskButton->setChecked(true);
    QObject::connect(showBachMaskButton, &QPushButton::clicked, std::bind(&MainWindow::showBachMap, this));

    auto showImageButton = new QPushButton(mWidget);
    showImageButton->setText("Toggle WSI");
    showImageButton->setFixedWidth(200);
    showImageButton->setFixedHeight(50);
    showImageButton->setCheckable(true);
    showImageButton->setChecked(true);
    QObject::connect(showImageButton, &QPushButton::clicked, std::bind(&MainWindow::showImage, this));

    auto showTumorButton = new QPushButton(mWidget);
    showTumorButton->setText("Toggle tumor map");
    showTumorButton->setFixedWidth(200);
    showTumorButton->setFixedHeight(50);
    showTumorButton->setCheckable(true);
    showTumorButton->setChecked(true);
    QObject::connect(showTumorButton, &QPushButton::clicked, std::bind(&MainWindow::showTumorMask, this));

    auto hideBackgroundButton = new QPushButton(mWidget);
    hideBackgroundButton->setText("Hide background class");
    hideBackgroundButton->setFixedWidth(200);
    hideBackgroundButton->setFixedHeight(50);
    hideBackgroundButton->setCheckable(true);
    hideBackgroundButton->setChecked(true);
    QObject::connect(hideBackgroundButton, &QPushButton::clicked, std::bind(&MainWindow::hideBackgroundClass, this));

    auto quitButton = new QPushButton(mWidget);
    quitButton->setText("Quit");
    quitButton->setFixedWidth(200);
    quitButton->setFixedHeight(50);
    quitButton->setStyleSheet("color: black; background-color: red"); //; border-style: outset; border-color: black; border-width: 3px");
    QObject::connect(quitButton, &QPushButton::clicked, std::bind(&Window::stop, this));

    auto segTissueButton = new QPushButton(mWidget);
    segTissueButton->setText("Segment Tissue");
    segTissueButton->setFixedWidth(200);
    segTissueButton->setFixedHeight(50);
    QObject::connect(segTissueButton, &QPushButton::clicked, std::bind(&MainWindow::segmentTissue, this));

    auto predGradeButton = new QPushButton(mWidget);
    predGradeButton->setText("Predict histological grade");
    predGradeButton->setFixedWidth(200);
    predGradeButton->setFixedHeight(50);
    QObject::connect(predGradeButton, &QPushButton::clicked, std::bind(&MainWindow::predictGrade, this));

    auto predTumorButton = new QPushButton(mWidget);
    predTumorButton->setText("Predict tumor");
    predTumorButton->setFixedWidth(200);
    predTumorButton->setFixedHeight(50);
    QObject::connect(predTumorButton, &QPushButton::clicked, std::bind(&MainWindow::predictTumor, this));

    auto predBachButton = new QPushButton(mWidget);
    predBachButton->setText("Predict BACH");
    predBachButton->setFixedWidth(200);
    predBachButton->setFixedHeight(50);
    QObject::connect(predBachButton, &QPushButton::clicked, std::bind(&MainWindow::predictBACH, this));

    auto fixTumorSegButton = new QPushButton(mWidget);
    fixTumorSegButton->setText("Fix tumor segment");
    fixTumorSegButton->setFixedWidth(200);
    fixTumorSegButton->setFixedHeight(50);
    QObject::connect(fixTumorSegButton, &QPushButton::clicked, std::bind(&MainWindow::fixTumorSegment, this));

    auto saveTumorPredButton = new QPushButton(mWidget);
    saveTumorPredButton->setText("Save tumor pred");
    saveTumorPredButton->setFixedWidth(200);
    saveTumorPredButton->setFixedHeight(50);
    QObject::connect(saveTumorPredButton, &QPushButton::clicked, std::bind(&MainWindow::saveTumorPred, this));

    auto fixImageButton = new QPushButton(mWidget);
    fixImageButton->setText("Reset View");
    fixImageButton->setFixedWidth(200);
    fixImageButton->setFixedHeight(50);
    QObject::connect(fixImageButton, &QPushButton::clicked, std::bind(&MainWindow::fixImage, this));


    auto opacityTissueSlider = new QSlider(Qt::Horizontal, mWidget);
    opacityTissueSlider->setFixedWidth(200);
    opacityTissueSlider->setMinimum(0);
    opacityTissueSlider->setMaximum(10);
    //opacityTissueSlider->setText("Tissue");
    opacityTissueSlider->setValue(4);
    opacityTissueSlider->setTickInterval(1);
    QObject::connect(opacityTissueSlider, &QSlider::valueChanged, std::bind(&MainWindow::opacityTissue, this, std::placeholders::_1));

    auto opacityHeatmapSlider = new QSlider(Qt::Horizontal, mWidget);
    opacityHeatmapSlider->setFixedWidth(200);
    opacityHeatmapSlider->setMaximum(0);
    opacityHeatmapSlider->setMaximum(10);
    opacityHeatmapSlider->setValue(6);
    opacityHeatmapSlider->setTickInterval(1);
    QObject::connect(opacityHeatmapSlider, &QSlider::valueChanged, std::bind(&MainWindow::opacityHeatmap, this, std::placeholders::_1));

    auto opacityTumorSlider = new QSlider(Qt::Horizontal, mWidget);
    opacityTumorSlider->setFixedWidth(200);
    opacityTumorSlider->setMaximum(0);
    opacityTumorSlider->setMaximum(10);
    opacityTumorSlider->setValue(6);
    opacityTumorSlider->setTickInterval(1);
    QObject::connect(opacityTumorSlider, &QSlider::valueChanged, std::bind(&MainWindow::opacityTumor, this, std::placeholders::_1));

    auto menuLayout = new QVBoxLayout;
    menuLayout->addWidget(selectFileButton);
    menuLayout->addWidget(segTissueButton);
    menuLayout->addWidget(predTumorButton);
    menuLayout->addWidget(predGradeButton);
    menuLayout->addWidget(predBachButton);
    menuLayout->addWidget(fixTumorSegButton);
    menuLayout->addWidget(saveTumorPredButton);
    menuLayout->addWidget(showHeatmapButton);
    menuLayout->addWidget(showTumorButton);
    menuLayout->addWidget(showBachMaskButton);
    menuLayout->addWidget(showTissueMaskButton);
    menuLayout->addWidget(showImageButton);
    menuLayout->addWidget(hideBackgroundButton);
    menuLayout->addWidget(fixImageButton);
    menuLayout->addWidget(opacityTissueSlider);
    menuLayout->addWidget(opacityHeatmapSlider);
    menuLayout->addWidget(opacityTumorSlider);
    menuLayout->addWidget(quitButton);


    auto view = createView();

    mainLayout->addLayout(menuLayout);
    mainLayout->addWidget(view);
    view->set2DMode();
    view->setBackgroundColor(Color(150.0f/255.0f, 150.0f/255.0f, 150.0f/255.0f)); // setting color to the background, around the WSI

    mainestLayout->addLayout(mainLayout);

    renderer = ImagePyramidRenderer::New();
    //renderer->setSynchronizedRendering(true);

    segRenderer = SegmentationRenderer::New(); // <- fails here...
    segRenderer->setOpacity(0.4); // does something
    segRenderer->setColor(Segmentation::LABEL_FOREGROUND, Color(255.0/255.0, 127.0/255.0, 80.0/255.0));

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

    //renderer->setSynchronizedRendering(true);
    //tumorRenderer->setSynchronizedRendering(true);
    //heatmapRenderer->setSynchronizedRendering(true);

    view->addRenderer(renderer);
    view->addRenderer(segRenderer);
    view->addRenderer(heatmapRenderer); // <- cannot add renderer here(?). I think it is because when adding the renderer, it automatically starts running, and there is no input defined...
    view->addRenderer(tumorRenderer);
    view->addRenderer(bachRenderer);
    view->setAutoUpdateCamera(true);
}


void MainWindow::selectFile() {

    auto fileName = QFileDialog::getOpenFileName(
            mWidget,
            "Open File", nullptr, "WSI Files (*.tiff *.tif *.svs)"
            );

    if(fileName == "")
        return;
    filename = fileName.toStdString();

    // "clear" pointers at the start
    /*
    m_tissue = NULL;
    m_image = NULL;
    m_tumorMap = NULL;
    m_bachMap = NULL;
    m_bachMap = NULL;
     */

    //tumorRenderer->clearHeatmap();


    // stop previous pipelines
    //renderer->stopPipeline();
    //tumorRenderer->stopPipeline();
    //stopComputationThread();
    //renderer->clearPyramid();
    //startComputationThread();

    // if new file is chosen, clear all cache of previous images
    renderer->clearPyramid();

    tumorRenderer->clearHeatmap();

    // Import image from file using the ImageFileImporter
    auto importer = WholeSlideImageImporter::New();
    importer->setFilename(fileName.toStdString());
    //m_image = importer->updateAndGetOutputData<ImagePyramid>();
    //renderer->setInputData(m_image);
    renderer->setInputConnection(importer->getOutputPort());
    m_image = importer->updateAndGetOutputData<ImagePyramid>();
}

bool MainWindow::segmentTissue() {

    // Patch wise classification -> heatmap
    auto tissueSegmentation = TissueSegmentation::New();
    tissueSegmentation->setInputData(m_image);
    m_tissue = tissueSegmentation->updateAndGetOutputData<Image>();

    segRenderer->setInputData(m_tissue);
    segRenderer->setOpacity(0.4); // <- necessary for the quickfix temporary solution

    return true;
}

bool MainWindow::predictGrade() {

    int size = 512;

    // - if 512x512, 10x model is chosen
    auto generator = PatchGenerator::New();
    generator->setPatchSize(size, size);
    generator->setPatchLevel(2);
    generator->setInputData(0, m_image);
    if (m_tissue)
        generator->setInputData(1, m_tissue);
    if (m_tumorMap)
        generator->setInputData(1, m_tumorMap);

    auto network = NeuralNetwork::New();
    network->setInputNode(0, "input_1", NodeType::IMAGE, TensorShape({ 1, size, size, 3 }));
    network->setOutputNode(0, "dense_1/Softmax", NodeType::TENSOR, TensorShape({ 1, size, size, 3 }));
    network->load("/mnt/ssd/workspace/FAST/models/bc_grade_model_3_grades_10x_" + std::to_string(size) + ".pb");
    network->setInputConnection(generator->getOutputPort());
    network->setScaleFactor(1.0f/255.0f);

    auto stitcher = PatchStitcher::New();
    stitcher->setInputConnection(network->getOutputPort());
    m_gradeMap = stitcher->updateAndGetOutputData<Tensor>();
    heatmapRenderer->setInputConnection(stitcher->getOutputPort());
    //heatmapRenderer->setInputData(m_gradeMap); // <- this results in prediction/pw-rendering crashing/freezing

    return true;
}


bool MainWindow::predictTumor() {

    int size = 256;

    // - if 256x256, 10x model is chosen
    auto generator = PatchGenerator::New();
    generator->setPatchSize(size, size);
    generator->setPatchLevel(2);
    generator->setInputData(0, m_image);
    if (m_tissue)
        generator->setInputData(1, m_tissue);

    auto network = NeuralNetwork::New();
    network->setInputNode(0, "input_1", NodeType::IMAGE, TensorShape({1, size, size, 3}));
    network->setOutputNode(0, "sequential/dense_1/Softmax", NodeType::TENSOR, TensorShape({1, size, size, 3}));
    network->load("/mnt/ssd/workspace/FAST/models/tumor_classify_model_10x_" + std::to_string(size) + ".pb");
    network->setInputConnection(generator->getOutputPort());
    network->setScaleFactor(1.0f/255.0f);

    auto stitcher = PatchStitcher::New();
    stitcher->setInputConnection(network->getOutputPort());
    //m_tumorMap = stitcher->updateAndGetOutputData<Tensor>();
    tumorRenderer->setInputConnection(stitcher->getOutputPort());

    return true;
}


bool MainWindow::predictBACH() {

    auto generator = PatchGenerator::New();
    generator->setPatchSize(512, 512);
    generator->setPatchLevel(0);
    generator->setInputData(0, m_image);
    generator->setInputData(1, m_tissue);

    auto network = NeuralNetwork::New();
    network->setInputNode(0, "input_1", NodeType::IMAGE, TensorShape({1, 512, 512, 3}));
    network->setOutputNode(0, "sequential/dense_1/Softmax", NodeType::TENSOR, TensorShape({1, 512, 512, 4}));
    network->load("/mnt/ssd/workspace/FAST/models/inception_v3_bach_model.pb");
    network->setInputConnection(generator->getOutputPort());
    network->setScaleFactor(1.0f/255.0f);

    auto stitcher = PatchStitcher::New();
    stitcher->setInputConnection(network->getOutputPort());
    m_bachMap = stitcher->updateAndGetOutputData<Tensor>();
    bachRenderer->setInputConnection(stitcher->getOutputPort());

    return true;
}

bool MainWindow::saveTumorPred() {
    std::cout<<"it worked!\n\n\n\n";



    return true;
}


bool MainWindow::fixTumorSegment() {
    std::cout<<"upsups...\n\n\n\n";

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
        segRenderer->setDisabled(!segRenderer->isDisabled());
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
    if (!renderer){
        return false;
    }else{
        renderer->setDisabled(!renderer->isDisabled());
        return true;
    }
}

bool MainWindow::hideBackgroundClass() {
    if (!tumorRenderer && !heatmapRenderer) {
        return false;
    }else if (tumorRenderer && !heatmapRenderer) {
        tumorRenderer->setChannelHidden(0, true);
        return true;
    } else if (!tumorRenderer && heatmapRenderer) {
        heatmapRenderer->setChannelHidden(3, true);
        return true;
    } else {
        heatmapRenderer->setChannelHidden(3, true);
        tumorRenderer->setChannelHidden(0, true);
        return true;
    }
}

bool MainWindow::fixImage() {
    if (!renderer){
        return false;
    }else{
        //stopComputationThread();
        getView(0)->reinitialize();
        //startComputationThread();
        return false;
    }
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
