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


using namespace fast;

int main(int argc, char** argv) {
    Reporter::setGlobalReportMethod(Reporter::INFO, Reporter::NONE);

    CommandLineParser parser("Measure neural network performance script");
    parser.addOption("disable-case-1");
    parser.addOption("disable-case-2");
    parser.addOption("disable-case-3");
    parser.addOption("disable-case-4");
    //parser.addOption("disable-case-3-batch");
    parser.addOption("disable-warmup");
    parser.parse(argc, argv);
    const int iterations = 10;  //10;
    const bool warmupIteration = !parser.getOption("disable-warmup");
    const bool case1 = !parser.getOption("disable-case-1");
    const bool case2 = !parser.getOption("disable-case-2");
    const bool case3 = !parser.getOption("disable-case-3");
    const bool case4 = !parser.getOption("disable-case-4");
    //const bool case3_batch = !parser.getOption("disable-case-3-batch");


    if(case1) {
        // CASE 1 - Patch-wise classification using BACH-model on 20x WSI
        const std::string resultFilename = "neural-network-runtimes-case-1-pw.csv";
        std::ofstream file(resultFilename.c_str());

        // Write header
        file << "Engine;Device Type;Iteration;Patch generator AVG;Patch generator STD;NN input AVG;NN input STD;NN inference AVG;NN inference STD;NN output AVG;NN output STD;Patch stitcher AVG;Patch stitcher STD;Total\n";

        for(auto &engine : InferenceEngineManager::getEngineList()) {
        //for(std::string engine : {"TensorFlowCUDA", "OpenVINO", "TensorFlowCPU"}) {
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
                    generator->setPatchSize(512, 512);
                    generator->setPatchLevel(0);
                    generator->setInputConnection(importer->getOutputPort());
                    generator->setInputConnection(1, tissueSegmentation->getOutputPort());
                    generator->enableRuntimeMeasurements();

                    auto network = NeuralNetwork::New();
                    network->setInferenceEngine(engine);
                    std::string postfix;
                    if(engine.substr(0, 10) == "TensorFlow") {
                        network->setOutputNode(0, "sequential/dense_1/Softmax", NodeType::TENSOR);
                    } else if(engine == "TensorRT") {
                        network->setInputNode(0, "input_1", NodeType::IMAGE, TensorShape{-1, 3, 512, 512});
                        network->setOutputNode(0, "sequential/dense_1/Softmax", NodeType::TENSOR, TensorShape{-1, 3});
                    } else if(engine == "OpenVINO") {
                        network->getInferenceEngine()->setDeviceType(deviceType.second);
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


    if(case3) {
        // CASE 3 - Patch-wise high-res semantic segmentation on 20x WSI (of nuceli)
        const std::string resultFilename = "neural-network-runtimes-case-3-hrss.csv";
        std::ofstream file(resultFilename.c_str());

        // Write header
        file << "Engine;Device Type;Iteration;Patch generator AVG;Patch generator STD;NN input AVG;NN input STD;NN inference AVG;NN inference STD;NN output AVG;NN output STD;Patch stitcher AVG;Patch stitcher STD;Total\n";

        for(auto &engine : InferenceEngineManager::getEngineList()) {
            //for(std::string engine : {"TensorFlowCUDA", "OpenVINO", "TensorFlowCPU"}) {
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
                    generator->setPatchSize(512, 512);
                    generator->setPatchLevel(0);
                    generator->setInputConnection(importer->getOutputPort());
                    generator->setInputConnection(1, tissueSegmentation->getOutputPort());
                    generator->enableRuntimeMeasurements();

                    auto network = SegmentationNetwork::New();
                    network->setInferenceEngine(engine);
                    std::string postfix;
                    if(engine.substr(0, 10) == "TensorFlow") {
                        network->setOutputNode(0, "sequential/dense_1/Softmax", NodeType::TENSOR);
                    } else if(engine == "TensorRT") {
                        network->setInputNode(0, "input_1", NodeType::IMAGE, TensorShape{-1, 3, 512, 512});
                        network->setOutputNode(0, "sequential/dense_1/Softmax", NodeType::TENSOR, TensorShape{-1, 3});
                    } else if(engine == "OpenVINO") {
                        network->getInferenceEngine()->setDeviceType(deviceType.second);
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
