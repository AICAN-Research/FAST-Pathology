//
// Created by dbouget on 02.11.2021.
//

#include "ProcessManager.h"
#include <FAST/Importers/WholeSlideImageImporter.hpp>
#include <FAST/Visualization/ImagePyramidRenderer/ImagePyramidRenderer.hpp>
#include <FAST/Data/ImagePyramid.hpp>
#include <FAST/Algorithms/ImageResizer/ImageResizer.hpp>
#include <source/utils/utilities.h>

namespace fast {

/**
 * Static methods should be defined outside the class.
 */

    ProcessManager* ProcessManager::_pinstance{nullptr};
    std::mutex ProcessManager::_mutex;

    ProcessManager::ProcessManager()
    {
        this->_advanced_mode = false;
        this->_fp_root_filepath = QDir::homePath().toStdString() + "/fastpathology/";
        this->_models_filepath = QDir::homePath().toStdString() + "/fastpathology/" + "data/Models";
        this->_pipelines_filepath = QDir::homePath().toStdString() + "/fastpathology/" + "data/Pipelines";
    }

/**
 * The first time we call GetInstance we will lock the storage location
 *      and then we make sure again that the variable is null and then we
 *      set the value. RU:
 */
    ProcessManager *ProcessManager::GetInstance()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_pinstance == nullptr)
        {
            _pinstance = new ProcessManager();
        }
        return _pinstance;
    }

    void ProcessManager::set_advanced_mode_status(bool status)
    {
        this->_advanced_mode = status;
    }

    void ProcessManager::importModel(const std::string& name)
    {
        if (this->_models.find(name) == this->_models.end())
        {
            // @TODO. Maybe a unique_ptr would make sense here?
            auto model(std::make_shared<LogicRuntimeModel>(this->_models_filepath, name));
//            auto model = new LogicRuntimeModel(this->_models_filepath, name);
            this->_models[name] = model;
        }
    }

    void ProcessManager::runProcess(const std::string image_uid, const std::string process_name)
    {
        // @TODO. Should the trigger case here be model or pipeline, the pipeline call would then call subsequent models.
        // Special case as the tissue segmentation is agnostic of any local model since performed with FAST.
        if (process_name == "tissue")
        {
            auto tissue_seg_process = new SegmentationProcess(image_uid);
            tissue_seg_process->segmentTissue();
        }
        else
        {
            std::cout << "Model name in wrapper: " << process_name << std::endl;

            //connect(currComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateChannelValue(int)));

            // read model metadata (.txt file)
            std::map<std::string, std::string> modelMetadata = this->_models[process_name]->get_model_metadata();

            // set parameters yourself (only enabled if advanced mode is ON)
            if (this->_advanced_mode)
            {
                auto successFlag = 1;
//                modelMetadata = setParameterDialog(modelMetadata, &successFlag);
                if (successFlag != 1)
                    return;
                for (const auto &[k, v] : modelMetadata)
                    std::cout << "m[" << k << "] = (" << v << ") " << std::endl;
            }
            std::cout << "Final model metadata config sent to pixelClassifier:" << std::endl;
            for (const auto &[k, v] : modelMetadata)
                std::cout << "m[" << k << "] = (" << v << ") " << std::endl;

            // if run for project is enabled, run the inference-export pipeline in a background thread, else don't
            if (false) // m_runForProject
            {
                std::atomic_bool stopped(false);
                std::thread inferenceThread([&, process_name]() {pixelClassifier(process_name);});
                inferenceThread.detach();
            }
            else
                pixelClassifier(process_name);
        }
    }

    void ProcessManager::pixelClassifier(std::string process_name)
    {

    }
//    void ProcessManager::pixelClassifier(std::string process_name)
//    {
//        std::map<std::string, std::string> modelMetadata = this->_models[process_name]->get_model_metadata();
//        std::cout << "Final model metadata config WITHIN to pixelClassifier:" << std::endl;
//        for (const auto &[k, v] : modelMetadata)
//            std::cout << "m[" << k << "] = (" << v << ") " << std::endl;

//        // try {
//        if (true) {
////            bool stopFlag = false;

////            // for run-for-project
////            std::vector<std::string> currentWSIs;
////            if (m_runForProject) {
////                currentWSIs = m_runForProjectWsis;
////            }
////            else {
////                currentWSIs.push_back(filename);
////            }

////            // add current model name to map
////            modelNames[someModelName] = someModelName;

////            if (stopFlag) { // if "Cancel" is selected in advanced mode in parameter selection, don't run analysis
////                return;
////            }

