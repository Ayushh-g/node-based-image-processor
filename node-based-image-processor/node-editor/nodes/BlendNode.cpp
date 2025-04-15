#include "BlendNode.h"
#include "../ImageDataManager.h"
#include "../../ImageEditorApp.h"
#include <imgui.h>
#include <opencv2/imgproc.hpp>

BlendNode::BlendNode(int id)
    : Node(id, "Blend", ImColor(65, 105, 225))  // Royal Blue color
{
    // Setup pins - two inputs and one output
    AddInputPin("Base Image", PinType::Image);  
    AddInputPin("Blend Image", PinType::Image);
    AddOutputPin("Result", PinType::Image);
}

void BlendNode::Process()
{
    // Get input images from the ImageDataManager
    if (Inputs.size() >= 2)
    {
        m_InputImage1 = ImageDataManager::GetInstance().GetImageData(Inputs[0].ID); // Base image
        m_InputImage2 = ImageDataManager::GetInstance().GetImageData(Inputs[1].ID); // Blend image
    }
    else
    {
        m_InputImage1 = cv::Mat();
        m_InputImage2 = cv::Mat();
    }
    
    // Check if both input images are valid
    if (m_InputImage1.empty() || m_InputImage2.empty())
    {
        // If either input is missing, clear output
        m_OutputImage = cv::Mat();
        CleanupTexture();
        return;
    }
    
    // Make sure both images have the same size - resize the second image if needed
    cv::Mat resizedImage2;
    if (m_InputImage1.size() != m_InputImage2.size())
    {
        cv::resize(m_InputImage2, resizedImage2, m_InputImage1.size(), 0, 0, cv::INTER_LINEAR);
    }
    else
    {
        resizedImage2 = m_InputImage2.clone();
    }
    
    // Make sure both images have the same type
    if (m_InputImage1.type() != resizedImage2.type())
    {
        // Convert the second image to match the type of the first
        if (m_InputImage1.channels() == 3 && resizedImage2.channels() == 4)
        {
            cv::cvtColor(resizedImage2, resizedImage2, cv::COLOR_BGRA2BGR);
        }
        else if (m_InputImage1.channels() == 4 && resizedImage2.channels() == 3)
        {
            cv::cvtColor(resizedImage2, resizedImage2, cv::COLOR_BGR2BGRA);
        }
        else if (m_InputImage1.channels() == 1 && resizedImage2.channels() > 1)
        {
            cv::cvtColor(resizedImage2, resizedImage2, cv::COLOR_BGR2GRAY);
        }
        else if (m_InputImage1.channels() > 1 && resizedImage2.channels() == 1)
        {
            cv::cvtColor(resizedImage2, resizedImage2, cv::COLOR_GRAY2BGR);
        }
    }
    
    // Apply blending
    m_OutputImage = ApplyBlend(m_InputImage1, resizedImage2);
    
    // Update the preview texture
    UpdatePreviewTexture();
    
    // Set the output data in ImageDataManager
    if (!Outputs.empty())
    {
        ImageDataManager::GetInstance().SetImageData(Outputs[0].ID, m_OutputImage);
    }
}

cv::Mat BlendNode::ApplyBlend(const cv::Mat& baseImg, const cv::Mat& blendImg)
{
    cv::Mat blendedResult;
    
    // Apply the selected blend mode
    switch (m_BlendMode)
    {
        case BlendMode::Normal:
            blendedResult = BlendNormal(baseImg, blendImg);
            break;
        case BlendMode::Multiply:
            blendedResult = BlendMultiply(baseImg, blendImg);
            break;
        case BlendMode::Screen:
            blendedResult = BlendScreen(baseImg, blendImg);
            break;
        case BlendMode::Overlay:
            blendedResult = BlendOverlay(baseImg, blendImg);
            break;
        case BlendMode::Difference:
            blendedResult = BlendDifference(baseImg, blendImg);
            break;
        case BlendMode::Lighten:
            blendedResult = BlendLighten(baseImg, blendImg);
            break;
        case BlendMode::Darken:
            blendedResult = BlendDarken(baseImg, blendImg);
            break;
        default:
            blendedResult = baseImg.clone();
    }
    
    // Apply opacity if not 100%
    if (m_Opacity < 1.0f)
    {
        blendedResult = ApplyOpacity(baseImg, blendedResult);
    }
    
    return blendedResult;
}

cv::Mat BlendNode::BlendNormal(const cv::Mat& baseImg, const cv::Mat& blendImg)
{
    // Normal blend simply returns the blend image
    return blendImg.clone();
}

cv::Mat BlendNode::BlendMultiply(const cv::Mat& baseImg, const cv::Mat& blendImg)
{
    cv::Mat result;
    // Multiply blend: multiply pixel values and normalize
    cv::multiply(baseImg, blendImg, result, 1.0/255.0);
    return result;
}

cv::Mat BlendNode::BlendScreen(const cv::Mat& baseImg, const cv::Mat& blendImg)
{
    cv::Mat result = baseImg.clone();
    
    // Screen blend: 1 - (1 - Base) * (1 - Blend)
    cv::Mat invBase, invBlend, temp;
    cv::subtract(cv::Scalar::all(255), baseImg, invBase);
    cv::subtract(cv::Scalar::all(255), blendImg, invBlend);
    cv::multiply(invBase, invBlend, temp, 1.0/255.0);
    cv::subtract(cv::Scalar::all(255), temp, result);
    
    return result;
}

