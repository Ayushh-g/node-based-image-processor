#include "ImageDataManager.h"

void ImageDataManager::SetImageData(ed::PinId outputPinId, const cv::Mat& image)
{
    // Store the image data for the output pin
    uint64_t pinId = outputPinId.Get();
    
    // Clone the image to ensure we have our own copy
    if (!image.empty())
    {
        m_ImageData[pinId] = image.clone();
    }
    else
    {   
        // If image is empty, remove any existing data
        auto it = m_ImageData.find(pinId);
        if (it != m_ImageData.end())
        {
            m_ImageData.erase(it);
        }
    }
}

cv::Mat ImageDataManager::GetImageData(ed::PinId inputPinId)
{
    uint64_t pinId = inputPinId.Get();
    
    // Check if this input pin is connected to an output pin
    auto connIt = m_Connections.find(pinId);
    if (connIt == m_Connections.end())
    {
        // No connection, return empty image
        return cv::Mat();
    }
    
    // Get the output pin ID this input is connected to
    uint64_t outputPinId = connIt->second;
    
    // Look up the image data for the output pin
    auto it = m_ImageData.find(outputPinId);
    if (it == m_ImageData.end())
    {
        // No image data available, return empty image
        return cv::Mat();
    }
    
    // Return the image data (return a clone to prevent modification)
    return it->second.clone();
}

void ImageDataManager::Clear()
{
    // Clear all stored image data and connections
    m_ImageData.clear();
    m_Connections.clear();
}

void ImageDataManager::UpdateConnections(const std::vector<Link*>& links)
{
    // Clear existing connections
    m_Connections.clear();
    
    // Update connections based on the links
    for (auto* link : links)
    {
        // In our convention, StartPinID is always an output pin and EndPinID is always an input pin
        uint64_t outputPinId = link->StartPinID.Get();
        uint64_t inputPinId = link->EndPinID.Get();
        
        // Store the connection
        m_Connections[inputPinId] = outputPinId;
    }
}