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
#include <FAST/Data/ImagePyramid.hpp>
#include <FAST/Exporters/TIFFImagePyramidExporter.hpp>
#include <FAST/Visualization/SegmentationRenderer/SegmentationRenderer.hpp>

using namespace fast;

int main(int argc, char** argv) {
    Reporter::setGlobalReportMethod(Reporter::INFO, Reporter::NONE);

    CommandLineParser parser("Measure neural network performance script");
    parser.addOption("disable-warmup");
    parser.parse(argc, argv);
    const int iterations = 10;  // 10
    const bool warmupIteration = !parser.getOption("disable-warmup");

    std::cout << "\nPatch-wise high-res semantic segmentation...\n" << std::endl;
    const std::string resultFilename = "C:/Users/andrp/workspace/FAST-Pathology/code-free-study/build/results_neural-network-runtime.csv";
    //const std::string resultFilename = "/home/andrep/workspace/FAST-Pathology/code-free-study/build/results_neural-network-runtime.csv";
    std::ofstream file(resultFilename.c_str());

    std::vector<int> img_size{256, 256};
    int patch_level = 1;

    int iter = 1;

    // Write header
    file << "Engine;Device Type;Iteration;Patch generator AVG;Patch generator STD;NN input AVG;NN input STD;NN inference AVG;NN inference STD;NN output AVG;NN output STD;Patch stitcher AVG;Patch stitcher STD;Exporter AVG; Exporter STD;Total\n";

    // initialize neural network once
    //auto network = SegmentationNetwork::New();

    for (std::string engine : {"TensorRT", "OpenVINO"}) {
        std::map<std::string, InferenceDeviceType> deviceTypes = {{"ANY", InferenceDeviceType::ANY}};
        if (engine == "OpenVINO") {
            // On OpenVINO, try all device types
            deviceTypes = std::map<std::string, InferenceDeviceType>{
                    {"CPU", InferenceDeviceType::CPU},
                    {"GPU", InferenceDeviceType::GPU},
            };
        }

        for (auto &&deviceType : deviceTypes) {
            std::cout << engine << " for device type " << deviceType.first << std::endl;
            std::cout << "====================================" << std::endl;

            for (int iteration = 0; iteration <= iterations; ++iteration) {

                auto importer = WholeSlideImageImporter::New();
                //importer->setFilename("C:/Users/andrp/workspace/Henrik-DP/file-and-model-to-henrik-210921/0018ae58b01bdadc8e347995b69f99aa.tiff");
                importer->setFilename("C:/Users/andrp/Downloads/transfer_190689_files_b2b897b6/11435_CD3.ndpi");
                //importer->setFilename("/home/andrep/Downloads/transfer_190689_files_b2b897b6/11435_CD3.ndpi");

                auto tissueSegmentation = TissueSegmentation::New();
                tissueSegmentation->setDilate(45);
                tissueSegmentation->setInputConnection(importer->getOutputPort());

                auto generator = PatchGenerator::New();
                generator->setPatchSize(img_size[0], img_size[1]);
                generator->setPatchLevel(patch_level);
                generator->setOverlap(0.0);
                generator->setMaskThreshold(0.01);
                generator->setInputConnection(importer->getOutputPort());
                generator->setInputConnection(1, tissueSegmentation->getOutputPort());
                generator->enableRuntimeMeasurements();

                auto network = SegmentationNetwork::New();
                network->setInferenceEngine(engine);
                if (engine == "OpenVINO")
                    network->getInferenceEngine()->setDeviceType(deviceType.second);
                //network->load("/home/andrep/Downloads/transfer_190689_files_b2b897b6/CD3_UNET_256_1P16B32FD6_w_Augm_290921.onnx");
                network->load("C:/Users/andrp/Downloads/transfer_190689_files_b2b897b6/CD3_UNET_256_1P16B32FD6_w_Augm_290921.onnx");
                //network->load("C:/Users/andrp/workspace/convert_test/transfer_190689_files_b2b897b6/CD3_UNET_256_1P16B32FD6_w_Augm_290921");
                //network->load("C:/Users/andrp/fastpathology/data/Models/high_res_nuclei_unet.pb");
                network->setScaleFactor(1.0f);
                network->setInputConnection(generator->getOutputPort());
                network->enableRuntimeMeasurements();

                auto stitcher = PatchStitcher::New();
                stitcher->setInputConnection(network->getOutputPort());
                stitcher->enableRuntimeMeasurements();

                auto start = std::chrono::high_resolution_clock::now();
                DataObject::pointer data;
                do {
                    data = stitcher->updateAndGetOutputData<DataObject>();
                } while (!data->isLastFrame());

                auto exporter = TIFFImagePyramidExporter::New();
                exporter->setFilename("C:/Users/andrp/workspace/FAST-Pathology/code-free-study/build/pred_seg" + std::to_string(iter) + ".tiff");
                exporter->setInputConnection(stitcher->getOutputPort());
                //exporter->setExecuteOnLastFrameOnly(false);
                exporter->enableRuntimeMeasurements();
                exporter->update();  // @TODO: Seems like I have to have this for it to actually start saving. Expected behaviour?

                /*
                auto someRenderer = SegmentationRenderer::New();
                someRenderer->setOpacity(0.4f);
                someRenderer->setInputConnection(stitcher->getOutputPort());
                someRenderer->update();
                 */

                std::chrono::duration<float, std::milli> timeUsed =
                        std::chrono::high_resolution_clock::now() - start;
                std::cout << "Total runtime: " << timeUsed.count() << std::endl;
                std::cout << "Patch generator runtime: " << std::endl;
                generator->getRuntime("create patch")->print();
                std::cout << "NN runtime: " << std::endl;
                network->getRuntime()->print();
                std::cout << "Patch stitcher runtime: " << std::endl;
                stitcher->getRuntime()->print();
                std::cout << "Exporter runtime" << std::endl;
                exporter->getRuntime()->print();

                iter++;

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
                     std::to_string(exporter->getRuntime()->getAverage()) + ";" +
                     "0" + ";" +
                     std::to_string(timeUsed.count())
                     << std::endl;
            }
        }
    }
}
