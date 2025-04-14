#pragma once

#include "../Node.h"

class EdgeDetectionNode : public Node {
public:
    EdgeDetectionNode(int id);
    ~EdgeDetectionNode() override = default;

    // Node interface implementation
    void Process() override;
    void DrawNodeContent() override;

private:
    // Input/Output images
    cv::Mat m_InputImage;
    ImTextureID m_PreviewTexture = nullptr;
    
    // Edge detection parameters
    int m_DetectionType = 0;      // 0: Sobel, 1: Canny, 2: Laplacian
    
    // Sobel parameters
    int m_SobelKernelSize = 3;    // 1, 3, 5, 7
    int m_SobelDx = 1;            // x derivative order
    int m_SobelDy = 1;            // y derivative order
    
    // Canny parameters
    double m_CannyThreshold1 = 100.0;
    double m_CannyThreshold2 = 200.0;
    int m_CannyApertureSize = 3;  // 3, 5, 7
    bool m_CannyL2Gradient = false;
    
    // Laplacian parameters
    int m_LaplacianKernelSize = 3; // 1, 3, 5, 7
    double m_LaplacianScale = 1.0;
    double m_LaplacianDelta = 0.0;
    
    // Helper methods
    void UpdatePreviewTexture();
    void CleanupTexture();
    cv::Mat ApplyEdgeDetection(const cv::Mat& inputImage);
};