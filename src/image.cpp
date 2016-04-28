#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "image.h"
#include "log.h"

Image::Image (unsigned int Width, unsigned int Height) :
        width (Width),
        height (Height)
{
        image = new unsigned char[3 * width * height];
}

Image::~Image ()
{
        delete[] image;
}

void Image::WriteToTga (const char *filename)
{
        FILE *f;
        fopen_s (&f, filename, "wb");
        if (f)
                Log ("Writing image to %s", filename);
        else
        {
                Log ("Can not open file %s", filename);
                exit (1);
        }

        char header[18] = {};
        header[2] = 2; //true color
        *(unsigned short int *)(header + 12) = (unsigned short int)width;  //width
        *(unsigned short int *)(header + 14) = (unsigned short int)height; //height
        header[16] = 24; //color depth

        fwrite ((void *)header, 1, 18, f);
        fwrite ((void *)image, 1, 3 * width * height, f);

        Log ("Image writed successfully");
        fclose (f);
}
