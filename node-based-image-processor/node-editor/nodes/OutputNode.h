#pragma once

#include "../Node.h"

class OutputNode : public Node {
public:
    OutputNode(int id);
    ~OutputNode() override = default;

    // Node interface implementation
    void Process() override;
    void DrawNodeContent() override;
    
    // Save functionality
    bool SaveImage(const std::string& path);
    
private:
    cv::Mat m_InputImage;
    cv::Mat m_PreviewImage;
    ImTextureID m_PreviewTexture = nullptr;
    
    // Save settings
    int m_OutputFormat = 0; // 0 = JPG, 1 = PNG, 2 = BMP
    int m_JpegQuality = 95;
    
    // Display helpers
    void UpdatePreviewTexture();
    void CleanupTexture();
    cv::Mat GetConnectedImage();
};