////            auto progDialog = QProgressDialog();
////            progDialog.setRange(0, currentWSIs.size());
////            //progDialog.setContentsMargins(0, 0, 0, 0);
////            progDialog.setValue(0);
////            progDialog.setVisible(true);
////            progDialog.setModal(false);
////            progDialog.setLabelText("Running inference...");
////            //QRect screenrect = mWidget->screen()[0].geometry();
////            progDialog.move(mWidget->width() - progDialog.width() * 1.1, progDialog.height() * 0.1);
////            progDialog.show();

////            QCoreApplication::processEvents(QEventLoop::AllEvents, 0);

//            auto counter = 1;
//            for (const auto &currWSI : DataManager::GetInstance()->getAllImages()) {

//                auto wsi_name = currWSI.first;
//                auto wsi_object = currWSI.second;
//                std::cout << "current WSI: " << wsi_name << std::endl;
//                if (!wsi_object->has_renderer(modelMetadata["name"])) { // only run analysis if it has not been ran previously on current WSI

//                     // based on predicted magnification level of WSI, set magnificiation level for optimal input to model based on predicted resolution of WSI
//                    int patch_lvl_model = 0; // defaults to 0
//                    if (!modelMetadata["magnification_level"].empty()) {
//                        patch_lvl_model = (int)(
//                            std::log(magn_lvl / (float)std::stoi(modelMetadata["magnification_level"])) /
//                            std::log(std::round(stof(metadata["openslide.level[1].downsample"]))));
//                    }
//                    else {
//                        std::cout << "magnification_level was not properly defined in the model config file. Defaults to using image plane 0." << std::endl;
//                    }

//                    std::cout << "Curr patch level: " << patch_lvl_model << std::endl;

//                    // read current wsi <=> or do we assume they are at all time in memory? @TODO.
//                    std::cout << "current WSI: " << wsi_name << std::endl;
//                    auto currImage = wsi_object->get_image_pyramid();
////                    auto someImporter = WholeSlideImageImporter::New();
////                    someImporter->setFilename(currWSI);
////                    auto currImage = someImporter->updateAndGetOutputData<ImagePyramid>();

////                    if (!m_runForProject) {
////                        currImage = m_image;
////                    }

//                    auto access = currImage->getAccess(ACCESS_READ);

//                    ImageResizer::pointer resizer = ImageResizer::New();
//                    int currLvl;
//                    if (modelMetadata["resolution"] == "low") {
//                        //auto access = currImage->getAccess(ACCESS_READ);
//                        // TODO: Should automatically find best suitable magn.lvl. (NOTE: 2x the image size as for selecting lvl!)

//                        int levelCount = std::stoi(metadata["openslide.level-count"]);
//                        int inputWidth = std::stoi(modelMetadata["input_img_size_x"]);
//                        int inputHeight = std::stoi(modelMetadata["input_img_size_y"]);
//                        bool breakFlag = false;
//                        for (int i = 0; i < levelCount; i++) {
//                            if ((std::stoi(metadata["openslide.level[" + std::to_string(i) + "].width"]) <=
//                                inputWidth * 2) ||
//                                (std::stoi(metadata["openslide.level[" + std::to_string(i) + "].height"]) <=
//                                    inputHeight * 2)) {
//                                currLvl = i - 1;
//                                breakFlag = true;
//                                break;
//                            }
//                        }
//                        if (!breakFlag)
//                            currLvl = levelCount - 1;

//                        std::cout << "Optimal patch level: " << std::to_string(currLvl) << std::endl;
//                        if (currLvl < 0) {
//                            std::cout << "Automatic chosen patch level for low_res is invalid: "
//                                << std::to_string(currLvl)
//                                << std::endl;
//                            return;
//                        }
//                        auto input = access->getLevelAsImage(currLvl);

//                        // resize
//                        //ImageResizer::pointer resizer = ImageResizer::New();
//                        resizer->setInputData(input);
//                        resizer->setWidth(inputWidth);
//                        resizer->setHeight(inputHeight);
//                    }

//                    // get available IEs as a list
//                    std::list<std::string> IEsList;
//                    QStringList tmpPaths = QDir(QString::fromStdString(Config::getLibraryPath())).entryList(
//                        QStringList(),
//                        QDir::Files);

