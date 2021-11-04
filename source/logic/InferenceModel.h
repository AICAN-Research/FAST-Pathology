//
// Created by dbouget on 03.11.2021.
//

#ifndef FASTPATHOLOGY_INFERENCEMODEL_H
#define FASTPATHOLOGY_INFERENCEMODEL_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <memory>
#include <map>

namespace fast{
    class NeuralNetwork;

    class InferenceModel {
        public:
            InferenceModel(const std::string filepath, const std::string name);
            ~InferenceModel();

            inline std::map<std::string, std::string> get_model_metadata(){return _metadata;}

        protected:
            void extractModelMetadata();
        private:
            std::string _filepath; /* Filepath where the model is stored on disk */
            std::string _name; /* Specific name of the current model */
            std::map<std::string, std::string> _metadata; /* Relevant information associated to the trained model */
    };
} // End of namespace fast

#endif //FASTPATHOLOGY_INFERENCEMODEL_H
