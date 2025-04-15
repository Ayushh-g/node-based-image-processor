#include "ColorChannelSplitterNode.h"
#include "../ImageDataManager.h"
#include "../../ImageEditorApp.h"  // For app instance
#include <imgui.h>
#include <opencv2/imgproc.hpp>

ColorChannelSplitterNode::ColorChannelSplitterNode(int id)
    : Node(id, "Color Channel Splitter", ImColor(255, 180, 50))
{
    // Setup pins
    AddInputPin("Image", PinType::Image);
    AddOutputPin("Red", PinType::Image);
    AddOutputPin("Green", PinType::Image);
    AddOutputPin("Blue", PinType::Image);
    AddOutputPin("Alpha", PinType::Image);
}

void ColorChannelSplitterNode::Process()
{
    // Get input image from the ImageDataManager
    if (!Inputs.empty())
    {
        m_InputImage = ImageDataManager::GetInstance().GetImageData(Inputs[0].ID);
    }
    else
    {
        m_InputImage = cv::Mat();
    }
    
    if (m_InputImage.empty())
    {
        // No input image, clear outputs
        m_RedChannel = cv::Mat();
        m_GreenChannel = cv::Mat();
        m_BlueChannel = cv::Mat();
        m_AlphaChannel = cv::Mat();
        CleanupTextures();
        return;
    }
    
    // Split the image into its color channels
    std::vector<cv::Mat> channels;
    cv::split(m_InputImage, channels);
    
    // Assign channels based on input image type
    int numChannels = m_InputImage.channels();
    
    // Initialize all channels to empty
    m_RedChannel = cv::Mat();
    m_GreenChannel = cv::Mat();
    m_BlueChannel = cv::Mat();
    m_AlphaChannel = cv::Mat();
    
    // BGR or BGRA image (OpenCV's default format)
    if (numChannels >= 3)
    {
        m_BlueChannel = channels[0];
        m_GreenChannel = channels[1];
        m_RedChannel = channels[2];
        
        if (numChannels == 4)
        {
            m_AlphaChannel = channels[3];
        }
    }
    else if (numChannels == 1)
    {
        // Grayscale image - use the same channel for R, G, B
        m_RedChannel = channels[0];
        m_GreenChannel = channels[0];
        m_BlueChannel = channels[0];
    }
    
    // If grayscale output is enabled, convert each channel to grayscale
    if (m_OutputGrayscale)
    {
        if (!m_RedChannel.empty())
            m_RedChannel = m_RedChannel.clone(); // Already grayscale
        if (!m_GreenChannel.empty())
            m_GreenChannel = m_GreenChannel.clone(); // Already grayscale
        if (!m_BlueChannel.empty())
            m_BlueChannel = m_BlueChannel.clone(); // Already grayscale
        if (!m_AlphaChannel.empty())
            m_AlphaChannel = m_AlphaChannel.clone(); // Already grayscale
    }
    else
    {
        // Convert single-channel images to 3-channel for visualization
        if (!m_RedChannel.empty())
        {
            cv::Mat zeros = cv::Mat::zeros(m_RedChannel.size(), m_RedChannel.type());
            std::vector<cv::Mat> redChannels = {zeros, zeros, m_RedChannel};
            cv::merge(redChannels, m_RedChannel);
        }
        
        if (!m_GreenChannel.empty())
        {
            cv::Mat zeros = cv::Mat::zeros(m_GreenChannel.size(), m_GreenChannel.type());
            std::vector<cv::Mat> greenChannels = {zeros, m_GreenChannel, zeros};
            cv::merge(greenChannels, m_GreenChannel);
        }
        
        if (!m_BlueChannel.empty())
        {
            cv::Mat zeros = cv::Mat::zeros(m_BlueChannel.size(), m_BlueChannel.type());
            std::vector<cv::Mat> blueChannels = {m_BlueChannel, zeros, zeros};
            cv::merge(blueChannels, m_BlueChannel);
        }
        
        if (!m_AlphaChannel.empty())
        {
            // For alpha, we'll create a grayscale visualization
            m_AlphaChannel = m_AlphaChannel.clone();
        }
    }
    
    // Set the output images in the ImageDataManager
    if (!Outputs.empty() && Outputs.size() >= 4)
    {
        ImageDataManager::GetInstance().SetImageData(Outputs[0].ID, m_RedChannel);
        ImageDataManager::GetInstance().SetImageData(Outputs[1].ID, m_GreenChannel);
        ImageDataManager::GetInstance().SetImageData(Outputs[2].ID, m_BlueChannel);
        ImageDataManager::GetInstance().SetImageData(Outputs[3].ID, m_AlphaChannel);
    }
    
    // Update preview textures
    UpdatePreviewTextures();
}

