#pragma once

#include <FAST/Visualization/Window.hpp>
#include <QPushButton>
#include <QMainWindow>
#include <QDialog>
#include <QObject>
#include <QWidget>
#include <QNetworkReply>
#include <QProgressDialog>
#include "source/utils/utilities.h"
//#include "source/gui/ProjectWidget.h"
#include "source/gui/MainSidePanelWidget.h"
#include "source/logic/DataManager.h"

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
    //class SegmentationPyramidRenderer;
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
        bool stopFlag;
        bool m_runForProject = false;
        bool background_flag = false;
        bool advancedMode = false;
        bool m_wsiSharpening = false;

        int curr_pos = 0;
        float magn_lvl;
        uint channel_value;

        std::string applicationName;
        std::string modelName;
        std::string wsiFormat;
        std::string cwd;
        std::string tmpDirPath;
        QString currScript;

        std::vector<std::string> wsiList;
        std::vector<std::string> savedList;
        std::vector<std::string> m_runForProjectWsis;

        std::map<std::string, std::string> modelNames;
        std::unordered_map<std::string, std::string> metadata; // make metadata information a global variable
        QList<QString> currentClassesInUse;

        // script editor related functions
        QDialog *scriptEditorWidget;
        QVBoxLayout *scriptLayout;
        QPlainTextEdit *scriptEditor;

        QWidget *mainWidget;
        QWidget *processWidget;
        QWidget *exportWidget;
        QWidget *statsWidget;
        QWidget *viewWidget;
//        QWidget *fileWidget;
        QWidget *menuWidget;
        QWidget *dynamicViewWidget;
        QWidget *scrollWidget;
        QComboBox *pageComboBox;
        QComboBox *exportComboBox;
