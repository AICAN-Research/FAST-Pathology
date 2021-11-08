//
// Created by dbouget on 02.11.2021.
//

#ifndef FASTPATHOLOGY_PROCESSMANAGER_H
#define FASTPATHOLOGY_PROCESSMANAGER_H

#include <QDir>
#include <QTemporaryDir>
#include <iostream>
#include <mutex>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include "source/logic/WholeSlideImage.h"
#include "source/logic/SegmentationProcess.h"
#include "source/logic/LogicRuntimeModel.h"
#include "source/utils/utilities.h"
#include "source/utils/qutilities.h"

namespace fast {
    class NeuralNetwork;
    class SegmentationNetwork;
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
    class ImageResizer;
    class Config;

    class ProcessManager
    {
        /**
         * The Singleton's constructor/destructor should always be private to
         * prevent direct construction/desctruction calls with the `new`/`delete`
         * operator.
         */
        protected:
            /**
             * @brief Contains all data pertaining to the ongoing project.
             */
            ProcessManager();
            ~ProcessManager() {}

        public:
            /**
             * Singletons should not be cloneable.
             */
            ProcessManager(ProcessManager &other) = delete;
            /**
             * Singletons should not be assignable.
             */
            void operator=(const ProcessManager &) = delete;
            /**
             * This is the static method that controls the access to the singleton
             * instance. On the first run, it creates a singleton object and places it
             * into the static field. On subsequent runs, it returns the client existing
             * object stored in the static field.
             */
            static ProcessManager *GetInstance();

            inline bool has_model(std::string model_name){
                return _models.find(model_name) != _models.end();
            }

            inline std::shared_ptr<LogicRuntimeModel> get_model(std::string model_name){return this->_models[model_name];}
            inline const std::string get_models_filepath(){return this->_models_filepath;}
            inline const std::string get_pipelines_filepath(){return this->_pipelines_filepath;}

            inline bool get_advanced_mode_status(){return this->_advanced_mode;}
            void set_advanced_mode_status(bool status);

            void importModel(const std::string& name);
            void runProcess(const std::string image_uid, const std::string process_name);
            void pixelClassifier(std::string process_name);

        private:
            static ProcessManager * _pinstance;
            static std::mutex _mutex;
            bool _advanced_mode; /* */
            std::string _fp_root_filepath; /* @TODO. can it change from a user input?*/
            std::string _models_filepath; /* @TODO. Can we consider models and pipelines to be inside the same root folder on disk? */
            std::string _pipelines_filepath; /* */
            std::map<std::string, std::shared_ptr<LogicRuntimeModel>> _models; /* Loaded model objects. */
    };
}
#endif //FASTPATHOLOGY_PROCESSMANAGER_H
