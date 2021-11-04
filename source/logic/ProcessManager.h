//
// Created by dbouget on 02.11.2021.
//

#ifndef FASTPATHOLOGY_PROCESSMANAGER_H
#define FASTPATHOLOGY_PROCESSMANAGER_H

#include <iostream>
#include <mutex>
#include <map>
#include <memory>
#include <unordered_map>
#include "source/logic/WholeSlideImage.h"
#include "source/logic/SegmentationProcess.h"
#include "source/logic/InferenceModel.h"
#include "source/utils/utilities.h"
#include "source/utils/qutilities.h"

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

    class ProcessManager {
        /**
         * The Singleton's constructor/destructor should always be private to
         * prevent direct construction/desctruction calls with the `new`/`delete`
         * operator.
         */
        protected:
            /**
             * @brief Contains all data pertaining to the ongoing project.
             */
            ProcessManager(){this->_advanced_mode = false;}
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

            inline std::shared_ptr<InferenceModel> get_model(std::string model_name){return this->_models[model_name];}

            inline bool get_advanced_mode_status(){return this->_advanced_mode;}
            void set_advanced_mode_status(bool status);

            void runProcess(const std::string image_uid, const std::string process_name);

        private:
            static ProcessManager * _pinstance;
            static std::mutex _mutex;
            bool _advanced_mode; /* */
            std::map<std::string, std::shared_ptr<InferenceModel>> _models; /* Loaded model objects. */
    };
}
#endif //FASTPATHOLOGY_PROCESSMANAGER_H
