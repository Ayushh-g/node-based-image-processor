#include "InputNode.h"
#include "../ImageDataManager.h"
#include <imgui.h>
#include <algorithm>
#include <string>

// Windows headers for file dialog
#include <Windows.h>
#include <commdlg.h>

// OpenGL headers for texture handling
#include <GL/glew.h>
#include <GLFW/glfw3.h>


InputNode::InputNode(int id)
    : Node(id, "Image Input", ImColor(255, 128, 128))
{
    // Setup pins - only output as this is a source node
    AddOutputPin("Image", PinType::Image);

    // Initialize an empty image to start with
    m_Image = cv::Mat(100, 100, CV_8UC3, cv::Scalar(0, 0, 0));
}

void InputNode::Process()
{
    // For input nodes, processing is just providing the loaded image
    // The image is already loaded in LoadImageFile, so just pass it on
    m_OutputImage = m_Image.clone();

    // Set the image data in the ImageDataManager for the output pin
    if (!m_OutputImage.empty() && !Outputs.empty())
    {
        ImageDataManager::GetInstance().SetImageData(Outputs[0].ID, m_OutputImage);
    }
}

bool InputNode::LoadImageFile(const std::string& path)
{
    // Load image using OpenCV
    cv::Mat loadedImage = cv::imread(path, cv::IMREAD_UNCHANGED);
    if (loadedImage.empty())
    {
        return false;
    }

    // Store loaded image
    m_Image = loadedImage;
    m_FilePath = path;

    // Extract file format from path
    size_t dot = path.find_last_of('.');
    m_FileFormat = (dot != std::string::npos) ? path.substr(dot + 1) : "unknown";
    std::transform(m_FileFormat.begin(), m_FileFormat.end(), m_FileFormat.begin(), ::toupper);

    m_ImageLoaded = true;

    // Update the preview texture
    UpdatePreviewTexture();

    // Mark node as dirty to ensure it's processed
    Dirty = true;

    return true;
}

bool InputNode::ShowOpenFileDialog()
{
    // Open file dialog
    char filename[MAX_PATH] = "";

    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = "Image Files\0*.jpg;*.jpeg;*.png;*.bmp\0All Files\0*.*\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = "Open Image";
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (GetOpenFileNameA(&ofn))
    {
        return LoadImageFile(filename);
    }

    return false;
}

void InputNode::DrawNodeContent()
{
    // Display image preview if loaded
    if (m_ImageLoaded)
    {
        // Calculate preview size
        const float maxPreviewWidth = 200.0f;
        const float maxPreviewHeight = 150.0f;

        float aspectRatio = (float)m_Image.cols / (float)m_Image.rows;
        float previewWidth = min(maxPreviewWidth, (float)m_Image.cols);
        float previewHeight = previewWidth / aspectRatio;

        if (previewHeight > maxPreviewHeight)
        {
            previewHeight = maxPreviewHeight;
            previewWidth = previewHeight * aspectRatio;
        }

        // Draw the preview
        if (m_PreviewTexture)
        {
            ImGui::Image(m_PreviewTexture, ImVec2(previewWidth, previewHeight));
        }

        // Display image metadata
        ImGui::Text("Size: %d x %d", m_Image.cols, m_Image.rows);
        ImGui::Text("Channels: %d", m_Image.channels());
        ImGui::Text("Format: %s", m_FileFormat.c_str());
        ImGui::Text("File Size: %.2f KB", GetSizeBytes() / 1024.0f);

        // Add a button to reload or change the image
        if (ImGui::Button("Change Image"))
        {
            ShowOpenFileDialog();
        }
    }
    else
    {
        ImGui::Text("No image loaded");
        if (ImGui::Button("Load Image"))
        {
            ShowOpenFileDialog();
        }
    }
}

void InputNode::OnSelected()
{
    // Called when the node is selected in the editor
    // We can use this to update the UI or preview if needed
}

void InputNode::UpdatePreviewTexture()
{
    // Clean up any existing texture
    CleanupTexture();

    if (m_Image.empty())
        return;

    // Convert image from OpenCV BGR format to RGB for OpenGL
    cv::Mat rgbImage;
    if (m_Image.channels() == 3)
        cv::cvtColor(m_Image, rgbImage, cv::COLOR_BGR2RGB);
    else if (m_Image.channels() == 4)
        cv::cvtColor(m_Image, rgbImage, cv::COLOR_BGRA2RGBA);
    else if (m_Image.channels() == 1)
        cv::cvtColor(m_Image, rgbImage, cv::COLOR_GRAY2RGB);
    else
        rgbImage = m_Image.clone(); // Just use as-is if format is unexpected

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

void InputNode::CleanupTexture()
{
    if (m_PreviewTexture)
    {
        GLuint textureID = (GLuint)(intptr_t)m_PreviewTexture;
        glDeleteTextures(1, &textureID);
        m_PreviewTexture = nullptr;
    }
}