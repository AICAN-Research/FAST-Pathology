//
// Created by dbouget on 03.11.2021.
//

#ifndef FASTPATHOLOGY_LOGICRUNTIMEMODEL_H
#define FASTPATHOLOGY_LOGICRUNTIMEMODEL_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <memory>
#include <map>

#include <QDir>
#include <QString>
#include <QDirIterator>
#include <QFileInfo>

namespace fast{
    class NeuralNetwork;

    class LogicRuntimeModel {
        public:
            LogicRuntimeModel(const std::string main_dir, const std::string name);
            ~LogicRuntimeModel();

            inline const std::string get_model_name(){return _name;}
            inline std::map<std::string, std::string> get_model_metadata(){return _metadata;}

        protected:
            /**
             * @brief parseModelContent
             * Parse the content of a model sub-folder to identify existing model files.
             */
            void parseModelContent();
            /**
             * @brief extractModelMetadata
             * Parse the metadata configuration file for the current model.
             */
            void extractModelMetadata();
        private:
            std::string _main_dir; /* Overall directory for the current model. */
            std::string _name; /* Overall name of the current model. */
            std::map<std::string, std::string> _metadata; /* Relevant information associated to the trained model. */
            std::map<std::string, std::string> _models_names; /* Folder content, execpt the information text file. */
    };
} // End of namespace fast

#endif //FASTPATHOLOGY_LOGICRUNTIMEMODEL_H
