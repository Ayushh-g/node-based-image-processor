#pragma once

#include "../Node.h"

// Enum for blend modes
enum class BlendMode {
    Normal,
    Multiply,
    Screen,
    Overlay,
    Difference,
    Lighten,
    Darken
};

class BlendNode : public Node {
public:
    BlendNode(int id);
    ~BlendNode() override = default;

    // Node interface implementation
    void Process() override;
    void DrawNodeContent() override;

private:
    // Blend parameters
    BlendMode m_BlendMode = BlendMode::Normal;
    float m_Opacity = 1.0f;  // Range: 0 to 1
    
    // Input/Output images
    cv::Mat m_InputImage1;   // Base image
    cv::Mat m_InputImage2;   // Blend image
    ImTextureID m_PreviewTexture = nullptr;
    
    // Helper methods
    void UpdatePreviewTexture();
    void CleanupTexture();
    
    // Blend operations
    cv::Mat ApplyBlend(const cv::Mat& baseImg, const cv::Mat& blendImg);
    cv::Mat BlendNormal(const cv::Mat& baseImg, const cv::Mat& blendImg);
    cv::Mat BlendMultiply(const cv::Mat& baseImg, const cv::Mat& blendImg);
    cv::Mat BlendScreen(const cv::Mat& baseImg, const cv::Mat& blendImg);
    cv::Mat BlendOverlay(const cv::Mat& baseImg, const cv::Mat& blendImg);
    cv::Mat BlendDifference(const cv::Mat& baseImg, const cv::Mat& blendImg);
    cv::Mat BlendLighten(const cv::Mat& baseImg, const cv::Mat& blendImg);
    cv::Mat BlendDarken(const cv::Mat& baseImg, const cv::Mat& blendImg);
    
    // Apply opacity/alpha to the blended result
    cv::Mat ApplyOpacity(const cv::Mat& baseImg, const cv::Mat& blendedImg);
};