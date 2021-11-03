//
// Created by dbouget on 03.11.2021.
//

#include "InferenceModel.h"
#include <FAST/Data/ImagePyramid.hpp>
#include <FAST/Data/Image.hpp>


namespace fast{
    InferenceModel::InferenceModel(const std::string filename): _filename(filename)
    {
    }

    InferenceModel::~InferenceModel()
    {
    }
} // End of namespace fast