//        QStackedWidget *stackedWidget;
        QPushButton *setModeButton;
        QMenu *runPipelineMenu;
        QScrollArea *scrollArea;
        QListWidget *scrollList;
        QStatusBar *statusBar;
        View *view;

        QHBoxLayout *mainLayout;
        QVBoxLayout *superLayout;
        QVBoxLayout *processLayout;
        QVBoxLayout *fileLayout;
        QVBoxLayout *viewLayout;
        QVBoxLayout *statsLayout;
        QVBoxLayout *exportLayout;
        QVBoxLayout *dynamicViewLayout;
        QVBoxLayout *scrollLayout;
        QStackedLayout *stackedLayout;
        QStackedLayout *exportStackedLayout;

        // EXPERIMENTAL METHODS FOR TESTING CUSTOM STUFF (Not relevant for the final program)
        void MTL_test();
        void MIL_test();
        void Kmeans_MTL_test();

        // custom split
        std::vector<std::string> splitCustom(const std::string& s, const std::string& delimiter);

        // STATIC METHODS, TODO THAT PROBABLY SHOULD BE ADDED TO A UTILS OR SOMETHING SIMILAR
        /**
         * Reads the model configuration metadata from the model config file on disk and stores them in a container.
         * @param modelName
         * @return
         */
        std::map<std::string, std::string> getModelMetadata(std::string modelName);
        /**
         * Reads the anchor information from a file and stores it in a container. Relevant for object detection
         * inference pipelines (specifically and currently only YOLOv3-esque models).
         * @param modelName
         * @return
         */
        std::vector<std::vector<Vector2f>> getAnchorMetadata(std::string anchorFileName);
        /**
         * Opens dialog for setting the parameters for a selected inference pipeline (available in advanced mode).
         * Essentially updates the model metadata variable depdendent on the adjustments made.
         * @param modelMetadata
		 * @pointer *successFlag
         * @return
         */
        std::map<std::string, std::string> setParameterDialog(std::map<std::string, std::string> modelMetadata, int *successFlag);
        /**
         * Shows a proof-of-concept that a histogram widget can be made using Qt. However, it was quite time-consuming.
         * @return
         */
        const bool calcTissueHist();

        // MISC
        /**
         * Downloads the test data (trained models, Pipelines, WSIs) currently located on the NTNU Apache server and
         * adds them to the default functionalities in the program. When finished, the user is prompted if they want to
         * import the WSIs and test the solutions on the data.
         */
        void downloadAndAddTestData();
        /**
         * Gets the magnification level of the current WSI.
         * @return
         */
        float getMagnificationLevel();

        /**
         * Updates the current WSI dependent on which file is selected form the WSI scroll bar widget on the left. It
         * will render the selected WSI and load all corresponding results, if a Project exists.
         * @param pos
         */
        void selectFileInProject(int pos);

        // GUI RELATED STUFF
        /**
         * Creates the main menu bar of the platform.
         */
        void createMenubar();
        /**
         * Creates the actual main menu widget. The widget includes all the WSI pipeline modules.
         */
        void createMainMenuWidget();
        /**
         * Creates the OpenGL window for rendering WSI-related stuff.
         */
        void createOpenGLWindow();
        /**
         * Creates the import module of the main menu widget.
         */
        void createFileWidget();
        /**
         * Creates the process module of the main menu widget.
         */
        void createProcessWidget();
        /**
         * Creates the view module of the main menu widget.
         */
        void createViewWidget();
        /**
         * Creates the export module of the main menu widget.
         */
        void createExportWidget();
        /**
         * Creates the stats module of the main menu widget.
         */
        void createStatsWidget();
        /**
         * Combines all the WSI pipeline module widgets into a single widget.
         */
        void createMenuWidget();
        /**
         * Updates the view widget given a new object.
         * @param someName
         * @param modelName
         */
        void createDynamicViewWidget(const std::string &someName, std::string modelName);
        /**
         * Updates the export widget dependent on which results that exists.
         * @param someName
         */
        void createDynamicExportWidget(const std::string &someName);
        /**
         * Creates the WSI scroll area for selecting which WSI to work with.
         */
        void createWSIScrollAreaWidget();
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
         * Wrapper for the file drag event to properly catch and execute the file import after drag event.
         * @param names
         */
        void receiveFileList(const QList<QString> &names); // FIXME: silly stuff
        /**
         * Opens a file explorer for selecting which Pipelines from disk to import to the program.
         * All selected Pipelines will be automatically added to the default Pipelines and the Pipeline menu.
         */
        void addPipelines();
        /**
         * Load saved segmentation file from disk. Requires both the path to the file and a given WSI, to properly
         * connect the two. Assumed to be a PNG image for now. TODO should all FAST supported
         * formats?
         * @param path
         * @param wsi
         */
        void loadSegmentation(QString path, QString wsi);
        /**
         * Load saved heatmap file from disk. Requires both the path to the file and a given WSI, to properly
         * connect the two.
         * @param path
         * @param wsi
         */
        void loadHeatmap(QString path, QString wsi);
        /**
         * Load high-res segmentation object from disk. Assumed to be stored as a set of PNGs in a single folder.
         * Requires both the path to the file and a given WSI, to properly connect the two.
         * @param path
         * @param name
         */
        void loadHighres(QString path, QString name);
        /**
         * Load text pipeline from disk. Assumed to be stored in a .txt-like format. Both .txt and .fpl are supported.
         */
        void loadPipelines();

        // PROCESS WIDGET RELATED STUFF
        /**
         * Runs inference of a selected pipeline. If ran with Projects enabled, it will be ran in a non-blocking
         * background thread and will not render any results. Otherwise the rendered results will be streamed on the
         * fly, but no results will be stored (relevant for simple demonstrations).
         * @param someModelName
         * @param modelMetadata
         */
        void pixelClassifier(std::string someModelName, std::map<std::string, std::string> modelMetadata);
        /**
         * Simple wrapper of the inference method, pixelClassifier. If ran with Projects enabled, it will be ran in a
         * non-blocking background thread and will not render any results. Otherwise the rendered results will be
         * streamed on the fly, but no results will be stored (relevant for simple demonstrations). If advanced mode
         * is enabled, a parameter dialog will be prompted for setting parameters before inference is ran.
         * @param someModelName
         */
        void pixelClassifier_wrapper(std::string someModelName);
        /**
         * Simple method for performing tissue segmentation through a smart morphological, thresholding filter. If
         * advanced mode is enabled, a parameter dialog will be prompted, and as values are tuned, a dynamic result
         * will be rendered showing the resultant output segmentation.
         * @return
         */
        bool segmentTissue();
        /**
         * Deploys a selected pipeline, through parsing the text pipeline script and creating FAST POs before execution.
         * @param path
         */
        void runPipeline(std::string path);

        // PROJECT RELATED STUFF
        /**
         * Opens a QDialog to enable RunForProject, which makes it possible for the user to run an analysis
         * on a set of WSIs sequentially. From the QDialog the WSIs may be selected, and when accepted pressing the
         * "apply" button updates the current WSIs in the RunForProject.
         */
        void runForProject();

        // VIEW WIDGET RELATED STUFF
        /**
         * Delete selected renderer object
         * @param name
         */
        void deleteViewObject(std::string name);
        /**
         * Toggle selected channel for the current heatmap renderer object
         * @param name
         * @return
         */
        bool hideChannel(const std::string &name); //, uint channel_value);
        /**
         * Adjust opacity for selected renderer object
         * @param value
         * @param someName
         * @return
         */
        bool opacityRenderer(int value, const std::string &name);
        /**
         * Toggle selected renderer
         * @param name
         * @return
         */
        bool toggleRenderer(std::string name);
        /**
         * Prompts the user whether or not to "reset", if accepted it will refresh the software to the initial state,
         * deleting all WSIs and rendered images from the current run.
         */
        void reset();
        /**
         * Hides the tissue segmentation if it exists in the view objects list and is enabled. Assumes that the tissue
         * segmentation object is given the ID "tissue".
         * @param flag
         * @return
         */
        bool hideTissueMask(bool flag);
        /**
         * Removes a named renderer from the view.
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
         * Remove all current renderers.
         */
        void removeAllRenderers();
        /**
         * Checks if a named renderer exists.
         * @param name
         * @return boolean
         */
        bool hasRenderer(std::string name);
        /**
         * Get renderer from container given an ID.
         * @param name
         * @return
         */
        std::shared_ptr<Renderer> getRenderer(std::string name);

        /**
         * Define the connections for all elements inside the current global widget.
         */
        void setupConnections();

        // EXPORT RELATED STUFF
        /**
         * Creates and saves the thumbnail image as a PNG image on disk in the current Project.
         */
        void saveThumbnail();
        /**
         * Saves the tissue segmentation image result as a PNG image on disk in a the current Project.
         */
        void saveTissueSegmentation();
        /**
         * Saves a specific heatmap object (I was testing that saving tumor segmentation results did work).
         * TODO: Remove when export is working as intended.
         */
        void saveHeatmap();
        /**
         * Saves a specific segmentation object (of tumor. I was testing that it was possible).
         * TODO: Remove when export is working as intended.
         */
        void saveTumor();

        // SCRIPT EDITOR RELATED STUFF
        /**
         * Defines and creates the script editor widget.
         */
        void customPipelineEditor();
        /**
         * Defined and created the menubar related to the script editor.
         */
        void createActionsScript();
        /**
         * Opens file explorer to select which script to use in the script editor.
         */
        void openScript();
        /**
         * Loads the script file from disk, relevant for the script editor.
         * @param fileName
         */
        void loadFileScript(const QString &fileName);
        /**
         * Sets the title of the script editor widget to the current file name.
         * @param fileName
         */
        void setCurrentFileScript(const QString &fileName);
        /**
         * Create a new file from the script editor. Opens a blank document. If already unsaved document exist,
         * it will prompt whether the user wish to save the unsaved changes.
         */
        void newFileScript();
        /**
         * To save the current document as a completely new document, from the script editor.
         * @return
         */
        bool saveAsScript();
        /**
         * Save the current document in the script editor.
         * @return
         */
        bool saveScript();
        /**
         * Prompt given to the user if they wish the current changes in the script editor or not.
         * @return
         */
        bool maybeSaveScript();
        /**
         * Actually saves document in script editor, given a save/saveAs event.
         * @param fileName
         * @return
         */
        bool saveFileScript(const QString &fileName);

    private:
        MainWindow();

        std::string filename;
        QString projectFolderName;

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


        ProjectWidget *fileWidget;
        MainSidePanelWidget *_side_panel_widget; /* Main widget for the left-hand panel */
        std::map<std::string, QAction*> _file_menu_actions; /* Holder for all actions in the File main menu bar */
        QAction* _file_menu_create_project_action;
        QAction* _file_menu_import_wsi_action;
        QAction* _file_menu_add_model_action;
        QAction* _file_menu_add_pipeline_action;
        QAction* _project_menu_create_project_action;
        QAction* _project_menu_open_project_action;
        QAction* _project_menu_save_project_action;
        QAction* _edit_menu_change_mode_action;
        QAction* _edit_menu_download_testdata_action;

    signals:
        void inferenceFinished(std::string name);

        /**
         * @brief Propagate the main menu bar action to the ProjectWidget
         */
//        void createProjectTriggered();
//        void selectFilesTriggered();

    public slots:
        void resetDisplay();
        void updateView(std::string uid_name, bool state);
        void updateAppTitle(std::string title_suffix);
    private slots:
        void updateChannelValue(int index);
    };

}
