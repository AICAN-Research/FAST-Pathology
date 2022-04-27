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
//        this->_pipeline_thread = ComputationThread::create();
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

    void PipelineProcess::setComputationThread(std::shared_ptr<ComputationThread> thread)
    {
        this->_computation_thread = thread;
    }

    void PipelineProcess::execute_independent()
    {
        try
        {
            auto current_path = this->_pipeline_filepath + "/" + this->_name + ".fpl";
            auto pipeline = Pipeline(current_path, this->_parameters);
            pipeline.parse({}, {}, true);
            auto thread = _computation_thread;
            thread->setPipeline(pipeline);

            bool doneSignaled = false;
            QObject::connect(thread.get(), &ComputationThread::pipelineFinished, [thread, &doneSignaled]() {
                // This is running in QThread, not in main thread..
                doneSignaled = true;
                std::cout << "DONE" << std::endl;
                thread->stopWithoutBlocking();
            });

            auto t = thread->start();
            t->wait(); // Wait until finished
//            CHECK(doneSignaled);
            std::cout<<"Pipeline done signal: "<<doneSignaled<<std::endl;
        }
        catch (const std::exception& e)
        {
            // @TODO. Should not crash FP but just prompt an error message
            std::cout<<"Pipeline crashed."<<std::endl;
        }
    }

//    void PipelineProcess::execute()
//    {
//        // Will have to see how it goes with segfault.
////        this->_renderers.clear();

//        // Saving the results from the previous pipeline, if any, before running a new one
//        // Until a signal from the Pipeline can be caught.

//        try
//        {
//            auto current_path = this->_pipeline_filepath + "/" + this->_name + ".fpl";
//            this->_fast_pipeline.reset(new Pipeline(current_path, this->_parameters));
//            // @TODO. Shouldn't there be a check between the ProcessManager inference engines, and the requested engine
//            // from the pipeline parameters, to see if there's compatibility or not?
//            // this->_fast_pipeline->parse();
////            this->_fast_pipeline->parse({}, {}, true);

////            auto data = this->_fast_pipeline->getAllPipelineOutputData([&, this](float progress) {
////                // this never runs? Why?
////                std::cout << "Progress: " << 100 * progress << "%" << std::endl;
////                if (int(progress) == 1) {  // run until finished
////                    std::cout << "PIPELINE IS DONE --- STOP SIGNAL WAS EMITTED!" << std::endl;
////                    emit currentPipelineFinished();
////                }
////            });
////            std::cout << "Done" << std::endl;
//            std::cout<<"Pipeline before parse"<<std::endl;
//            this->_fast_pipeline->parse({}, {}, true);
//            std::cout<<"Pipeline after parse"<<std::endl;
//            auto thread = ComputationThread::create();
//            thread->setPipeline(*this->_fast_pipeline.get());

//            bool doneSignaled = false;
//            QObject::connect(thread.get(), &ComputationThread::pipelineFinished, [thread, &doneSignaled]() {
//                // This is running in QThread, not in main thread..
//                doneSignaled = true;
//                std::cout << "DONE" << std::endl;
//                thread->stopWithoutBlocking();
//            });

//            auto t = thread->start();
//            t->wait(); // Wait until finished
////            CHECK(doneSignaled);
//            std::cout<<"Pipeline done signal: "<<doneSignaled<<std::endl;
//        }
//        catch (const std::exception& e)
//        {
//            // @TODO. Should not crash FP but just prompt an error message
//            std::cout<<"Pipeline crashed."<<std::endl;
//        }
////        auto pipeline = Pipeline(current_path, this->_parameters);
////        pipeline.parse();

//        // get and start running POs
////        for (auto&& po : pipeline.getProcessObjects()) {
////            if (po.second->getNrOfOutputPorts() == 0 && std::dynamic_pointer_cast<Renderer>(po.second) == nullptr) {
////                // Process object has no output ports, must add to window to make sure it is updated.
////                reportInfo() << "Process object " << po.first << " had no output ports defined in pipeline, therefore adding to window so it is updated." << reportEnd();
////                addProcessObject(po.second);
////            }
////        }
////        // load renderers, if any
////        for (const auto& renderer : pipeline.getRenderers()) {
////            auto currId = createRandomNumbers_(8);
////            insertRenderer("result_" + currId, renderer);
////            createDynamicViewWidget("result_" + currId, "result_" + currId);
////        }

////        this->_renderers = pipeline.getRenderers();
//    }
} // End of namespace fast