//                    auto currOperatingSystem = QSysInfo::productType();
//                    auto currKernel = QSysInfo::kernelType();
//                    std::cout << "Current OS is: " << currOperatingSystem.toStdString() << std::endl;
//                    std::cout << "Current kernel is: " << currKernel.toStdString() << std::endl;
//                    if (currKernel == "linux") {
//                        foreach(QString filePath, tmpPaths) {
//                            if (filePath.toStdString().find("libInferenceEngine") != std::string::npos) {
//                                IEsList.push_back(
//                                    splitCustom(splitCustom(filePath.toStdString(), "libInferenceEngine").back(),
//                                        ".so")[0]);
//                            }
//                        }
//                    }
//                    else if ((currKernel == "winnt") || (currKernel == "wince")) {
//                        foreach(QString filePath, tmpPaths) {
//                            if (filePath.toStdString().find("InferenceEngine") != std::string::npos) {
//                                IEsList.push_back(
//                                    splitCustom(splitCustom(filePath.toStdString(), "InferenceEngine").back(),
//                                        ".dll")[0]);
//                            }
//                        }
//                    }
//                    else {
//                        std::cout
//                            << "Current operating system is not using any of the supported kernels: linux and winnt. Current kernel is: "
//                            << currKernel.toStdString() << std::endl;
//                    }

//                    // check which model formats exists, before choosing inference engine
//                    QDir directory(QString::fromStdString(cwd + "data/Models/"));
//                    QStringList models = directory.entryList(QStringList(), QDir::Files);

//                    std::list<std::string> acceptedModels;
//                    foreach(QString currentModel, models) {
//                        if (currentModel.toStdString().find(someModelName) != std::string::npos) {
//                            acceptedModels.push_back(
//                                "." + splitCustom(currentModel.toStdString(), someModelName + ".").back());
//                            std::cout
//                                << "accepted models: ." +
//                                splitCustom(currentModel.toStdString(), someModelName + ".").back()
//                                << std::endl;
//                        }
//                    }

//                    // init network
//                    auto network = NeuralNetwork::New(); // default, need special case for high_res segmentation
//                    if ((modelMetadata["problem"] == "segmentation") && (modelMetadata["resolution"] == "high")) {
//                        network = SegmentationNetwork::New();
//                    }
//                    else if ((modelMetadata["problem"] == "segmentation") &&
//                        (modelMetadata["resolution"] == "low")) {
//                        network = SegmentationNetwork::New();
//                    }

//                    bool checkFlag = true;

//                    std::cout << "Current available IEs: " << std::endl;
//                    foreach(std::string elem, IEsList) {
//                        std::cout << elem << ", " << std::endl;
//                    }

//                    std::cout << "Which model formats are available and that there exists an IE for: " << std::endl;
//                    foreach(std::string elem, acceptedModels) {
//                        std::cout << elem << ", " << std::endl;
//                    }

//                    std::string chosenIE;

//                    // /*
//                    // Now select best available IE based on which extensions exist for chosen model
//                    // TODO: Current optimization profile is: 0. Please ensure there are no enqueued operations pending in this context prior to switching profiles
//                    if ((std::find(acceptedModels.begin(), acceptedModels.end(), ".onnx") !=
//                         acceptedModels.end()) &&
//                        (std::find(IEsList.begin(), IEsList.end(), "TensorRT") != IEsList.end())) {
//                        // @TODO: I don't think this works exactly how I want it to. TensorRT is still find as it is found in the lib/ directory, even though
//                        //  it is not installed.
//                        std::cout << "TensorRT (using ONNX) selected" << std::endl;
//                        network->setInferenceEngine("TensorRT");
//                        chosenIE = "onnx";
//                    }
//                    else if ((std::find(acceptedModels.begin(), acceptedModels.end(), ".uff") != acceptedModels.end()) &&
//                             (std::find(IEsList.begin(), IEsList.end(), "TensorRT") != IEsList.end())) {
//                        std::cout << "TensorRT selected (using UFF)" << std::endl;
//                        network->setInferenceEngine("TensorRT");
//                        chosenIE = "uff";
//                    }
//                    else if ((std::find(acceptedModels.begin(), acceptedModels.end(), ".onnx") !=
//                        acceptedModels.end()) &&
//                        (std::find(IEsList.begin(), IEsList.end(), "OpenVINO") != IEsList.end())) {
//                        std::cout << "OpenVINO (using ONNX) selected" << std::endl;
//                        network->setInferenceEngine("OpenVINO");
//                        chosenIE = "onnx";
//                    }
//                    else if ((std::find(acceptedModels.begin(), acceptedModels.end(), ".xml") !=
//                              acceptedModels.end()) &&
//                             (std::find(IEsList.begin(), IEsList.end(), "OpenVINO") != IEsList.end())) {
//                        std::cout << "OpenVINO (using IR) selected" << std::endl;
//                        network->setInferenceEngine("OpenVINO");
//                        chosenIE = "xml";
//                    }
//                    else if (std::find(acceptedModels.begin(), acceptedModels.end(), ".pb") !=
//                        acceptedModels.end() &&
//                        std::find(IEsList.begin(), IEsList.end(), "TensorFlow") != IEsList.end()) {
//                        std::cout << "TensorFlow selected" << std::endl;
//                        network->setInferenceEngine("TensorFlow");
//                    }
//                    /* else {
//                        std::cout << "Model does not exist in Models/ folder. Please add it using AddModels(). "
//                                     "It might also be that the model exists, but the Inference Engine does not. "
//                                     "Available IEs are: ";
//                        foreach(std::string elem, IEsList) {
//                            std::cout << elem << ", ";
//                        }
//                        checkFlag = false;
//                    }
//                     */

