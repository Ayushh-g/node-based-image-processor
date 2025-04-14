#pragma once

#include "../Node.h"

class BlurNode : public Node {
public:
    BlurNode(int id);
    ~BlurNode() override = default;

    // Node interface implementation
    void Process() override;
    void DrawNodeContent() override;

private:
    // Input/Output images
    cv::Mat m_InputImage;
    ImTextureID m_PreviewTexture = nullptr;
    
    // Blur parameters
    int m_BlurRadius = 5;        // Range: 1-20
    bool m_DirectionalBlur = false;
    float m_DirectionalAngle = 0.0f;  // In degrees, 0-360
    float m_DirectionalFactor = 5.0f; // How strong the directional effect is
    
    // Kernel visualization
    cv::Mat m_Kernel;
    
    // Helper methods
    void UpdatePreviewTexture();
    void CleanupTexture();
    void GenerateKernel();
    cv::Mat ApplyBlur(const cv::Mat& inputImage);
};