#pragma once

#include "../Node.h"

class ThresholdNode : public Node {
public:
    ThresholdNode(int id);
    ~ThresholdNode() override = default;

    // Node interface implementation
    void Process() override;
    void DrawNodeContent() override;

private:
    // Input/Output images
    cv::Mat m_InputImage;
    cv::Mat m_Histogram;
    ImTextureID m_PreviewTexture = nullptr;
    ImTextureID m_HistogramTexture = nullptr;
    
    // Thresholding parameters
    int m_ThresholdType = 0;      // 0: Binary, 1: Adaptive, 2: Otsu
    double m_ThresholdValue = 128;  // For binary threshold (0-255)
    int m_AdaptiveBlockSize = 11;   // For adaptive threshold (odd values 3-99)
    double m_AdaptiveConstant = 2;  // For adaptive threshold
    bool m_InvertThreshold = false; // Invert the threshold result
    bool m_ShowPreview = true; // Added for optional preview

    // Helper methods
    void UpdatePreviewTexture();
    void CleanupTextures();
    void UpdateHistogram();
    cv::Mat ApplyThreshold(const cv::Mat& inputImage);
};
