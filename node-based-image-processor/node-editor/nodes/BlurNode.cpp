#include "BlurNode.h"
#include "../ImageDataManager.h"
#include "../../ImageEditorApp.h"  // Added this include to access the app instance
#include <imgui.h>
#include <opencv2/imgproc.hpp>

// ... existing code ...

BlurNode::BlurNode(int id)
    : Node(id, "Blur", ImColor(100, 150, 250))
{
    // Setup pins
    AddInputPin("Image", PinType::Image);
    AddOutputPin("Image", PinType::Image);
    
    // Initialize kernel
    GenerateKernel();
}

void BlurNode::Process()
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
    
    // Generate the kernel based on current parameters
    GenerateKernel();
    
    // Apply the blur effect to the input image
    m_OutputImage = ApplyBlur(m_InputImage);
    
    // Set the output image in the ImageDataManager
    if (!m_OutputImage.empty() && !Outputs.empty())
    {
        ImageDataManager::GetInstance().SetImageData(Outputs[0].ID, m_OutputImage);
    }
    
    // Update the preview texture
    UpdatePreviewTexture();
}

void BlurNode::DrawNodeContent()
{
    ImGui::PushID(ID.AsPointer()); // Ensure unique IDs for widgets within this node instance

    // Show blur parameters
    bool changed = false;
    const float itemWidth = 150.0f; // Define a width for the sliders

    // Blur radius slider
    ImGui::PushItemWidth(itemWidth);
    changed |= ImGui::SliderInt("Radius", &m_BlurRadius, 1, 20);
    ImGui::PopItemWidth();

    // Directional blur checkbox
    changed |= ImGui::Checkbox("Directional Blur", &m_DirectionalBlur);
    
    // Additional parameters for directional blur
    if (m_DirectionalBlur)
    {
        ImGui::PushItemWidth(itemWidth);
        changed |= ImGui::SliderFloat("Angle", &m_DirectionalAngle, 0.0f, 360.0f, "%.1f°");
        changed |= ImGui::SliderFloat("Strength", &m_DirectionalFactor, 1.0f, 10.0f, "%.1f");
        ImGui::PopItemWidth();
    }

    // Mark node as dirty if any parameter changed
    if (changed)
    {
        Dirty = true;
    }
    
    ImGui::Separator();
    
    // Kernel visualization
    if (!m_Kernel.empty())
    {
        ImGui::Text("Kernel:");
        
        // Determine the size to display the kernel
        const float cellSize = 20.0f;
        int kernelSize = m_Kernel.rows;
        
        // Draw the kernel as a grid of values
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 startPos = ImGui::GetCursorScreenPos();
        
        // Add some padding
        startPos.x += 10;
        startPos.y += 10;
        
        for (int y = 0; y < kernelSize; y++)
        {
            for (int x = 0; x < kernelSize; x++)
            {
                // Calculate cell position
                ImVec2 cellPos(startPos.x + x * cellSize, startPos.y + y * cellSize);
                
                // Get kernel value at this position
                float value = m_Kernel.at<float>(y, x);
                
                // Scale the value for visualization (kernel values are usually small)
                value = value * 255 * 5; // Scaling for better visibility
                value = std::min(value, 255.0f);
                
                // Cell background color based on value
                ImU32 cellColor = IM_COL32(value, value, value, 255);
                
                // Draw cell background
                drawList->AddRectFilled(
                    cellPos,
                    ImVec2(cellPos.x + cellSize, cellPos.y + cellSize),
                    cellColor);
                
                // Draw cell border
                drawList->AddRect(
                    cellPos,
                    ImVec2(cellPos.x + cellSize, cellPos.y + cellSize),
                    IM_COL32(100, 100, 100, 255));
            }
        }
        
        // Update cursor position after drawing the kernel
        ImGui::Dummy(ImVec2(kernelSize * cellSize + 20, kernelSize * cellSize + 20));
    }

    // Add checkbox for preview
    ImGui::Checkbox("Show Preview", &m_ShowPreview);

    // Display preview if enabled and we have an output image
    // Note: m_OutputImage is a local variable in Process(), but we check m_PreviewTexture which is updated based on it.
    if (m_ShowPreview && m_PreviewTexture) // Check if texture exists (implies output was generated)
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

void BlurNode::GenerateKernel()
{
    // Calculate kernel size based on blur radius (must be odd)
    int kernelSize = m_BlurRadius * 2 + 1;
    
    if (m_DirectionalBlur)
    {
        // Create directional blur kernel
        m_Kernel = cv::Mat::zeros(kernelSize, kernelSize, CV_32F);
        
        // Center of kernel
        int center = kernelSize / 2;
        
        // Convert angle to radians
        float angleRad = m_DirectionalAngle * CV_PI / 180.0f;
        
        // Direction vector
        float dirX = std::cos(angleRad);
        float dirY = std::sin(angleRad);
        
        // Generate directional kernel
        float sum = 0.0f;
        
        for (int y = 0; y < kernelSize; y++)
        {
            for (int x = 0; x < kernelSize; x++)
            {
                // Distance from center
                float dx = (x - center);
                float dy = (y - center);
                
                // Distance along the direction vector
                float distance = std::abs(dirX * dy - dirY * dx);
                
                // Gaussian-like value based on distance
                float value = std::exp(-(distance * distance) / (2.0f * m_DirectionalFactor));
                
                // Linear distance from center also affects the value
                float centerDistance = std::sqrt(dx * dx + dy * dy);
                value *= std::exp(-(centerDistance * centerDistance) / (2.0f * m_BlurRadius * m_BlurRadius));
                
                m_Kernel.at<float>(y, x) = value;
                sum += value;
            }
        }
        
        // Normalize the kernel so the sum equals 1
        if (sum > 0)
        {
            m_Kernel /= sum;
        }
    }
    else
    {
        // Create standard Gaussian blur kernel
        m_Kernel = cv::getGaussianKernel(kernelSize, m_BlurRadius);
        
        // Convert 1D kernel to 2D
        m_Kernel = m_Kernel * m_Kernel.t();
    }
}

cv::Mat BlurNode::ApplyBlur(const cv::Mat& inputImage)
{
    cv::Mat result;
    
    // Apply the kernel to the input image
    cv::filter2D(inputImage, result, -1, m_Kernel);
    
    return result;
}

void BlurNode::UpdatePreviewTexture()
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

void BlurNode::CleanupTexture()
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
