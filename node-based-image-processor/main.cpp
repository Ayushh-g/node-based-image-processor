#include "ImageEditorApp.h"

// Global pointer to store the application instance
static ImageEditorApp* g_AppInstance = nullptr;

// Function to retrieve the application instance from anywhere
ImageEditorApp* GetApplicationInstance()
{
    return g_AppInstance;
}

// Main function implementation
int main(int argc, char** argv)
{
    ImageEditorApp app;

    // Store the application instance in the global pointer
    g_AppInstance = &app;

    if (app.Create(1280, 720))
        return app.Run();

    // Clear the global pointer before exiting
    g_AppInstance = nullptr;

    return 1;
}