#include "ThresholdNode.h"
#include "../ImageDataManager.h"
#include <imgui.h>
#include <algorithm>
#include "../../ImageEditorApp.h" // Add ImageEditorApp header

// OpenGL headers for texture handling
//#include <GL/glew.h>
//#include <GLFW/glfw3.h>

ThresholdNode::ThresholdNode(int id)
    : Node(id, "Threshold", ImColor(128, 230, 150))
{
    // Setup pins
    AddInputPin("Image", PinType::Image);
    AddOutputPin("Image", PinType::Image);
    
    // Initialize histogram
    m_Histogram = cv::Mat::zeros(100, 256, CV_8UC3);
}

void ThresholdNode::Process()
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
        CleanupTextures();
        return;
    }
    
    // Update histogram
    UpdateHistogram();
    
    // Apply threshold
    m_OutputImage = ApplyThreshold(m_InputImage);
    
    // Set the output image in the ImageDataManager
    if (!m_OutputImage.empty() && !Outputs.empty())
    {
        ImageDataManager::GetInstance().SetImageData(Outputs[0].ID, m_OutputImage);
    }
    
    // Update the preview texture
    UpdatePreviewTexture();
}

void ThresholdNode::DrawNodeContent()
{
    ImGui::PushID(ID.AsPointer()); // Ensure unique IDs for widgets within this node instance

    // Show threshold parameters based on type
    bool changed = false;
    const float itemWidth = 120.0f; // Define a width for the widgets

    // Select threshold type
    ImGui::PushItemWidth(itemWidth);
    const char* thresholdTypes[] = { "Binary", "Adaptive", "Otsu" };
    changed |= ImGui::Combo("Threshold Type", &m_ThresholdType, thresholdTypes, 3);
    ImGui::PopItemWidth();

    // Show parameters based on selected type
    ImGui::PushItemWidth(itemWidth);
    if (m_ThresholdType == 0) // Binary threshold
    {
        float threshValue = static_cast<float>(m_ThresholdValue);
        if (ImGui::SliderFloat("Threshold Value", &threshValue, 0.0f, 255.0f))
        {
            m_ThresholdValue = static_cast<double>(threshValue);
            changed = true;
        }
    }
    else if (m_ThresholdType == 1) // Adaptive threshold
    {
        // Block size must be odd and >= 3
        changed |= ImGui::SliderInt("Block Size", &m_AdaptiveBlockSize, 3, 99);
        if (m_AdaptiveBlockSize % 2 == 0)
            m_AdaptiveBlockSize++; // Make sure it's odd

        float constant = static_cast<float>(m_AdaptiveConstant);
        if (ImGui::SliderFloat("C Value", &constant, -10.0f, 10.0f))
        {
            m_AdaptiveConstant = static_cast<double>(constant);
            changed = true;
        }
    }
    ImGui::PopItemWidth(); // Pop item width after parameter widgets

    // Invert option
    changed |= ImGui::Checkbox("Invert Result", &m_InvertThreshold);
    
    // Mark node as dirty if any parameter changed
    if (changed)
    {
        Dirty = true;
    }
    
    // Display histogram if available
    if (!m_Histogram.empty() && m_HistogramTexture)
    {
        ImGui::Text("Histogram:");
        ImGui::Image(m_HistogramTexture, ImVec2(256, 100));
        
        // Draw threshold line on histogram for binary mode
        if (m_ThresholdType == 0 || m_ThresholdType == 2)
        {
            float threshold = (m_ThresholdType == 0) ? (float)m_ThresholdValue : (float)cv::threshold(m_InputImage, cv::Mat(), 0, 255, cv::THRESH_OTSU);
            ImVec2 cursorPos = ImGui::GetCursorScreenPos();
            cursorPos.y -= 100; // Move up to histogram position
            
            ImGui::GetWindowDrawList()->AddLine(
                ImVec2(cursorPos.x + threshold, cursorPos.y), 
                ImVec2(cursorPos.x + threshold, cursorPos.y + 100),
                IM_COL32(255, 0, 0, 255), 1.0f);
        }
    }

    // Add checkbox for preview
    ImGui::Checkbox("Show Preview", &m_ShowPreview);

    // Display preview if enabled and we have an output image
    if (m_ShowPreview && !m_OutputImage.empty() && m_PreviewTexture)
    {
        ImGui::Separator();
        ImGui::Text("Preview:");
        
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

    ImGui::PopID(); // Pop the node instance ID
}

cv::Mat ThresholdNode::ApplyThreshold(const cv::Mat& inputImage)
{
    cv::Mat result;
    cv::Mat grayImage;
    
    // Convert to grayscale if needed
    if (inputImage.channels() == 1)
        grayImage = inputImage.clone();
    else
        cv::cvtColor(inputImage, grayImage, cv::COLOR_BGR2GRAY);
    
    // Apply thresholding based on selected type
    if (m_ThresholdType == 0) // Binary
    {
        int thresholdType = m_InvertThreshold ? cv::THRESH_BINARY_INV : cv::THRESH_BINARY;
        cv::threshold(grayImage, result, m_ThresholdValue, 255, thresholdType);
    }
    else if (m_ThresholdType == 1) // Adaptive
    {
        int adaptiveMethod = cv::ADAPTIVE_THRESH_GAUSSIAN_C;
        int thresholdType = m_InvertThreshold ? cv::THRESH_BINARY_INV : cv::THRESH_BINARY;
        cv::adaptiveThreshold(grayImage, result, 255, adaptiveMethod, thresholdType, m_AdaptiveBlockSize, m_AdaptiveConstant);
    }
    else if (m_ThresholdType == 2) // Otsu
    {
        int thresholdType = m_InvertThreshold ? cv::THRESH_BINARY_INV : cv::THRESH_BINARY;
        cv::threshold(grayImage, result, 0, 255, thresholdType | cv::THRESH_OTSU);
    }
    
    // If input was color, convert result back to color for consistent output
    if (inputImage.channels() > 1)
    {
        cv::Mat colorResult;
        cv::cvtColor(result, colorResult, cv::COLOR_GRAY2BGR);
        return colorResult;
    }
    
    return result;
}

void ThresholdNode::UpdateHistogram()
{
    if (m_InputImage.empty())
        return;
    
    // Create grayscale image if needed
    cv::Mat grayImage;
    if (m_InputImage.channels() == 1)
        grayImage = m_InputImage.clone();
    else
        cv::cvtColor(m_InputImage, grayImage, cv::COLOR_BGR2GRAY);
    
    // Calculate histogram
    int histSize = 256;
    float range[] = { 0, 256 };
    const float* histRange = { range };
    cv::Mat hist;
    cv::calcHist(&grayImage, 1, 0, cv::Mat(), hist, 1, &histSize, &histRange);
    
    // Normalize histogram for display
    cv::normalize(hist, hist, 0, 100, cv::NORM_MINMAX);
    
    // Create histogram visualization
    m_Histogram = cv::Mat::zeros(100, 256, CV_8UC3);
    for (int i = 0; i < 256; i++)
    {
        int binHeight = cvRound(hist.at<float>(i));
        cv::rectangle(
            m_Histogram,
            cv::Point(i, 100),
            cv::Point(i + 1, 100 - binHeight),
            cv::Scalar(200, 200, 200),
            cv::FILLED
        );
    }
    
    // Update histogram texture
    // Get app instance
    ImageEditorApp* app = ImageEditorApp::GetInstance();
    
    // Delete previous texture if it exists
    if (m_HistogramTexture)
    {
        app->DestroyTexture(m_HistogramTexture);
        m_HistogramTexture = nullptr;
    }
    
    // Convert histogram to RGB for OpenGL
    cv::Mat histRGB;
    cv::cvtColor(m_Histogram, histRGB, cv::COLOR_BGR2RGBA);
    
    // Use app to create texture
    m_HistogramTexture = app->CreateTexture(histRGB.data, histRGB.cols, histRGB.rows);
}

void ThresholdNode::UpdatePreviewTexture()
{
    // Delete previous texture if it exists
    if (m_PreviewTexture)
    {
        ImageEditorApp* app = ImageEditorApp::GetInstance();
        app->DestroyTexture(m_PreviewTexture);
        m_PreviewTexture = nullptr;
    }

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

void ThresholdNode::CleanupTextures()
{
    // Get app instance
    ImageEditorApp* app = ImageEditorApp::GetInstance();
    
    // Destroy textures using app instance
    if (m_PreviewTexture)
    {
        app->DestroyTexture(m_PreviewTexture);
        m_PreviewTexture = nullptr;
    }
    
    if (m_HistogramTexture)
    {
        app->DestroyTexture(m_HistogramTexture);
        m_HistogramTexture = nullptr;
    }
}
