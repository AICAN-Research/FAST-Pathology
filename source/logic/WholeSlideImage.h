//
// Created by dbouget on 07.10.2021.
//

#ifndef FASTPATHOLOGY_WHOLESLIDEIMAGE_H
#define FASTPATHOLOGY_WHOLESLIDEIMAGE_H

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <map>
#include <QImage>

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

    class WholeSlideImage {
        public:
            WholeSlideImage(const std::string filename);
            ~WholeSlideImage();

            inline float get_magnification_level() const {return this->_magnification_level;}
            inline std::string get_filename(){return _filename;}
            inline QImage get_thumbnail() {return _thumbnail;}
            inline std::shared_ptr<ImagePyramid> get_image_pyramid(){return _image;}

            /***
             * Load the current whole slide image in memory for display and interaction.
             ***/
            void memory_load();

            /***
             * Unload the current whole slide image from memory when not used for user interaction.
             * But all other relevant parameters are kept for future re-loading.
             ***/
            void memory_unload();

            inline bool has_renderer(std::string renderer_name){
                return _renderers.find(renderer_name) != _renderers.end();
            }

            std::shared_ptr<Renderer> get_renderer(std::string name){
                return this->_renderers[name];
            }


        private:
            void compute_magnification_level();
            /**
             * Gets the thumbnail image and stores it as a QImage.
             * @return
             */
            void create_thumbnail();

        private:
            const std::string _filename; /* Disk location for the whole slide image*/
            std::string _format; /* Former wsiFormat */
            std::unordered_map<std::string, std::string> _metadata; /* */
            float _magnification_level; /* */
            std::shared_ptr<ImagePyramid> _image; /* Loaded WSI */
            std::map<std::string, std::shared_ptr<Renderer>> _renderers; /* */
            std::map<std::string, std::string> _renderers_types; /* */
            QImage _thumbnail; /* Thumbnail for the WSI */
    };
}

#endif //FASTPATHOLOGY_WHOLESLIDEIMAGE_H
