#pragma once

#include "../Node.h"
#include <ctime>

class OutputNode : public Node {
public:
    OutputNode(int id);
    ~OutputNode() override;

    // Node interface implementation
    void Process() override;
    void DrawNodeContent() override;

    // Save functionality
    bool SaveImage(const std::string& path);
    bool ShowSaveFileDialog(); // New method to show file dialog and save image

private:
    cv::Mat m_InputImage;
    cv::Mat m_PreviewImage;
    ImTextureID m_PreviewTexture = nullptr;

    // Save settings
    int m_OutputFormat = 0; // 0 = JPG, 1 = PNG, 2 = BMP
    int m_JpegQuality = 95;
    int m_PngCompressionLevel = 3; // Default medium compression

    // Save feedback
    std::string m_LastSavePath;
    std::time_t m_SaveTimestamp = 0;
    bool m_SaveSuccess = false;

    // Display helpers
    void UpdatePreviewTexture();
    void CleanupTexture();
    cv::Mat GetConnectedImage();
};