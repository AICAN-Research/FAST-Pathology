//
// Created by dbouget on 05.11.2021.
//

#ifndef FASTPATHOLOGY_PIPELINEPROCESS_H
#define FASTPATHOLOGY_PIPELINEPROCESS_H

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <memory>
#include <QObject>
#include <FAST/Pipeline.hpp>
#include <FAST/Visualization/ComputationThread.hpp>


namespace fast
{
    class Pipeline;
    class Renderer;
    class ProcessObject;
    class Reporter;

    class PipelineProcess: public QObject
    {
        Q_OBJECT
        public:
            PipelineProcess(const std::string& filepath, const std::string& pipeline_name);
            ~PipelineProcess();

            inline std::string getPipelineFilepath()const {return this->_pipeline_filepath;}
            inline std::map<std::string, std::string> getParameters() const{return this->_parameters;}
//            inline std::vector<std::shared_ptr<Renderer>> getRenderers() const{return this->_fast_pipeline->getRenderers();}
//            inline std::map<std::string, std::shared_ptr<ProcessObject>> getProcessObjects() const{return this->_fast_pipeline->getProcessObjects();}
            std::string getPipelineFullFilename() const;
            void setParameters(std::map<std::string, std::string> parameters);
            void setComputationThread(std::shared_ptr<ComputationThread> thread);

//            void execute();
            void execute_independent();
        private:
            std::shared_ptr<ComputationThread> _computation_thread;
            std::string _name; /* Unique id for the pipeline. */
            std::string _pipeline_filepath; /* Disk location of the current pipeline. */
            std::map<std::string, std::string> _parameters; /* Collection of pipeline-specific parameters, some of which being user-modified (with advanced mode?). */
//            std::vector<std::shared_ptr<Renderer>> _renderers;
            //std::unique_ptr<Pipeline> _fast_pipeline;
//            std::shared_ptr<Pipeline> _fast_pipeline;
//            std::shared_ptr<ComputationThread> _pipeline_thread;
        signals:
            void currentPipelineFinished();
    };
} // End of namespace fast

#endif //FASTPATHOLOGY_PIPELINEPROCESS_H