//                    if (checkFlag) {
//                        std::cout << "Model was found." << std::endl;

//                        // TODO: Need to handle if model is in Models/, but inference engine is not available
//                        //Config::getLibraryPath();


//                        //if ((modelMetadata["problem"] == "segmentation") && (modelMetadata["resolution"] == "high")) {
//                        //	network->setInferenceEngine("OpenVINO");
//                        //}
//                        if (true) {
//                            // If model has CPU flag only, need to check if TensorFlowCPU is available, else run on OpenVINO, else use best available
//                            if (std::stoi(modelMetadata["cpu"]) == 1) {
//                                if (std::find(acceptedModels.begin(), acceptedModels.end(), ".pb") !=
//                                    acceptedModels.end() &&
//                                    std::find(IEsList.begin(), IEsList.end(), "TensorFlow") != IEsList.end()) {
//                                    std::cout << "GPU is disabled! (with TensorFlow)" << std::endl;
//                                    network->setInferenceEngine("TensorFlow");
//                                    network->getInferenceEngine()->setDeviceType(InferenceDeviceType::CPU);  // Andre: 0 or 1 for personal Ubuntu Desktop
//                                }
//                                else if (std::find(acceptedModels.begin(), acceptedModels.end(), ".xml") !=
//                                    acceptedModels.end() &&
//                                    std::find(IEsList.begin(), IEsList.end(), "OpenVINO") != IEsList.end()) {
//                                    std::cout << "GPU is disabled! (with OpenVINO)" << std::endl;
//                                    network->setInferenceEngine("OpenVINO");
//                                    //network->getInferenceEngine()->setDeviceType(InferenceDeviceType::CPU);
//                                    network->getInferenceEngine()->setDeviceType(InferenceDeviceType::CPU);
//                                }
//                                else {
//                                    std::cout
//                                        << "CPU only was selected, but was not able to find any CPU devices..."
//                                        << std::endl;
//                                }
//                            }

//                            // if stated in the model txt file, use the specified inference engine
//                            if (!((modelMetadata.count("IE") == 0) || modelMetadata["IE"] == "none")) {
//                                std::cout << "Preselected IE was used: " << modelMetadata["IE"] << std::endl;
//                                network->setInferenceEngine(modelMetadata["IE"]);
//                                chosenIE = getModelFileExtension(network->getInferenceEngine()->getPreferredModelFormat());
//                            }

//                            const auto engine = network->getInferenceEngine()->getName();
//                            // IEs like TF and TensorRT need to be handled differently than IEs like OpenVINO
//                            if (engine.substr(0, 10) == "TensorFlow") {
//                                // apparently this is needed if model has unspecified input size
//                                network->setInputNode(0, modelMetadata["input_node"], NodeType::IMAGE, TensorShape(
//                                    { 1, std::stoi(modelMetadata["input_img_size_y"]),
//                                     std::stoi(modelMetadata["input_img_size_x"]),
//                                     std::stoi(modelMetadata["nb_channels"]) })); //{1, size, size, 3}

