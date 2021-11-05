//
// Created by dbouget on 07.10.2021.
//

#include "WholeSlideImage.h"
#include <FAST/Importers/WholeSlideImageImporter.hpp>
#include <FAST/Visualization/ImagePyramidRenderer/ImagePyramidRenderer.hpp>
#include <FAST/Data/ImagePyramid.hpp>
#include <FAST/Data/Image.hpp>


namespace fast{
    WholeSlideImage::WholeSlideImage(const std::string filename): _filename(filename)
    {
        this->init();
//        // @TODO. Renderers memory release leads to segfault...
//        this->memory_load();
//        this->create_thumbnail();
////        this->memory_unload();
    }

    WholeSlideImage::~WholeSlideImage()
    {
        // @FIXME. Only uncommented for debugging purposes, the same process happens by default regardless.
        // Segfault is generated here if multiple WSI are loaded, but not all were clicked (and viewed).
        // If all are clicked and viewed/unviewed, it seems to work fine...
        this->memory_unload();
    }

    void WholeSlideImage::init()
    {
        auto importer = WholeSlideImageImporter::New();
        importer->setFilename(this->_filename);
        auto currImage = importer->updateAndGetOutputData<ImagePyramid>();
        this->_image = currImage;
        this->_metadata = this->_image->getMetadata(); // Can be dropped?
        this->_format = this->_metadata["openslide.vendor"];
        this->compute_magnification_level();
        this->create_thumbnail();
    }

    void WholeSlideImage::memory_load()
    {
        auto importer = WholeSlideImageImporter::New();
        importer->setFilename(this->_filename);
        auto currImage = importer->updateAndGetOutputData<ImagePyramid>();
        this->_image = currImage;
        this->_metadata = this->_image->getMetadata(); // Can be dropped?
        this->_format = this->_metadata["openslide.vendor"];
        this->compute_magnification_level();

        auto renderer = ImagePyramidRenderer::New();
        renderer->setSharpening(false); // Parameters for sharpening, coming from where?
        renderer->setInputData(this->_image);
        renderer->setSynchronizedRendering(false);
        _renderers["WSI"] = renderer;
        this->_renderers_types["WSI"] = "ImagePyramidRenderer";
    }

    void WholeSlideImage::load_renderer()
    {
        auto renderer = ImagePyramidRenderer::New();
        renderer->setSharpening(false); // Parameters for sharpening, coming from where?
        renderer->setInputData(this->_image);
        renderer->setSynchronizedRendering(false);
        this->_renderers["WSI"] = renderer;
        this->_renderers_types["WSI"] = "ImagePyramidRenderer";
    }

    void WholeSlideImage::unload_renderer()
    {
        if(!this->_renderers.empty())
        {
            this->_renderers.clear();
            this->_renderers_types.clear();
        }
    }

    void WholeSlideImage::memory_unload()
    {
        std::cout<<"Just before clearing renderers"<<std::endl;
        if(!_renderers.empty()) {
            _renderers.clear();
            _renderers_types.clear();
        }
        _image.reset();
    }

    const std::string WholeSlideImage::get_renderer_type(const std::string& name){
        if(has_renderer(name))
            return this->_renderers_types[name];
        else
            return "";
    }

    void WholeSlideImage::compute_magnification_level() {
        float magnification_lvl = 0.0f;

        // TODO: Should check which formats are supported by OpenSlide, and do this for all (in a generalized matter...)
        if ((this->_format == "generic-tiff") || (this->_format == "philips") || (this->_format == "ventana"))
        {
            int level_count = this->_image->getNrOfLevels(); //(int)stof(metadata["openslide.level-count"]);

            // assuming each 400X resolution correspond to 50000 cm, and thus 200x => 100000 cm ... (for .tiff format)
            // can predict which WSI resolution image is captured with, as some models is trained on 200x images
            std::vector<float> templateResVector;
            float resFractionValue = 50000;
            for (int i = 0; i < level_count; i++) {
                templateResVector.push_back(resFractionValue * ((float) i + 1));
            }

            auto resolution = 1;  // needed to initialize outside of if statement -> set some silly dummy value
            //auto resolution = std::stof(m_image->getMetadata("tiff.XResolution")); //(int)stof(metadata["tiff.XResolution"]);

            if (this->_format == "generic-tiff") {
                resolution = std::stof(this->_image->getMetadata("tiff.XResolution")); //(int)stof(metadata["tiff.XResolution"]);
            } else if ((this->_format == "phillips") || (this->_format == "ventata")) {
                resolution = std::stof(this->_metadata["mpp-x"]);
            }

            // find closest value => get magnification level
            auto i = min_element(begin(templateResVector), end(templateResVector), [=](int x, int y) {
                return abs(x - resolution) < abs(y - resolution);
            });
            float location = std::distance(begin(templateResVector), i);

            magnification_lvl = 40.0f / pow(2.0f, location);

        } else if (this->_format == "aperio") {
            magnification_lvl = std::stof(this->_metadata["aperio.AppMag"]); //atof(openslide_get_property_value(osr, "aperio.AppMag"));
        } else if (this->_format == "hamamatsu") {
            std::cout << "Vendor: hamamatsu" << std::endl;
            magnification_lvl = std::stof(this->_metadata["openslide.objective-power"]);
            std::cout << "Magn lvl: " << magnification_lvl << std::endl;
        } else {  //"TODO: Make this more general, test different image formats to see how the magn_lvl metadata vary"
            std::cout << "WSI format not set, uses default format: " << this->_metadata["aperio.AppMag"] << std::endl;
            magnification_lvl = std::stof(this->_metadata["aperio.AppMag"]);
        }
        this->_magnification_level = magnification_lvl;
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

    void WholeSlideImage::insert_renderer(std::string renderer_name, std::string renderer_type, std::shared_ptr<Renderer> renderer)
    {
        this->_renderers_types[renderer_name] = renderer_type;
        this->_renderers[renderer_name] = renderer;
    }
} // End of namespace fast