void ColorChannelSplitterNode::DrawNodeContent()
{
    ImGui::Checkbox("Output as Grayscale", &m_OutputGrayscale);
    
    if (ImGui::IsItemEdited())
    {
        Dirty = true;
    }
    
    ImGui::Separator();

    // Add checkbox for preview
    ImGui::Checkbox("Show Previews", &m_ShowPreview);

    if (m_ShowPreview) {
        // Calculate small preview size
    const float previewWidth = 80.0f;
    float previewHeight = previewWidth;
    
    if (!m_InputImage.empty())
    {
        float aspectRatio = (float)m_InputImage.cols / (float)m_InputImage.rows;
        previewHeight = previewWidth / aspectRatio;
    }
    
    ImVec2 previewSize(previewWidth, previewHeight);
    
    // Display channel previews in a 2x2 grid
    if (m_RedTexture)
    {
        ImGui::Text("Red Channel");
        ImGui::Image(m_RedTexture, previewSize);
    }
    
    ImGui::SameLine();
    
    if (m_GreenTexture)
    {
        ImGui::Text("Green Channel");
        ImGui::Image(m_GreenTexture, previewSize);
    }
    
    if (m_BlueTexture)
    {
        ImGui::Text("Blue Channel");
        ImGui::Image(m_BlueTexture, previewSize);
    }
    
    ImGui::SameLine();
    
    if (m_AlphaTexture)
    {
        ImGui::Text("Alpha Channel");
        ImGui::Image(m_AlphaTexture, previewSize);
    }
    else
    {
        ImGui::SameLine();
        ImGui::BeginGroup();
        ImGui::Text("Alpha Channel");
        ImGui::Text("(None)");
        ImGui::EndGroup();
    }
    } // End if(m_ShowPreview)
}

void ColorChannelSplitterNode::UpdatePreviewTextures()
{
    // Clean up any existing textures
    CleanupTextures();
    
    // Get app instance for texture operations
    ImageEditorApp* app = ImageEditorApp::GetInstance();
    if (!app) return;
    
    // Create textures for each channel
    if (!m_RedChannel.empty())
    {
        cv::Mat displayImage;
        if (m_RedChannel.channels() == 3)
            cv::cvtColor(m_RedChannel, displayImage, cv::COLOR_BGR2RGBA);
        else if (m_RedChannel.channels() == 4)
            cv::cvtColor(m_RedChannel, displayImage, cv::COLOR_BGRA2RGBA);
        else if (m_RedChannel.channels() == 1)
            cv::cvtColor(m_RedChannel, displayImage, cv::COLOR_GRAY2RGBA);
        else
            displayImage = m_RedChannel.clone();
            
        m_RedTexture = app->CreateTexture(displayImage.data, displayImage.cols, displayImage.rows);
    }
    
    if (!m_GreenChannel.empty())
    {
        cv::Mat displayImage;
        if (m_GreenChannel.channels() == 3)
            cv::cvtColor(m_GreenChannel, displayImage, cv::COLOR_BGR2RGBA);
        else if (m_GreenChannel.channels() == 4)
            cv::cvtColor(m_GreenChannel, displayImage, cv::COLOR_BGRA2RGBA);
        else if (m_GreenChannel.channels() == 1)
            cv::cvtColor(m_GreenChannel, displayImage, cv::COLOR_GRAY2RGBA);
        else
            displayImage = m_GreenChannel.clone();
            
        m_GreenTexture = app->CreateTexture(displayImage.data, displayImage.cols, displayImage.rows);
    }
    
    if (!m_BlueChannel.empty())
    {
        cv::Mat displayImage;
        if (m_BlueChannel.channels() == 3)
            cv::cvtColor(m_BlueChannel, displayImage, cv::COLOR_BGR2RGBA);
        else if (m_BlueChannel.channels() == 4)
            cv::cvtColor(m_BlueChannel, displayImage, cv::COLOR_BGRA2RGBA);
        else if (m_BlueChannel.channels() == 1)
            cv::cvtColor(m_BlueChannel, displayImage, cv::COLOR_GRAY2RGBA);
        else
            displayImage = m_BlueChannel.clone();
            
        m_BlueTexture = app->CreateTexture(displayImage.data, displayImage.cols, displayImage.rows);
    }
    
    if (!m_AlphaChannel.empty())
    {
        cv::Mat displayImage;
        if (m_AlphaChannel.channels() == 3)
            cv::cvtColor(m_AlphaChannel, displayImage, cv::COLOR_BGR2RGBA);
        else if (m_AlphaChannel.channels() == 4)
            cv::cvtColor(m_AlphaChannel, displayImage, cv::COLOR_BGRA2RGBA);
        else if (m_AlphaChannel.channels() == 1)
            cv::cvtColor(m_AlphaChannel, displayImage, cv::COLOR_GRAY2RGBA);
        else
            displayImage = m_AlphaChannel.clone();
            
        m_AlphaTexture = app->CreateTexture(displayImage.data, displayImage.cols, displayImage.rows);
    }
}

void ColorChannelSplitterNode::CleanupTextures()
{
    ImageEditorApp* app = ImageEditorApp::GetInstance();
    if (!app) return;
    
    if (m_RedTexture)
    {
        app->DestroyTexture(m_RedTexture);
        m_RedTexture = nullptr;
    }
    
    if (m_GreenTexture)
    {
        app->DestroyTexture(m_GreenTexture);
        m_GreenTexture = nullptr;
    }
    
    if (m_BlueTexture)
    {
        app->DestroyTexture(m_BlueTexture);
        m_BlueTexture = nullptr;
    }
    
    if (m_AlphaTexture)
    {
        app->DestroyTexture(m_AlphaTexture);
        m_AlphaTexture = nullptr;
    }
}
