#pragma once

#include "application.h"
#include <imgui.h>
#include <imgui_node_editor.h>
#include <memory>
#include "node-editor/NodeEditorManager.h" // Include the full header instead of forward declaration

namespace ed = ax::NodeEditor;

class Node; // We can keep this forward declaration

class ImageEditorApp : public Application
{
public:
    ImageEditorApp() : Application("Node-Based Image Editor") {}

    void OnStart() override;
    void OnFrame(float deltaTime) override;
    void OnStop() override;

    // UI components
    void ShowMainMenuBar();
    void ShowNodeEditor();
    void ShowPropertiesPanel();

    // Node management
    Node* CreateInputNode();
    Node* CreateOutputNode();
    void CreateProcessingNode(int nodeType);

    // Application state
    bool m_ShowDemoWindow = false;
    bool m_ShowImGuiDemoWindow = false;

    // Node editor
    std::unique_ptr<NodeEditorManager> m_NodeEditor;

    // Active elements
    Node* m_SelectedNode = nullptr;
};