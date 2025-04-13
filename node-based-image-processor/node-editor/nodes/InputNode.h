#pragma once

#include "../Node.h"

class ImageInputNode : public Node {
public:
    ImageInputNode(int id);
    ~ImageInputNode() override = default;

    // Node interface implementation
    void Process() override;
    void DrawNodeContent() override;

    // Image loading functionality
    bool LoadImageFile(const std::string& path);  // Renamed from LoadImage to avoid Windows macro conflict
    const cv::Mat& GetImage() const { return m_Image; }

    // Get image metadata
    int GetWidth() const { return m_Image.cols; }
    int GetHeight() const { return m_Image.rows; }
    int GetChannels() const { return m_Image.channels(); }
    size_t GetSizeBytes() const { return m_Image.total() * m_Image.elemSize(); }
    std::string GetImageFormat() const { return m_FileFormat; }

private:
    cv::Mat m_Image;
    std::string m_FilePath;
    std::string m_FileFormat;
    bool m_ImageLoaded = false;

    // For preview display
    ImTextureID m_PreviewTexture = nullptr;
    void UpdatePreviewTexture();
    void CleanupTexture();
};