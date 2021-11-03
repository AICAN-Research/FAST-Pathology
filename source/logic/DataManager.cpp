//
// Created by dbouget on 07.10.2021.
//

#include "DataManager.h"
#include <FAST/Importers/WholeSlideImageImporter.hpp>
#include <FAST/Visualization/ImagePyramidRenderer/ImagePyramidRenderer.hpp>
#include <FAST/Data/ImagePyramid.hpp>
#include <source/utils/utilities.h>

namespace fast {

/**
 * Static methods should be defined outside the class.
 */

    DataManager* DataManager::_pinstance{nullptr};
    std::mutex DataManager::_mutex;

/**
 * The first time we call GetInstance we will lock the storage location
 *      and then we make sure again that the variable is null and then we
 *      set the value. RU:
 */
    DataManager *DataManager::GetInstance()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_pinstance == nullptr)
        {
            _pinstance = new DataManager();
        }
        return _pinstance;
    }

    void DataManager::doEmpty()
    {
//        for(auto it=_images.begin(); it != _images.end(); it++)
//            it->second->memory_unload();
        _images.clear();
        _wsi_filenames_list.clear();
    }

    const std::string DataManager::IncludeImage(std::string currFileName){
        auto image(std::make_shared<WholeSlideImage>(currFileName));
        std::string img_name_short = splitCustom(currFileName, "/").back();
        bool in_use = _images.find(img_name_short) != _images.end();
        if(in_use)
        {
            bool new_in_use = true;
            while(new_in_use)
            {
                int rand_int = rand() % 10000;
                std::string new_name = img_name_short;
                new_name.append(std::to_string(rand_int));
                if(_images.find(new_name) == _images.end())
                {
                    new_in_use = false;
                    img_name_short = new_name;
                }
            }
        }

//        image->memory_load();
//        image->memory_unload();
//        image->memory_load();
//        this->_images[std::to_string(this->_images.size())] = image;
        this->_images[img_name_short] = image;
        _wsi_filenames_list.push_back(currFileName);

        return img_name_short;
//        this->_images[std::to_string(this->_images.size())] = std::make_unique<WholeSlideImage>(currFileName);
//        this->_images[std::to_string(this->_images.size() - 1)]->memory_unload();
//        this->_images[std::to_string(this->_images.size() - 1)]->memory_load();

//        // Import image from file using the ImageFileImporter
//        auto importer = WholeSlideImageImporter::New();
//        importer->setFilename(currFileName);
//        auto currImage = importer->updateAndGetOutputData<ImagePyramid>();
//
//        // for reading of multiple WSIs, only render first one
//        // current WSI (global)
//        this->_filename = currFileName;
//
//        this->_image = currImage;
//        this->_metadata = this->_image->getMetadata(); // get metadata
//
//        auto renderer = ImagePyramidRenderer::New();
//        renderer->setSharpening(false);
//        renderer->setInputData(this->_image);
//        this->_renderers["WSI"] = renderer;
//        std::cout<<"plop\n";

////         TODO: Something here results in me not being able to run analysis on new images (after the first)
//        removeAllRenderers();
//        this->_renderers_types["WSI"] = "ImagePyramidRenderer";
//        insertRenderer("WSI", renderer);
//        getView(0)->reinitialize(); // Must call this after removing all renderers
//
//        this->_wsiFormat = metadata["openslide.vendor"]; // get WSI format
//        this->_magn_lvl = getMagnificationLevel(); // get magnification level of current WSI

    }

    void DataManager::RemoveImage(std::string uid)
    {
        _wsi_filenames_list.erase(std::remove(_wsi_filenames_list.begin(), _wsi_filenames_list.end(), _images[uid]->get_filename()), _wsi_filenames_list.end());
        _images.erase(uid);
    }
}
