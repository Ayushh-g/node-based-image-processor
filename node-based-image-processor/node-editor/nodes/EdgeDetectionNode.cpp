#include "EdgeDetectionNode.h"
#include "../ImageDataManager.h"
#include "../../ImageEditorApp.h"  // Added this include to access the app instance
#include <imgui.h>
#include <algorithm>
#include <opencv2/imgproc.hpp>

EdgeDetectionNode::EdgeDetectionNode(int id)
    : Node(id, "Edge Detection", ImColor(200, 150, 100))
{
    // Setup pins
    AddInputPin("Image", PinType::Image);
    AddOutputPin("Image", PinType::Image);
}

void EdgeDetectionNode::Process()
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
		m_OutputImage = cv::Mat();
        CleanupTexture();
        return;
    }
    
    // Apply the edge detection algorithm based on current settings
    m_OutputImage = ApplyEdgeDetection(m_InputImage);
    
    // Set the output image in the ImageDataManager
    if (!m_OutputImage.empty() && !Outputs.empty())
    {
        ImageDataManager::GetInstance().SetImageData(Outputs[0].ID, m_OutputImage);
    }
    
    // Update the preview texture
    UpdatePreviewTexture();
}

void EdgeDetectionNode::DrawNodeContent()
{
    ImGui::PushID(ID.AsPointer()); // Ensure unique IDs for widgets within this node instance
    // Show edge detection parameters
    bool changed = false;
    const float itemWidth = 150.0f; // Define a width for the widgets

    // Select detection type
    ImGui::PushItemWidth(itemWidth);
    const char* detectionTypes[] = { "Sobel", "Canny", "Laplacian" };
    changed |= ImGui::Combo("Detection Type", &m_DetectionType, detectionTypes, 3);
    ImGui::PopItemWidth();

    // Show parameters based on selected type
    ImGui::PushItemWidth(itemWidth);
    if (m_DetectionType == 0) // Sobel
    {
        int kernelSizes[] = { 1, 3, 5, 7 };
        int kernelSizeIndex = 0;
        
        // Find the current kernel size index
        for (int i = 0; i < 4; i++)
        {
            if (kernelSizes[i] == m_SobelKernelSize)
            {
                kernelSizeIndex = i;
                break;
            }
        }
        
        // Display kernel size combo
        const char* kernelLabels[] = { "1x1", "3x3", "5x5", "7x7" };
        if (ImGui::Combo("Kernel Size", &kernelSizeIndex, kernelLabels, 4))
        {
            m_SobelKernelSize = kernelSizes[kernelSizeIndex];
            changed = true;
        }

        // X and Y derivative orders
        changed |= ImGui::SliderInt("X Derivative", &m_SobelDx, 0, 2);
        changed |= ImGui::SliderInt("Y Derivative", &m_SobelDy, 0, 2);

        // Ensure at least one derivative is used
        if (m_SobelDx == 0 && m_SobelDy == 0)
        {
            m_SobelDx = 1;
            changed = true;
        }
    }
    else if (m_DetectionType == 1) // Canny
    {
        float threshold1 = static_cast<float>(m_CannyThreshold1);
        float threshold2 = static_cast<float>(m_CannyThreshold2);

        if (ImGui::SliderFloat("Threshold 1", &threshold1, 0.0f, 300.0f))
        {
            m_CannyThreshold1 = static_cast<double>(threshold1);
            changed = true;
        }

        if (ImGui::SliderFloat("Threshold 2", &threshold2, 0.0f, 300.0f))
        {
            m_CannyThreshold2 = static_cast<double>(threshold2);
            changed = true;
        }

        // Ensure threshold1 <= threshold2
        if (m_CannyThreshold1 > m_CannyThreshold2)
        {
            m_CannyThreshold1 = m_CannyThreshold2;
            changed = true;
        }
        
        int apertureSizes[] = { 3, 5, 7 };
        int apertureSizeIndex = 0;
        
        // Find the current aperture size index
        for (int i = 0; i < 3; i++)
        {
            if (apertureSizes[i] == m_CannyApertureSize)
            {
                apertureSizeIndex = i;
                break;
            }
        }
        
        // Display aperture size combo
        const char* apertureLabels[] = { "3x3", "5x5", "7x7" };
        if (ImGui::Combo("Aperture Size", &apertureSizeIndex, apertureLabels, 3))
        {
            m_CannyApertureSize = apertureSizes[apertureSizeIndex];
            changed = true;
        }

        changed |= ImGui::Checkbox("L2 Gradient", &m_CannyL2Gradient);
    }
    else if (m_DetectionType == 2) // Laplacian
    {
        int kernelSizes[] = { 1, 3, 5, 7 };
        int kernelSizeIndex = 0;
        
        // Find the current kernel size index
        for (int i = 0; i < 4; i++)
        {
            if (kernelSizes[i] == m_LaplacianKernelSize)
            {
                kernelSizeIndex = i;
                break;
            }
        }
        
        // Display kernel size combo
        const char* kernelLabels[] = { "1x1", "3x3", "5x5", "7x7" };
        if (ImGui::Combo("Kernel Size", &kernelSizeIndex, kernelLabels, 4))
        {
            m_LaplacianKernelSize = kernelSizes[kernelSizeIndex];
            changed = true;
        }

        float scale = static_cast<float>(m_LaplacianScale);
        if (ImGui::SliderFloat("Scale", &scale, 0.1f, 5.0f))
        {
            m_LaplacianScale = static_cast<double>(scale);
            changed = true;
        }

        float delta = static_cast<float>(m_LaplacianDelta);
        if (ImGui::SliderFloat("Delta", &delta, -100.0f, 100.0f))
        {
            m_LaplacianDelta = static_cast<double>(delta);
            changed = true;
        }
    }
    ImGui::PopItemWidth(); // Pop item width after parameter widgets

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

	ImGui::PopID(); // Pop ID for this node instance
}

