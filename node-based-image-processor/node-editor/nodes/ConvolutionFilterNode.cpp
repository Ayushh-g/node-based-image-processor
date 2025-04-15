#include "ConvolutionFilterNode.h"
#include "../ImageDataManager.h"
#include "../../ImageEditorApp.h" // For texture handling
#include <imgui.h>
#include <vector>
#include <numeric> // For std::accumulate

ConvolutionFilterNode::ConvolutionFilterNode(int id)
    : Node(id, "Convolution Filter", ImColor(150, 150, 150))
{
    // Setup pins
    AddInputPin("Image", PinType::Image);
    AddOutputPin("Image", PinType::Image);

    // Initialize kernel (default to 3x3 identity)
    m_KernelSize = 3;
    m_KernelValues.resize(m_KernelSize * m_KernelSize, 0.0f);
    m_KernelValues[m_KernelSize * m_KernelSize / 2] = 1.0f; // Center element is 1
    UpdateKernelFromUI(); // Create the initial cv::Mat kernel
}

ConvolutionFilterNode::~ConvolutionFilterNode()
{
    CleanupTexture();
}

void ConvolutionFilterNode::Process()
{
    // Get input image
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

    // Apply convolution using the current kernel
    cv::filter2D(m_InputImage, m_OutputImage, -1, m_Kernel);

    // Set output image
    if (!Outputs.empty())
    {
        ImageDataManager::GetInstance().SetImageData(Outputs[0].ID, m_OutputImage);
    }

    // Update preview
    UpdatePreviewTexture();
}

