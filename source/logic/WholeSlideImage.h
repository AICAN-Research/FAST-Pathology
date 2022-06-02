#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <map>
#include <QImage>

namespace fast{
    class ImagePyramid;

    class WholeSlideImage {
        public:
            WholeSlideImage(const std::string filename);
            WholeSlideImage(const std::string filename, const QImage thumbnail);
            ~WholeSlideImage();

            std::string get_filename(){return _filename;}
            QImage get_thumbnail() {return _thumbnail;}
            std::shared_ptr<ImagePyramid> get_image_pyramid(){return _image;}

            void init();

        private:
            /**
             * Gets the thumbnail image and stores it as a QImage.
             * @return
             */
            void create_thumbnail();

        private:
            const std::string _filename; /* Disk location for the whole slide image*/
            std::map<std::string, std::string> _metadata; /* */
            std::shared_ptr<ImagePyramid> _image; /* Loaded WSI */
            QImage _thumbnail; /* Thumbnail for the WSI */
    };
}