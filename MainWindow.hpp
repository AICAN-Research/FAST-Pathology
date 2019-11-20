#pragma once

#include <FAST/Visualization/Window.hpp>
#include <QPushButton>

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

class MainWindow : public Window {
    FAST_OBJECT(MainWindow);
    public:
        void selectFile();
        bool showHeatmap();
        bool showTissueMask();
        bool opacityTissue(int value);
        bool opacityHeatmap(int value);
        bool opacityTumor(int value);
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
        SharedPointer<Tensor> m_tumorMap;
        SharedPointer<Tensor> m_bachMap;
        std::string filename;
        SharedPointer<View> view;
};



}