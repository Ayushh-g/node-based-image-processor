#pragma once

#include "../Node.h"

class BrightnessContrastNode : public Node {
public:
    BrightnessContrastNode(int id);
    ~BrightnessContrastNode() override = default;

    // Node interface implementation
    void Process() override;
    void DrawNodeContent() override;

private:
    // Parameters
    float m_Brightness = 0.0f;  // Range: -100 to +100
    float m_Contrast = 1.0f;    // Range: 0 to 3
    
    // Input/Output images
    cv::Mat m_InputImage;
    ImTextureID m_PreviewTexture = nullptr;
    
    // Display helpers
    void UpdatePreviewTexture();
    void CleanupTexture();
    cv::Mat GetConnectedImage();
    
    // Reset functionality
    void ResetBrightness() { m_Brightness = 0.0f; Dirty = true; }
    void ResetContrast() { m_Contrast = 1.0f; Dirty = true; }
};