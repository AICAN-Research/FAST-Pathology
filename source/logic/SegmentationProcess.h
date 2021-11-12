//
// Created by dbouget on 03.11.2021.
//

#ifndef FASTPATHOLOGY_SEGMENTATIONPROCESS_H
#define FASTPATHOLOGY_SEGMENTATIONPROCESS_H

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <map>
#include <QImage>
#include "source/logic/DataManager.h"
#include "source/utils/qutilities.h"

namespace fast{
    class NeuralNetwork;
    class PatchStitcher;
    class SegmentationRenderer;
    class Renderer;
    class TissueSegmentation;
    class WholeSlideImageImporter;
    class ImagePyramid;
    class Segmentation;
    class Image;
    class Tensor;
    class View;
    class ImageExporter;

    // @TODO. Should it be just PipelineProcess to be more generic?
    // Especially since the FAST pipeline will be used anyway.
    class SegmentationProcess {
        public:
            SegmentationProcess(const std::string image_uid);
            ~SegmentationProcess();

            bool segmentTissue();
       private:
            std::string _image_uid; /* unique id for the processed image (if only one is processed?)*/
            bool _stop_flag; /* ? */
            bool _advanced_mode; /* ? */
    };
} // End of namespace fast

#endif //FASTPATHOLOGY_SEGMENTATIONPROCESS_H
