#include "OutputNode.h"
#include "../ImageDataManager.h"
#include <imgui.h>
#include <filesystem>

// Windows headers for file dialog
#include <Windows.h>
#include <commdlg.h>

// OpenGL headers for texture handling
#include <GL/glew.h>
#include <GLFW/glfw3.h>

OutputNode::OutputNode(int id)
    : Node(id, "Output", ImColor(128, 195, 248))
{
    // Setup pins - only input as this is an output node
    AddInputPin("Image", PinType::Image);
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

        // Quality slider for JPEG
        if (m_OutputFormat == 0) // JPEG
        {
            ImGui::SliderInt("Quality", &m_JpegQuality, 1, 100);
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
        params.push_back(3); // Medium compression
    }

    // Try to save the image
    return cv::imwrite(path, m_InputImage, params);
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

    // Convert image from OpenCV BGR format to RGB for OpenGL
    cv::Mat rgbImage;
    if (m_PreviewImage.channels() == 3)
        cv::cvtColor(m_PreviewImage, rgbImage, cv::COLOR_BGR2RGB);
    else if (m_PreviewImage.channels() == 4)
        cv::cvtColor(m_PreviewImage, rgbImage, cv::COLOR_BGRA2RGBA);
    else if (m_PreviewImage.channels() == 1)
        cv::cvtColor(m_PreviewImage, rgbImage, cv::COLOR_GRAY2RGB);
    else
        rgbImage = m_PreviewImage.clone(); // Just use as-is if format is unexpected

    // Create OpenGL texture
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Setup texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Upload image data to texture
    GLenum format = (rgbImage.channels() == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, rgbImage.cols, rgbImage.rows, 0, format, GL_UNSIGNED_BYTE, rgbImage.data);

    // Store the texture ID as ImTextureID for ImGui
    m_PreviewTexture = (ImTextureID)(intptr_t)textureID;
}

void OutputNode::CleanupTexture()
{
    if (m_PreviewTexture)
    {
        GLuint textureID = (GLuint)(intptr_t)m_PreviewTexture;
        glDeleteTextures(1, &textureID);
        m_PreviewTexture = nullptr;
    }
}

cv::Mat OutputNode::GetConnectedImage()
{
    // Get input pin
    if (Inputs.empty())
        return cv::Mat(); // Return empty image if no inputs

    auto& inputPin = Inputs[0];

    // Check if pin is connected
    // This will need to be updated when link management is fully implemented
    // For now, we'll just return an empty image as a placeholder
    return cv::Mat();
}