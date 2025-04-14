#pragma once

#include "Node.h"
#include <unordered_map>
#include <deque>
#include <functional>

// Forward declaration to solve circular dependencies
class NodeEditorManager;

// Global pointer for Pin::IsConnected to access the editor
extern NodeEditorManager* g_NodeEditorManager;

// Represents a link between nodes
struct Link
{
    ed::LinkId ID;
    ed::PinId StartPinID;
    ed::PinId EndPinID;
    ImColor Color;

    Link(ed::LinkId id, ed::PinId startPinId, ed::PinId endPinId)
        : ID(id), StartPinID(startPinId), EndPinID(endPinId), Color(255, 255, 255) {
    }
};

class NodeEditorManager
{
public:
    NodeEditorManager();
    ~NodeEditorManager();

    void Initialize();
    void Shutdown();

    void Render();
    void ProcessNodes();
    void SyncAllNodes();

    // Node management
    Node* CreateNode(int nodeType, ImVec2 position = ImVec2(0, 0));
    void DeleteNode(ed::NodeId id);
    Node* FindNode(ed::NodeId id);

    // Link management
    Link* CreateLink(Pin* output, Pin* input);
    void DeleteLink(ed::LinkId id);
    Link* FindLink(ed::LinkId id);
    bool IsLinkValid(Pin* output, Pin* input);

    // Pin management
    Pin* FindPin(ed::PinId id);
    bool IsPinLinked(ed::PinId id);
    std::vector<Link*> GetLinksForPin(ed::PinId id);

    // Selection management
    Node* GetSelectedNode();
    void ProcessSelection();

    // Get next available ID for nodes, links
    int GetNextId();

    // Get all links in the editor
    std::vector<Link*> GetLinks() const;

    // Access to node editor context
    ed::EditorContext* GetEditorContext() const { return m_EditorContext; }

private:
    // Node editor context
    ed::EditorContext* m_EditorContext = nullptr;

    // Graph data
    std::vector<std::unique_ptr<Node>> m_Nodes;
    std::vector<std::unique_ptr<Link>> m_Links;

    // Cache for quick lookups
    std::unordered_map<uint64_t, Node*> m_NodeMap; // Map of NodeId -> Node*
    std::unordered_map<uint64_t, Link*> m_LinkMap; // Map of LinkId -> Link*

    // Processing queue to handle node evaluation in correct order
    bool CalculateProcessingOrder();
    std::deque<Node*> m_ProcessingQueue;
    int m_NextId = 1;

    // Handle interaction
    void HandleCreation();
    void HandleDeletion();

    // For detecting selection changes
    ed::NodeId m_SelectedNodeId;
};