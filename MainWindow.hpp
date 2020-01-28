#pragma once

#include <FAST/Visualization/Window.hpp>

#include <QPushButton>
#include <QMainWindow>
#include <QDialog>

QT_BEGIN_NAMESPACE
class QAction;
class QDialogButtonBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QMenuBar;
class QMenu;
class QPushButton;
class QTextEdit;
QT_END_NAMESPACE

namespace fast {

    class HeatmapRenderer;
    class SegmentationRenderer;
    class ImagePyramidRenderer;
    class TissueSegmentation;
    class WholeSlideImageImporter;
    class ImagePyramid;
    class Segmentation;
    class Image;
    class Tensor;
    class View;
    class segTumorRenderer;

class MainWindow : public Window {
    FAST_OBJECT(MainWindow);
    public:
        void drawHist();
        void selectFile();
        void createActions();
        //void createMenus();
        bool showHeatmap();
        bool showTissueMask();
        bool opacityTissue(int value);
        bool opacityHeatmap(int value);
        bool opacityTumor(int value);
        bool exportSeg();
        bool calcTissueHist();
        bool showImage();
        bool segmentTissue();
        //bool segmentTissueOtsu();
        bool predictGrade();
        bool predictTumor();
        bool predictBACH();
        bool showTumorMask();
        bool hideBackgroundClass();
        bool fixImage();
        bool showBachMap();
        bool fixTumorSegment();
        bool saveTumorPred();

        bool background_flag = false; // = false;
        bool tissue_flag = false;
        std::string cwd;

    private:
        MainWindow();
        SharedPointer<HeatmapRenderer> bachRenderer;
        SharedPointer<HeatmapRenderer> heatmapRenderer;
        SharedPointer<HeatmapRenderer> tumorRenderer;
        SharedPointer<SegmentationRenderer> segRenderer;
        SharedPointer<ImagePyramidRenderer> renderer;
        SharedPointer<TissueSegmentation> tissueSegmentation;
        SharedPointer<WholeSlideImageImporter> importer;
        SharedPointer<ImagePyramid> m_image;
        SharedPointer<Image> m_tissue;
        SharedPointer<Tensor> m_gradeMap;
        SharedPointer<Tensor> m_tumorMap_tensor;
        SharedPointer<Image> m_tumorMap;
        SharedPointer<Tensor> m_bachMap;
        std::string filename;
        //SharedPointer<View> view;
        SharedPointer<SegmentationRenderer> segTumorRenderer;

        QAction *newAct{};
        //QWidget *topFiller;
        //QMenu *fileMenus{};
        QAction *exitAction{};

        //void createMenu();
        //void createHorizontalGroupBox();
        //void createGridGroupBox();
        //void createFormGroupBox();

        enum { NumGridRows = 3, NumButtons = 4 };

        //QMenuBar *menuBar{};
        QGroupBox *horizontalGroupBox{};
        QGroupBox *gridGroupBox{};
        QGroupBox *formGroupBox{};
        QTextEdit *smallEditor{};
        //QTextEdit *bigEditor;
        QLabel *labels[NumGridRows]{};
        QLineEdit *lineEdits[NumGridRows]{};
        QPushButton *buttons[NumButtons]{};
        //QDialogButtonBox *buttonBox;
};



}