//                                // TensorFlow needs to know what the output node is called
//                                if (modelMetadata["problem"] == "classification") {
//                                    network->setOutputNode(0, modelMetadata["output_node"], NodeType::TENSOR,
//                                        TensorShape(
//                                            { 1, std::stoi(modelMetadata["nb_classes"]) }));
//                                }
//                                else if (modelMetadata["problem"] == "segmentation") {
//                                    network->setOutputNode(0, modelMetadata["output_node"], NodeType::TENSOR,
//                                        TensorShape(
//                                            { 1, std::stoi(modelMetadata["input_img_size_y"]),
//                                             std::stoi(modelMetadata["input_img_size_x"]),
//                                             std::stoi(modelMetadata["nb_classes"]) }));
//                                }
//                                else if (modelMetadata["problem"] == "object_detection") {
//                                    // FIXME: This is outdated for YOLOv3, as it has multiple output nodes -> need a way of handling this!
//                                    network->setOutputNode(0, modelMetadata["output_node"], NodeType::TENSOR,
//                                        TensorShape(
//                                            { 1, std::stoi(modelMetadata["nb_classes"]) }));
//                                }
//                            }
//                            else if ((engine == "TensorRT") && (chosenIE == "uff")) {
//                                // TensorRT needs to know everything about the input and output nodes
//                                network->setInputNode(0, modelMetadata["input_node"], NodeType::IMAGE, TensorShape(
//                                    { 1, std::stoi(modelMetadata["nb_channels"]),
//                                     std::stoi(modelMetadata["input_img_size_y"]),
//                                     std::stoi(modelMetadata["input_img_size_y"]) })); //{1, size, size, 3}
//                                network->setOutputNode(0, modelMetadata["output_node"], NodeType::TENSOR,
//                                    TensorShape({ 1, std::stoi(modelMetadata["nb_classes"]) }));
//                            }

//                            if ((engine != "TensorRT") && (engine != "OpenVINO")) {
//                                chosenIE = getModelFileExtension(network->getInferenceEngine()->getPreferredModelFormat());
//                            }
//                            network->load(cwd + "data/Models/" + someModelName + "." + chosenIE);
//                        }

//                        auto generator = PatchGenerator::New();
//                        if (modelMetadata["resolution"] == "low") { // special case handling for low_res NN inference
//                            auto port = resizer->getOutputPort();
//                            resizer->update();
//                            network->setInputData(port->getNextFrame<Image>());
//                        } else {
//                            // whether or not to run tissue segmentation
//                            if (modelMetadata["tissue_threshold"] == "none") {
//                                std::cout
//                                    << "No tissue segmentation filtering will be applied before this analysis."
//                                    << std::endl;
//                            } else if (!modelMetadata["tissue_threshold"].empty()) {
//                                std::cout << "Threshold was defined: " << modelMetadata["tissue_threshold"] << std::endl;
//                                auto tissueSegmentation = TissueSegmentation::New();
//                                tissueSegmentation->setInputData(m_image);
//                                tissueSegmentation->setThreshold(std::stoi(modelMetadata["tissue_threshold"]));

//                                generator->setInputConnection(1, tissueSegmentation->getOutputPort());

//                                std::cout << "tissue_threshold was defined, so is performing thresholding as preprocessing step." << std::endl;
//                            }
//                            else {
//                                std::cout
//                                    << "The tissue_threshold has not been properly defined in the model config file, and thus the method will use any existing segmentation masks as filtering (if available)."
//                                    << std::endl;
//                                // TODO: This should be handled more generically. For pipelines that allow the user to use
//                                //   an already existing segmentation as mask for another method, they should be able to
//                                //   set this method themselves from the GUI (at least in advanced mode), or perhaps where
//                                //   results from previous runs may be used if available (instead through hard-coded variable
//                                //   names such as m_tissue and m_tumorMap.
//                                if (m_tissue) {
//                                    generator->setInputData(1, m_tissue);
//                                } else if (m_tumorMap) {
//                                    generator->setInputData(1, m_tumorMap);
//                                }
//                            }

//                            generator->setPatchSize(std::stoi(modelMetadata["input_img_size_y"]),
//                                std::stoi(modelMetadata["input_img_size_x"]));
//                            generator->setPatchLevel(patch_lvl_model);
//                            if (modelMetadata["mask_threshold"].empty()) {
//                                std::cout << "No mask_threshold variable exists. Defaults to 0.5." << std::endl;
//                            } else {
//                                std::cout << "Setting mask_threshold to: " << modelMetadata["mask_threshold"] << std::endl;
//                                generator->setMaskThreshold(std::stof(modelMetadata["mask_threshold"]));
//                            }
//                            if (modelMetadata["patch_overlap"].empty()) {
//                                std::cout << "No patch_overlap variable exists. Defaults to 0." << std::endl;
//                            } else {
//                                generator->setOverlap(std::stof(modelMetadata["patch_overlap"]));
//                            }
//                            generator->setInputData(0, currImage);

//                            //auto batchgen = ImageToBatchGenerator::New();  // TODO: Can't use this with TensorRT (!)
//                            //batchgen->setInputConnection(generator->getOutputPort());
//                            //batchgen->setMaxBatchSize(std::stoi(modelMetadata["batch_process"])); // set 256 for testing stuff (tumor -> annotation, then grade from tumor segment)

