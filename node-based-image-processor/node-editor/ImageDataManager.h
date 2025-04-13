#pragma once

#include <opencv2/opencv.hpp>
#include <unordered_map>
#include <string>
#include "NodeEditorManager.h"

// This class manages the image data flow between nodes
class ImageDataManager {
public:
    static ImageDataManager& GetInstance() {
        static ImageDataManager instance;
        return instance;
    }

    // Set image data for a pin
    void SetImageData(ed::PinId outputPinId, const cv::Mat& image);

    // Get image data from a pin
    cv::Mat GetImageData(ed::PinId inputPinId);

    // Clear all image data (e.g., when resetting the editor)
    void Clear();

    // Update connections based on links in the editor
    void UpdateConnections(const std::vector<Link*>& links);

private:
    ImageDataManager() = default;
    ~ImageDataManager() = default;

    // Maps output pin IDs to the image data they produce
    std::unordered_map<uint64_t, cv::Mat> m_ImageData;

    // Maps input pin IDs to the output pin IDs they're connected to
    std::unordered_map<uint64_t, uint64_t> m_Connections;
};