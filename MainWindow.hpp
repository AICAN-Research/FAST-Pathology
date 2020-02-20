#pragma once

#include <FAST/Visualization/Window.hpp>

#include <QPushButton>
#include <QMainWindow>
#include <QDialog>
#include <QObject>

//#include "FAST/ProcessObject.hpp"
#include <openslide/openslide.h>

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
class QHBoxLayout;
class QVBoxLayout;
class QStackedWidget;
class QComboBox;
class QStackedLayout;
QT_END_NAMESPACE

// Forward declare
typedef struct _openslide openslide_t;

namespace fast {

    class SegmentationRenderer;
    //class ImagePyramidRenderer;
    //class HeatmapRenderer;
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
    Q_OBJECT
    public:
        void drawHist();
        void selectFile();
        void addModels();
        void createActions();
        void createMainMenuWidget();
        void createFileWidget();
        void createProcessWidget();
        void createViewWidget();
        void createExportWidget();
        void createStatsWidget();
        void createMenuWidget();
        void createDynamicViewWidget(const std::string& someName, std::string modelName);

        bool hideChannel(const std::string &someName); //, uint channel_value);
        bool opacityRenderer(int value, const std::string& someName);
        bool toggleRenderer(std::string name);
        bool patchClassifier(std::string modelName);
        bool showHeatmap();
        bool hideTissueMask(bool flag);
        bool toggleTissueMask();
        bool opacityTissue(int value);
        bool opacityHeatmap(int value);
        bool opacityTumor(int value);
        bool exportSeg();
        bool calcTissueHist();
        bool showImage();
        bool segmentTissue();
        bool predictGrade();
        bool predictTumor();
        bool predictBACH();
        bool showTumorMask();
        bool hideBackgroundClass(std::string someName);
        bool fixImage();
        bool showBachMap();
        bool fixTumorSegment();
        bool saveTumorPred();
        bool background_flag = false; // = false;
        bool tissue_flag = false;
        void get_cwd();
        void create_tmp_folder_file();
        float getDownsamplingAtLevel(int value);
        float getMagnificationLevel();
        float magn_lvl;
        float getDownsamplingAtLevel();
        int dirExists(const char *path);

        uint channel_value;

        std::string modelName;
        std::string getWsiFormat();
        std::string wsiFormat;
        std::string cwd;
        std::map<std::string, std::string>getModelMetadata(std::string modelName);
        std::vector<float> getDownsamplingLevels();
        std::vector<std::string> split (std::string s, std::string delimiter);

        std::map<std::string, std::string> metadata; // make metadata information a global variable
        QList<QString> currentClassesInUse;

        QWidget *processWidget;
        QWidget *exportWidget;
        QWidget *statsWidget;
        QWidget *viewWidget;
        QWidget *fileWidget;
        QWidget *menuWidget;
        QHBoxLayout *mainLayout;
        QVBoxLayout *processLayout;
        QStackedWidget *stackedWidget;
        QVBoxLayout *fileLayout;
        QVBoxLayout *viewLayout;
        QVBoxLayout *statsLayout;
        QVBoxLayout *exportLayout;
        QVBoxLayout *dockLayout;
        QVBoxLayout *dynamicViewLayout;
        QWidget *dynamicViewWidget;
        QComboBox *pageComboBox;
        QStackedLayout *stackedLayout;

        /**
         * Removes a named renderer from the view
         * @param name
         */
        void removeRenderer(std::string name);
        /**
         * Insert a renderer into the view with a given name.
         * If a renderer with that name already exist, replace it.
         * @param name
         */
        void insertRenderer(std::string name, SharedPointer<Renderer> renderer);
        /**
         * Remove all current renderers
         */
        void removeAllRenderers();
        bool hasRenderer(std::string name);
        SharedPointer<Renderer> getRenderer(std::string name);

    //signals:
    //    void my_signal(int, const std::string&);

    //signals:
    //    void valueChanged(int newValue);



    private:
        std::map<std::string, SharedPointer<Renderer>> m_rendererList;
        MainWindow();
        /*
        //SharedPointer<ImagePyramidRenderer> renderer;
        SharedPointer<HeatmapRenderer> bachRenderer;
        SharedPointer<HeatmapRenderer> heatmapRenderer;
        SharedPointer<HeatmapRenderer> tumorRenderer;
        */

        //SharedPointer<SegmentationRenderer> segRenderer;
        SharedPointer<TissueSegmentation> tissueSegmentation;
        SharedPointer<WholeSlideImageImporter> importer;
        SharedPointer<ImagePyramid> m_image;
        SharedPointer<Image> m_tissue;
        SharedPointer<Tensor> m_gradeMap;
        SharedPointer<Tensor> m_tumorMap_tensor;
        SharedPointer<Image> m_tumorMap;
        SharedPointer<Tensor> m_bachMap;
        //SharedPointer<QWidget> processWidget;
        //SharedPointer<View> view;
        //SharedPointer<Window> view;
        //SharedPointer<Window> mWidget;
        std::string filename;
        //openslide_t* files;
        //QWidget *processWidget;
        //QWidget *viewWidget;

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

    private slots:
        void updateChannelValue (int index);


};



}