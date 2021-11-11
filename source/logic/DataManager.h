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

#include <QTemporaryDir>
#include <QDir>
#include <QTextStream>
#include "source/logic/Project.h"
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
            DataManager();
            ~DataManager();

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

            inline std::shared_ptr<Project> getCurrentProject()const {return this->_project;}

            inline std::string getVisibleImageName()const {return this->_visible_wsi_uid;}
            /**
             * @brief get_visible_image
             * @return
             */
            std::shared_ptr<WholeSlideImage> get_visible_image();
            /**
             * @brief setVisibleImageName
             * @param id_name
             */
            void setVisibleImageName(std::string id_name){this->_visible_wsi_uid = id_name;}
            /**
             * @brief doEmpty
             */
            void doEmpty();


        protected:
            std::string _visible_wsi_uid; /* Unique id_name of the currently rendered (hence visible) WSI. */
            // @TODO. If making the above member a unique_ptr, all of its methods should be forwarded from here, maybe too annoying/code-redundant.
            std::shared_ptr<Project> _project; /* Current project instance to which all images/results are linked. */

        private:
            static DataManager * _pinstance;
            static std::mutex _mutex;
    };
}
#endif //FASTPATHOLOGY_DATAMANAGER_H
