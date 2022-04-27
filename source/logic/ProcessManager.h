//
// Created by dbouget on 02.11.2021.
//

#ifndef FASTPATHOLOGY_PROCESSMANAGER_H
#define FASTPATHOLOGY_PROCESSMANAGER_H

#include <FAST/Visualization/ComputationThread.hpp>
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
#include "source/logic/PipelineProcess.h"
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

            inline bool has_pipeline(std::string pipeline_name){
                return _pipelines.find(pipeline_name) != _pipelines.end();
            }

            inline std::shared_ptr<LogicRuntimeModel> get_model(std::string model_name) {return this->_models[model_name];}
            inline std::shared_ptr<PipelineProcess> get_pipeline(std::string pipeline_name) {return this->_pipelines[pipeline_name];}
            inline std::map<std::string, std::shared_ptr<PipelineProcess>> getAllPipelines() const{return this->_pipelines;}
            inline const std::string get_models_filepath(){return this->_models_filepath;}
            inline const std::string get_pipelines_filepath(){return this->_pipelines_filepath;}

            inline bool get_advanced_mode_status(){return this->_advanced_mode;}
            void set_advanced_mode_status(bool status);
            void set_computation_thread(std::shared_ptr<ComputationThread> thr);
            void execute_independent();

            void importModel(const std::string& name);
            /**
             * Simple wrapper of the inference method, pixelClassifier. If ran with Projects enabled, it will be ran in a
             * non-blocking background thread and will not render any results. Otherwise the rendered results will be
             * streamed on the fly, but no results will be stored (relevant for simple demonstrations). If advanced mode
             * is enabled, a parameter dialog will be prompted for setting parameters before inference is ran.
             * @param someModelName
             */
            void runProcess(const std::string image_uid, const std::string process_name);
            /**
             * Runs inference of a selected pipeline. If ran with Projects enabled, it will be ran in a non-blocking
             * background thread and will not render any results. Otherwise the rendered results will be streamed on the
             * fly, but no results will be stored (relevant for simple demonstrations).
             * @param someModelName
             * @param modelMetadata
             */
            void pixelClassifier(std::string process_name);

            void runPipeline(const std::string& pipeline_uid);

        private:
            /**
             * Get the specifications of the current Operating System used to run FastPathology.
             */
            void identifySystem();
            /**
             * @brief loadModels Loading the approved inference models being located in ~/fastpathology
             */
            void loadApprovedModels();
            /**
             * @brief loadPipelines Loading the approved pipelines being located in ~/fastpathology
             */
            void loadApprovedPipelines();

            const std::pair<std::string, std::string> selectOptimalInferenceEngine(const std::string& model_name) const;

        private:
            static ProcessManager * _pinstance;
            static std::mutex _mutex;
            std::shared_ptr<ComputationThread> _computation_thread;
            bool _advanced_mode; /* */ //@TODO. With avanced mode, only system specific parameters can be modified. The rest is part of a pipeline and its parameters.'
            std::string _fp_root_filepath; /* @TODO. can it change from a user input?*/
            std::string _models_filepath; /* @TODO. Can we consider models and pipelines to be inside the same root folder on disk? */
            std::string _pipelines_filepath; /* */
            std::string _operating_system; /* Operating system of the user's computer. */
            std::string _kernel; /* Operating system kernel of the user's computer. */
            std::list<std::string> _inference_engines; /* List of inference engines available on the user's computer. */
            std::map<std::string, std::shared_ptr<LogicRuntimeModel>> _models; /* Loaded model objects, the key is the model_name as defined in the text file. */
            std::map<std::string, std::shared_ptr<PipelineProcess>> _pipelines; /* Loaded pipelines objects, the key will be some attribute from a text file. */
    };
}
#endif //FASTPATHOLOGY_PROCESSMANAGER_H
