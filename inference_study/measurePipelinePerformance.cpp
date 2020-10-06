//
// Created by andrep on 14.09.2020.
// - Adapted from Erik Smistad's:
// https://github.com/smistad/FAST/blob/master/source/FAST/Algorithms/NeuralNetwork/measureNeuralNetworkPerformance.cpp
//
#include <FAST/Testing.hpp>
#include <FAST/Algorithms/NeuralNetwork/SegmentationNetwork.hpp>
#include <FAST/Importers/ImageFileImporter.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>
#include <FAST/Streamers/ImageFileStreamer.hpp>
#include <FAST/Algorithms/NeuralNetwork/InferenceEngineManager.hpp>
#include <fstream>
#include <FAST/Algorithms/ImagePatch/PatchGenerator.hpp>
#include <FAST/Algorithms/ImagePatch/PatchStitcher.hpp>
#include <FAST/Importers/WholeSlideImageImporter.hpp>
#include <FAST/Algorithms/TissueSegmentation/TissueSegmentation.hpp>
#include <FAST/Tools/CommandLineParser.hpp>
#include <FAST/Algorithms/ImagePatch/ImageToBatchGenerator.hpp>
#include <FAST/Algorithms/ImageResizer/ImageResizer.hpp>
#include <FAST/Data/ImagePyramid.hpp>
#include <FAST/Algorithms/NeuralNetwork/TensorToSegmentation.hpp>
#include <FAST/Algorithms/NeuralNetwork/BoundingBoxNetwork.hpp>
#include <FAST/Data/BoundingBox.hpp>

using namespace fast;

