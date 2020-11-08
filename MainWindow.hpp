#pragma once

#include <FAST/Visualization/Window.hpp>

#include <QPushButton>
#include <QMainWindow>
#include <QDialog>
#include <QObject>
#include <QWidget>

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
class QDragEnterEvent;
class QDropEvent;
QT_END_NAMESPACE

namespace fast {

    class NeuralNetwork;
    class PatchStitcher;
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
        void createDynamicExportWidget(const std::string& someName);
        void createWSIScrollAreaWidget();
        void saveThumbnail();
        void saveTissueSegmentation();
        void saveTumor();
        void saveGrade();
        void saveResults(std::string result);
        void displayMessage(QString message);
        void killInference(std::string someName);
        void deleteViewObject(std::string someName);

        bool hideChannel(const std::string &someName); //, uint channel_value);
        bool opacityRenderer(int value, const std::string& someName);
        bool toggleRenderer(std::string name);
        bool pixelClassifier(std::string modelName);
        std::map<std::string, std::string> setParameterDialog(std::map<std::string, std::string> modelMetadata);
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
        bool stopFlag;
		
		bool m_runForProject = false;
		std::vector<std::string> m_runForProjectWsis;
		void runForProject_apply(std::string method);

        static void helpUrl();
        static void reportIssueUrl();
		
		void aboutProgram();
		QColor changeColor();

        void selectFile();
        void selectFileDrag(const QList<QString> &fileNames);
        void addModels();
        void addPipelines();
        void createProject();
        void openProject();
        void saveProject();
		void runForProject();
        void createPipeline();
        void customPipelineEditor();
        void selectFileInProject(int pos);  // int pos);
        int curr_pos = 0;
        void loadTissue(QString tissuePath);
        void loadTumor(QString tumorPath);
        void loadPipelines();

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

		std::string tmpDirPath;
		std::string createRandomNumbers_(int n);

        uint channel_value;

        std::string applicationName;
        std::string modelName;
		std::map<std::string, std::string> modelNames;
        bool advancedMode = false;
        std::string getWsiFormat();
        std::string wsiFormat;
        std::string cwd;
        std::map<std::string, std::string>getModelMetadata(std::string modelName);
        //std::map<std::string, std::any> availableResults; // <- doesn't work // TODO: Fix thisinsert
        std::vector<std::vector<Vector2f>>getAnchorMetadata(std::string anchorFileName);
        std::vector<float> getDownsamplingLevels();
        std::vector<std::string> split (std::string s, std::string delimiter);

        void setApplicationMode();

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
        QPushButton *setModeButton;

        QMenu *runPipelineMenu;
        void runPipeline(std::string path);

        void receiveFileList(const QList<QString> &names); // FIXME: silly stuff

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
        void removeRendererObject(std::string name);
        /**
         * Insert a renderer into the view with a given name.
         * If a renderer with that name already exist, replace it.
         * @param name
         */
        void insertRenderer(std::string name, std::shared_ptr<Renderer> renderer);
        /**
         * Remove all current renderers
         */
        void removeAllRenderers();
        /**
         * Checks if a named renderer exists
         * @param name
         * @return boolean
         */
        bool hasRenderer(std::string name);
        std::shared_ptr<Renderer> getRenderer(std::string name);

    //signals:
    //    void my_signal(int, const std::string&);

    //signals:
    //    void valueChanged(int newValue);

    private:
        std::map<std::string, std::shared_ptr<Renderer>> m_rendererList;
        std::map<std::string, std::string> m_rendererTypeList;
        std::map<std::string, std::shared_ptr<NeuralNetwork>> m_neuralNetworkList;
        std::map<std::string, std::shared_ptr<PatchStitcher>> m_patchStitcherList;
        std::map<std::string, std::map<std::string, std::string>> m_modelMetadataList;
        std::map<std::string, std::shared_ptr<Image>> availableResults;
        MainWindow();
        //std::map<std::string, std::future<std::shared_ptr<Tensor>>> m_futureData;
        //std::future<std::shared_ptr<Tensor>> m_futureData;
        //std::shared_ptr<TissueSegmentation> tissueSegmentation;
        std::shared_ptr<WholeSlideImageImporter> importer;
        std::shared_ptr<ImagePyramid> m_image;
        std::shared_ptr<Image> m_tissue;
        std::shared_ptr<Image> m_gradeMap;
        std::shared_ptr<Image> m_tumorHeatmap;
        std::shared_ptr<Tensor> m_tumorMap_tensor;
        std::shared_ptr<Image> m_tumorMap;
        std::shared_ptr<Tensor> m_bachMap;
        std::string filename;
        QString projectFolderName;



    private slots:
        void updateChannelValue (int index);
        //void receiveFileList(QList<QString> &names);
        //void itemClicked(QListWidgetItem *item);


};



}