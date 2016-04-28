#include "scene.h"

const unsigned int width = 320;
const unsigned int height = 180;
const unsigned int samples = 10;
const char scene_file[] = "scene.txt";
const char image_file[] = "output.tga";

int main ()
{
        InitLog ();
        Log ("Path tracing started");
        Log ("Image size: %u x %u", width, height);
        Image image (width, height);
        Scene scene (scene_file);
        scene.Render (image, samples);
        image.WriteToTga (image_file);
        Log ("Close path tracing");
        CloseLog ();
        return 0;
}
