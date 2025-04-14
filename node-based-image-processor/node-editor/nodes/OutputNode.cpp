#include "OutputNode.h"
#include "../ImageDataManager.h"
#include <imgui.h>
#include <filesystem>
#include <../../ImageEditorApp.h>

// Windows headers for file dialog
#include <Windows.h>
#include <commdlg.h>

OutputNode::OutputNode(int id)
    : Node(id, "Output", ImColor(128, 195, 248))
{
    // Setup pins - only input as this is an output node
    AddInputPin("Image", PinType::Image);
}

OutputNode::~OutputNode()
{
    // Clean up OpenGL resources
    CleanupTexture();
}

void OutputNode::Process()
{
    // Get the image from the connected input node using ImageDataManager
    if (!Inputs.empty())
    {
        m_InputImage = ImageDataManager::GetInstance().GetImageData(Inputs[0].ID);
    }
    else
    {
        m_InputImage = cv::Mat();
    }

    // Generate a preview image (possibly scaled down)
    if (!m_InputImage.empty())
    {
        m_PreviewImage = m_InputImage.clone();

        // Update the preview texture
        UpdatePreviewTexture();
    }
}

void OutputNode::DrawNodeContent()
{
    // Display image preview if we have one
    if (!m_PreviewImage.empty() && m_PreviewTexture)
    {
        // Calculate preview size
        const float maxPreviewWidth = 200.0f;
        const float maxPreviewHeight = 150.0f;

        float aspectRatio = (float)m_PreviewImage.cols / (float)m_PreviewImage.rows;
        float previewWidth = min(maxPreviewWidth, (float)m_PreviewImage.cols);
        float previewHeight = previewWidth / aspectRatio;

        if (previewHeight > maxPreviewHeight)
        {
            previewHeight = maxPreviewHeight;
            previewWidth = previewHeight * aspectRatio;
        }

        // Draw the preview
        ImGui::Image(m_PreviewTexture, ImVec2(previewWidth, previewHeight));

        // Display image info
        ImGui::Text("Size: %d x %d", m_PreviewImage.cols, m_PreviewImage.rows);
        ImGui::Text("Channels: %d", m_PreviewImage.channels());

        // Output format selection
        const char* formats[] = { "JPEG", "PNG", "BMP" };
        ImGui::Combo("Format", &m_OutputFormat, formats, 3);

        // Format-specific controls
        if (m_OutputFormat == 0) // JPEG
        {
            ImGui::SliderInt("Quality", &m_JpegQuality, 1, 100);
            ImGui::SameLine();
            ImGui::TextDisabled("(?)");
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("Higher quality values result in less compression\n"
                    "but larger file sizes. 95 is high quality.");
            }
        }
        else if (m_OutputFormat == 1) // PNG
        {
            ImGui::SliderInt("Compression", &m_PngCompressionLevel, 0, 9);
            ImGui::SameLine();
            ImGui::TextDisabled("(?)");
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("0: No compression, 9: Maximum compression\n"
                    "Higher values result in smaller files but slower saving");
            }
        }

        // Show save feedback if we have saved an image
        if (!m_LastSavePath.empty() && m_SaveSuccess)
        {
            // Only show feedback for a few seconds
            const int feedbackDuration = 5; // seconds
            auto currentTime = std::time(nullptr);
            if (currentTime - m_SaveTimestamp < feedbackDuration)
            {
                // Extract filename without using std::filesystem
                std::string filename = m_LastSavePath;
                size_t lastSlash = filename.find_last_of("/\\");
                if (lastSlash != std::string::npos)
                    filename = filename.substr(lastSlash + 1);

                ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.0f, 1.0f), "Saved: %s", filename.c_str());
            }
        }

        // Save button
        if (ImGui::Button("Save Image"))
        {
            ShowSaveFileDialog();
        }
    }
    else
    {
        ImGui::Text("No input image");
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Connect an input to save an image");
    }
}

