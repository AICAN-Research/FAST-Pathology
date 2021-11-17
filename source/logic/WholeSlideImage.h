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
            WholeSlideImage(const std::string filename, const QImage thumbnail);
            ~WholeSlideImage();

            inline float get_magnification_level() const {return this->_magnification_level;}
            inline std::string get_filename(){return _filename;}
            inline QImage get_thumbnail() {return _thumbnail;}
            inline std::shared_ptr<ImagePyramid> get_image_pyramid(){return _image;}
            inline std::unordered_map<std::string, std::string> get_metadata(){return _metadata;}

            /***
             * Load the current whole slide image in memory for display and interaction.
             ***/
            void memory_load();

            void init();
            /***
             * @TODO. Should it only be for the WSI renderer, other results renderers cannot be invoked from here
             * and will be lost everytime a new thumbnail is clicked?
             ***/
            void load_renderer();
            void unload_renderer();

            /***
             * Unload the current whole slide image from memory when not used for user interaction.
             * But all other relevant parameters are kept for future re-loading.
             ***/
            void memory_unload();

            inline bool has_renderer(std::string renderer_name){
                return _renderers.find(renderer_name) != _renderers.end();
            }

            const std::vector<std::string> get_renderer_keys();
            std::shared_ptr<Renderer> get_renderer(std::string name){
                return this->_renderers[name];
            }

            const std::string get_renderer_type(const std::string& name);

            void insert_renderer(std::string renderer_name, std::string renderer_type, std::shared_ptr<Renderer> renderer);
            void remove_renderer(std::string renderer_name);

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
            // @TODO. The key is obviously unique, and should link to how results are generated so that multiple segmentation renderers using the same model can be stored
            // (only difference being some parameters modified by the user).
            std::map<std::string, std::shared_ptr<Renderer>> _renderers; /* List of loaded renderers associated with the image (e.g., Segmentation/HeatmapRenderer). */
            std::map<std::string, std::string> _renderers_types; /* Type of each renderer. */
            QImage _thumbnail; /* Thumbnail for the WSI */
            // @TODO. When the renderers are not in memory the image files are dumped on disk. Should we store here the text file for each pipeline that was used on this image
            // so that reloading the results for viewing is easier to perform.
            std::map<std::string, std::string> _results_filenames;
    };
}

#endif //FASTPATHOLOGY_WHOLESLIDEIMAGE_H
