//
// Created by dbouget on 03.11.2021.
//

#include "InferenceModel.h"
#include <FAST/Data/ImagePyramid.hpp>
#include <FAST/Data/Image.hpp>
#include <FAST/Utility.hpp>


namespace fast{
    InferenceModel::InferenceModel(const std::string filepath, const std::string name): _filepath(filepath), _name(name)
    {
        this->extractModelMetadata();
    }

    InferenceModel::~InferenceModel()
    {
    }

    void InferenceModel::extractModelMetadata()
    {
        // parse corresponding txt file for relevant information regarding model
        std::ifstream infile(this->_filepath + "data/Models/" + this->_name + ".txt");
        std::string key, value, str;
        std::string delimiter = ":";
        while (std::getline(infile, str))
        {
            std::vector<std::string> v = split(str, delimiter);
            //key = v[0];
            //value = v[1];
            //metadata[key] = value;
            this->_metadata.emplace(std::move(v[0]), std::move(v[1]));
        }
    }

} // End of namespace fast
