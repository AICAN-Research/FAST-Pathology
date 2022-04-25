//
// Created by dbouget on 05.11.2021.
//

#include "PipelineProcess.h"
#include <FAST/Visualization/Renderer.hpp>
#include <FAST/ProcessObject.hpp>
#include <FAST/Reporter.hpp>


namespace fast{
    PipelineProcess::PipelineProcess(const std::string& filepath, const std::string& pipeline_name):_pipeline_filepath(filepath), _name(pipeline_name)
    {
    }

    PipelineProcess::~PipelineProcess()
    {
    }

    std::string PipelineProcess::getPipelineFullFilename() const
    {
        // @TODO. The unique id name might not be the actual file name on disk, will have to deal with proper configuration files for pipelines.
        return this->_pipeline_filepath + "/" + this->_name + ".fpl";
    }

    void PipelineProcess::setParameters(std::map<std::string, std::string> parameters)
    {
        this->_parameters = parameters;
    }

    void PipelineProcess::execute()
    {
        // Will have to see how it goes with segfault.
//        this->_renderers.clear();
        try
        {
            auto current_path = this->_pipeline_filepath + "/" + this->_name + ".fpl";
            this->_fast_pipeline.reset(new Pipeline(current_path, this->_parameters));
            this->_fast_pipeline->parse();
        }
        catch (const std::exception& e)
        {
            std::cout<<"Sucks."<<std::endl;
        }
//        auto pipeline = Pipeline(current_path, this->_parameters);
//        pipeline.parse();

        // get and start running POs
//        for (auto&& po : pipeline.getProcessObjects()) {
//            if (po.second->getNrOfOutputPorts() == 0 && std::dynamic_pointer_cast<Renderer>(po.second) == nullptr) {
//                // Process object has no output ports, must add to window to make sure it is updated.
//                reportInfo() << "Process object " << po.first << " had no output ports defined in pipeline, therefore adding to window so it is updated." << reportEnd();
//                addProcessObject(po.second);
//            }
//        }
//        // load renderers, if any
//        for (const auto& renderer : pipeline.getRenderers()) {
//            auto currId = createRandomNumbers_(8);
//            insertRenderer("result_" + currId, renderer);
//            createDynamicViewWidget("result_" + currId, "result_" + currId);
//        }

//        this->_renderers = pipeline.getRenderers();
    }
} // End of namespace fast
