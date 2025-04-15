#include "NoiseGenerationNode.h"
#include "../ImageDataManager.h"
#include "../../ImageEditorApp.h" // For texture handling
#include <imgui.h>
#include <opencv2/imgproc.hpp> // For cvtColor if needed

NoiseGenerationNode::NoiseGenerationNode(int id)
    : Node(id, "Noise Generation", ImColor(180, 180, 50))
{
    // Setup pins - only output
    AddOutputPin("Noise", PinType::Image);

    // Generate initial noise
    GenerateNoise();
}

NoiseGenerationNode::~NoiseGenerationNode()
{
    CleanupTexture();
}

void NoiseGenerationNode::Process()
{
    // Processing involves generating the noise based on current parameters
    // The actual generation happens in GenerateNoise(), called when parameters change
    // Here, we just ensure the output data manager has the latest generated image
    if (!Outputs.empty())
    {
        ImageDataManager::GetInstance().SetImageData(Outputs[0].ID, m_OutputImage);
    }
    // Preview texture is updated within GenerateNoise()
}

void NoiseGenerationNode::DrawNodeContent()
{
    ImGui::PushID(ID.AsPointer()); // Ensure unique IDs for widgets within this node instance

    bool changed = false;

    // Output dimensions
    ImGui::PushItemWidth(80);
    changed |= ImGui::InputInt("Width", &m_Width);
    ImGui::SameLine();
    changed |= ImGui::InputInt("Height", &m_Height);
    ImGui::PopItemWidth();

    // Clamp dimensions
    m_Width = std::max(1, m_Width);
    m_Height = std::max(1, m_Height);

    const float itemWidth = 150.0f; // Define a width for the main widgets

    // Noise type selection
    ImGui::PushItemWidth(itemWidth);
    const char* noiseTypes[] = { "Uniform Random", "Gaussian Random" };
    changed |= ImGui::Combo("Noise Type", &m_NoiseType, noiseTypes, IM_ARRAYSIZE(noiseTypes));
    ImGui::PopItemWidth();

    // Color vs Grayscale
    changed |= ImGui::Checkbox("Color Noise", &m_IsColor);

    // Parameters based on noise type
    if (m_NoiseType == 0) // Uniform Random
    {
        // No specific parameters for basic uniform noise yet
        ImGui::TextDisabled("Uniform distribution [0, 255]");
    }
    else if (m_NoiseType == 1) // Gaussian Random
    {
        ImGui::PushItemWidth(itemWidth);
        float mean = static_cast<float>(m_Mean);
        float stddev = static_cast<float>(m_StdDev);
        changed |= ImGui::SliderFloat("Mean", &mean, 0.0f, 255.0f);
        changed |= ImGui::SliderFloat("Std Dev", &stddev, 0.0f, 100.0f);
        ImGui::PopItemWidth();
        if (changed) { // Check if sliders changed the value
             // Check if the sliders actually changed the value before assigning
            bool meanChanged = (static_cast<double>(mean) != m_Mean);
            bool stdDevChanged = (static_cast<double>(stddev) != m_StdDev);
            if (meanChanged) m_Mean = static_cast<double>(mean);
            if (stdDevChanged) m_StdDev = static_cast<double>(stddev);
            // 'changed' is already true if we entered this block due to slider interaction
        }
    }

    // General scale (might be more relevant for procedural noise later)
    // changed |= ImGui::SliderFloat("Scale", &m_Scale, 0.1f, 10.0f);

    if (changed)
    {
        GenerateNoise(); // Regenerate noise if parameters changed
        Dirty = true;    // Mark as dirty to ensure Process() updates data manager
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
	ImGui::PopID(); // Pop ID for this node instance
}

void NoiseGenerationNode::GenerateNoise()
{
    int channels = m_IsColor ? 3 : 1;
    int type = m_IsColor ? CV_8UC3 : CV_8UC1;

    // Create the output image matrix
    m_OutputImage = cv::Mat(m_Height, m_Width, type);

    // Generate noise based on type
    if (m_NoiseType == 0) // Uniform Random
    {
        cv::randu(m_OutputImage, cv::Scalar::all(0), cv::Scalar::all(255));
    }
    else if (m_NoiseType == 1) // Gaussian Random
    {
        cv::Mat noise(m_Height, m_Width, type);
        cv::randn(noise, cv::Scalar::all(m_Mean), cv::Scalar::all(m_StdDev));
        // Ensure values are within the valid 8-bit range [0, 255]
        noise.convertTo(m_OutputImage, type);
    }
    // Future: Add cases for Perlin, Simplex, Worley noise here

    // Update the preview texture
    UpdatePreviewTexture();

     // Also ensure the data manager gets updated immediately in Process()
    Dirty = true;
}


void NoiseGenerationNode::UpdatePreviewTexture()
{
    CleanupTexture();
    if (m_OutputImage.empty() || m_OutputImage.data == nullptr) return;

    cv::Mat rgbaImage;
     try {
        if (m_OutputImage.channels() == 3)
            cv::cvtColor(m_OutputImage, rgbaImage, cv::COLOR_BGR2RGBA);
        else if (m_OutputImage.channels() == 1)
            cv::cvtColor(m_OutputImage, rgbaImage, cv::COLOR_GRAY2RGBA);
        else // Should not happen for noise generation
            rgbaImage = m_OutputImage.clone();
     } catch (const cv::Exception& e) {
         // Handle potential conversion errors
         return;
     }


    if (ImageEditorApp* app = ImageEditorApp::GetInstance())
    {
        m_PreviewTexture = app->CreateTexture(rgbaImage.data, rgbaImage.cols, rgbaImage.rows);
    }
}

void NoiseGenerationNode::CleanupTexture()
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
