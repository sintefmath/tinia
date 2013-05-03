#pragma once
#include <QTextStream>
#include <QString>

namespace tinia { namespace qtcontroller {

    /**
     * Abstract interface for an ImageSource. The most
     * used subclass is the OpenGLServerGrabber
     */
    class ImageSource {
    public:

        virtual ~ImageSource() {}

        /**
         * Fills os with the next image. The image should be encoded as PNG and then as base64. 
         */
        virtual void getImageAsText(QTextStream& os, unsigned int width, unsigned int height, QString key) = 0;
    };
}}