//                            network->setInputConnection(generator->getOutputPort());
//                        }
//                        if (modelMetadata["scale_factor"].empty()) {
//                            std::cout << "scale_factor not defined. Defaults to using using no intensity normalization/scaling in preprocessing." << std::endl;
//                        }
//                        else {
//                            vector scale_factor = splitCustom(modelMetadata["scale_factor"], "/"); // get scale factor from metadata
//                            network->setScaleFactor(
//                                (float)std::stoi(scale_factor[0]) /
//                                (float)std::stoi(scale_factor[1]));   // 1.0f/255.0f
//                        }

//                        // define renderer from metadata
//                        if ((modelMetadata["problem"] == "classification") && (modelMetadata["resolution"] == "high")) {
//                            auto stitcher = PatchStitcher::New();
//                            stitcher->setInputConnection(network->getOutputPort());

//                            auto currentHeatmapName = modelMetadata["name"];
//                            std::cout << "currentHeatmapName: " << currentHeatmapName << ", currWSI: " << currWSI << std::endl;

//                            if (!m_runForProject) {
//                                m_patchStitcherList[modelMetadata["name"]] = stitcher;

//                                auto someRenderer = HeatmapRenderer::New();
//                                someRenderer->setInterpolation(std::stoi(modelMetadata["interpolation"]));
//                                someRenderer->setInputConnection(stitcher->getOutputPort());
//                                someRenderer->setMaxOpacity(0.6f);
//                                vector<string> colors = splitCustom(modelMetadata["class_colors"], ";");
//                                for (int i = 0; i < std::stoi(modelMetadata["nb_classes"]); i++) {
//                                    vector<string> rgb = splitCustom(colors[i], ",");
//                                    someRenderer->setChannelColor(i, Color((float)std::stoi(rgb[0]) / 255.0f,
//                                        (float)std::stoi(rgb[1]) / 255.0f,
//                                        (float)std::stoi(rgb[2]) / 255.0f));
//                                }

//                                m_rendererTypeList[modelMetadata["name"]] = "HeatmapRenderer";
//                                insertRenderer(modelMetadata["name"], someRenderer);
//                            }

//                            if (m_runForProject) {

//                                //auto start = std::chrono::high_resolution_clock::now();
//                                DataObject::pointer data;
//                                do {
//                                    data = stitcher->updateAndGetOutputData<Tensor>();

//                                } while (!data->isLastFrame());
//                                // check if folder for current WSI exists, if not, create one
//                                QString wsiResultPath = (projectFolderName.toStdString() + "/results/" +
//                                    splitCustom(splitCustom(currWSI, "/").back(), ".")[0]).c_str();
//                                wsiResultPath = wsiResultPath.replace("//", "/");
//                                if (!QDir(wsiResultPath).exists()) {
//                                    QDir().mkdir(wsiResultPath);
//                                }

//                                auto exporter = HDF5TensorExporter::New();
//                                exporter->setFilename(wsiResultPath.toStdString() + "/" + splitCustom(wsiResultPath.toStdString(), "/").back() + "_" + currentHeatmapName + ".h5");
//                                exporter->setDatasetName(currentHeatmapName);
//                                exporter->setInputData(data);
//                                exporter->update();
//                            }

//                        }
//                        else if ((modelMetadata["problem"] == "segmentation") && (modelMetadata["resolution"] == "high")) {
//                            if (!m_runForProject) {
//                                auto stitcher = PatchStitcher::New();
//                                stitcher->setInputConnection(network->getOutputPort());
//                                auto port = stitcher->getOutputPort();

//                                /*
//                                auto start = std::chrono::high_resolution_clock::now();
//                                DataObject::pointer data;
//                                do {
//                                    data = stitcher->updateAndGetOutputData<DataObject>();
//                                } while (!data->isLastFrame());
//                                 */

//                                auto someRenderer = SegmentationRenderer::New();
//                                someRenderer->setOpacity(0.7f, 1.0f);
//                                vector<string> colors = splitCustom(modelMetadata["class_colors"], ";");
//                                for (int i = 0; i < std::stoi(modelMetadata["nb_classes"]); i++) {
//                                    vector<string> rgb = splitCustom(colors[i], ",");
//                                    someRenderer->setColor(i, Color((float)std::stoi(rgb[0]) / 255.0f,
//                                        (float)std::stoi(rgb[1]) / 255.0f,
//                                        (float)std::stoi(rgb[2]) / 255.0f));
//                                }
//                                someRenderer->setInputConnection(stitcher->getOutputPort());