int main(int argc, char** argv) {
    Reporter::setGlobalReportMethod(Reporter::INFO, Reporter::NONE);

    CommandLineParser parser("Measure neural network performance script");
    parser.addOption("disable-case-1");
    parser.addOption("disable-case-2");
    parser.addOption("disable-case-3");
    parser.addOption("disable-case-4");
    parser.addOption("disable-case-3a");
    parser.addOption("disable-case-4a");
    parser.addOption("disable-case-1-batch");
    parser.addOption("disable-case-1_inceptionv3");
    //parser.addOption("disable-case-3-batch");
    parser.addOption("disable-warmup");
    parser.parse(argc, argv);
    const int iterations = 10;  //10;
    const bool warmupIteration = !parser.getOption("disable-warmup");
    const bool case1 = !parser.getOption("disable-case-1");
    const bool case2 = !parser.getOption("disable-case-2");
    const bool case3 = !parser.getOption("disable-case-3");
    const bool case4 = !parser.getOption("disable-case-4");
    const bool case3a = !parser.getOption("disable-case-3a");
    const bool case4a = !parser.getOption("disable-case-4a");
    const bool case1_batch = !parser.getOption("disable-case-1-batch");
    const bool case1_inceptionv3 = !parser.getOption("disable-case-1_inceptionv3");
    //const bool case3_batch = !parser.getOption("disable-case-3-batch");
    const std::string machine = "windows";  // ubuntu or windows, just for storing results on both machines used in the experiments


    if(case1) {
        std::cout << "\nPatch-wise classification...\n" << std::endl;
        // CASE 1 - Patch-wise classification using BACH-model on 20x WSI
        std::string resultFilename = "../results_" + machine + "/neural-network-runtimes-case-1.csv";
        if (machine == "windows") {
            resultFilename = "../" + resultFilename;
        }
        std::ofstream file(resultFilename.c_str());

        std::vector<int> img_size {512, 512};
        int patch_level = 0;
        int nb_gpus = 1;

        // Write header
        file << "Engine;Device Type;Iteration;Patch generator AVG;Patch generator STD;NN input AVG;NN input STD;NN inference AVG;NN inference STD;NN output AVG;NN output STD;Patch stitcher AVG;Patch stitcher STD;Total\n";

        auto engines = {"TensorRT", "TensorFlowCUDA", "OpenVINO", "TensorFlowCPU"};
        if (machine == "windows") {
            engines = {"OpenVINO", "TensorFlowCPU"};
        }
        //for(auto &engine : InferenceEngineManager::getEngineList()) {TensorFlowCUDA
        for(std::string engine : engines) {
            std::map<std::string, InferenceDeviceType> deviceTypes = {{"ANY", InferenceDeviceType::ANY}};
            if(engine == "OpenVINO") {
                // On OpenVINO, try all device types
                deviceTypes = std::map<std::string, InferenceDeviceType>{
                        {"CPU", InferenceDeviceType::CPU},
                        //{"GPU", InferenceDeviceType::GPU},
                        //{"VPU", InferenceDeviceType::VPU},
                };
                if (machine == "windows") {
                    deviceTypes = std::map<std::string, InferenceDeviceType>{
                        {"CPU", InferenceDeviceType::CPU},
                        {"GPU", InferenceDeviceType::GPU},
                        //{"VPU", InferenceDeviceType::VPU},
                };
                }
            }
            for(auto &&deviceType : deviceTypes) {
                std::cout << engine << " for device type " << deviceType.first << std::endl;
                std::cout << "====================================" << std::endl;

                for(int currDevice = 0; currDevice <= nb_gpus; ++currDevice) {  // for each GPU
                    //if (!((engine == "TensorRT") || (engine == "TensorFlowCUDA")) && (currDevice > 0))
                    if (!((engine == "TensorRT")) && (currDevice > 0))
                        break;

                    std::cout << "Current device (GPU): " << currDevice << std::endl;
                    std::cout << "//////////////////////////////" << std::endl;

                    for (int iteration = 0; iteration <= iterations; ++iteration) {
                        auto importer = WholeSlideImageImporter::New();
                        if (machine == "windows") {
                            importer->setFilename("C:/Users/andrep/workspace/FAST-Pathology_old/A05.svs");
                        } else {
                            importer->setFilename(Config::getTestDataPath() + "/WSI/A05.svs");
                        }

                        auto tissueSegmentation = TissueSegmentation::New();
                        tissueSegmentation->setInputConnection(importer->getOutputPort());

                        auto generator = PatchGenerator::New();
                        generator->setPatchSize(img_size[0], img_size[1]);
                        generator->setPatchLevel(patch_level);
                        generator->setInputConnection(importer->getOutputPort());
                        generator->setInputConnection(1, tissueSegmentation->getOutputPort());
                        generator->enableRuntimeMeasurements();

                        auto network = NeuralNetwork::New();
                        network->setInferenceEngine(engine);
                        if (engine != "TensorRT") {
                        //if (!((engine == "TensorRT") || (engine == "TensorFlowCUDA"))) {
                            network->getInferenceEngine()->setDevice(1);
                        } else {
                            network->getInferenceEngine()->setDevice(currDevice);
                        }
                        std::string postfix;
                        if (engine.substr(0, 10) == "TensorFlow") {
                            network->setInputNode(0, "input_1", NodeType::IMAGE,
                                                  TensorShape({1, img_size[0], img_size[1], 3}));
                            network->setOutputNode(0, "dense_1/Softmax", NodeType::TENSOR);
                        } else if (engine == "TensorRT") {
                            network->setInputNode(0, "input_1", NodeType::IMAGE,
                                                  TensorShape{-1, 3, img_size[0], img_size[1]});
                            network->setOutputNode(0, "dense_1/Softmax", NodeType::TENSOR, TensorShape{-1, 3});
                        } else if (engine == "OpenVINO") {
                            network->getInferenceEngine()->setDeviceType(deviceType.second);
                            if (deviceType.first == "VPU") {
                                postfix = "_fp16";
                            }
                        }
                        //network->load(Config::getTestDataPath() + "NeuralNetworkModels/wsi_classification" + postfix + "." + network->getInferenceEngine()->getDefaultFileExtension());
                        if (machine == "ubuntu") {
                            network->load("/home/andrep/FastPathology/data/Models/mobilenet_v2_bach_model" + postfix + "." +
                                      network->getInferenceEngine()->getDefaultFileExtension());
                        }
                        else if (machine == "windows") {
                            network->load("C:/Users/andrep/FastPathology/data/Models/mobilenet_v2_bach_model" + postfix + "." +
                                      network->getInferenceEngine()->getDefaultFileExtension());
                        }
                        network->setInputConnection(generator->getOutputPort());
                        network->setScaleFactor(1.0f / 255.0f);
                        network->enableRuntimeMeasurements();

                        auto stitcher = PatchStitcher::New();
                        stitcher->setInputConnection(network->getOutputPort());
                        stitcher->enableRuntimeMeasurements();

                        auto start = std::chrono::high_resolution_clock::now();
                        DataObject::pointer data;
                        do {
                            data = stitcher->updateAndGetOutputData<DataObject>();
                        } while (!data->isLastFrame());

                        std::chrono::duration<float, std::milli> timeUsed =
                                std::chrono::high_resolution_clock::now() - start;
                        std::cout << "Total runtime: " << timeUsed.count() << std::endl;
                        std::cout << "Patch generator runtime: " << std::endl;
                        generator->getRuntime("create patch")->print();
                        std::cout << "NN runtime: " << std::endl;
                        network->getRuntime()->print();
                        std::cout << "Patch stitcher runtime: " << std::endl;
                        stitcher->getRuntime()->print();

                        if (iteration == 0 && warmupIteration)
                            continue;

                        auto deviceUsed = deviceType.first;
                        if ((engine == "TensorRT") || (engine == "TensorFlowCUDA")) {
                            deviceUsed = deviceType.first + std::to_string(currDevice);
                        } else {
                            deviceUsed = deviceType.first;
                        }

                        //network->stopPipeline();  // free memory when finished

                        file <<
                             engine + ";" +
                             deviceUsed + ";" +  //deviceType.first + ";" +
                             std::to_string(iteration) + ";" +
                             std::to_string(generator->getRuntime("create patch")->getAverage()) + ";" +
                             std::to_string(generator->getRuntime("create patch")->getStdDeviation()) + ";" +
                             std::to_string(network->getRuntime("input_processing")->getAverage()) + ";" +
                             std::to_string(network->getRuntime("input_processing")->getStdDeviation()) + ";" +
                             std::to_string(network->getRuntime("inference")->getAverage()) + ";" +
                             std::to_string(network->getRuntime("inference")->getStdDeviation()) + ";" +
                             std::to_string(network->getRuntime("output_processing")->getAverage()) + ";" +
                             std::to_string(network->getRuntime("output_processing")->getStdDeviation()) + ";" +
                             std::to_string(stitcher->getRuntime("stitch patch")->getAverage()) + ";" +
                             std::to_string(stitcher->getRuntime("stitch patch")->getStdDeviation()) + ";" +
                             std::to_string(timeUsed.count())
                             << std::endl;
                    }
                }
            }
        }
    }


    if(case2) {
        std::cout << "\nLow-res semantic segmentation...\n" << std::endl;
        // CASE 2 - Low-res semantic segmentation of nuclei on 20x WSI
        const std::string resultFilename = "../results_" + machine + "neural-network-runtimes-case-2.csv";
        std::ofstream file(resultFilename.c_str());

        std::vector<int> img_size {1024, 1024};
        int patch_level = 1;
        int nb_gpus = 1;

        // Write header
        file << "Engine;Device Type;Iteration;Image Read AVG;Image Read STD;Resize AVG;Resize STD;NN input AVG;NN input STD;NN inference AVG;NN inference STD;NN output AVG;NN output STD;Convert AVG;Convert STD;Resize Back AVG;Resize Back STD;Total\n";

        //for(auto &engine : InferenceEngineManager::getEngineList()) {
        for(std::string engine : {"TensorFlowCUDA", "OpenVINO", "TensorFlowCPU"}) {  // {"TensorFlowCUDA", "OpenVINO", "TensorFlowCPU"}) {
            std::map<std::string, InferenceDeviceType> deviceTypes = {{"ANY", InferenceDeviceType::ANY}};
            if(engine == "OpenVINO") {
                // On OpenVINO, try all device types
                deviceTypes = std::map<std::string, InferenceDeviceType>{
                        {"CPU", InferenceDeviceType::CPU},
                        //{"GPU", InferenceDeviceType::GPU},
                        //{"VPU", InferenceDeviceType::VPU},
                };
            }
            for(auto &&deviceType : deviceTypes) {
                std::cout << engine << " for device type " << deviceType.first << std::endl;
                std::cout << "====================================" << std::endl;

                for(int currDevice = 0; currDevice < nb_gpus; ++currDevice) {  // for each GPU
                    //if (!((engine == "TensorRT") || (engine == "TensorFlowCUDA")) && (currDevice > 0))
                    //    break;

                    std::cout << "Current device (GPU): " << currDevice << std::endl;
                    std::cout << "//////////////////////////////" << std::endl;
                    for (int iteration = 0; iteration <= iterations; ++iteration) {  // run N number of times
                        auto importer = WholeSlideImageImporter::New();
                        importer->setFilename(Config::getTestDataPath() + "/WSI/A05.svs");
                        importer->enableRuntimeMeasurements();

                        auto m_image = importer->updateAndGetOutputData<ImagePyramid>();
                        auto access = m_image->getAccess(ACCESS_READ);
                        //auto input = access->getLevelAsImage(m_image->getNrOfLevels()-1);
                        auto input = access->getLevelAsImage(patch_level); //m_image->getNrOfLevels()-1); // FIXME: Should automatically find best suitable magn.lvl.
                        importer->update();

                        // resize
                        ImageResizer::pointer resizer = ImageResizer::New();
                        resizer->setInputData(input);
                        resizer->setWidth(img_size[0]);
                        resizer->setHeight(img_size[1]);
                        resizer->enableRuntimeMeasurements();
                        auto port = resizer->getOutputPort();
                        resizer->update();
                        //Image::pointer resized = port->getNextFrame<Image>();

                        auto network = SegmentationNetwork::New();
                        network->setInferenceEngine(engine);
                        network->getInferenceEngine()->setDevice(1);
                        std::string postfix;
                        if (engine.substr(0, 10) == "TensorFlow") {
                            network->setInputNode(0, "input_1", NodeType::IMAGE,
                                                  TensorShape({1, img_size[0], img_size[1], 3}));
                            network->setOutputNode(0, "conv2d_34/truediv", NodeType::TENSOR,
                                                   {1, img_size[0], img_size[1], 2});
                        } else if (engine == "TensorRT") {
                            network->setInputNode(0, "input_1", NodeType::IMAGE,
                                                  TensorShape{-1, 3, img_size[0], img_size[1]});
                            network->setOutputNode(0, "dense_1/Softmax", NodeType::TENSOR, TensorShape{-1, 3});
                        } else if (engine == "OpenVINO") {
                            //network->setInputNode(0, "input_1", NodeType::IMAGE, TensorShape({1, img_size[0], img_size[1], 3}));
                            //network->setOutputNode(0, "conv2d_34/truediv", NodeType::TENSOR, TensorShape({1, img_size[0], img_size[1], 2}));
                            network->getInferenceEngine()->setDeviceType(deviceType.second);
                            if (deviceType.first == "VPU") {
                                postfix = "_fp16";
                            }
                        }
                        //network->setInputNode(0, "input_1", NodeType::IMAGE, {1, img_size[0], img_size[1], 3});
                        //network->setOutputNode(0, "conv2d_34/truediv", NodeType::TENSOR, {1, img_size[0], img_size[1], 2});
                        network->load("/home/andrep/FastPathology/data/Models/low_res_tumor_unet." +
                                      network->getInferenceEngine()->getDefaultFileExtension());
                        //network->setInputData(resizer->updateAndGetOutputData<Image>());  // FIXME: This hangs, use solution below instead
                        network->setInputData(port->getNextFrame<Image>());
                        network->setScaleFactor(1.0f / 255.0f);   // 1.0f/255.0f
                        network->enableRuntimeMeasurements();

                        auto converter = TensorToSegmentation::New();  // FIXME: This is where it hangs for whatever reason... And only with openvino IE (not TF)
                        converter->setInputConnection(network->getOutputPort());
                        converter->enableRuntimeMeasurements();
                        //converter->update();

                        /*
                        if ((engine == "TensorRT") || (engine == "OpenVINO")) {
                            converter->setNCHW(true);
                        }
                         */

                        // resize back
                        ImageResizer::pointer resizer2 = ImageResizer::New();
                        resizer2->setInputData(converter->updateAndGetOutputData<Image>());
                        resizer2->setWidth(input->getWidth());
                        resizer2->setHeight(input->getHeight());
                        resizer2->enableRuntimeMeasurements();

                        auto port2 = resizer2->getOutputPort();
                        resizer2->update();

                        auto m_tumorMap = port2->getNextFrame<Image>();
                        m_tumorMap->setSpacing((float) m_image->getFullHeight() / (float) input->getHeight(), (float) m_image->getFullWidth() / (float) input->getWidth(), 1.0f);


                        /*
                        auto start = std::chrono::high_resolution_clock::now();
                        //network->update();
                        //auto m_tumorMap = port2->getNextFrame<Image>();
                        //network->update();
                        //resizer2->update();
                        std::chrono::duration<float, std::milli> timeUsed = std::chrono::high_resolution_clock::now() - start;
                         */

                        //std::chrono::duration<float, std::milli>

                        auto timeUsed = importer->getRuntime()->getAverage() +
                                        resizer->getRuntime()->getAverage() +
                                        network->getRuntime()->getAverage() +
                                        converter->getRuntime()->getAverage() +
                                        resizer2->getRuntime()->getAverage();

                        /*
                        std::cout << "Total runtime: " << timeUsed.count() << std::endl;
                        std::cout << "Patch generator runtime: " << patch_time << std::endl;
                        //resizer->getAllRuntimes()->printAll();
                        //resizer->getRuntime()->print();
                        std::cout << "NN runtime: " << nn_time << std::endl;
                        //network->getRuntime()->print();
                        std::cout << "Patch stitcher runtime: " << std::endl;
                        resizer2->getRuntime()->print();
                         */

                        std::cout << "\nTotal runtime: " << timeUsed << std::endl;
                        std::cout << "Read image runtime: " << importer->getRuntime()->getAverage() << std::endl;
                        std::cout << "Resize runtime: " << resizer->getRuntime()->getAverage() << std::endl;
                        std::cout << "NN runtime: " << network->getRuntime()->getAverage() << std::endl;
                        std::cout << "Convert to segmentation runtime: " << converter->getRuntime()->getAverage() << std::endl;
                        std::cout << "Resize back runtime: " << resizer2->getRuntime()->getAverage() << std::endl;
                        //resizer2->getRuntime()->print();

                        if (iteration == 0 && warmupIteration)
                            continue;

                        auto deviceUsed = deviceType.first;
                        if ((engine == "TensorRT") || (engine == "TensorFlowCUDA")) {
                            deviceUsed = deviceType.first + std::to_string(currDevice);
                        } else {
                            deviceUsed = deviceType.first;
                        }

                        file <<
                             engine + ";" +
                             deviceUsed + ";" +  // deviceType.first + ";" +
                             std::to_string(iteration) + ";" +
                             std::to_string(importer->getRuntime()->getAverage()) + ";" +
                             std::to_string(importer->getRuntime()->getStdDeviation()) + ";" +
                             std::to_string(resizer->getRuntime()->getAverage()) + ";" +
                             std::to_string(resizer->getRuntime()->getStdDeviation()) + ";" +
                             std::to_string(network->getRuntime("input_processing")->getAverage()) + ";" +
                             std::to_string(network->getRuntime("input_processing")->getStdDeviation()) +
                             ";" +
                             std::to_string(network->getRuntime("inference")->getAverage()) + ";" +
                             std::to_string(network->getRuntime("inference")->getStdDeviation()) + ";" +
                             std::to_string(network->getRuntime("output_processing")->getAverage()) + ";" +
                             std::to_string(network->getRuntime("output_processing")->getStdDeviation()) +
                             ";" +
                             std::to_string(converter->getRuntime()->getAverage()) + ";" +
                             std::to_string(converter->getRuntime()->getStdDeviation()) + ";" +
                             std::to_string(resizer2->getRuntime()->getAverage()) + ";" +
                             std::to_string(resizer2->getRuntime()->getStdDeviation()) + ";" +
                             std::to_string(timeUsed)  // std::to_string(timeUsed.count())
                             << std::endl;
                    }
                }
            }
        }
    }


    if(case3) {
        std::cout << "\nPatch-wise high-res semantic segmentation...\n" << std::endl;
        // CASE 3 - Patch-wise high-res semantic segmentation on 20x WSI (of nuceli)
        const std::string resultFilename = "../results_" + machine + "neural-network-runtimes-case-3.csv";
        std::ofstream file(resultFilename.c_str());

        std::vector<int> img_size{512, 512};
        int patch_level = 0;

        // Write header
        file << "Engine;Device Type;Iteration;Patch generator AVG;Patch generator STD;NN input AVG;NN input STD;NN inference AVG;NN inference STD;NN output AVG;NN output STD;Patch stitcher AVG;Patch stitcher STD;Total\n";

        //for(auto &engine : InferenceEngineManager::getEngineList()) {
        for (std::string engine : {"TensorFlowCUDA", "OpenVINO", "TensorFlowCPU"}) {
            std::map<std::string, InferenceDeviceType> deviceTypes = {{"ANY", InferenceDeviceType::ANY}};
            if (engine == "OpenVINO") {
                // On OpenVINO, try all device types
                deviceTypes = std::map<std::string, InferenceDeviceType>{
                        {"CPU", InferenceDeviceType::CPU},
                        //{"GPU", InferenceDeviceType::GPU},
                        //{"VPU", InferenceDeviceType::VPU},
                };
            }
            for (auto &&deviceType : deviceTypes) {
                std::cout << engine << " for device type " << deviceType.first << std::endl;
                std::cout << "====================================" << std::endl;

                for (int iteration = 0; iteration <= iterations; ++iteration) {
                    auto importer = WholeSlideImageImporter::New();
                    importer->setFilename(Config::getTestDataPath() + "/WSI/A05.svs");

                    auto tissueSegmentation = TissueSegmentation::New();
                    tissueSegmentation->setInputConnection(importer->getOutputPort());

                    auto generator = PatchGenerator::New();
                    generator->setPatchSize(img_size[0], img_size[1]);
                    generator->setPatchLevel(patch_level);
                    generator->setInputConnection(importer->getOutputPort());
                    generator->setInputConnection(1, tissueSegmentation->getOutputPort());
                    generator->enableRuntimeMeasurements();

                    auto network = SegmentationNetwork::New();
                    network->setInferenceEngine(engine);
                    network->getInferenceEngine()->setDevice(1);
                    std::string postfix;
                    if (engine.substr(0, 10) == "TensorFlow") {
                        network->setOutputNode(0, "conv2d_26/truediv", NodeType::TENSOR);
                    } else if (engine == "TensorRT") {
                        network->setInputNode(0, "input_1", NodeType::IMAGE,
                                              TensorShape{-1, 3, img_size[0], img_size[1]});
                        network->setOutputNode(0, "conv2d_26/truediv", NodeType::TENSOR, TensorShape{-1, 3});
                    } else if (engine == "OpenVINO") {
                        network->getInferenceEngine()->setDeviceType(deviceType.second);
                        if (deviceType.first == "VPU") {
                            postfix = "_fp16";
                        }
                    }
                    //network->load(Config::getTestDataPath() + "NeuralNetworkModels/wsi_classification" + postfix + "." + network->getInferenceEngine()->getDefaultFileExtension());
                    network->load("/home/andrep/FastPathology/data/Models/high_res_nuclei_unet" + postfix + "." + network->getInferenceEngine()->getDefaultFileExtension());
                    network->setInputConnection(generator->getOutputPort());
                    network->setScaleFactor(1.0f / 255.0f);
                    network->enableRuntimeMeasurements();

                    auto stitcher = PatchStitcher::New();
                    stitcher->setInputConnection(network->getOutputPort());
                    stitcher->enableRuntimeMeasurements();

                    auto start = std::chrono::high_resolution_clock::now();
                    DataObject::pointer data;
                    do {
                        data = stitcher->updateAndGetOutputData<DataObject>();
                    } while (!data->isLastFrame());
                    std::chrono::duration<float, std::milli> timeUsed =
                            std::chrono::high_resolution_clock::now() - start;
                    std::cout << "Total runtime: " << timeUsed.count() << std::endl;
                    std::cout << "Patch generator runtime: " << std::endl;
                    generator->getRuntime("create patch")->print();
                    std::cout << "NN runtime: " << std::endl;
                    network->getRuntime()->print();
                    std::cout << "Patch stitcher runtime: " << std::endl;
                    stitcher->getRuntime()->print();

                    if (iteration == 0 && warmupIteration)
                        continue;

                    file <<
                         engine + ";" +
                         deviceType.first + ";" +
                         std::to_string(iteration) + ";" +
                         std::to_string(generator->getRuntime("create patch")->getAverage()) + ";" +
                         std::to_string(generator->getRuntime("create patch")->getStdDeviation()) + ";" +
                         std::to_string(network->getRuntime("input_processing")->getAverage()) + ";" +
                         std::to_string(network->getRuntime("input_processing")->getStdDeviation()) + ";" +
                         std::to_string(network->getRuntime("inference")->getAverage()) + ";" +
                         std::to_string(network->getRuntime("inference")->getStdDeviation()) + ";" +
                         std::to_string(network->getRuntime("output_processing")->getAverage()) + ";" +
                         std::to_string(network->getRuntime("output_processing")->getStdDeviation()) + ";" +
                         std::to_string(stitcher->getRuntime("stitch patch")->getAverage()) + ";" +
                         std::to_string(stitcher->getRuntime("stitch patch")->getStdDeviation()) + ";" +
                         std::to_string(timeUsed.count())
                         << std::endl;
                }
            }
        }
    }

    if(case4) {
        std::cout << "\nPatch-wise object detection and classification...\n" << std::endl;
        // CASE 4 - Patch-wise object detection and classification 20x WSI of nuclei (ODAC)
        const std::string resultFilename = "../results_" + machine + "neural-network-runtimes-case-4.csv";
        std::ofstream file(resultFilename.c_str());

        std::vector<int> img_size {256, 256};
        int patch_level = 0;
        int nb_classes = 1;
        int nb_anchors = 3;

        // Write header
        file << "Engine;Device Type;Iteration;Patch generator AVG;Patch generator STD;NN input AVG;NN input STD;NN inference AVG;NN inference STD;NN output AVG;NN output STD;Patch stitcher AVG;Patch stitcher STD;Total\n";

        //for(auto &engine : InferenceEngineManager::getEngineList()) {
        //for(std::string engine : {"TensorFlowCUDA", "TensorRT", "OpenVINO", "TensorFlowCPU"}) {
        for(std::string engine : {"TensorFlowCUDA", "OpenVINO", "TensorFlowCPU"}) {
            std::map<std::string, InferenceDeviceType> deviceTypes = {{"ANY", InferenceDeviceType::ANY}};
            if(engine == "OpenVINO") {
                // On OpenVINO, try all device types
                deviceTypes = std::map<std::string, InferenceDeviceType>{
                        {"CPU", InferenceDeviceType::CPU},
                        //{"GPU", InferenceDeviceType::GPU},
                        //{"VPU", InferenceDeviceType::VPU},
                };
            }
            for(auto &&deviceType : deviceTypes) {
                std::cout << engine << " for device type " << deviceType.first << std::endl;
                std::cout << "====================================" << std::endl;

                for(int iteration = 0; iteration <= iterations; ++iteration) {
                    auto importer = WholeSlideImageImporter::New();
                    importer->setFilename(Config::getTestDataPath() + "/WSI/A05.svs");

                    auto tissueSegmentation = TissueSegmentation::New();
                    tissueSegmentation->setInputConnection(importer->getOutputPort());

                    auto generator = PatchGenerator::New();
                    generator->setPatchSize(img_size[0], img_size[1]);
                    generator->setPatchLevel(patch_level);
                    generator->setInputConnection(importer->getOutputPort());
                    generator->setInputConnection(1, tissueSegmentation->getOutputPort());
                    generator->enableRuntimeMeasurements();

                    auto network = BoundingBoxNetwork::New();
                    network->setThreshold(0.1); //0.01); // default: 0.5
                    network->setInferenceEngine(engine);
                    network->getInferenceEngine()->setDevice(1);
                    std::string postfix;
                    if(engine.substr(0, 10) == "TensorFlow") {
                        network->setInputNode(0, "input_1", NodeType::IMAGE, TensorShape{-1, img_size[0], img_size[1], 3});
                        network->setOutputNode(0, "conv2d_10/BiasAdd", NodeType::TENSOR, TensorShape{-1, 8, 8, (int)(nb_anchors * (nb_classes + 5))});
                    } else if(engine == "TensorRT") {
                        network->setInputNode(0, "input_1", NodeType::IMAGE, TensorShape{-1, 3, img_size[0], img_size[1]});
                        network->setOutputNode(0, "conv2d_10/BiasAdd", NodeType::TENSOR, TensorShape{-1, 8, 8, 18});
                    } else if(engine == "OpenVINO") {
                        network->getInferenceEngine()->setDeviceType(deviceType.second);
                        if(deviceType.first == "VPU") {
                            postfix = "_fp16";
                        }
                    }

                    // read anchors from corresponding anchor file
                    std::vector<std::vector<Vector2f> > anchors;
                    std::ifstream infile("/home/andrep/FastPathology/data/Models/yolo_test_model_fixed_output_nodes.anchors");
                    std::string anchorStr;
                    while (std::getline(infile, anchorStr)) {
                        std::vector<std::string> anchorVector = split(anchorStr, " ");
                        anchorVector.resize(6); // for TinyYOLOv3 should only be 6 pairs, 3 for each level (2 levels)
                        int cntr = 0;
                        for (int i = 1; i < 3; i++) { // assumes TinyYOLOv3 (only two output layers)
                            std::vector<Vector2f> levelAnchors;
                            for (int j = 0; j < 3; j++) {
                                auto currentPair = split(anchorVector[cntr], ",");
                                levelAnchors.push_back(Vector2f(std::stoi(currentPair[0]), std::stoi(currentPair[1])));
                                cntr++;
                            }
                            anchors.push_back(levelAnchors);
                        }
                    }
                    network->setAnchors(anchors); // finally set anchors

                    //network->load(Config::getTestDataPath() + "NeuralNetworkModels/wsi_classification" + postfix + "." + network->getInferenceEngine()->getDefaultFileExtension());
                    network->load("/home/andrep/FastPathology/data/Models/yolo_test_model_fixed_output_nodes" + postfix + "." + network->getInferenceEngine()->getDefaultFileExtension());
                    network->setInputConnection(generator->getOutputPort());
                    network->setScaleFactor(1.0f / 255.0f);
                    network->enableRuntimeMeasurements();

                    auto boxAccum = BoundingBoxSetAccumulator::New();
                    //boxAccum->setInputConnection(nms->getOutputPort());
                    boxAccum->setInputConnection(network->getOutputPort());
                    boxAccum->enableRuntimeMeasurements();

                    // FIXME: add update of this
                    // MERGE <- for waiting for threads/syncs

                    auto start = std::chrono::high_resolution_clock::now();
                    DataObject::pointer data;
                    do {
                        data = boxAccum->updateAndGetOutputData<DataObject>();
                    } while(!data->isLastFrame());
                    std::chrono::duration<float, std::milli> timeUsed = std::chrono::high_resolution_clock::now() - start;
                    std::cout << "Total runtime: " << timeUsed.count() << std::endl;
                    std::cout << "Patch generator runtime: " << std::endl;
                    generator->getRuntime("create patch")->print();
                    std::cout << "NN runtime: " << std::endl;
                    network->getRuntime()->print();
                    std::cout << "Patch stitcher runtime: " << std::endl;
                    boxAccum->getRuntime()->print();

                    if(iteration == 0 && warmupIteration)
                        continue;

                    file <<
                         engine + ";" +
                         deviceType.first + ";" +
                         std::to_string(iteration) + ";" +
                         std::to_string(generator->getRuntime("create patch")->getAverage()) + ";" +
                         std::to_string(generator->getRuntime("create patch")->getStdDeviation()) + ";" +
                         std::to_string(network->getRuntime("input_processing")->getAverage()) + ";" +
                         std::to_string(network->getRuntime("input_processing")->getStdDeviation()) + ";" +
                         std::to_string(network->getRuntime("inference")->getAverage()) + ";" +
                         std::to_string(network->getRuntime("inference")->getStdDeviation()) + ";" +
                         std::to_string(network->getRuntime("output_processing")->getAverage()) + ";" +
                         std::to_string(network->getRuntime("output_processing")->getStdDeviation()) + ";" +
                         std::to_string(boxAccum->getRuntime("stitch patch")->getAverage()) + ";" +
                         std::to_string(boxAccum->getRuntime("stitch patch")->getStdDeviation()) + ";" +
                         std::to_string(timeUsed.count())
                         << std::endl;
                }
            }
        }
    }

    if(case1_batch) {
        std::cout << "\nBatch-inference with use-case patch-wise classification...\n" << std::endl;
        const std::string resultFilename = "../results_" + machine + "neural-network-runtimes-case-1_batch.csv";
        std::ofstream file(resultFilename.c_str());

        std::vector<int> img_size {512, 512};
        int patch_level = 0;
        int nb_gpus = 1;
        int max_batch_size = 16;

        // Write header
        file << "Engine;Device Type;Iteration;Patch generator AVG;Patch generator STD;NN input AVG;NN input STD;NN inference AVG;NN inference STD;NN output AVG;NN output STD;Patch stitcher AVG;Patch stitcher STD;Total\n";

        //for(auto &engine : InferenceEngineManager::getEngineList()) {TensorFlowCUDA
        for(std::string engine : {"TensorFlowCUDA", "OpenVINO", "TensorFlowCPU"}) {
            std::map<std::string, InferenceDeviceType> deviceTypes = {{"ANY", InferenceDeviceType::ANY}};
            if(engine == "OpenVINO") {
                // On OpenVINO, try all device types
                deviceTypes = std::map<std::string, InferenceDeviceType>{
                        {"CPU", InferenceDeviceType::CPU},
                        //{"GPU", InferenceDeviceType::GPU},
                        //{"VPU", InferenceDeviceType::VPU},
                };
            }
            for(auto &&deviceType : deviceTypes) {
                std::cout << engine << " for device type " << deviceType.first << std::endl;
                std::cout << "====================================" << std::endl;

                for(int currDevice = 0; currDevice <= nb_gpus; ++currDevice) {  // for each GPU
                    //if (!((engine == "TensorRT") || (engine == "TensorFlowCUDA")) && (currDevice > 0))
                    if (!((engine == "TensorRT")) && (currDevice > 0))
                        break;

                    std::cout << "Current device (GPU): " << currDevice << std::endl;
                    std::cout << "//////////////////////////////" << std::endl;

                    for (int iteration = 0; iteration <= iterations; ++iteration) {
                        auto importer = WholeSlideImageImporter::New();
                        importer->setFilename(Config::getTestDataPath() + "/WSI/A05.svs");

                        auto tissueSegmentation = TissueSegmentation::New();
                        tissueSegmentation->setInputConnection(importer->getOutputPort());

                        auto generator = PatchGenerator::New();
                        generator->setPatchSize(img_size[0], img_size[1]);
                        generator->setPatchLevel(patch_level);
                        generator->setInputConnection(importer->getOutputPort());
                        generator->setInputConnection(1, tissueSegmentation->getOutputPort());
                        generator->enableRuntimeMeasurements();

                        auto batchGenerator = ImageToBatchGenerator::New();
                        batchGenerator->setMaxBatchSize(max_batch_size);
                        batchGenerator->setInputConnection(generator->getOutputPort());

                        auto network = NeuralNetwork::New();
                        network->setInferenceEngine(engine);
                        if (engine != "TensorRT") {
                            //if (!((engine == "TensorRT") || (engine == "TensorFlowCUDA"))) {
                            network->getInferenceEngine()->setDevice(1);
                        } else {
                            network->getInferenceEngine()->setDevice(currDevice);
                        }
                        std::string postfix;
                        if (engine.substr(0, 10) == "TensorFlow") {
                            network->setInputNode(0, "input_1", NodeType::IMAGE,
                                                  TensorShape({1, img_size[0], img_size[1], 3}));
                            network->setOutputNode(0, "dense_1/Softmax", NodeType::TENSOR);
                        } else if (engine == "TensorRT") {
                            network->setInputNode(0, "input_1", NodeType::IMAGE,
                                                  TensorShape{-1, 3, img_size[0], img_size[1]});
                            network->setOutputNode(0, "dense_1/Softmax", NodeType::TENSOR, TensorShape{-1, 3});
                        } else if (engine == "OpenVINO") {
                            network->getInferenceEngine()->setDeviceType(deviceType.second);
                            if (deviceType.first == "VPU") {
                                postfix = "_fp16";
                            }
                        }
                        //network->load(Config::getTestDataPath() + "NeuralNetworkModels/wsi_classification" + postfix + "." + network->getInferenceEngine()->getDefaultFileExtension());
                        network->load("/home/andrep/FastPathology/data/Models/mobilenet_v2_bach_model" + postfix + "." +
                                      network->getInferenceEngine()->getDefaultFileExtension());
                        network->setInputConnection(batchGenerator->getOutputPort());
                        network->setScaleFactor(1.0f / 255.0f);
                        network->enableRuntimeMeasurements();

                        auto stitcher = PatchStitcher::New();
                        stitcher->setInputConnection(network->getOutputPort());
                        stitcher->enableRuntimeMeasurements();

                        auto start = std::chrono::high_resolution_clock::now();
                        DataObject::pointer data;
                        do {
                            data = stitcher->updateAndGetOutputData<DataObject>();
                        } while (!data->isLastFrame());

                        std::chrono::duration<float, std::milli> timeUsed =
                                std::chrono::high_resolution_clock::now() - start;
                        std::cout << "Total runtime: " << timeUsed.count() << std::endl;
                        std::cout << "Patch generator runtime: " << std::endl;
                        generator->getRuntime("create patch")->print();
                        std::cout << "NN runtime: " << std::endl;
                        network->getRuntime()->print();
                        std::cout << "Patch stitcher runtime: " << std::endl;
                        stitcher->getRuntime()->print();

                        if (iteration == 0 && warmupIteration)
                            continue;

                        auto deviceUsed = deviceType.first;
                        if ((engine == "TensorRT") || (engine == "TensorFlowCUDA")) {
                            deviceUsed = deviceType.first + std::to_string(currDevice);
                        } else {
                            deviceUsed = deviceType.first;
                        }

                        //network->stopPipeline();  // free memory when finished

                        file <<
                             engine + ";" +
                             deviceUsed + ";" +  //deviceType.first + ";" +
                             std::to_string(iteration) + ";" +
                             std::to_string(generator->getRuntime("create patch")->getAverage()) + ";" +
                             std::to_string(generator->getRuntime("create patch")->getStdDeviation()) + ";" +
                             std::to_string(network->getRuntime("input_processing")->getAverage()) + ";" +
                             std::to_string(network->getRuntime("input_processing")->getStdDeviation()) + ";" +
                             std::to_string(network->getRuntime("inference")->getAverage()) + ";" +
                             std::to_string(network->getRuntime("inference")->getStdDeviation()) + ";" +
                             std::to_string(network->getRuntime("output_processing")->getAverage()) + ";" +
                             std::to_string(network->getRuntime("output_processing")->getStdDeviation()) + ";" +
                             std::to_string(stitcher->getRuntime("stitch patch")->getAverage()) + ";" +
                             std::to_string(stitcher->getRuntime("stitch patch")->getStdDeviation()) + ";" +
                             std::to_string(timeUsed.count())
                             << std::endl;
                    }
                }
            }
        }
    }

    if(case1_inceptionv3) {
        std::cout << "\nPatch-wise classification with InceptionV3...\n" << std::endl;
        /*
        std::string resultFilename = "../results_" + machine + "/neural-network-runtimes-case-1_inceptionv3.csv";
        if (machine == "windows") {
            resultFilename = "../" + resultFilename;
        }
         */
        const std::string resultFilename = "C:/Users/andrep/workspace/FAST-Pathology/inference_study/results_windows/neural-network-runtimes-case-1_inceptionv3.csv";
        std::ofstream file(resultFilename.c_str());

        std::cout << "\nFile: " << resultFilename << std::endl;

        std::vector<int> img_size {512, 512};
        int patch_level = 0;
        int nb_gpus = 1;
        int max_batch_size = 16;

        // Write header
        file << "Engine;Device Type;Iteration;Patch generator AVG;Patch generator STD;NN input AVG;NN input STD;NN inference AVG;NN inference STD;NN output AVG;NN output STD;Patch stitcher AVG;Patch stitcher STD;Total\n";

        //for(auto &engine : InferenceEngineManager::getEngineList()) {TensorFlowCUDA
        auto engines = {"TensorFlowCUDA", "OpenVINO", "TensorFlowCPU"};
        if (machine == "windows") {
            engines = {"OpenVINO", "TensorFlowCPU"};
        }
        for(std::string engine : engines) {
            std::map<std::string, InferenceDeviceType> deviceTypes = {{"ANY", InferenceDeviceType::ANY}};
            if(engine == "OpenVINO") {
                // On OpenVINO, try all device types
                deviceTypes = std::map<std::string, InferenceDeviceType>{
                        {"CPU", InferenceDeviceType::CPU},
                        //{"GPU", InferenceDeviceType::GPU},
                        //{"VPU", InferenceDeviceType::VPU},
                };
                if (machine == "windows") {
                    deviceTypes = std::map<std::string, InferenceDeviceType>{
                        {"CPU", InferenceDeviceType::CPU},
                        {"GPU", InferenceDeviceType::GPU},
                        //{"VPU", InferenceDeviceType::VPU},
                };
                }
            }
            for(auto &&deviceType : deviceTypes) {
                std::cout << engine << " for device type " << deviceType.first << std::endl;
                std::cout << "====================================" << std::endl;

                for(int currDevice = 0; currDevice <= nb_gpus; ++currDevice) {  // for each GPU
                    //if (!((engine == "TensorRT") || (engine == "TensorFlowCUDA")) && (currDevice > 0))
                    if (!((engine == "TensorRT")) && (currDevice > 0))
                        break;

                    std::cout << "Current device (GPU): " << currDevice << std::endl;
                    std::cout << "//////////////////////////////" << std::endl;

                    for (int iteration = 0; iteration <= iterations; ++iteration) {
                        auto importer = WholeSlideImageImporter::New();
                        //importer->setFilename(Config::getTestDataPath() + "/WSI/A05.svs");
                        if (machine == "windows") {
                            importer->setFilename("C:/Users/andrep/workspace/FAST-Pathology_old/A05.svs");
                        } else {
                            importer->setFilename(Config::getTestDataPath() + "/WSI/A05.svs");
                        }

                        auto tissueSegmentation = TissueSegmentation::New();
                        tissueSegmentation->setInputConnection(importer->getOutputPort());

                        auto generator = PatchGenerator::New();
                        generator->setPatchSize(img_size[0], img_size[1]);
                        generator->setPatchLevel(patch_level);
                        generator->setInputConnection(importer->getOutputPort());
                        generator->setInputConnection(1, tissueSegmentation->getOutputPort());
                        generator->enableRuntimeMeasurements();

                        /*
                        auto batchGenerator = ImageToBatchGenerator::New();
                        batchGenerator->setMaxBatchSize(max_batch_size);
                        batchGenerator->setInputConnection(generator->getOutputPort());
                         */

                        auto network = NeuralNetwork::New();
                        network->setInferenceEngine(engine);
                        if (engine != "TensorRT") {
                            //if (!((engine == "TensorRT") || (engine == "TensorFlowCUDA"))) {
                            network->getInferenceEngine()->setDevice(1);
                        } else {
                            network->getInferenceEngine()->setDevice(currDevice);
                        }
                        std::string postfix;
                        if (engine.substr(0, 10) == "TensorFlow") {
                            network->setInputNode(0, "input_1", NodeType::IMAGE,
                                                  TensorShape({1, img_size[0], img_size[1], 3}));
                            network->setOutputNode(0, "sequential/dense_1/Softmax", NodeType::TENSOR);
                        } else if (engine == "TensorRT") {
                            network->setInputNode(0, "input_1", NodeType::IMAGE,
                                                  TensorShape{-1, 3, img_size[0], img_size[1]});
                            network->setOutputNode(0, "sequential/dense_1/Softmax", NodeType::TENSOR, TensorShape{-1, 3});
                        } else if (engine == "OpenVINO") {
                            network->getInferenceEngine()->setDeviceType(deviceType.second);
                            if (deviceType.first == "VPU") {
                                postfix = "_fp16";
                            }
                        }
                        /*
                        if (machine == "ubuntu") {
                            network->load(Config::getTestDataPath() + "NeuralNetworkModels/wsi_classification" + postfix + "." + network->getInferenceEngine()->getDefaultFileExtension());
                        } else if (machine == "windows") {
                            network->load("C:/Users/andrep/FastPathology/data/Models/mobilenet_v2_bach_model" + postfix + "." +
                                      network->getInferenceEngine()->getDefaultFileExtension());
                        }
                         */
                        network->load("C:/Users/andrep/Downloads/FAST_Test_Data/data/NeuralNetworkModels/wsi_classification" + postfix + "." + network->getInferenceEngine()->getDefaultFileExtension());
                        //network->load(Config::getTestDataPath() + "NeuralNetworkModels/wsi_classification" + postfix + "." + network->getInferenceEngine()->getDefaultFileExtension());
                        //network->load("/home/andrep/FastPathology/data/Models/mobilenet_v2_bach_model" + postfix + "." + network->getInferenceEngine()->getDefaultFileExtension());
                        network->setInputConnection(generator->getOutputPort());
                        network->setScaleFactor(1.0f / 255.0f);
                        network->enableRuntimeMeasurements();

                        auto stitcher = PatchStitcher::New();
                        stitcher->setInputConnection(network->getOutputPort());
                        stitcher->enableRuntimeMeasurements();

                        auto start = std::chrono::high_resolution_clock::now();
                        DataObject::pointer data;
                        do {
                            data = stitcher->updateAndGetOutputData<DataObject>();
                        } while (!data->isLastFrame());

                        std::chrono::duration<float, std::milli> timeUsed =
                                std::chrono::high_resolution_clock::now() - start;
                        std::cout << "Total runtime: " << timeUsed.count() << std::endl;
                        std::cout << "Patch generator runtime: " << std::endl;
                        generator->getRuntime("create patch")->print();
                        std::cout << "NN runtime: " << std::endl;
                        network->getRuntime()->print();
                        std::cout << "Patch stitcher runtime: " << std::endl;
                        stitcher->getRuntime()->print();

                        if (iteration == 0 && warmupIteration)
                            continue;

                        auto deviceUsed = deviceType.first;
                        if ((engine == "TensorRT") || (engine == "TensorFlowCUDA")) {
                            deviceUsed = deviceType.first + std::to_string(currDevice);
                        } else {
                            deviceUsed = deviceType.first;
                        }

                        //network->stopPipeline();  // free memory when finished

                        file <<
                             engine + ";" +
                             deviceUsed + ";" +  //deviceType.first + ";" +
                             std::to_string(iteration) + ";" +
                             std::to_string(generator->getRuntime("create patch")->getAverage()) + ";" +
                             std::to_string(generator->getRuntime("create patch")->getStdDeviation()) + ";" +
                             std::to_string(network->getRuntime("input_processing")->getAverage()) + ";" +
                             std::to_string(network->getRuntime("input_processing")->getStdDeviation()) + ";" +
                             std::to_string(network->getRuntime("inference")->getAverage()) + ";" +
                             std::to_string(network->getRuntime("inference")->getStdDeviation()) + ";" +
                             std::to_string(network->getRuntime("output_processing")->getAverage()) + ";" +
                             std::to_string(network->getRuntime("output_processing")->getStdDeviation()) + ";" +
                             std::to_string(stitcher->getRuntime("stitch patch")->getAverage()) + ";" +
                             std::to_string(stitcher->getRuntime("stitch patch")->getStdDeviation()) + ";" +
                             std::to_string(timeUsed.count())
                             << std::endl;
                    }
                }
            }
        }
    }

    if(case3a) {
        std::cout << "\nCASE 3a - Inference on different magnification levels with rendering with high-res U-Net model...\n" << std::endl;
        //std::cout << "\nCASE 3a - Inference on different magnification levels with rendering with tiny-YOLOv3 model...\n" << std::endl;
        // CASE 3a - Inference on different magnification levels (40x, 20x, 10x, 5x, 2.5x) with tiny-YOLOv3 model
        const std::string resultFilename = "../results_" + machine + "neural-network-runtimes-case-3a.csv";
        std::ofstream file(resultFilename.c_str());

        //std::vector<int> img_size {256, 256};
        std::vector<int> img_sizes {1024, 512, 256};
        //int patch_level = 0;
        std::vector<float> patch_levels {40, 20, 10, 5, 2.5};  //, 20, 10, 5, 2.5};
        std::reverse(patch_levels.begin(), patch_levels.end());
        float wsi_res = 40;
        int nb_classes = 1;
        int nb_anchors = 3;

        // Write header
        file << "Engine;Device Type;Patch level;Iteration;Patch generator AVG;Patch generator STD;NN input AVG;NN input STD;NN inference AVG;NN inference STD;NN output AVG;NN output STD;Patch stitcher AVG;Patch stitcher STD;Total\n";

        //for(auto &engine : InferenceEngineManager::getEngineList()) {
        //for(std::string engine : {"TensorFlowCUDA", "TensorRT", "OpenVINO", "TensorFlowCPU"}) {
        for(std::string engine : {"TensorFlowCUDA", "OpenVINO"}) {
            std::map<std::string, InferenceDeviceType> deviceTypes = {{"ANY", InferenceDeviceType::ANY}};
            if(engine == "OpenVINO") {
                // On OpenVINO, try all device types
                deviceTypes = std::map<std::string, InferenceDeviceType>{
                        {"CPU", InferenceDeviceType::CPU},
                        //{"GPU", InferenceDeviceType::GPU},
                        //{"VPU", InferenceDeviceType::VPU},
                };
            }
            for(auto &&deviceType : deviceTypes) {
                std::cout << engine << " for device type " << deviceType.first << std::endl;
                std::cout << "====================================" << std::endl;

                for(auto const& patch_level : patch_levels) {
                    std::cout << "current patch level: " << std::to_string(patch_level) << std::endl;
                    std::cout << "------------------------------------" << std::endl;

                    int curr_patch_level = (int) (log2(wsi_res / patch_level) / log2(2));

                    for(auto const& img_size : img_sizes) {
                        std::cout << "current img size: " << std::to_string(img_size) << " x " << std::to_string(img_size) << std::endl;
                        std::cout << "------------------------------------" << std::endl;

                        for(int iteration = 0; iteration <= iterations; ++iteration) {
                            auto importer = WholeSlideImageImporter::New();
                            importer->setFilename("/home/andrep/workspace/FAST-Pathology/fastPathology_stuff/images/camelyon16_tumor_047-043.tif");

                            /*
                            auto image = importer->updateAndGetOutputData<ImagePyramid>();
                            for(auto const& [key, val] : image->getMetadata()) {
                                std::cout << "\nInfo: " << key << ": " << val << std::endl;
                            }
                            std::cout << "\nFinished!" << std::endl;
                             */

                            auto tissueSegmentation = TissueSegmentation::New();
                            tissueSegmentation->setInputConnection(importer->getOutputPort());

                            auto generator = PatchGenerator::New();
                            generator->setPatchSize(img_size, img_size);
                            std::cout << "\nCurrent p-level: " << curr_patch_level << std::endl;
                            generator->setPatchLevel(curr_patch_level);
                            generator->setInputConnection(importer->getOutputPort());
                            generator->setInputConnection(1, tissueSegmentation->getOutputPort());
                            generator->enableRuntimeMeasurements();

                            auto network = SegmentationNetwork::New();
                            network->setInferenceEngine(engine);
                            network->getInferenceEngine()->setDevice(1);
                            std::string postfix;
                            if (engine.substr(0, 10) == "TensorFlow") {
                                network->setOutputNode(0, "conv2d_26/truediv", NodeType::TENSOR);
                            } else if (engine == "TensorRT") {
                                network->setInputNode(0, "input_1", NodeType::IMAGE,
                                                      TensorShape{-1, 3, img_size, img_size});
                                network->setOutputNode(0, "conv2d_26/truediv", NodeType::TENSOR, TensorShape{-1, 3});
                            } else if (engine == "OpenVINO") {
                                network->getInferenceEngine()->setDeviceType(deviceType.second);
                                if (deviceType.first == "VPU") {
                                    postfix = "_fp16";
                                }
                            }
                            //network->load(Config::getTestDataPath() + "NeuralNetworkModels/wsi_classification" + postfix + "." + network->getInferenceEngine()->getDefaultFileExtension());
                            network->load("/home/andrep/FastPathology/data/Models/high_res_nuclei_unet" + postfix + "." + network->getInferenceEngine()->getDefaultFileExtension());
                            network->setInputConnection(generator->getOutputPort());
                            network->setScaleFactor(1.0f / 255.0f);
                            network->enableRuntimeMeasurements();

                            auto stitcher = PatchStitcher::New();
                            stitcher->setInputConnection(network->getOutputPort());
                            stitcher->enableRuntimeMeasurements();

                            auto start = std::chrono::high_resolution_clock::now();
                            DataObject::pointer data;
                            do {
                                data = stitcher->updateAndGetOutputData<DataObject>();
                            } while (!data->isLastFrame());
                            std::chrono::duration<float, std::milli> timeUsed =
                                    std::chrono::high_resolution_clock::now() - start;
                            std::cout << "Total runtime: " << timeUsed.count() << std::endl;
                            std::cout << "Patch generator runtime: " << std::endl;
                            generator->getRuntime("create patch")->print();
                            std::cout << "NN runtime: " << std::endl;
                            network->getRuntime()->print();
                            std::cout << "Patch stitcher runtime: " << std::endl;
                            stitcher->getRuntime()->print();

                            if (iteration == 0 && warmupIteration)
                                continue;

                            file <<
                                 engine + ";" +
                                 deviceType.first + ";" +
                                 std::to_string(curr_patch_level) + ";" +
                                 std::to_string(img_size) + ";" +
                                 std::to_string(iteration) + ";" +
                                 std::to_string(generator->getRuntime("create patch")->getAverage()) + ";" +
                                 std::to_string(generator->getRuntime("create patch")->getStdDeviation()) + ";" +
                                 std::to_string(network->getRuntime("input_processing")->getAverage()) + ";" +
                                 std::to_string(network->getRuntime("input_processing")->getStdDeviation()) + ";" +
                                 std::to_string(network->getRuntime("inference")->getAverage()) + ";" +
                                 std::to_string(network->getRuntime("inference")->getStdDeviation()) + ";" +
                                 std::to_string(network->getRuntime("output_processing")->getAverage()) + ";" +
                                 std::to_string(network->getRuntime("output_processing")->getStdDeviation()) + ";" +
                                 std::to_string(stitcher->getRuntime("stitch patch")->getAverage()) + ";" +
                                 std::to_string(stitcher->getRuntime("stitch patch")->getStdDeviation()) + ";" +
                                 std::to_string(timeUsed.count())
                                 << std::endl;
                    }
                    }
                }
            }
        }
    }


    if(case4a) {
        std::cout << "\nCASE 4a - Inference on different magnification levels with rendering with high-res U-Net model...\n" << std::endl;
        // CASE 4a - Inference on different magnification levels (40x, 20x, 10x, 5x, 2.5x) with high-res U-Net for nuclei seg.
        const std::string resultFilename = "../results_" + machine + "neural-network-runtimes-case-4a.csv";
        std::ofstream file(resultFilename.c_str());

        std::vector<int> img_size {256, 256};
        //int patch_level = 0;
        std::vector<float> patch_levels {40, 20, 10, 5, 2.5};
        float wsi_res = 40;
        int nb_classes = 1;
        int nb_anchors = 3;

        // Write header
        file << "Engine;Device Type;Patch level;Iteration;Patch generator AVG;Patch generator STD;NN input AVG;NN input STD;NN inference AVG;NN inference STD;NN output AVG;NN output STD;Patch stitcher AVG;Patch stitcher STD;Total\n";

        //for(auto &engine : InferenceEngineManager::getEngineList()) {
        //for(std::string engine : {"TensorFlowCUDA", "TensorRT", "OpenVINO", "TensorFlowCPU"}) {
        for(std::string engine : {"TensorFlowCUDA", "OpenVINO"}) {
            std::map<std::string, InferenceDeviceType> deviceTypes = {{"ANY", InferenceDeviceType::ANY}};
            if(engine == "OpenVINO") {
                // On OpenVINO, try all device types
                deviceTypes = std::map<std::string, InferenceDeviceType>{
                        {"CPU", InferenceDeviceType::CPU},
                        {"GPU", InferenceDeviceType::GPU},
                        //{"VPU", InferenceDeviceType::VPU},
                };
            }
            for(auto &&deviceType : deviceTypes) {
                std::cout << engine << " for device type " << deviceType.first << std::endl;
                std::cout << "====================================" << std::endl;

                for(auto const& patch_level : patch_levels) {
                    std::cout << "current patch level: " << std::to_string(patch_level) << std::endl;
                    std::cout << "------------------------------------" << std::endl;
                    int curr_patch_level = (int) (log2(wsi_res / patch_level) / log2(2));

                    for(int iteration = 0; iteration <= iterations; ++iteration) {
                        auto importer = WholeSlideImageImporter::New();
                        importer->setFilename("/home/andrep/workspace/FAST-Pathology/fastPathology_stuff/images/camelyon16_tumor_047-043.tif");

                        auto image = importer->updateAndGetOutputData<ImagePyramid>();
                        for(auto const& [key, val] : image->getMetadata()) {
                            std::cout << "\nInfo: " << key << ": " << val << std::endl;
                        }
                        std::cout << "\nFinished!" << std::endl;

                        auto tissueSegmentation = TissueSegmentation::New();
                        tissueSegmentation->setInputConnection(importer->getOutputPort());

                        auto generator = PatchGenerator::New();
                        generator->setPatchSize(img_size[0], img_size[1]);
                        generator->setPatchLevel(curr_patch_level);
                        generator->setInputConnection(importer->getOutputPort());
                        generator->setInputConnection(1, tissueSegmentation->getOutputPort());
                        generator->enableRuntimeMeasurements();

                        auto network = BoundingBoxNetwork::New();
                        network->setThreshold(0.1); //0.01); // default: 0.5
                        network->setInferenceEngine(engine);
                        network->getInferenceEngine()->setDevice(1);
                        std::string postfix;
                        if (engine.substr(0, 10) == "TensorFlow") {
                            network->setInputNode(0, "input_1", NodeType::IMAGE,
                                                  TensorShape{-1, img_size[0], img_size[1], 3});
                            network->setOutputNode(0, "conv2d_10/BiasAdd", NodeType::TENSOR,
                                                   TensorShape{-1, 8, 8, (int) (nb_anchors * (nb_classes + 5))});
                        } else if (engine == "TensorRT") {
                            network->setInputNode(0, "input_1", NodeType::IMAGE,
                                                  TensorShape{-1, 3, img_size[0], img_size[1]});
                            network->setOutputNode(0, "conv2d_10/BiasAdd", NodeType::TENSOR, TensorShape{-1, 8, 8, 18});
                        } else if (engine == "OpenVINO") {
                            network->getInferenceEngine()->setDeviceType(deviceType.second);
                            if (deviceType.first == "VPU") {
                                postfix = "_fp16";
                            }
                        }

                        // read anchors from corresponding anchor file
                        std::vector<std::vector<Vector2f> > anchors;
                        std::ifstream infile(
                                "/home/andrep/FastPathology/data/Models/yolo_test_model_fixed_output_nodes.anchors");
                        std::string anchorStr;
                        while (std::getline(infile, anchorStr)) {
                            std::vector<std::string> anchorVector = split(anchorStr, " ");
                            anchorVector.resize(
                                    6); // for TinyYOLOv3 should only be 6 pairs, 3 for each level (2 levels)
                            int cntr = 0;
                            for (int i = 1; i < 3; i++) { // assumes TinyYOLOv3 (only two output layers)
                                std::vector<Vector2f> levelAnchors;
                                for (int j = 0; j < 3; j++) {
                                    auto currentPair = split(anchorVector[cntr], ",");
                                    levelAnchors.push_back(
                                            Vector2f(std::stoi(currentPair[0]), std::stoi(currentPair[1])));
                                    cntr++;
                                }
                                anchors.push_back(levelAnchors);
                            }
                        }
                        network->setAnchors(anchors); // finally set anchors

                        //network->load(Config::getTestDataPath() + "NeuralNetworkModels/wsi_classification" + postfix + "." + network->getInferenceEngine()->getDefaultFileExtension());
                        network->load(
                                "/home/andrep/FastPathology/data/Models/yolo_test_model_fixed_output_nodes" + postfix +
                                "." + network->getInferenceEngine()->getDefaultFileExtension());
                        network->setInputConnection(generator->getOutputPort());
                        network->setScaleFactor(1.0f / 255.0f);
                        network->enableRuntimeMeasurements();

                        auto boxAccum = BoundingBoxSetAccumulator::New();
                        //boxAccum->setInputConnection(nms->getOutputPort());
                        boxAccum->setInputConnection(network->getOutputPort());
                        boxAccum->enableRuntimeMeasurements();

                        // FIXME: add update of this
                        // MERGE <- for waiting for threads/syncs

                        auto start = std::chrono::high_resolution_clock::now();
                        DataObject::pointer data;
                        do {
                            data = boxAccum->updateAndGetOutputData<DataObject>();
                        } while (!data->isLastFrame());
                        std::chrono::duration<float, std::milli> timeUsed =
                                std::chrono::high_resolution_clock::now() - start;
                        std::cout << "Total runtime: " << timeUsed.count() << std::endl;
                        std::cout << "Patch generator runtime: " << std::endl;
                        generator->getRuntime("create patch")->print();
                        std::cout << "NN runtime: " << std::endl;
                        network->getRuntime()->print();
                        std::cout << "Patch stitcher runtime: " << std::endl;
                        boxAccum->getRuntime()->print();

                        if (iteration == 0 && warmupIteration)
                            continue;

                        file <<
                             engine + ";" +
                             deviceType.first + ";" +
                             std::to_string(curr_patch_level) + ";" +
                             std::to_string(iteration) + ";" +
                             std::to_string(generator->getRuntime("create patch")->getAverage()) + ";" +
                             std::to_string(generator->getRuntime("create patch")->getStdDeviation()) + ";" +
                             std::to_string(network->getRuntime("input_processing")->getAverage()) + ";" +
                             std::to_string(network->getRuntime("input_processing")->getStdDeviation()) + ";" +
                             std::to_string(network->getRuntime("inference")->getAverage()) + ";" +
                             std::to_string(network->getRuntime("inference")->getStdDeviation()) + ";" +
                             std::to_string(network->getRuntime("output_processing")->getAverage()) + ";" +
                             std::to_string(network->getRuntime("output_processing")->getStdDeviation()) + ";" +
                             std::to_string(boxAccum->getRuntime("stitch patch")->getAverage()) + ";" +
                             std::to_string(boxAccum->getRuntime("stitch patch")->getStdDeviation()) + ";" +
                             std::to_string(timeUsed.count())
                             << std::endl;
                    }
                }
            }
        }
    }



    if(false) {  // (case3_batch) {
        // CASE 3 - WSI CLASSIFICATION with batches
        const int batchSize = 16;
        const std::string resultFilename = "neural-network-runtimes-case-3-batch.csv";
        std::ofstream file(resultFilename.c_str());

        // Write header
        file << "Engine;Device Type;Iteration;Patch generator AVG;Patch generator STD;NN input AVG;NN input STD;NN inference AVG;NN inference STD;NN output AVG;NN output STD;Patch stitcher AVG;Patch stitcher STD;Total\n";

        //for(std::string engine : {"TensorRT", "TensorFlowCUDA", "TensorFlowROCm", "OpenVINO"}) {
        for(std::string engine : {"TensorFlowCUDA", "OpenVINO", "TensorFlowCPU"}) {
            if(!InferenceEngineManager::isEngineAvailable(engine))
                continue;
            std::map<std::string, InferenceDeviceType> deviceTypes = {{"ANY", InferenceDeviceType::ANY}};
            if(engine == "OpenVINO") {
                // On OpenVINO, try all device types
                deviceTypes = std::map<std::string, InferenceDeviceType>{
                        //{"CPU", InferenceDeviceType::CPU},
                        {"GPU", InferenceDeviceType::GPU},
                        {"VPU", InferenceDeviceType::VPU},
                };
            }
            for(auto &&deviceType : deviceTypes) {
                std::cout << engine << " for device type " << deviceType.first << std::endl;
                std::cout << "====================================" << std::endl;

                for(int iteration = 0; iteration <= iterations; ++iteration) {
                    auto importer = WholeSlideImageImporter::New();
                    importer->setFilename(Config::getTestDataPath() + "/WSI/A05.svs");

                    auto tissueSegmentation = TissueSegmentation::New();
                    tissueSegmentation->setInputConnection(importer->getOutputPort());

                    auto generator = PatchGenerator::New();
                    generator->setPatchSize(512, 512);
                    generator->setPatchLevel(0);
                    generator->setInputConnection(importer->getOutputPort());
                    generator->setInputConnection(1, tissueSegmentation->getOutputPort());
                    generator->enableRuntimeMeasurements();

                    auto batchGenerator = ImageToBatchGenerator::New();
                    batchGenerator->setMaxBatchSize(16);

                    auto network = NeuralNetwork::New();
                    network->setInferenceEngine(engine);
                    network->getInferenceEngine()->setMaxBatchSize(batchSize);
                    std::string postfix;
                    if(engine.substr(0, 10) == "TensorFlow") {
                        network->setOutputNode(0, "sequential/dense_1/Softmax", NodeType::TENSOR);
                    } else if(engine == "TensorRT") {
                        network->setInputNode(0, "input_1", NodeType::IMAGE, TensorShape{-1, 3, 512, 512});
                        network->setOutputNode(0, "sequential/dense_1/Softmax", NodeType::TENSOR, TensorShape{-1, 3});
                    } else if(engine == "OpenVINO") {
                        network->getInferenceEngine()->setDeviceType(deviceType.second);
                        postfix = "batch_" + std::to_string(batchSize);
                        if(deviceType.first == "VPU") {
                            postfix = "_fp16";
                        }
                    }
                    network->load(Config::getTestDataPath() + "NeuralNetworkModels/wsi_classification" + postfix + "." +
                                  network->getInferenceEngine()->getDefaultFileExtension());
                    network->setInputConnection(generator->getOutputPort());
                    network->setScaleFactor(1.0f / 255.0f);
                    network->enableRuntimeMeasurements();

                    auto stitcher = PatchStitcher::New();
                    stitcher->setInputConnection(network->getOutputPort());
                    stitcher->enableRuntimeMeasurements();

                    auto start = std::chrono::high_resolution_clock::now();
                    DataObject::pointer data;
                    do {
                        data = stitcher->updateAndGetOutputData<DataObject>();
                    } while(!data->isLastFrame());
                    std::chrono::duration<float, std::milli> timeUsed = std::chrono::high_resolution_clock::now() - start;
                    std::cout << "Total runtime: " << timeUsed.count() << std::endl;
                    std::cout << "Patch generator runtime: " << std::endl;
                    generator->getRuntime("create patch")->print();
                    std::cout << "NN runtime: " << std::endl;
                    network->getRuntime()->print();
                    std::cout << "Patch stitcher runtime: " << std::endl;
                    stitcher->getRuntime()->print();

                    if(iteration == 0 && warmupIteration)
                        continue;

                    file <<
                         engine + ";" +
                         deviceType.first + ";" +
                         std::to_string(iteration) + ";" +
                         std::to_string(generator->getRuntime("create patch")->getAverage()) + ";" +
                         std::to_string(generator->getRuntime("create patch")->getStdDeviation()) + ";" +
                         std::to_string(network->getRuntime("input_processing")->getAverage()) + ";" +
                         std::to_string(network->getRuntime("input_processing")->getStdDeviation()) + ";" +
                         std::to_string(network->getRuntime("inference")->getAverage()) + ";" +
                         std::to_string(network->getRuntime("inference")->getStdDeviation()) + ";" +
                         std::to_string(network->getRuntime("output_processing")->getAverage()) + ";" +
                         std::to_string(network->getRuntime("output_processing")->getStdDeviation()) + ";" +
                         std::to_string(stitcher->getRuntime("stitch patch")->getAverage()) + ";" +
                         std::to_string(stitcher->getRuntime("stitch patch")->getStdDeviation()) + ";" +
                         std::to_string(timeUsed.count())
                         << std::endl;
                }
            }
        }
    }

}
