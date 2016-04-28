#pragma once
#include "3dmath.h"

class Image
{
public:
        unsigned int width, height;
        unsigned char *image;
        Image (unsigned int Width, unsigned int Height);
        ~Image ();
        void WriteToTga (const char *filename);
};
