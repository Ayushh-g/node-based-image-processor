#include "ImageEditorApp.h"
#include "node-editor/nodes/InputNode.h"
#include "node-editor/nodes/OutputNode.h"
#include <vector>
#include <string>


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

Node* ImageEditorApp::CreateInputNode()
{
    if (!m_NodeEditor)
        return nullptr;

    // Position at the mouse cursor
    auto pos = ImGui::GetMousePos();

    // Create the node
    Node* node = m_NodeEditor->CreateNode(0, pos); // 0 = InputNode

    // Let the node handle file selection automatically
    if (node) {
        InputNode* imageNode = dynamic_cast<InputNode*>(node);
        if (imageNode) {
            imageNode->ShowOpenFileDialog();
        }
    }

    return node;
}

Node* ImageEditorApp::CreateOutputNode()
{
    if (!m_NodeEditor)
        return nullptr;

    // Position at the mouse cursor
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