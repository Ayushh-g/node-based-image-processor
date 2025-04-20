#include "InputNode.h"
#include "../ImageDataManager.h"
#include <imgui.h>
#include <algorithm>
#include <string>
#include <../../ImageEditorApp.h>

// Windows headers for file dialog
#include <Windows.h>
#include <commdlg.h>

InputNode::InputNode(int id)
    : Node(id, "Image Input", ImColor(255, 128, 128))
{
    // Setup pins - only output as this is a source node
    AddOutputPin("Image", PinType::Image);
    
    // Initialize an empty image to start with
    m_Image = cv::Mat(100, 100, CV_8UC3, cv::Scalar(0, 0, 0));
}

// Add explicit destructor for proper cleanup
InputNode::~InputNode()
{
    // Clean up OpenGL resources
    CleanupTexture();
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
        m_LastErrorMessage = "Failed to load image: " + path;
        return false;
    }

    // Auto-resize large images if enabled
    if (m_EnableAutoResize)
    {
        int maxDim = std::max<int>(loadedImage.cols, loadedImage.rows);
        if (maxDim > m_MaxDimension)
        {
            double scale = (double)m_MaxDimension / maxDim;
            cv::Mat resizedImage;
            cv::resize(loadedImage, resizedImage, cv::Size(), scale, scale, cv::INTER_AREA);
            loadedImage = resizedImage;
        }
    }
    
    // Store loaded image
    m_Image = loadedImage;
    m_FilePath = path;
    
    // Extract file format from path
    size_t dot = path.find_last_of('.');
    m_FileFormat = (dot != std::string::npos) ? path.substr(dot + 1) : "unknown";
    std::transform(m_FileFormat.begin(), m_FileFormat.end(), m_FileFormat.begin(), ::toupper);
    
    m_ImageLoaded = true;
    m_LastErrorMessage.clear();
    
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
    ImGui::PushID(ID.AsPointer()); // Ensure unique IDs for widgets within this node instance
    // Display image preview if loaded
    if (m_ImageLoaded)
    {
        // Calculate preview size
        const float maxPreviewWidth = 200.0f;
        const float maxPreviewHeight = 150.0f;
        
        float aspectRatio = (float)m_Image.cols / (float)m_Image.rows;
        float previewWidth = std::min<int>(maxPreviewWidth, (float)m_Image.cols);
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
        
        // Auto-resize options
        if (ImGui::Checkbox("Resize large images", &m_EnableAutoResize))
        {
            // User changed resize option - reload image if we have one
            if (!m_FilePath.empty())
            {
                LoadImageFile(m_FilePath);
            }
        }
        
        const float itemWidth = 150.0f; // Define a width for the widgets
        if (m_EnableAutoResize)
        {
            ImGui::SameLine();
            ImGui::PushItemWidth(itemWidth);
            if (ImGui::SliderInt("Max dimension", &m_MaxDimension, 256, 4096))
            {
                // User changed max dimension - reload image if we have one
                if (!m_FilePath.empty())
                {
                    LoadImageFile(m_FilePath);
                }
            }
            ImGui::PopItemWidth();
        }

        // Add a button to reload or change the image
        if (ImGui::Button("Change Image"))
        {
            ShowOpenFileDialog();
        }
    }
    else
    {
        ImGui::Text("No image loaded");
        
        // Display error message if any
        if (!m_LastErrorMessage.empty())
        {
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "%s", m_LastErrorMessage.c_str());
        }
        
        // Auto-resize options
        const float itemWidth = 150.0f; // Define a width for the widgets
        ImGui::Checkbox("Resize large images", &m_EnableAutoResize);
        if (m_EnableAutoResize)
        {
            ImGui::SameLine();
            ImGui::PushItemWidth(itemWidth);
            ImGui::SliderInt("Max dimension", &m_MaxDimension, 256, 4096);
            ImGui::PopItemWidth();
        }

        if (ImGui::Button("Load Image"))
        {
            ShowOpenFileDialog();
        }
    }
    ImGui::PopID(); // Pop the node instance ID
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
    
    // Verify we have valid image data
    if (m_Image.data == nullptr) {
        m_LastErrorMessage = "Invalid image data";
        return;
    }
    
    // Convert image from OpenCV BGR format to RGB for texture creation
    cv::Mat rgbImage;
    try {
        if (m_Image.channels() == 3)
            cv::cvtColor(m_Image, rgbImage, cv::COLOR_BGR2RGBA); // Convert to RGBA
        else if (m_Image.channels() == 4)
            cv::cvtColor(m_Image, rgbImage, cv::COLOR_BGRA2RGBA);
        else if (m_Image.channels() == 1)
            cv::cvtColor(m_Image, rgbImage, cv::COLOR_GRAY2RGBA);
        else
            rgbImage = m_Image.clone(); // Just use as-is if format is unexpected
    }
    catch (const cv::Exception& e) {
        m_LastErrorMessage = "Image conversion error: " + std::string(e.what());
        return;
    }
    
    try {
        // Get the application instance using GetInstance()
        if (ImageEditorApp* app = ImageEditorApp::GetInstance()) {
            // Use the Application's texture creation API
            m_PreviewTexture = app->CreateTexture(rgbImage.data, rgbImage.cols, rgbImage.rows);
            if (!m_PreviewTexture) {
                m_LastErrorMessage = "Failed to create texture";
            }
        } else {
            m_LastErrorMessage = "Application instance not available";
        }
    }
    catch (const std::exception& e) {
        m_LastErrorMessage = "Texture creation error: " + std::string(e.what());
    }
    catch (...) {
        m_LastErrorMessage = "Unknown texture creation error";
    }
}

void InputNode::CleanupTexture()
{
    if (m_PreviewTexture)
    {
        try {
            // Use GetInstance() to get the application instance
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