cv::Mat EdgeDetectionNode::ApplyEdgeDetection(const cv::Mat& inputImage)
{
    if (inputImage.empty())
        return cv::Mat();
    
    cv::Mat grayImage;
    cv::Mat result;
    
    // Convert to grayscale if needed
    if (inputImage.channels() == 1)
        grayImage = inputImage.clone();
    else
        cv::cvtColor(inputImage, grayImage, cv::COLOR_BGR2GRAY);
    
    // Apply edge detection based on selected type
    if (m_DetectionType == 0) // Sobel
    {
        cv::Mat gradX, gradY, gradMag;
        
        // Apply Sobel in X direction
        if (m_SobelDx > 0)
        {
            cv::Sobel(grayImage, gradX, CV_16S, m_SobelDx, 0, m_SobelKernelSize);
            cv::convertScaleAbs(gradX, gradX);
        }
        else
        {
            gradX = cv::Mat::zeros(grayImage.size(), CV_8UC1);
        }
        
        // Apply Sobel in Y direction
        if (m_SobelDy > 0)
        {
            cv::Sobel(grayImage, gradY, CV_16S, 0, m_SobelDy, m_SobelKernelSize);
            cv::convertScaleAbs(gradY, gradY);
        }
        else
        {
            gradY = cv::Mat::zeros(grayImage.size(), CV_8UC1);
        }
        
        // Combine results
        cv::addWeighted(gradX, 0.5, gradY, 0.5, 0, result);
    }
    else if (m_DetectionType == 1) // Canny
    {
        cv::Canny(grayImage, result, m_CannyThreshold1, m_CannyThreshold2, m_CannyApertureSize, m_CannyL2Gradient);
    }
    else if (m_DetectionType == 2) // Laplacian
    {
        cv::Mat laplacianResult;
        cv::Laplacian(grayImage, laplacianResult, CV_16S, m_LaplacianKernelSize, m_LaplacianScale, m_LaplacianDelta);
        cv::convertScaleAbs(laplacianResult, result);
    }
    
    // Convert result to 3-channel if input was 3-channel
    if (inputImage.channels() > 1)
    {
        cv::Mat colorResult;
        cv::cvtColor(result, colorResult, cv::COLOR_GRAY2BGR);
        return colorResult;
    }
    
    return result;
}

void EdgeDetectionNode::UpdatePreviewTexture()
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

void EdgeDetectionNode::CleanupTexture()
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