//                                m_rendererTypeList[modelMetadata["name"]] = "SegmentationRenderer";
//                                insertRenderer(modelMetadata["name"], someRenderer);
//                            }
//                            else {
//                                // check if folder for current WSI exists, if not, create one
//                                QString wsiResultPath = (projectFolderName.toStdString() + "/results/" +
//                                    splitCustom(splitCustom(currWSI, "/").back(), ".")[0]).c_str();
//                                wsiResultPath = wsiResultPath.replace("//", "/");
//                                if (!QDir(wsiResultPath).exists()) {
//                                    QDir().mkdir(wsiResultPath);
//                                }
//                                auto currPath =
//                                    wsiResultPath.toStdString() + "/" +
//                                    splitCustom(wsiResultPath.toStdString(), "/").back() +
//                                    "_" + modelMetadata["name"] + "/";
//                                std::cout << "current high-res result path: " << currPath << std::endl;

//                                auto exporter = ImagePyramidPatchExporter::New();
//                                //exporter->setInputData(network->updateAndGetOutputData<Image>());
//                                exporter->setInputConnection(network->getOutputPort());
//                                exporter->setPath(currPath);

//                                //addProcessObject(exporter);  // TODO: Is this required when running the analysis without multi-threading? If included it seems like the segmentation is off-by-one (right-skewed)?

//                                auto port = network->getOutputPort();
//                                DataObject::pointer data;
//                                do {
//                                    exporter->update();
//                                    data = port->getNextFrame<DataObject>();
//                                } while (!data->isLastFrame());
//                            }
//                        }
//                        else if ((modelMetadata["problem"] == "object_detection") && (modelMetadata["resolution"] == "high")) {  // TODO: Perhaps use switch() instead of tons of if-statements?
//                            // FIXME: Currently, need to do special handling for object detection as setThreshold and setAnchors only exist for BBNetwork and not NeuralNetwork

//                            auto currNetwork = BoundingBoxNetwork::New();
//                            if (modelMetadata["pred_threshold"].empty()) {
//                                std::cout << "No pred_threshold variable exists. Defaults to 0.1." << std::endl;
//                            }
//                            else {
//                                currNetwork->setThreshold(std::stof(modelMetadata["nms_threshold"])); //0.01); // default: 0.5
//                            }

//                            std::cout << "Current anchor file path: "
//                                << cwd + "data/Models/" + someModelName + ".anchors"
//                                << std::endl;

//                            // read anchors from corresponding anchor file
//                            std::vector<std::vector<Vector2f> > anchors;
//                            std::ifstream infile(cwd + "data/Models/" + someModelName + ".anchors");
//                            std::string anchorStr;
//                            while (std::getline(infile, anchorStr)) {
//                                std::vector<std::string> anchorVector = splitCustom(anchorStr, " ");
//                                anchorVector.resize(
//                                    6); // for TinyYOLOv3 should only be 6 pairs, 3 for each level (2 levels)
//                                int cntr = 0;
//                                for (int i = 1; i < 3; i++) { // assumes TinyYOLOv3 (only two output layers)
//                                    std::vector<Vector2f> levelAnchors;
//                                    for (int j = 0; j < 3; j++) {
//                                        auto currentPair = splitCustom(anchorVector[cntr], ",");
//                                        levelAnchors.push_back(
//                                            Vector2f(std::stoi(currentPair[0]), std::stoi(currentPair[1])));
//                                        cntr++;
//                                    }
//                                    anchors.push_back(levelAnchors);
//                                }
//                            }
//                            currNetwork->setAnchors(anchors); // finally set anchors

//                            auto scale_factor = splitCustom(modelMetadata["scale_factor"],
//                                "/"); // get scale factor from metadata
//                            currNetwork->setScaleFactor((float)std::stoi(scale_factor[0]) /
//                                (float)std::stoi(scale_factor[1]));   // 1.0f/255.0f
//                            currNetwork->setInferenceEngine(
//                                "OpenVINO"); // FIXME: OpenVINO only currently, as I haven't generalized multiple output nodes case
//                            currNetwork->load(cwd + "data/Models/" + someModelName + "." + getModelFileExtension(
//                                currNetwork->getInferenceEngine()->getPreferredModelFormat())); //".uff");
//                            currNetwork->setInputConnection(generator->getOutputPort());

