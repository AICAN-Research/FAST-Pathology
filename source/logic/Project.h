#pragma once

#include <iostream>
#include <string>

#include <QString>
#include <QTemporaryDir>
#include <QFile>
#include <QIODevice>
#include <QTextStream>

#include "source/utils/utilities.h"
#include "source/logic/WholeSlideImage.h"

namespace fast{
    class DataObject;
    class Pipeline;

    class Result {
        public:
            std::string name;
            std::string pipelineName;
            std::string WSI_uid;
            std::shared_ptr<Renderer> renderer;
    };

    class Project {
        public:
            Project();
            ~Project();

            inline bool hasUserSelectedDestinationFolder() const {return !this->_temporary_dir_flag;}
            inline const std::string getRootFolder(){return this->_root_folder;}
            inline bool isProjectEmpty() const{return _images.empty();}
            inline int getWSICountInProject() const{return this->_images.size();}
            std::vector<std::string> getAllWsiUids() const;
            std::shared_ptr<WholeSlideImage> getImage(const std::string& name);
            std::shared_ptr<WholeSlideImage> getImage(int i);

            void setRootFolder(const std::string& root_folder);

            void emptyProject();
            void loadProject();
            void saveProject();
            void saveResults(const std::string& wsi_uid, std::shared_ptr<Pipeline> pipeline, std::map<std::string, std::shared_ptr<DataObject>> data);
            /**
             * @brief saveResult Purpose is to save on disk all results linked to one WSI.
             * Will evolve when pipeline outputs can be properly caught.
             * @param wsi_uid
             * @param current_renderer_type
             */
            void saveResult(const std::string& wsi_uid, const std::string& current_renderer_type);

            std::vector<Result> loadResults(const std::string& wsi_uid);

            /**
             * @brief includeImage Include image to the current project.
             * @param image_filepath Disk location of the WSI to include.
             * @return
             */
            const std::string includeImage(const std::string& image_filepath);
            /**
             * @brief includeImageFromProject Reload a WSI from a previously saved project.
             * @param uid_name Unique identifier for the WSI.
             * @param image_filepath Disk location of the WSI.
             */
            void includeImageFromProject(const std::string& uid_name, const std::string& image_filepath);
            /**
             * @brief removeImage Remove a WSI from the current project.
             * @param uid Unique identifier for the WSI to remove.
             */
            void removeImage(const std::string& uid);

       protected:
            /**
             * @brief createFolderDirectoryArchitecture Prepare the folder structure with sub-folders
             * such as thumbnails or results.
             */
            void createFolderDirectoryArchitecture();
            /**
             * @brief saveThumbnails Iteratively saving on disk the thumbnail of each opened WSI.
             */
            void saveThumbnails();
            /**
             * @brief saveThumbnail Saving only the thumbnail of a specific WSI.
             * @param wsi_uid unique id of the WSI whose thumbnail should be saved.
             */
            void saveThumbnail(const std::string& wsi_uid);
       private:
            bool _temporary_dir_flag; /* Flag indicating if a project destination folder has been chosen by the user. */
            std::string _root_folder;  /* Location on disk where to save all data for the current project. */
            std::map<std::string, std::shared_ptr<WholeSlideImage>> _images; /* Loaded image objects. */
    };
} // End of namespace fast