cv::Mat BlendNode::BlendOverlay(const cv::Mat& baseImg, const cv::Mat& blendImg)
{
    cv::Mat result = baseImg.clone();
    
    // Overlay blend: if Base < 128, 2 * Base * Blend / 255, else 255 - 2 * (255 - Base) * (255 - Blend) / 255
    
    // Process each channel
    int channels = baseImg.channels();
    int height = baseImg.rows;
    int width = baseImg.cols;
    
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            for (int c = 0; c < channels; c++)
            {
                if (channels == 1)
                {
                    uchar baseVal = baseImg.at<uchar>(y, x);
                    uchar blendVal = blendImg.at<uchar>(y, x);
                    
                    if (baseVal < 128)
                        result.at<uchar>(y, x) = cv::saturate_cast<uchar>((2.0 * baseVal * blendVal) / 255.0);
                    else
                        result.at<uchar>(y, x) = cv::saturate_cast<uchar>(255.0 - (2.0 * (255.0 - baseVal) * (255.0 - blendVal)) / 255.0);
                }
                else if (channels == 3)
                {
                    cv::Vec3b basePixel = baseImg.at<cv::Vec3b>(y, x);
                    cv::Vec3b blendPixel = blendImg.at<cv::Vec3b>(y, x);
                    cv::Vec3b& resultPixel = result.at<cv::Vec3b>(y, x);
                    
                    if (basePixel[c] < 128)
                        resultPixel[c] = cv::saturate_cast<uchar>((2.0 * basePixel[c] * blendPixel[c]) / 255.0);
                    else
                        resultPixel[c] = cv::saturate_cast<uchar>(255.0 - (2.0 * (255.0 - basePixel[c]) * (255.0 - blendPixel[c])) / 255.0);
                }
                else if (channels == 4)
                {
                    cv::Vec4b basePixel = baseImg.at<cv::Vec4b>(y, x);
                    cv::Vec4b blendPixel = blendImg.at<cv::Vec4b>(y, x);
                    cv::Vec4b& resultPixel = result.at<cv::Vec4b>(y, x);
                    
                    if (basePixel[c] < 128)
                        resultPixel[c] = cv::saturate_cast<uchar>((2.0 * basePixel[c] * blendPixel[c]) / 255.0);
                    else
                        resultPixel[c] = cv::saturate_cast<uchar>(255.0 - (2.0 * (255.0 - basePixel[c]) * (255.0 - blendPixel[c])) / 255.0);
                }
            }
        }
    }
    
    return result;
}

cv::Mat BlendNode::BlendDifference(const cv::Mat& baseImg, const cv::Mat& blendImg)
{
    cv::Mat result;
    // Difference blend: |Base - Blend|
    cv::absdiff(baseImg, blendImg, result);
    return result;
}

cv::Mat BlendNode::BlendLighten(const cv::Mat& baseImg, const cv::Mat& blendImg)
{
    cv::Mat result;
    // Lighten blend: max(Base, Blend)
    cv::max(baseImg, blendImg, result);
    return result;
}

cv::Mat BlendNode::BlendDarken(const cv::Mat& baseImg, const cv::Mat& blendImg)
{
    cv::Mat result;
    // Darken blend: min(Base, Blend)
    cv::min(baseImg, blendImg, result);
    return result;
}

cv::Mat BlendNode::ApplyOpacity(const cv::Mat& baseImg, const cv::Mat& blendedImg)
{
    // Apply opacity (blend between base image and blended result)
    cv::Mat result;
    cv::addWeighted(baseImg, 1.0f - m_Opacity, blendedImg, m_Opacity, 0.0, result);
    return result;
}

void BlendNode::DrawNodeContent()
{
    // Show parameter controls
    bool changed = false;

    // Blend Mode combo box
    const char* blendModes[] = { "Normal", "Multiply", "Screen", "Overlay", "Difference", "Lighten", "Darken" };
    int currentMode = static_cast<int>(m_BlendMode);
    const float itemWidth = 150.0f; // Define a width for the widgets

    ImGui::Text("Blend Mode:");
    ImGui::PushItemWidth(itemWidth);
    changed |= ImGui::Combo("##BlendMode", &currentMode, blendModes, IM_ARRAYSIZE(blendModes));
    ImGui::PopItemWidth();
    if (changed)
    {
        m_BlendMode = static_cast<BlendMode>(currentMode);
    }

    // Opacity slider
    ImGui::Text("Opacity:");
    ImGui::PushItemWidth(itemWidth);
    changed |= ImGui::SliderFloat("##Opacity", &m_Opacity, 0.0f, 1.0f, "%.2f");
    ImGui::PopItemWidth();
    ImGui::SameLine();
    if (ImGui::Button("Reset"))
    {
        m_Opacity = 1.0f;
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

void BlendNode::UpdatePreviewTexture()
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

    // Use ImageEditorApp singleton to create texture
    if (ImageEditorApp* app = ImageEditorApp::GetInstance())
    {
        m_PreviewTexture = app->CreateTexture(rgbImage.data, rgbImage.cols, rgbImage.rows);
    }
}

void BlendNode::CleanupTexture()
{
    // Clean up the preview texture
    if (m_PreviewTexture)
    {
        auto app = ImageEditorApp::GetInstance();
        if (app)
        {
            app->DestroyTexture(m_PreviewTexture);
        }
        m_PreviewTexture = nullptr;
    }
}
