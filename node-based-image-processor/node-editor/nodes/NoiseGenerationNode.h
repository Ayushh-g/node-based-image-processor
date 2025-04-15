#pragma once

#include "../Node.h"
#include <opencv2/opencv.hpp>

class NoiseGenerationNode : public Node
{
public:
    NoiseGenerationNode(int id);
    ~NoiseGenerationNode() override;

    void Process() override;
    void DrawNodeContent() override;

private:
    void UpdatePreviewTexture();
    void CleanupTexture();
    void GenerateNoise();

    // m_OutputImage is inherited from Node base class
    void* m_PreviewTexture = nullptr; // Use void* for texture handle
    bool m_ShowPreview = true; // Added for optional preview

    // Noise parameters
    int m_Width = 256;
    int m_Height = 256;
    int m_NoiseType = 0; // 0: Uniform Random, 1: Gaussian Random (Future: Perlin, Simplex, Worley)
    float m_Scale = 1.0f; // General scale factor
    bool m_IsColor = false; // Generate color or grayscale noise

    // Parameters for specific noise types (example for Gaussian)
    double m_Mean = 128.0;
    double m_StdDev = 50.0;
};
