#pragma once

#include <FAST/Visualization/Window.hpp>

#include <QPushButton>
#include <QMainWindow>
#include <QDialog>
#include <QObject>

//#include "FAST/ProcessObject.hpp"

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
class QSplitter;
class QPlainTextEdit;
class QStatusbar;
class QString;
class QScrollArea;
class QListWidget;
QT_END_NAMESPACE

namespace fast {

    class SegmentationRenderer;
    class SegmentationPyramidRenderer;
    //class ImagePyramidRenderer;
    //class HeatmapRenderer;
    class TissueSegmentation;
    class WholeSlideImageImporter;
    class ImagePyramid;
    class Segmentation;
    class Image;
    class Tensor;
    class View;
    //class segTumorRenderer;

class MainWindow : public Window {
    FAST_OBJECT(MainWindow);
    Q_OBJECT
    public:
        void drawHist();
        void createActions();
        void createMenubar();
        void createMainMenuWidget();
        void createFileWidget();
        void createProcessWidget();
        void createViewWidget();
        void createExportWidget();
        void createStatsWidget();
        void createMenuWidget();
        void createDynamicViewWidget(const std::string& someName, std::string modelName);
        void createWSIScrollAreaWidget();
        void saveThumbnail();
        void saveTissueSegmentation();
        void saveTumor();
        void saveGrade();
        void deleteViewObject(std::string someName);

        bool hideChannel(const std::string &someName); //, uint channel_value);
        bool opacityRenderer(int value, const std::string& someName);
        bool toggleRenderer(std::string name);
        bool patchClassifier(std::string modelName);
        bool lowresSegmenter();
        //bool imageSegmenter(std::string modelName);
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

        static void helpUrl();
        static void reportIssueUrl();

        void selectFile();
        void addModels();
        void addPipelines();
        void createProject();
        void openProject();
        void saveProject();
        void createPipeline();
        void pipelineEditor();
        void selectFileInProject(int pos);  // int pos);
        int curr_pos;
        void loadTissue(QString tissuePath);
        void loadTumor(QString tumorPath);

        QImage extractThumbnail();

    // script editor related functions
        QDialog *scriptEditorWidget;
        //QWidget *scriptEditorWidget;
        QVBoxLayout *scriptLayout;
        QPlainTextEdit *scriptEditor;
        void createActionsScript();
        void openScript();
        void loadFileScript(const QString &fileName);
        void setCurrentFileScript(const QString &fileName);
        void newFileScript();
        void createStatusBarScript();
        bool saveAsScript();
        bool saveScript();
        bool maybeSaveScript();
        bool saveFileScript(const QString &fileName);
        QString currScript;
        QStatusBar *statusBar;

        void createOpenGLWindow();
        View *view;

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

        int mkdir(const char *path);
        int rmdir(const char *path);

        uint channel_value;

        std::string modelName;
        std::string getWsiFormat();
        std::string wsiFormat;
        std::string cwd;
        std::map<std::string, std::string>getModelMetadata(std::string modelName);
        std::vector<float> getDownsamplingLevels();
        std::vector<std::string> split (std::string s, std::string delimiter);

        std::unordered_map<std::string, std::string> metadata; // make metadata information a global variable
        QList<QString> currentClassesInUse;

        QWidget *mainWidget;
        QWidget *processWidget;
        QWidget *exportWidget;
        QWidget *statsWidget;
        QWidget *viewWidget;
        QWidget *fileWidget;
        QWidget *menuWidget;
        QVBoxLayout *superLayout;
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
        QComboBox *exportComboBox;
        QStackedLayout *stackedLayout;
        QStackedLayout *exportStackedLayout;
        //QSplitter *mainSplitter;

        QScrollArea *scrollArea;
        QWidget *scrollWidget;
        QVBoxLayout *scrollLayout;
        QListWidget *scrollList;

        void reset();

        std::vector<std::string> wsiList;
        std::vector<std::string> savedList;

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

        std::future<SharedPointer<Tensor>> m_futureData;
        //SharedPointer<TissueSegmentation> tissueSegmentation;
        SharedPointer<WholeSlideImageImporter> importer;
        SharedPointer<ImagePyramid> m_image;
        SharedPointer<Image> m_tissue;
        SharedPointer<Image> m_gradeMap;
        SharedPointer<Tensor> m_tumorMap_tensor;
        SharedPointer<Image> m_tumorMap;
        SharedPointer<Tensor> m_bachMap;
        std::string filename;
        QString projectFolderName;
        //SharedPointer<View> view;
        //View *MainWindow::view;

        /*
        //QAction *newAct{};
        //QWidget *topFiller;
        //QMenu *fileMenus{};
        //QAction *exitAction{};

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
         */

        private slots:
            void updateChannelValue (int index);
            //void itemClicked(QListWidgetItem *item);


};



}