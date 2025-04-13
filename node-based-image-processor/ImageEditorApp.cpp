#include "ImageEditorApp.h"

#include "node-editor/nodes/InputNode.h"
#include "node-editor/nodes/OutputNode.h"
#include <vector>
#include <string>

// Simple file dialog implementation
#include <Windows.h>
#include <commdlg.h>
#include <direct.h>
#include <ShlObj.h>

void ImageEditorApp::OnStart()
{
    // Initialize the node editor manager
    m_NodeEditor = std::make_unique<NodeEditorManager>();
    m_NodeEditor->Initialize();
}

void ImageEditorApp::OnFrame(float deltaTime)
{
    // Show main menu bar
    ShowMainMenuBar();

    // Main layout
    ImGui::Columns(2);

    // Node editor area (left)
    ImGui::SetColumnWidth(0, ImGui::GetWindowWidth() * 0.7f);
    ShowNodeEditor();

    // Properties panel (right)
    ImGui::NextColumn();
    ShowPropertiesPanel();
    ImGui::Columns(1);

    // Process nodes (only if we have changes)
    m_NodeEditor->ProcessNodes();

    // Show demo windows if enabled
    if (m_ShowImGuiDemoWindow)
        ImGui::ShowDemoWindow(&m_ShowImGuiDemoWindow);
}

void ImageEditorApp::OnStop()
{
    // Cleanup resources
    if (m_NodeEditor)
        m_NodeEditor->Shutdown();
}

void ImageEditorApp::ShowMainMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New Graph", "Ctrl+N"))
            {
                // Reset node editor
                m_NodeEditor->Shutdown();
                m_NodeEditor->Initialize();
            }

            if (ImGui::MenuItem("Open Image", "Ctrl+O"))
            {
                OpenImage();
            }

            if (ImGui::MenuItem("Save Output", "Ctrl+S"))
            {
                SaveOutput();
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Exit", "Alt+F4"))
            {
                Close();
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Create"))
        {
            if (ImGui::MenuItem("Input Node"))
            {
                CreateInputNode();
            }

            if (ImGui::MenuItem("Output Node"))
            {
                CreateOutputNode();
            }

            ImGui::Separator();

            if (ImGui::BeginMenu("Processing Nodes"))
            {
                if (ImGui::MenuItem("Brightness/Contrast"))
                {
                    CreateProcessingNode(2);
                }

                if (ImGui::MenuItem("Color Channel Splitter"))
                {
                    CreateProcessingNode(3);
                }

                if (ImGui::MenuItem("Blur"))
                {
                    CreateProcessingNode(4);
                }

                if (ImGui::MenuItem("Threshold"))
                {
                    CreateProcessingNode(5);
                }

                if (ImGui::MenuItem("Edge Detection"))
                {
                    CreateProcessingNode(6);
                }

                if (ImGui::MenuItem("Blend"))
                {
                    // To be implemented
                    // CreateProcessingNode(7);
                }

                if (ImGui::MenuItem("Noise Generation"))
                {
                    // To be implemented
                    // CreateProcessingNode(8);
                }

                if (ImGui::MenuItem("Convolution Filter"))
                {
                    // To be implemented
                    // CreateProcessingNode(9);
                }

                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View"))
        {
            ImGui::MenuItem("ImGui Demo", nullptr, &m_ShowImGuiDemoWindow);

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void ImageEditorApp::ShowNodeEditor()
{
    ImGui::BeginChild("NodeEditorRegion", ImVec2(0, 0), true);

    // We'll let the node editor manager render the actual node editor
    if (m_NodeEditor)
    {
        m_NodeEditor->Render();
    }

    ImGui::EndChild();
}

void ImageEditorApp::ShowPropertiesPanel()
{
    ImGui::BeginChild("PropertiesPanel", ImVec2(0, 0), true);
    ImGui::Text("Properties");
    ImGui::Separator();

    // Get selected node from the node editor
    if (m_NodeEditor)
    {
        Node* selectedNode = m_NodeEditor->GetSelectedNode();
        if (selectedNode)
        {
            ImGui::Text("Selected Node: %s", selectedNode->Name.c_str());
            ImGui::Separator();

            // Depending on the node type, we might want to show specific properties
            // but most of this is handled directly by the node's DrawNodeContent method

        }
        else
        {
            ImGui::Text("No node selected");
        }
    }

    ImGui::EndChild();
}

void ImageEditorApp::OpenImage()
{
    // Open file dialog
    char filename[MAX_PATH] = "";

    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = "Image Files\0*.jpg;*.jpeg;*.png;*.bmp\0All Files\0*.*\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = "Open Image";
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (GetOpenFileNameA(&ofn))
    {
        // Create an input node and load the image
        Node* node = CreateInputNode();
        if (node)
        {
            // Check if the node is actually an ImageInputNode before calling LoadImageFile
            ImageInputNode* imageNode = dynamic_cast<ImageInputNode*>(node);
            if (imageNode)
            {
                imageNode->LoadImageFile(filename);
            }
        }
    }
}

void ImageEditorApp::SaveOutput()
{
    // Get output nodes
    std::vector<OutputNode*> outputNodes;

    // This is a naive approach - in a real app, we'd want to query the node graph
    // for all the output nodes and let the user choose which one to save
    if (m_NodeEditor)
    {
        for (int i = 0; i < 100; i++) // Arbitrary limit
        {
            auto node = m_NodeEditor->FindNode(ed::NodeId(i));
            if (node && dynamic_cast<OutputNode*>(node))
            {
                outputNodes.push_back(static_cast<OutputNode*>(node));
            }
        }
    }

    // If no output nodes, create one
    if (outputNodes.empty())
    {
        auto node = static_cast<OutputNode*>(CreateOutputNode());
        if (node)
        {
            outputNodes.push_back(node);
        }
    }

    // If we have output nodes, save the first one
    if (!outputNodes.empty())
    {
        // Open save file dialog
        char filename[MAX_PATH] = "output.jpg";

        OPENFILENAMEA ofn;
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = NULL;
        ofn.lpstrFilter = "JPEG Image\0*.jpg\0PNG Image\0*.png\0BMP Image\0*.bmp\0All Files\0*.*\0";
        ofn.lpstrFile = filename;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrTitle = "Save Image";
        ofn.Flags = OFN_OVERWRITEPROMPT;
        ofn.lpstrDefExt = "jpg";

        if (GetSaveFileNameA(&ofn))
        {
            // Save the image
            outputNodes[0]->SaveImage(filename);
        }
    }
}

Node* ImageEditorApp::CreateInputNode()
{
    if (!m_NodeEditor)
        return nullptr;

    // Position at the center of the visible area
    auto pos = ImGui::GetMousePos();

    // Create the node
    Node* node = m_NodeEditor->CreateNode(0, pos); // 0 = ImageInputNode

    return node;
}

Node* ImageEditorApp::CreateOutputNode()
{
    if (!m_NodeEditor)
        return nullptr;

    // Position at the center of the visible area
    auto pos = ImGui::GetMousePos();

    // Create the node
    Node* node = m_NodeEditor->CreateNode(1, pos); // 1 = OutputNode

    return node;
}

void ImageEditorApp::CreateProcessingNode(int nodeType)
{
    if (!m_NodeEditor)
        return;

    // Position at the center of the visible area
    auto pos = ImGui::GetMousePos();

    // Create the node
    m_NodeEditor->CreateNode(nodeType, pos);
}