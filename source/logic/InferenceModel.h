//
// Created by dbouget on 03.11.2021.
//

#ifndef FASTPATHOLOGY_INFERENCEMODEL_H
#define FASTPATHOLOGY_INFERENCEMODEL_H

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

namespace fast{
    class NeuralNetwork;

    class InferenceModel {
        public:
            InferenceModel(const std::string filename);
            ~InferenceModel();

        private:
            std::string _filename;
    };
} // End of namespace fast

#endif //FASTPATHOLOGY_INFERENCEMODEL_H
