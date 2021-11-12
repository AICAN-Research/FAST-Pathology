//
// Created by dbouget on 05.11.2021.
//

#include "PipelineProcess.h"
#include <FAST/Pipeline.hpp>
#include <FAST/Visualization/Renderer.hpp>


namespace fast{
    PipelineProcess::PipelineProcess(const std::string& filepath, const std::string& pipeline_name):_pipeline_filepath(filepath), _name(pipeline_name)
    {
    }

    PipelineProcess::~PipelineProcess()
    {
    }

    void PipelineProcess::setParameters(std::map<std::string, std::string> parameters)
    {
        this->_parameters = parameters;
    }

    void PipelineProcess::execute()
    {
        // Will have to see how it goes with segfault.
        this->_renderers.clear();

        auto pipeline = Pipeline(this->_pipeline_filepath, this->_parameters);
        pipeline.parse();
        this->_renderers = pipeline.getRenderers();
    }
} // End of namespace fast
