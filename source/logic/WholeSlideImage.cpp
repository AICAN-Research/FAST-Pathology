#include "WholeSlideImage.h"
#include <FAST/Importers/WholeSlideImageImporter.hpp>
#include <FAST/Visualization/ImagePyramidRenderer/ImagePyramidRenderer.hpp>
#include <FAST/Data/ImagePyramid.hpp>
#include <FAST/Data/Image.hpp>

namespace fast{
    WholeSlideImage::WholeSlideImage(const std::string filename): _filename(filename)
    {
        this->init();
    }

    WholeSlideImage::WholeSlideImage(const std::string filename, const QImage thumbnail):_filename(filename), _thumbnail(thumbnail)
    {
        auto importer = WholeSlideImageImporter::New();
        importer->setFilename(this->_filename);
        auto currImage = importer->updateAndGetOutputData<ImagePyramid>();
        this->_image = currImage;
        this->_metadata = this->_image->getMetadata(); // Can be dropped?
    }

    WholeSlideImage::~WholeSlideImage()
    {
    }

    void WholeSlideImage::init()
    {
        auto importer = WholeSlideImageImporter::New();
        importer->setFilename(this->_filename);
        auto currImage = importer->updateAndGetOutputData<ImagePyramid>();
        this->_image = currImage;
        this->_metadata = this->_image->getMetadata(); // Can be dropped?
        this->create_thumbnail();
    }

    void WholeSlideImage::create_thumbnail()
    {
        // @TODO: This is a little bit slow. Possible to speed it up? Bottleneck is probably the creation of thumbnails
        auto access = this->_image->getAccess(ACCESS_READ);
        auto input = access->getLevelAsImage(this->_image->getNrOfLevels() - 1);

        // try to convert to FAST Image -> QImage
//        QImage image(input->getWidth(), input->getHeight(), QImage::Format_RGB32);
        this->_thumbnail = QImage(input->getWidth(), input->getHeight(), QImage::Format_RGB32);

        // @TODO have to do some type conversion here, assuming float for now
        unsigned char *pixelData = this->_thumbnail.bits();

        ImageAccess::pointer new_access = input->getImageAccess(ACCESS_READ);
        void *inputData = new_access->get();
        uint nrOfComponents = input->getNrOfChannels();

        for (uint x = 0; x < (uint)input->getWidth(); x++) {
            for (uint y = 0; y < (uint)input->getHeight(); y++) {
                uint i = x + y * input->getWidth();
                for (uint c = 0; c < (uint)input->getNrOfChannels(); c++) {
                    float data;
                    data = ((uchar *) inputData)[i * nrOfComponents + c]; // assumes TYPE_UINT8
                    pixelData[i * 4 + (2-c)] = (unsigned char) data;  // TODO: NOTE (2-c) fixed BGR->RGB, but maybe there is a smarter solution?
                    pixelData[i * 4 + 3] = 255; // Alpha
                }
            }
        }
    }
} // End of namespace fast
