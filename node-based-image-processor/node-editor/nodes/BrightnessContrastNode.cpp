#include "BrightnessContrastNode.h"
#include "../ImageDataManager.h"
#include "../../ImageEditorApp.h"  // Added this include to access the app instance
#include <imgui.h>
#include <opencv2/imgproc.hpp>


BrightnessContrastNode::BrightnessContrastNode(int id)
    : Node(id, "Brightness/Contrast", ImColor(218, 112, 214))
{
    // Setup pins
    AddInputPin("Image", PinType::Image);
    AddOutputPin("Image", PinType::Image);
}

void BrightnessContrastNode::Process()
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
        // No input image, clear output
        m_OutputImage = cv::Mat();
        CleanupTexture();
        return;
    }
    
    // Apply brightness and contrast adjustments
    m_OutputImage = cv::Mat::zeros(m_InputImage.size(), m_InputImage.type());
    
    // Calculate alpha (contrast) and beta (brightness) for linear transformation
    double alpha = m_Contrast; // Contrast control (1.0 - 3.0)
    int beta = (int)m_Brightness; // Brightness control (-100 -> +100)
    
    // Apply the transformation: new_pixel = alpha * pixel + beta
    m_InputImage.convertTo(m_OutputImage, -1, alpha, beta);
    
    // Set the image data in the ImageDataManager for the output pin
    if (!m_OutputImage.empty() && !Outputs.empty())
    {
        ImageDataManager::GetInstance().SetImageData(Outputs[0].ID, m_OutputImage);
    }
    
    // Update the preview texture
    UpdatePreviewTexture();
}

void BrightnessContrastNode::DrawNodeContent()
{
    // Show parameter controls
    bool changed = false;
    const float itemWidth = 150.0f; // Define a width for the sliders

    ImGui::PushItemWidth(itemWidth); // Push item width before sliders
    // Brightness slider
    changed |= ImGui::SliderFloat("Brightness", &m_Brightness, -100.0f, 100.0f, "%.1f");
    ImGui::PopItemWidth(); // Pop item width after slider
    ImGui::SameLine();
    if (ImGui::Button("Reset##Brightness"))
    {
        ResetBrightness();
        changed = true;
    }

    ImGui::PushItemWidth(itemWidth); // Push item width before sliders
    // Contrast slider
    changed |= ImGui::SliderFloat("Contrast", &m_Contrast, 0.0f, 3.0f, "%.2f");
    ImGui::PopItemWidth(); // Pop item width after slider
    ImGui::SameLine();
    if (ImGui::Button("Reset##Contrast"))
    {
        ResetContrast();
        changed = true;
    }
    
    // Mark node as dirty if any parameter changed
    if (changed)
    {
        Dirty = true;
    }

    // Add checkbox for preview
    ImGui::Checkbox("Show Preview", &m_ShowPreview);

    // Display preview if enabled and we have an output image
    if (m_ShowPreview && !m_OutputImage.empty() && m_PreviewTexture)
    {
        ImGui::Separator(); // Add separator before preview
        // Calculate preview size
        const float maxPreviewWidth = 200.0f;
        const float maxPreviewHeight = 150.0f;
        
        float aspectRatio = (float)m_OutputImage.cols / (float)m_OutputImage.rows;
        float previewWidth = std::min(maxPreviewWidth, (float)m_OutputImage.cols);
        float previewHeight = previewWidth / aspectRatio;
        
        if (previewHeight > maxPreviewHeight)
        {
            previewHeight = maxPreviewHeight;
            previewWidth = previewHeight * aspectRatio;
        }
        
        // Draw the preview
        ImGui::Image(m_PreviewTexture, ImVec2(previewWidth, previewHeight));
    }
    else
    {
        ImGui::Text("No preview available");
    }
}

void BrightnessContrastNode::UpdatePreviewTexture()
{
    // Clean up any existing texture
    CleanupTexture();

    // Use the local output image
    if (m_OutputImage.empty())
        return;

    // Verify we have valid image data
    if (m_OutputImage.data == nullptr)
        return;

    // Convert image from OpenCV BGR format to RGB for OpenGL
    cv::Mat rgbImage;
    if (m_OutputImage.channels() == 3)
        cv::cvtColor(m_OutputImage, rgbImage, cv::COLOR_BGR2RGBA);
    else if (m_OutputImage.channels() == 4)
        cv::cvtColor(m_OutputImage, rgbImage, cv::COLOR_BGRA2RGBA);
    else if (m_OutputImage.channels() == 1)
        cv::cvtColor(m_OutputImage, rgbImage, cv::COLOR_GRAY2RGBA);
    else
        rgbImage = m_OutputImage.clone(); // Just use as-is if format is unexpected

    // Use ImageEditorApp singleton to create texture instead of direct OpenGL calls
    if (ImageEditorApp* app = ImageEditorApp::GetInstance())
    {
        m_PreviewTexture = app->CreateTexture(rgbImage.data, rgbImage.cols, rgbImage.rows);
    }
}

void BrightnessContrastNode::CleanupTexture()
{
    if (m_PreviewTexture)
    {
        // Use ImageEditorApp singleton to destroy texture instead of direct OpenGL calls
        if (ImageEditorApp* app = ImageEditorApp::GetInstance())
        {
            app->DestroyTexture(m_PreviewTexture);
        }
        m_PreviewTexture = nullptr;
    }
}

cv::Mat BrightnessContrastNode::GetConnectedImage()
{
    // This function is no longer used as the ImageDataManager is now handling image data.
    return cv::Mat();
}
