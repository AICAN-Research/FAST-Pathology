#pragma once

#include <FAST/Visualization/Window.hpp>
#include <QPushButton>
#include <QMainWindow>
#include <QDialog>
#include <QObject>
#include <QWidget>
#include <QNetworkReply>
#include <QProgressDialog>


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
class QString;
class QScrollArea;
class QListWidget;
QT_END_NAMESPACE

namespace fast {

    class NeuralNetwork;
    class PatchStitcher;
    class SegmentationRenderer;
    class SegmentationPyramidRenderer;
    class TissueSegmentation;
    class WholeSlideImageImporter;
    class ImagePyramid;
    class Segmentation;
    class Image;
    class Tensor;
    class View;

class MainWindow : public Window {
    FAST_OBJECT(MainWindow);
    Q_OBJECT
    public:
        // GUI RELATED STUFF
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

        // EXPORT RELATED STUFF
        void saveThumbnail();
        void saveTissueSegmentation();
		void saveHeatmap();
        void saveTumor();
        void deleteViewObject(std::string someName);

        bool hideChannel(const std::string &someName); //, uint channel_value);
        bool opacityRenderer(int value, const std::string& someName);
        bool toggleRenderer(std::string name);
        void pixelClassifier(std::string someModelName);
        std::map<std::string, std::string> setParameterDialog(std::map<std::string, std::string> modelMetadata);
		void MTL_test();
		void MIL_test();
        bool hideTissueMask(bool flag);
        const bool calcTissueHist();
        bool segmentTissue();
        bool stopFlag;
		
		bool m_runForProject = false;
		std::vector<std::string> m_runForProjectWsis;

        void customPipelineEditor();
        void selectFileInProject(int pos);  // int pos);
        int curr_pos = 0;
		void loadSegmentation(QString path, QString wsi);
		void loadHeatmap(QString path, QString wsi);
		void loadHighres(QString path, QString name);
        void loadPipelines();

        QImage extractThumbnail();

		// script editor related functions
        QDialog *scriptEditorWidget;
        QVBoxLayout *scriptLayout;
        QPlainTextEdit *scriptEditor;
        void createActionsScript();
        void openScript();
        void loadFileScript(const QString &fileName);
        void setCurrentFileScript(const QString &fileName);
        void newFileScript();
        bool saveAsScript();
        bool saveScript();
        bool maybeSaveScript();
        bool saveFileScript(const QString &fileName);
        QString currScript;
        QStatusBar *statusBar;

        void createOpenGLWindow();
        View *view;

        bool background_flag = false;
        float getMagnificationLevel();
        float magn_lvl;

		std::string tmpDirPath;
		std::string createRandomNumbers_(int n);

        uint channel_value;

        std::string applicationName;
        std::string modelName;
		std::map<std::string, std::string> modelNames;
        bool advancedMode = false;
        std::string wsiFormat;
        std::string cwd;
        std::map<std::string, std::string>getModelMetadata(std::string modelName);
        std::vector<std::vector<Vector2f>>getAnchorMetadata(std::string anchorFileName);
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
        QVBoxLayout *dynamicViewLayout;
        QWidget *dynamicViewWidget;
        QComboBox *pageComboBox;
        QComboBox *exportComboBox;
        QStackedLayout *stackedLayout;
        QStackedLayout *exportStackedLayout;
        QPushButton *setModeButton;

        QMenu *runPipelineMenu;
        void runPipeline(std::string path);

        void receiveFileList(const QList<QString> &names); // FIXME: silly stuff

        QScrollArea *scrollArea;
        QWidget *scrollWidget;
        QVBoxLayout *scrollLayout;
        QListWidget *scrollList;

        std::vector<std::string> wsiList;
        std::vector<std::string> savedList;

        /**
         * Opens the default browser and directs the user to the FastPathology GitHub page to seek for assistance.
         */
        static void helpUrl();

        /**
         * Opens the default browser and directs the user to the FastPathology GitHub "Issues" section, to easily
         * report an issue/bug or request a feature.
         */
        static void reportIssueUrl();

        /**
         * Opens a simple dialog which contains information about the software.
         */
        void aboutProgram();

        // IMPORT RELATED STUFF
        /**
         * Opens a file explorer for selecting which WSIs from disk to import to the program.
         * The last one will be rendered.
         */
        void selectFile();

        /**
         * Imports WSIs to the program, where selections are made from a drag-and-drop event.
         * @param fileNames
         */
        void selectFileDrag(const QList<QString> &fileNames);

        /**
         * Opens a file explorer for selecting which deep learning modules from disk to import to the program.
         * All selected models will be automatically added to the Progress widget.
         */
        void addModels();

        /**
         * Imports models to the program, where selections are made from a drag-and-drop event.
         * @param fileNames
         */
        void addModelsDrag(const QList<QString> &fileNames);

        /**
         * Opens a file explorer for selecting which Pipelines from disk to import to the program.
         * All selected Pipelines will be automatically added to the default Pipelines and the Pipeline menu.
         */
        void addPipelines();

        // PROJECT RELATED STUFF
        /**
         * To create a Project. Opens a file explorer to choose which empty directory to create the project.
         * A new directory may be created from the file explorer, before selection.
         */
        void createProject();

        /**
         * To open a created Project from disk. Opens a file explorer to select which project.txt file open.
         * When selected, thumbnails from all WSIs will be added to the left scroll widget.
         * For the image, the WSI will be rendered with corresponding existing results.
         */
        void openProject();

        /**
         * To save the current Project. This simply updates the project.txt file to include the current
         * WSIs that are in use in the program.
         */
        void saveProject();

        /**
         * Opens a QDialog to enable RunForProject, which makes it possible for the user to run an analysis
         * on a set of WSIs sequentially. From the QDialog the WSIs may be selected, and when accepted pressing the
         * "apply" button updates the current WSIs in the RunForProject.
         */
        void runForProject();

        // MISC
        /**
         * Downloads the test data (trained models, Pipelines, WSIs) currently located on the NTNU Apache server and
         * adds them to the default functionalities in the program. When finished, the user is prompted if they want to
         * import the WSIs and test the solutions on the data.
         */
        void downloadAndAddTestData();

        /**
         * Prompts the user whether or not to "reset", if accepted it will refresh the software to the initial state,
         * deleting all WSIs and rendered images from the current run.
         */
        void reset();

        /**
         * Removes a named renderer from the view
         * @param name
         */

        //void removeRendererObject(std::string name);
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

    private:
        MainWindow();
        std::map<std::string, std::shared_ptr<Renderer>> m_rendererList;
        std::map<std::string, std::string> m_rendererTypeList;
        std::map<std::string, std::shared_ptr<NeuralNetwork>> m_neuralNetworkList;
        std::map<std::string, std::shared_ptr<PatchStitcher>> m_patchStitcherList;
        std::map<std::string, std::map<std::string, std::string>> m_modelMetadataList;
        std::map<std::string, std::shared_ptr<Image>> availableResults;
        std::shared_ptr<WholeSlideImageImporter> importer;
        std::shared_ptr<ImagePyramid> m_image;
        std::shared_ptr<Image> m_tissue;
        std::shared_ptr<Image> m_tumorMap;
        std::string filename;
        QString projectFolderName;

    private slots:
        void updateChannelValue (int index);


};



}