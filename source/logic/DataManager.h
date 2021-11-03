//
// Created by dbouget on 07.10.2021.
//

#ifndef FASTPATHOLOGY_DATAMANAGER_H
#define FASTPATHOLOGY_DATAMANAGER_H

#include <iostream>
#include <mutex>
#include <map>
#include <memory>
#include <unordered_map>
#include "source/logic/WholeSlideImage.h"
#include "source/utils/utilities.h"

namespace fast {
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

    class DataManager {
        /**
         * The Singleton's constructor/destructor should always be private to
         * prevent direct construction/desctruction calls with the `new`/`delete`
         * operator.
         */
        protected:
            /**
             * @brief Contains all data pertaining to the ongoing project.
             */
            DataManager()
            {
                _visible_wsi_uid = "";
            }
            ~DataManager() {}

        public:
            /**
             * Singletons should not be cloneable.
             */
            DataManager(DataManager &other) = delete;
            /**
             * Singletons should not be assignable.
             */
            void operator=(const DataManager &) = delete;
            /**
             * This is the static method that controls the access to the singleton
             * instance. On the first run, it creates a singleton object and places it
             * into the static field. On subsequent runs, it returns the client existing
             * object stored in the static field.
             */

            static DataManager *GetInstance();

            inline std::vector<std::string> getProjectFilenames(){return _wsi_filenames_list;}
            inline bool isEmpty(){return _images.empty();}
            inline std::string getVisibleImageName(){return this->_visible_wsi_uid;}
            void setVisibleImageName(std::string id_name){this->_visible_wsi_uid = id_name;}

            std::shared_ptr<WholeSlideImage> get_image(std::string name){
                return this->_images[name];
            }

            std::shared_ptr<WholeSlideImage> get_visible_image(){
                if(_visible_wsi_uid != "")
                    return this->_images[this->_visible_wsi_uid];
                else
                    return NULL;
            }

            void doEmpty();

            /**
             * Instanciates an image object from the selected WSI filename.
             */
            const std::string IncludeImage(std::string currFileName);

            /**
            *
            */
            void RemoveImage(std::string uid);

//            std::shared_ptr<Renderer> get_renderer(std::string name){
//                return this->_renderers[name];
//            }


        protected:
            std::string _filename; /* ? */
            std::string _projectFolderName; /* */
            std::map<std::string, std::shared_ptr<WholeSlideImage>> _images; /* Loaded image objects. */
            std::vector<std::string> _wsi_filenames_list;/* Filenames of the loaded images, project-info. */
            std::string _visible_wsi_uid; /* Unique id_name of the currently rendered (hence visible) WSI. */
        private:
            static DataManager * _pinstance;
            static std::mutex _mutex;
    };
}
#endif //FASTPATHOLOGY_DATAMANAGER_H
