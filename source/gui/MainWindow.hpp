#pragma once

#include <FAST/Visualization/Window.hpp>
#include <QPushButton>
#include <QMainWindow>
#include <QDialog>
#include <QObject>
#include <QWidget>
#include <QCloseEvent>
#include <QNetworkReply>
#include <QMessageBox>
#include <QProgressDialog>
#include "source/utils/utilities.h"
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

        std::string modelName;
        std::string wsiFormat;
        std::string cwd;
        std::string tmpDirPath;
        QString currScript;
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

        // GUI RELATED STUFF
        /**
         * Creates the main menu bar of the platform.
         */
        void createMenubar();
        /**
         * Creates the OpenGL window for rendering WSI-related stuff.
         */
        void createOpenGLWindow();
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

        std::shared_ptr<ComputationThread> getComputationThread();

        std::shared_ptr<Project> getCurrentProject();

        std::string getRootFolder() const;
    protected:
        /**
         * Define the interface for the current global widget.
         */
        void setupInterface();
        /**
         * Define the connections for all elements inside the current global widget.
         */
        void setupConnections();

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


        std::string _application_name; /* */
        MainSidePanelWidget *_side_panel_widget; /* Main widget for the left-hand panel */
        std::map<std::string, QAction*> _file_menu_actions; /* Holder for all actions in the File main menu bar */
        QMenu* _pipeline_menu; /* */
        QMenu* _deploy_menu; /* */
        QMenu* _help_menu; /* */
        QAction* _file_menu_create_project_action;
        QAction* _file_menu_import_wsi_action;
        QAction* _file_menu_add_model_action;
        QAction* _file_menu_add_pipeline_action;
        QAction* _project_menu_create_project_action;
        QAction* _project_menu_open_project_action;
        QAction* _project_menu_save_project_action;
        QAction* _edit_menu_change_mode_action;
        QAction* _edit_menu_download_testdata_action;
        QAction* _pipeline_menu_import_action;
        QAction* _pipeline_menu_editor_action;
        QAction* _help_menu_about_action;

    signals:
        void inferenceFinished(std::string name);

        /**
         * @brief Propagate the main menu bar action to the ProjectWidget
         */
//        void createProjectTriggered();
//        void selectFilesTriggered();

    public slots:
        void closeEvent (QCloseEvent *event);
        void resetDisplay();

        void changeWSIDisplayReceived(std::string uid_name, bool state);

        void updateAppTitleReceived(std::string title_suffix);

        /**
         * @brief addRendererToViewReceived Adds a new renderer for the currently visible image to the main View.
         * @param name: Identifier for the renderer to add.
         */
        void addRendererToViewReceived(const std::string& name);
        void removeRendererFromViewReceived(const std::string& name);
        void runPipelineReceived(QString pipeline_uid);

    private slots:
        void updateChannelValue(int index);
    };

}
