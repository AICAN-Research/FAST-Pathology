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
    int m_patch_level = 1;
    float m_maskThreshold = 0.2;
    int iter = 1;

    // Write header
    file << "Engine;Device Type;Iteration;Patch generator AVG;Patch generator STD;NN input AVG;NN input STD;NN inference AVG;NN inference STD;NN output AVG;NN output STD;Patch stitcher AVG;Patch stitcher STD;Exporter AVG; Exporter STD;Total\n";

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
                //importer->setSynchronizedRendering(true);
                //importer->setFilename("E:/DigitalPathology/WSI/C.tif");
                //importer->setFilename("../../../../wsi-2_HE.ndpi");
                //importer->setFilename("../../703.tif");
                importer->setFilename("../../../../A05.svs");

                //auto tissueSegmentation = TissueSegmentation::New();
                //tissueSegmentation->setInputConnection(importer->getOutputPort());

                auto generator = PatchGenerator::New();
                generator->setPatchSize(m_img_size[0], m_img_size[1]);
                generator->setPatchLevel(m_patch_level);
                generator->setOverlap(0.0);
                generator->setMaskThreshold(m_maskThreshold);
                generator->setInputConnection(importer->getOutputPort());
                //generator->setInputConnection(1, tissueSegmentation->getOutputPort());
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

                auto start = std::chrono::high_resolution_clock::now();
                DataObject::pointer data;
                do {
                    data = stitcher->updateAndGetOutputData<DataObject>();
                } while (!data->isLastFrame());  // wait until stitcher is finished before starting the refinement stage

                // extract low-resolution image and resize it
                auto currImage = importer->updateAndGetOutputData<ImagePyramid>();
                auto access = currImage->getAccess(ACCESS_READ);
                auto input = access->getLevelAsImage(1);

                auto resizer = ImageResizer::New();
                resizer->setInputData(input);
                resizer->setWidth(1024);
                resizer->setHeight(1024);

                auto port = resizer->getOutputPort();
                resizer->update();

                // resize heatmap to match size of low-res image
                auto converter = TensorToImage::New();
                converter->setInputConnection(stitcher->getOutputPort());

                auto resizer2 = ImageResizer::New();
                resizer2->setInputConnection(converter->getOutputPort());
                //resizer2->setInputData(stitcher->updateAndGetOutputData<Image>());
                resizer2->setWidth(1024);
                resizer2->setHeight(1024);

                auto port2 = resizer2->getOutputPort();
                resizer2->update();

                // when heatmap is finished stitching, we feed both the low-resolution WSI and produced heatmap to the refinement network
                auto refinement = SegmentationNetwork::New();
                refinement->setInferenceEngine("TensorFlow");
                refinement->load("../../unet_tumour_refinement_model.pb");
                refinement->setScaleFactor(0.00392156862f);
                refinement->setInputData(1, port->getNextFrame<Image>());
                refinement->setInputData(0, port2->getNextFrame<Image>());
                //refinement->setInputConnection(0, stitcher->getOutputPort());
                //refinement->setInputData(1, stitcher->updateAndGetOutputData<Image>());  // @FIXME: Input here is null
                refinement->enableRuntimeMeasurements();
                refinement->update();

                // finally, export final result to disk
                auto finalSegExporter = ImageFileExporter::New();
                finalSegExporter->setFilename("../../pred_tumour_seg_" + std::to_string(iter) + ".png");
                finalSegExporter->setInputData(refinement->updateAndGetOutputData<Image>());
                finalSegExporter->enableRuntimeMeasurements();
                finalSegExporter->update();  // runs the exporter

                auto start2 = std::chrono::high_resolution_clock::now();
                DataObject::pointer data2;
                do {
                    data2 = stitcher->updateAndGetOutputData<DataObject>();
                } while (!data2->isLastFrame());

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
                     std::to_string(finalSegExporter->getRuntime()->getAverage()) + ";" +
                     "0" + ";" +
                     std::to_string(timeUsed.count())
                     << std::endl;
            }
        }
    }
}
