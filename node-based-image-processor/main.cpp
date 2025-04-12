#include "ImageEditorApp.h"

// Main function implementation
int main(int argc, char** argv)
{
    ImageEditorApp app;

    if (app.Create(1280, 720))
        return app.Run();

    return 1;
}