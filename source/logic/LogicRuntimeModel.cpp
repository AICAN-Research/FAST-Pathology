//
// Created by dbouget on 03.11.2021.
//

#include "LogicRuntimeModel.h"
#include <FAST/Data/ImagePyramid.hpp>
#include <FAST/Data/Image.hpp>
#include <FAST/Utility.hpp>


namespace fast{
    LogicRuntimeModel::LogicRuntimeModel(const std::string main_dir, const std::string name): _main_dir(main_dir), _name(name)
    {
        this->parseModelContent();
        this->extractModelMetadata();
    }

    LogicRuntimeModel::~LogicRuntimeModel()
    {
    }

    void LogicRuntimeModel::parseModelContent()
    {
        auto model_dir = QDir(QString::fromStdString("" + this->_main_dir + "/" + this->_name));

        auto dir_iterator = QDirIterator(model_dir, QDirIterator::Subdirectories);
        QStringList contents = model_dir.entryList(QDir::Files);

        foreach(QString file, contents)
        {
            auto file_info = QFileInfo(file);

            if (file_info.suffix() != "txt")
                this->_models_names[file_info.suffix().toStdString()] = file.toStdString();
        }
    }

    void LogicRuntimeModel::extractModelMetadata()
    {
        // @TODO. Ugly af. Is there an os.path in c++?
        std::string config_filename = std::string("");
        config_filename.append(this->_main_dir);
        config_filename.append(std::string("/"));
        config_filename.append(this->_name);
        config_filename.append(std::string("/"));
        config_filename.append(this->_name);
        config_filename.append(std::string(".txt"));

        // @TODO. Should check if the metadata file exists, os.path.exists T_T
        std::ifstream infile(config_filename);
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
