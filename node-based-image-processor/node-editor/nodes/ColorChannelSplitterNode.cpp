#include "ColorChannelSplitterNode.h"
#include "../ImageDataManager.h"
#include <imgui.h>

// OpenGL headers for texture handling
#include <GL/glew.h>
#include <GLFW/glfw3.h>

ColorChannelSplitterNode::ColorChannelSplitterNode(int id)
    : Node(id, "Color Channel Splitter", ImColor(255, 180, 50))
{
    // Setup pins
    AddInputPin("Image", PinType::Image);
    AddOutputPin("Red", PinType::Channel);
    AddOutputPin("Green", PinType::Channel);
    AddOutputPin("Blue", PinType::Channel);
    AddOutputPin("Alpha", PinType::Channel);
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
}

void ColorChannelSplitterNode::UpdatePreviewTextures()
{
    // Clean up existing textures
    CleanupTextures();
    
    // Helper function to create texture from a single channel
    auto createTexture = [](const cv::Mat& channel) -> ImTextureID
    {
        if (channel.empty())
            return nullptr;
        
        // Convert to RGB for display if needed
        cv::Mat rgbImage;
        if (channel.channels() == 1)
            cv::cvtColor(channel, rgbImage, cv::COLOR_GRAY2RGB);
        else if (channel.channels() == 3)
            cv::cvtColor(channel, rgbImage, cv::COLOR_BGR2RGB);
        else if (channel.channels() == 4)
            cv::cvtColor(channel, rgbImage, cv::COLOR_BGRA2RGBA);
        else
            rgbImage = channel.clone();
        
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
        
        return (ImTextureID)(intptr_t)textureID;
    };
    
    // Create textures for each channel
    if (!m_RedChannel.empty())
        m_RedTexture = createTexture(m_RedChannel);
    
    if (!m_GreenChannel.empty())
        m_GreenTexture = createTexture(m_GreenChannel);
    
    if (!m_BlueChannel.empty())
        m_BlueTexture = createTexture(m_BlueChannel);
    
    if (!m_AlphaChannel.empty())
        m_AlphaTexture = createTexture(m_AlphaChannel);
}

void ColorChannelSplitterNode::CleanupTextures()
{
    auto deleteTexture = [](ImTextureID& texture)
    {
        if (texture)
        {
            GLuint textureID = (GLuint)(intptr_t)texture;
            glDeleteTextures(1, &textureID);
            texture = nullptr;
        }
    };
    
    deleteTexture(m_RedTexture);
    deleteTexture(m_GreenTexture);
    deleteTexture(m_BlueTexture);
    deleteTexture(m_AlphaTexture);
}