#pragma once

#include "../Node.h"

class InputNode : public Node {
public:
    InputNode(int id);
    ~InputNode() override = default;

    // Node interface implementation
    void Process() override;
    void DrawNodeContent() override;
    void OnSelected() override;

    // Image loading functionality
    bool LoadImageFile(const std::string& path);  // Renamed from LoadImage to avoid Windows macro conflict
    bool ShowOpenFileDialog();  // New method to show file dialog and load image
    const cv::Mat& GetImage() const { return m_Image; }

    // Get image metadata
    int GetWidth() const { return m_Image.cols; }
    int GetHeight() const { return m_Image.rows; }
    int GetChannels() const { return m_Image.channels(); }
    size_t GetSizeBytes() const { return m_Image.total() * m_Image.elemSize(); }
    std::string GetImageFormat() const { return m_FileFormat; }

private:
    cv::Mat m_Image;
    // m_OutputImage is already defined in Node class
    std::string m_FilePath;
    std::string m_FileFormat;
    bool m_ImageLoaded = false;

    // For preview display
    ImTextureID m_PreviewTexture = nullptr;
    void UpdatePreviewTexture();
    void CleanupTexture();
};