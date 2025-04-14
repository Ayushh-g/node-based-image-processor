#include "ImageEditorApp.h"

// Main function implementation
int main(int argc, char** argv)
{
    // Create a single instance of the application using the singleton pattern
    ImageEditorApp app;

    if (app.Create(1280, 720))
        return app.Run();

    return 1;
}