bool OutputNode::SaveImage(const std::string& path)
{
    if (m_InputImage.empty())
        return false;

    std::vector<int> params;

    // Set format-specific parameters
    if (m_OutputFormat == 0) // JPEG
    {
        params.push_back(cv::IMWRITE_JPEG_QUALITY);
        params.push_back(m_JpegQuality);
    }
    else if (m_OutputFormat == 1) // PNG
    {
        params.push_back(cv::IMWRITE_PNG_COMPRESSION);
        params.push_back(m_PngCompressionLevel);
    }

    // Try to save the image
    bool success = cv::imwrite(path, m_InputImage, params);

    // Store result info for feedback
    m_SaveSuccess = success;
    if (success) {
        m_LastSavePath = path;
        m_SaveTimestamp = std::time(nullptr);
    }

    return success;
}

bool OutputNode::ShowSaveFileDialog()
{
    if (m_InputImage.empty())
        return false;

    // Set default file extension based on selected format
    const char* fileExtension;
    const char* filterStr;

    switch (m_OutputFormat)
    {
    case 0: // JPEG
        fileExtension = ".jpg";
        filterStr = "JPEG Images\0*.jpg;*.jpeg\0All Files\0*.*\0";
        break;
    case 1: // PNG
        fileExtension = ".png";
        filterStr = "PNG Images\0*.png\0All Files\0*.*\0";
        break;
    case 2: // BMP
        fileExtension = ".bmp";
        filterStr = "BMP Images\0*.bmp\0All Files\0*.*\0";
        break;
    default:
        fileExtension = ".jpg";
        filterStr = "All Image Files\0*.jpg;*.jpeg;*.png;*.bmp\0All Files\0*.*\0";
        break;
    }

    // Prepare file name buffer with default name
    char filename[MAX_PATH] = "output";
    strcat_s(filename, fileExtension);

    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = filterStr;
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = "Save Image";
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
    ofn.lpstrDefExt = fileExtension + 1; // Skip the dot

    if (GetSaveFileNameA(&ofn))
    {
        // Save the image to the selected file
        return SaveImage(filename);
    }

    return false;
}

void OutputNode::UpdatePreviewTexture()
{
    // Clean up any existing texture
    CleanupTexture();

    if (m_PreviewImage.empty())
        return;

    // Verify we have valid image data
    if (m_PreviewImage.data == nullptr)
        return;

    // Convert image from OpenCV BGR format to RGBA for texture creation
    cv::Mat rgbImage;
    try {
        if (m_PreviewImage.channels() == 3)
            cv::cvtColor(m_PreviewImage, rgbImage, cv::COLOR_BGR2RGBA);
        else if (m_PreviewImage.channels() == 4)
            cv::cvtColor(m_PreviewImage, rgbImage, cv::COLOR_BGRA2RGBA);
        else if (m_PreviewImage.channels() == 1)
            cv::cvtColor(m_PreviewImage, rgbImage, cv::COLOR_GRAY2RGBA);
        else
            rgbImage = m_PreviewImage.clone(); // Just use as-is if format is unexpected
    }
    catch (const cv::Exception&) {
        // Handle conversion error - just return without creating texture
        return;
    }

    try {
        // Use the Application's texture creation API
        if (ImageEditorApp* app = ImageEditorApp::GetInstance()) {
            m_PreviewTexture = app->CreateTexture(rgbImage.data, rgbImage.cols, rgbImage.rows);
        }
    }
    catch (...) {
        // Silent failure - just don't update the texture
        return;
    }
}

void OutputNode::CleanupTexture()
{
    if (m_PreviewTexture)
    {
        try {
            // Use the Application's texture destruction API through ImageEditorApp instance
            if (ImageEditorApp* app = ImageEditorApp::GetInstance()) {
                app->DestroyTexture(m_PreviewTexture);
            }
        }
        catch (...) {
            // Silent catch for any errors during cleanup
        }
        m_PreviewTexture = nullptr;
    }
}

cv::Mat OutputNode::GetConnectedImage()
{
    // Get input pin
    if (Inputs.empty())
        return cv::Mat();

    auto& inputPin = Inputs[0];

    // Use ImageDataManager to get the image
    return ImageDataManager::GetInstance().GetImageData(inputPin.ID);
}