void ConvolutionFilterNode::DrawNodeContent()
{
    bool changed = false;

    const float itemWidth = 150.0f; // Define a width for the combo box

    // Kernel size selection
    ImGui::PushItemWidth(itemWidth);
    const char* sizes[] = { "3x3", "5x5" };
    int currentSizeIndex = (m_KernelSize == 3) ? 0 : 1;
    if (ImGui::Combo("Kernel Size", &currentSizeIndex, sizes, 2))
    {
        int newSize = (currentSizeIndex == 0) ? 3 : 5;
        if (newSize != m_KernelSize)
        {
            m_KernelSize = newSize;
            // Resize kernel values vector, preserving center if possible
            std::vector<float> oldValues = m_KernelValues;
            int oldSize = (m_KernelSize == 3) ? 5 : 3; // The size it was before changing
            m_KernelValues.assign(m_KernelSize * m_KernelSize, 0.0f);
            // Simple reset to identity on resize for now
            m_KernelValues[m_KernelSize * m_KernelSize / 2] = 1.0f;
            changed = true;
        }
    }
    ImGui::PopItemWidth();

    // Kernel matrix input
    ImGui::Text("Kernel Matrix:");
    ImGui::PushItemWidth(40); // Make input fields smaller
    for (int y = 0; y < m_KernelSize; ++y)
    {
        for (int x = 0; x < m_KernelSize; ++x)
        {
            int index = y * m_KernelSize + x;
            ImGui::PushID(index); // Unique ID for each input field
            if (ImGui::InputFloat("", &m_KernelValues[index], 0.0f, 0.0f, "%.2f"))
            {
                changed = true;
            }
            ImGui::PopID();
            if (x < m_KernelSize - 1)
            {
                ImGui::SameLine();
            }
        }
    }
    ImGui::PopItemWidth();

    // Preset buttons
    ImGui::Text("Presets:");
    if (ImGui::Button("Identity"))
    {
        m_KernelValues.assign(m_KernelSize * m_KernelSize, 0.0f);
        m_KernelValues[m_KernelSize * m_KernelSize / 2] = 1.0f;
        changed = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Sharpen"))
    {
        ApplyPreset(Preset::SHARPEN);
        changed = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Emboss"))
    {
        ApplyPreset(Preset::EMBOSS);
        changed = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Edge Enhance"))
    {
        ApplyPreset(Preset::EDGE_ENHANCE);
        changed = true;
    }

    if (changed)
    {
        UpdateKernelFromUI();
        Dirty = true;
    }

    // Add checkbox for preview
    ImGui::Checkbox("Show Preview", &m_ShowPreview);

    // Display preview if enabled and we have an output image
    if (m_ShowPreview && !m_OutputImage.empty() && m_PreviewTexture)
    {
        ImGui::Separator();
        ImGui::Text("Preview:");
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
        ImGui::Image(m_PreviewTexture, ImVec2(previewWidth, previewHeight));
    }
    else
    {
        ImGui::Text("No preview available");
    }
}

void ConvolutionFilterNode::UpdateKernelFromUI()
{
    // Create/update the cv::Mat kernel from m_KernelValues
    m_Kernel = cv::Mat(m_KernelSize, m_KernelSize, CV_32F, m_KernelValues.data()).clone();

    // Normalize kernel if sum is not zero (optional, but common for filters like blur)
    // float sum = std::accumulate(m_KernelValues.begin(), m_KernelValues.end(), 0.0f);
    // if (std::abs(sum) > 1e-6) {
    //     m_Kernel /= sum;
    // }
}

void ConvolutionFilterNode::ApplyPreset(int presetIndex)
{
    m_KernelValues.assign(m_KernelSize * m_KernelSize, 0.0f); // Clear existing values

    if (m_KernelSize == 3)
    {
        switch (presetIndex)
        {
        case Preset::SHARPEN:
            m_KernelValues = { 0, -1, 0, -1, 5, -1, 0, -1, 0 };
            break;
        case Preset::EMBOSS:
            m_KernelValues = { -2, -1, 0, -1, 1, 1, 0, 1, 2 };
            break;
        case Preset::EDGE_ENHANCE:
            m_KernelValues = { 0, 0, 0, -1, 1, 0, 0, 0, 0 }; // Simple edge enhance
            break;
        default: // Identity
            m_KernelValues[4] = 1.0f;
            break;
        }
    }
    else if (m_KernelSize == 5)
    {
        // Define 5x5 presets if needed, otherwise default to identity
        switch (presetIndex)
        {
        case Preset::SHARPEN: // Example 5x5 sharpen
             m_KernelValues = {
                -1, -1, -1, -1, -1,
                -1, -1, -1, -1, -1,
                -1, -1, 25, -1, -1, // Center value is 25 (sum is 1)
                -1, -1, -1, -1, -1,
                -1, -1, -1, -1, -1
            };
             // Normalize (sum is 1 already)
            break;
        // Add other 5x5 presets if desired
        default: // Identity
            m_KernelValues[12] = 1.0f; // Center element for 5x5
            break;
        }
    }
     else // Default to identity if size is unexpected
    {
        m_KernelValues[m_KernelSize * m_KernelSize / 2] = 1.0f;
    }
}


void ConvolutionFilterNode::UpdatePreviewTexture()
{
    CleanupTexture();
    if (m_OutputImage.empty() || m_OutputImage.data == nullptr) return;

    cv::Mat rgbaImage;
    try {
        if (m_OutputImage.channels() == 3)
            cv::cvtColor(m_OutputImage, rgbaImage, cv::COLOR_BGR2RGBA);
        else if (m_OutputImage.channels() == 4)
             cv::cvtColor(m_OutputImage, rgbaImage, cv::COLOR_BGRA2RGBA);
        else if (m_OutputImage.channels() == 1)
            cv::cvtColor(m_OutputImage, rgbaImage, cv::COLOR_GRAY2RGBA);
        else
            rgbaImage = m_OutputImage.clone(); // Should not happen with filter2D
    } catch (const cv::Exception& e) {
        // Handle potential conversion errors
        return;
    }


    if (ImageEditorApp* app = ImageEditorApp::GetInstance())
    {
        m_PreviewTexture = app->CreateTexture(rgbaImage.data, rgbaImage.cols, rgbaImage.rows);
    }
}

void ConvolutionFilterNode::CleanupTexture()
{
    if (m_PreviewTexture)
    {
        if (ImageEditorApp* app = ImageEditorApp::GetInstance())
        {
            app->DestroyTexture(m_PreviewTexture);
        }
        m_PreviewTexture = nullptr;
    }
}
