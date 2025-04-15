#pragma once

#include "../Node.h"
#include <opencv2/opencv.hpp>
#include <vector>

class ConvolutionFilterNode : public Node
{
public:
    ConvolutionFilterNode(int id);
    ~ConvolutionFilterNode() override; // Add virtual destructor

    void Process() override;
    void DrawNodeContent() override;

private:
    void UpdatePreviewTexture();
    void CleanupTexture();
    void ApplyPreset(int presetIndex);
    void UpdateKernelFromUI();

    cv::Mat m_InputImage;
    // m_OutputImage is inherited from Node base class
    void* m_PreviewTexture = nullptr; // Use void* for texture handle
    bool m_ShowPreview = true; // Added for optional preview

    // Kernel parameters
    int m_KernelSize = 3; // 3 for 3x3, 5 for 5x5
    std::vector<float> m_KernelValues; // Flattened kernel matrix
    cv::Mat m_Kernel; // OpenCV kernel matrix

    // Presets
    enum Preset { SHARPEN, EMBOSS, EDGE_ENHANCE };
};
