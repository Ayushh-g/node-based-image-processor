#pragma once

#include "../Node.h"

class ColorChannelSplitterNode : public Node {
public:
    ColorChannelSplitterNode(int id);
    ~ColorChannelSplitterNode() override = default;

    // Node interface implementation
    void Process() override;
    void DrawNodeContent() override;

private:
    // Input/Output images
    cv::Mat m_InputImage;
    cv::Mat m_RedChannel;
    cv::Mat m_GreenChannel;
    cv::Mat m_BlueChannel;
    cv::Mat m_AlphaChannel;
    
    // Display options
    bool m_OutputGrayscale = true;
    bool m_ShowPreview = true; // Added for optional preview

    // Preview textures
    ImTextureID m_RedTexture = nullptr;
    ImTextureID m_GreenTexture = nullptr;
    ImTextureID m_BlueTexture = nullptr;
    ImTextureID m_AlphaTexture = nullptr;
    
    // Helper functions
    void UpdatePreviewTextures();
    void CleanupTextures();
};