//                            auto nms = NonMaximumSuppression::New();
//                            if (modelMetadata["nms_threshold"].empty()) {
//                                std::cout << "No nms_threshold variable exists. Defaults to 0.5." << std::endl;
//                            }
//                            else {
//                                nms->setThreshold(std::stof(modelMetadata["nms_threshold"]));
//                            }
//                            nms->setInputConnection(currNetwork->getOutputPort());

//                            auto boxAccum = BoundingBoxSetAccumulator::New();
//                            boxAccum->setInputConnection(nms->getOutputPort());
//                            //boxAccum->setInputConnection(currNetwork->getOutputPort());

//                            auto boxRenderer = BoundingBoxRenderer::New();
//                            boxRenderer->setInputConnection(boxAccum->getOutputPort());

//                            m_rendererTypeList[modelMetadata["name"]] = "BoundingBoxRenderer";
//                            insertRenderer(modelMetadata["name"], boxRenderer);
//                        }
//                        else if ((modelMetadata["problem"] == "segmentation") &&
//                            (modelMetadata["resolution"] == "low")) {

//                            //auto converter = TensorToSegmentation::New();
//                            //converter->setInputConnection(network->getOutputPort());

//                            // resize back
//                            //auto access = currImage->getAccess(ACCESS_READ);
//                            auto input = access->getLevelAsImage(currLvl);

//                            ImageResizer::pointer resizer2 = ImageResizer::New();
//                            //resizer2->setInputData(converter->updateAndGetOutputData<Image>());
//                            resizer2->setInputConnection(network->getOutputPort());
//                            resizer2->setWidth(input->getWidth());
//                            resizer2->setHeight(input->getHeight());


//                            auto port2 = resizer2->getOutputPort();
//                            //m_tumorMap = port2->getNextFrame<Image>();
//                            resizer2->update();

//                            auto currMap = port2->getNextFrame<Image>();
//                            m_tumorMap = currMap;
//                            //auto currMap = m_tumorMap;

//                            if (!m_runForProject) {
//                                //m_tumorMap = currMap;

//                                currMap->setSpacing((float)currImage->getFullWidth() / (float)input->getWidth(),
//                                    (float)currImage->getFullHeight() / (float)input->getHeight(),
//                                    1.0f);

//                                auto someRenderer = SegmentationRenderer::New();
//                                someRenderer->setOpacity(0.4f);
//                                vector<string> colors = splitCustom(modelMetadata["class_colors"], ";");
//                                for (int i = 0; i < std::stoi(modelMetadata["nb_classes"]); i++) {
//                                    vector<string> rgb = splitCustom(colors[i], ",");
//                                    someRenderer->setColor(i, Color((float)std::stoi(rgb[0]) / 255.0f,
//                                        (float)std::stoi(rgb[1]) / 255.0f,
//                                        (float)std::stoi(rgb[2]) / 255.0f));
//                                }
//                                someRenderer->setInputData(currMap);
//                                //someRenderer->setInterpolation(false);
//                                someRenderer->update();

//                                m_rendererTypeList[modelMetadata["name"]] = "SegmentationRenderer";
//                                insertRenderer(modelMetadata["name"], someRenderer);
//                            }
//                            else {
//                                // save result
//                                QString wsiResultPath = (projectFolderName.toStdString() + "/results/" +
//                                    splitCustom(splitCustom(currWSI, "/").back(), ".")[0]).c_str();
//                                wsiResultPath = wsiResultPath.replace("//", "/");
//                                std::cout << "current result path: " << wsiResultPath.toStdString() << std::endl;
//                                if (!QDir(wsiResultPath).exists()) {
//                                    QDir().mkdir(wsiResultPath);
//                                }
//                                currMap->setSpacing(1.0f, 1.0f, 1.0f);

//                                auto exporter = ImageFileExporter::New();
//                                exporter->setFilename(
//                                    wsiResultPath.toStdString() + "/" +
//                                    splitCustom(splitCustom(currWSI, "/").back(), ".")[0] +
//                                    "_" + modelMetadata["name"] + ".png");
//                                exporter->setInputData(currMap);
//                                exporter->update();
//                            }
//                        }
//                    }
//                }
//                // update progress bar
//                // TODO: these are not updated in the main thread. Need to introduce signals such that the Qt-related stuff are properly updated in the main thread
//                progDialog.setValue(counter);
//                QCoreApplication::processEvents(QEventLoop::AllEvents, 0);
//                counter++;
//            }

//            //emit inferenceFinished(someModelName);
//            std::cout << "Inference thread is finished..." << std::endl;
//        //} catch (const std::exception& e){
//        //    simpleInfoPrompt("Something went wrong during inference.");
//        };
//    }

} // End of namespace fast
