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
#include <FAST/Exporters/ImageFileExporter.hpp>
#include <FAST/Exporters/TIFFImagePyramidExporter.hpp>
#include <FAST/Algorithms/NeuralNetwork/TensorToImage.hpp>
#include <FAST/Algorithms/ImageResizer/ImageResizer.hpp>
#include <FAST/Visualization/SegmentationRenderer/SegmentationRenderer.hpp>
#include <FAST/Data/Image.hpp>
#include <FAST/Algorithms/BinaryThresholding/BinaryThresholding.hpp>
#include <FAST/Algorithms/ImagePyramidLevelExtractor/ImagePyramidLevelExtractor.hpp>
#include <FAST/Algorithms/IntensityNormalization/IntensityNormalization.hpp>
#include <FAST/Algorithms/NeuralNetwork/TensorToSegmentation.hpp>

using namespace fast;

int main(int argc, char** argv) {
    Reporter::setGlobalReportMethod(Reporter::INFO, Reporter::NONE);

    CommandLineParser parser("Measure neural network performance script");
    parser.addOption("disable-warmup");
    parser.parse(argc, argv);
    const int iterations = 10;  // 10
    const bool warmupIteration = !parser.getOption("disable-warmup");

    std::cout << "\nPatch-wise high-res semantic segmentation...\n" << std::endl;
    const std::string resultFilename = "../../results-pipeline-runtime.csv";
    std::ofstream file(resultFilename.c_str());

    std::vector<int> m_img_size{256, 256};
    int m_patch_level = 2;
    int low_res_patch_level = 5;
    float m_maskThreshold = 0.2;
    int iter = 1;

    // Write header
    file << "Engine;Device Type;Iteration;Patch generator AVG;Patch generator STD;NN input AVG;NN input STD;NN inference AVG;NN inference STD;NN output AVG;NN output STD;Patch stitcher AVG;Patch stitcher STD;Stage1;Stage2;Total\n";

    for (std::string engine : {"TensorRT", "OpenVINO"}) {
        std::map<std::string, InferenceDeviceType> deviceTypes = {{"ANY", InferenceDeviceType::ANY}};
        if (engine == "OpenVINO") {
            // On OpenVINO, try all device types
            deviceTypes = std::map<std::string, InferenceDeviceType>{
                    {"CPU", InferenceDeviceType::CPU},
                    //{"GPU", InferenceDeviceType::GPU},
            };
        }

        for (auto &&deviceType : deviceTypes) {
            std::cout << engine << " for device type " << deviceType.first << std::endl;
            std::cout << "====================================" << std::endl;

            for (int iteration = 0; iteration <= iterations; ++iteration) {

                // start total runtime, from start to scratch
                auto total_runtime_start = std::chrono::high_resolution_clock::now();

                auto importer = WholeSlideImageImporter::New();
                importer->setFilename("../../../../WSI/283.tif");
                auto m_image = importer->updateAndGetOutputData<ImagePyramid>();

                auto tissueSegmentation = TissueSegmentation::New();
                tissueSegmentation->setInputData(m_image);

                auto generator = PatchGenerator::New();
                generator->setPatchSize(256, 256);
                generator->setPatchLevel(2);
                generator->setOverlap(0.0);
                generator->setMaskThreshold(0.1);
                generator->setInputData(0, m_image);
                generator->setInputConnection(1, tissueSegmentation->getOutputPort());
                generator->enableRuntimeMeasurements();

                auto network = NeuralNetwork::New();
                network->setInferenceEngine(engine);
                if (engine == "OpenVINO")
                    network->getInferenceEngine()->setDeviceType(deviceType.second);
                network->load("../../pw_tumour_mobilenetv2_model.onnx");
                network->setScaleFactor(0.00392156862f);
                network->setInputConnection(generator->getOutputPort());
                network->enableRuntimeMeasurements();

                auto stitcher = PatchStitcher::New();
                stitcher->setInputConnection(network->getOutputPort());
                stitcher->enableRuntimeMeasurements();

                //auto start1 = std::chrono::high_resolution_clock::now();
                DataObject::pointer data;
                do {
                    data = stitcher->updateAndGetOutputData<DataObject>();
                } while (!data->isLastFrame());  // wait until stitcher is finished before starting the refinement stage

                // first stage finished, catch total runtime at this stage
                auto runtime_stage1 = std::chrono::high_resolution_clock::now() - total_runtime_start;

                // start second stage runtime
                auto second_runtime_start = std::chrono::high_resolution_clock::now();

                auto converter = TensorToImage::New();
                converter->setChannels({ 1 });
                converter->setInputConnection(stitcher->getOutputPort());

                // extract tumour channel heatmap
                auto lowresExtractor = ImagePyramidLevelExtractor::New();
                lowresExtractor->setLevel(-1);
                lowresExtractor->setInputData(m_image);

                auto scaler = IntensityNormalization::create(0, 1, 0, 255);
                scaler->setInputConnection(lowresExtractor->getOutputPort());

                auto refinement = NeuralNetwork::New();
                refinement->setInferenceEngine(engine);
                if (engine == "OpenVINO") {
                    refinement->getInferenceEngine()->setDeviceType(deviceType.second);
                    refinement->load("C:/Users/andrp/workspace/FAST-Pathology/studies/tumour-study/unet_tumour_refinement_model_fix-opset9.onnx");
                } else {
                    refinement->load("C:/Users/andrp/workspace/FAST-Pathology/studies/tumour-study/unet_tumour_refinement_model_fix.onnx");
                }
                refinement->setInputConnection(0, scaler->getOutputPort());
                refinement->setInputConnection(1, converter->getOutputPort());

                auto converter2 = TensorToSegmentation::New();
                converter2->setInputConnection(refinement->getOutputPort());

                // finally, export final result to disk
                auto finalSegExporter = ImageFileExporter::New();
                finalSegExporter->setFilename("../../pred_tumour_seg_" + std::to_string(iter) + ".png");
                finalSegExporter->setExecuteOnLastFrameOnly(true);
                finalSegExporter->connect(converter2->updateAndGetOutputData<Image>());
                finalSegExporter->enableRuntimeMeasurements();
                finalSegExporter->update();  // runs the exporter

                DataObject::pointer data_;
                do {
                    data_ = refinement->updateAndGetOutputData<DataObject>();
                } while (!data_->isLastFrame());

                // stage 2 finished
                auto time_final = std::chrono::high_resolution_clock::now();
                auto runtime_stage2 = time_final - second_runtime_start;

                std::cout << "Total runtime: " << (time_final - total_runtime_start).count() << std::endl;
                std::cout << "Runtime Stage 1: " << runtime_stage1.count() << std::endl;
                std::cout << "Runtime Stage 2: " << runtime_stage2.count() << std::endl;
                std::cout << "Patch generator runtime: " << std::endl;
                generator->getRuntime("create patch")->print();
                std::cout << "NN runtime: " << std::endl;
                network->getRuntime()->print();
                std::cout << "Patch stitcher runtime: " << std::endl;
                stitcher->getRuntime()->print();
                std::cout << "Exporter runtime" << std::endl;
                finalSegExporter->getRuntime()->print();

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
                     std::to_string(runtime_stage1.count()) + ";" +
                     std::to_string(runtime_stage2.count()) + ";" +
                     std::to_string((time_final - total_runtime_start).count())
                     << std::endl;
            }
        }
    }
}
