#include "NodeEditorManager.h"
#include "ImageDataManager.h"
#include <queue>
#include <set>
#include <algorithm>

// Initialize the global pointer
NodeEditorManager* g_NodeEditorManager = nullptr;

NodeEditorManager::NodeEditorManager()
    : m_EditorContext(nullptr), m_SelectedNodeId(0)
{
    // Set the global pointer to this instance
    g_NodeEditorManager = this;
}

NodeEditorManager::~NodeEditorManager()
{
    Shutdown();

    // Clear the global pointer if it's pointing to this instance
    if (g_NodeEditorManager == this)
        g_NodeEditorManager = nullptr;
}

void NodeEditorManager::Initialize()
{
    // Configure and create editor context
    ed::Config config;
    config.SettingsFile = "NodeEditor.json";
    config.UserPointer = this;

    m_EditorContext = ed::CreateEditor(&config);
    ed::SetCurrentEditor(m_EditorContext);
}

void NodeEditorManager::Shutdown()
{
    if (m_EditorContext)
    {
        ed::DestroyEditor(m_EditorContext);
        m_EditorContext = nullptr;
    }

    m_Nodes.clear();
    m_Links.clear();
    m_NodeMap.clear();
    m_LinkMap.clear();
}

void NodeEditorManager::Render()
{
    if (!m_EditorContext)
        return;

    ed::SetCurrentEditor(m_EditorContext);

    // Begin the node editor canvas
    ed::Begin("Image Processing Editor");

    // Draw all nodes
    for (auto& node : m_Nodes)
    {
        ed::BeginNode(node->ID);

        ImGui::Text("%s", node->Name.c_str());
        ImGui::Dummy(ImVec2(0, 5));

        // Draw node content (parameters, previews, etc.)
        node->DrawNodeContent();

        // Draw input pins
        for (auto& input : node->Inputs)
        {
            ed::BeginPin(input.ID, ed::PinKind::Input);
            ImGui::BeginHorizontal(input.ID.AsPointer());

            // Draw pin icon
            const float pinSize = 24.0f;
            ImColor color = input.GetColor();
            ImVec2 cursorPos = ImGui::GetCursorScreenPos();
            ImGui::GetWindowDrawList()->AddCircleFilled(
                ImVec2(cursorPos.x + pinSize / 2, cursorPos.y + pinSize / 2),
                pinSize / 4, color, 12);

            ImGui::Dummy(ImVec2(pinSize, pinSize));
            ImGui::Spring(0);
            ImGui::TextUnformatted(input.Name.c_str());
            ImGui::EndHorizontal();
            ed::EndPin();
        }

        // Draw output pins
        for (auto& output : node->Outputs)
        {
            ed::BeginPin(output.ID, ed::PinKind::Output);
            ImGui::BeginHorizontal(output.ID.AsPointer());

            ImGui::TextUnformatted(output.Name.c_str());
            ImGui::Spring(0);

            // Draw pin icon
            const float pinSize = 24.0f;
            ImColor color = output.GetColor();
            ImVec2 cursorPos = ImGui::GetCursorScreenPos();
            ImGui::GetWindowDrawList()->AddCircleFilled(
                ImVec2(cursorPos.x + pinSize / 2, cursorPos.y + pinSize / 2),
                pinSize / 4, color, 12);

            ImGui::Dummy(ImVec2(pinSize, pinSize));
            ImGui::EndHorizontal();
            ed::EndPin();
        }

        ed::EndNode();
    }

    // Draw all links
    for (auto& link : m_Links)
    {
        ed::Link(link->ID, link->StartPinID, link->EndPinID, link->Color, 2.0f);
    }

    // Handle interactions (creating/deleting links and nodes)
    HandleCreation();
    HandleDeletion();

    // Process selection changes
    ProcessSelection();

    ed::End();
    ed::SetCurrentEditor(nullptr);
}

Node* NodeEditorManager::CreateNode(int nodeType, ImVec2 position)
{
    auto node = std::unique_ptr<Node>(NodeFactory::CreateNode(nodeType, GetNextId()));// taking ownership of raw pointer
    if (!node)
        return nullptr;

    // Store node in our structures
    Node* nodePtr = node.get(); // Get raw pointer 
    m_Nodes.push_back(std::move(node)); // transfer ownership to vector 
    m_NodeMap[(uint64_t)nodePtr->ID.Get()] = nodePtr;

    // Set node position - make sure editor context is set
    if (m_EditorContext) {
        ed::SetCurrentEditor(m_EditorContext);
        ed::SetNodePosition(nodePtr->ID, position);
        ed::SetCurrentEditor(nullptr);
    }

    // Mark as dirty to ensure it gets processed
    nodePtr->Dirty = true;

    return nodePtr;
}

void NodeEditorManager::DeleteNode(ed::NodeId id)
{
    auto it = std::find_if(m_Nodes.begin(), m_Nodes.end(),
        [id](const std::unique_ptr<Node>& node) { return node->ID == id; });

    if (it != m_Nodes.end())
    {
        // Remove all links connected to this node
        auto linksToRemove = std::vector<ed::LinkId>();
        for (auto& link : m_Links)
        {
            auto startPin = FindPin(link->StartPinID);
            auto endPin = FindPin(link->EndPinID);

            if ((startPin && startPin->Node->ID == id) ||
                (endPin && endPin->Node->ID == id))
            {
                linksToRemove.push_back(link->ID);
            }
        }

        for (auto linkId : linksToRemove)
        {
            DeleteLink(linkId);
        }

        // Remove from map
        m_NodeMap.erase((uint64_t)id.Get());

        // Remove the node
        m_Nodes.erase(it);
    }
}

Node* NodeEditorManager::FindNode(ed::NodeId id)
{
    auto it = m_NodeMap.find((uint64_t)id.Get());
    return (it != m_NodeMap.end()) ? it->second : nullptr;
}

Link* NodeEditorManager::CreateLink(Pin* output, Pin* input)
{
    if (!IsLinkValid(output, input))
        return nullptr;

    // Create a new link
    auto link = std::make_unique<Link>(ed::LinkId(GetNextId()), output->ID, input->ID);
    Link* linkPtr = link.get();

    // Store in our structures
    m_Links.push_back(std::move(link));
    m_LinkMap[(uint64_t)linkPtr->ID.Get()] = linkPtr;

    // Directly mark nodes as dirty
    if (output->Node)
        output->Node->Dirty = true;
    if (input->Node)
        input->Node->Dirty = true;

    return linkPtr;
}

void NodeEditorManager::DeleteLink(ed::LinkId id)
{
    auto it = std::find_if(m_Links.begin(), m_Links.end(),
        [id](const std::unique_ptr<Link>& link) { return link->ID == id; });

    if (it != m_Links.end())
    {
        // Get pins connected by this link
        auto startPin = FindPin((*it)->StartPinID);
        auto endPin = FindPin((*it)->EndPinID);

        // Directly mark nodes as dirty
        if (startPin && startPin->Node)
            startPin->Node->Dirty = true;
        if (endPin && endPin->Node)
            endPin->Node->Dirty = true;

        // Remove from map
        m_LinkMap.erase((uint64_t)id.Get());

        // Remove the link
        m_Links.erase(it);
    }
}

Link* NodeEditorManager::FindLink(ed::LinkId id)
{
    auto it = m_LinkMap.find((uint64_t)id.Get());
    return (it != m_LinkMap.end()) ? it->second : nullptr;
}

bool NodeEditorManager::IsLinkValid(Pin* output, Pin* input)
{
    // Basic validation
    if (!output || !input || output == input ||
        output->Kind == input->Kind ||
        output->Type != input->Type ||
        output->Node == input->Node)
    {
        return false;
    }

    // Make sure input pin is actually an input and output pin is actually an output
    if (output->Kind != PinKind::Output || input->Kind != PinKind::Input)
        return false;

    // Check for existing links - inputs can only have one connection
    if (IsPinLinked(input->ID))
        return false;

    // Check for circular dependencies
    // This would require traversing the graph - implemented later

    return true;
}

Pin* NodeEditorManager::FindPin(ed::PinId id)
{
    // Search all nodes for this pin
    for (auto& node : m_Nodes)
    {
        if (auto pin = node->FindPin(id))
            return pin;
    }

    return nullptr;
}

bool NodeEditorManager::IsPinLinked(ed::PinId id)
{
    // Check if this pin is connected to any link
    for (auto& link : m_Links)
    {
        if (link->StartPinID == id || link->EndPinID == id)
            return true;
    }

    return false;
}

std::vector<Link*> NodeEditorManager::GetLinksForPin(ed::PinId id)
{
    std::vector<Link*> result;

    for (auto& link : m_Links)
    {
        if (link->StartPinID == id || link->EndPinID == id)
            result.push_back(link.get());
    }

    return result;
}

Node* NodeEditorManager::GetSelectedNode()
{
    if (m_SelectedNodeId)
        return FindNode(m_SelectedNodeId);
    return nullptr;
}

void NodeEditorManager::ProcessSelection()
{
    // Get current selection
    auto nodeCount = ed::GetSelectedObjectCount();
    if (nodeCount > 0)
    {
        std::vector<ed::NodeId> selectedNodes(nodeCount);
        auto count = ed::GetSelectedNodes(selectedNodes.data(), nodeCount);

        if (count > 0)
        {
            // For simplicity, just use the first selected node
            auto selectedNodeId = selectedNodes[0];

            // Check if selection changed
            if (selectedNodeId != m_SelectedNodeId)
            {
                // Notify previously selected node
                if (m_SelectedNodeId)
                {
                    if (auto node = FindNode(m_SelectedNodeId))
                        node->OnDeselected();
                }

                // Update selection
                m_SelectedNodeId = selectedNodeId;

                // Notify newly selected node
                if (auto node = FindNode(m_SelectedNodeId))
                    node->OnSelected();
            }
        }
    }
    else if (m_SelectedNodeId) // Nothing selected now
    {
        // Notify previously selected node
        if (auto node = FindNode(m_SelectedNodeId))
            node->OnDeselected();

        m_SelectedNodeId = 0;
    }
}

bool NodeEditorManager::CalculateProcessingOrder()
{
    // Clear the processing queue
    m_ProcessingQueue.clear();

    // Find all nodes with no inputs (source nodes)
    std::vector<Node*> sourceNodes;
    for (auto& node : m_Nodes)
    {
        bool isSourceNode = true;
        for (auto& input : node->Inputs)
        {
            if (IsPinLinked(input.ID))
            {
                isSourceNode = false;
                break;
            }
        }

        if (isSourceNode)
            sourceNodes.push_back(node.get());
    }

    // No source nodes, can't process
    if (sourceNodes.empty())
        return false;

    // Use topological sort to order nodes
    std::set<Node*> visited;
    std::set<Node*> inStack; // For cycle detection
    std::function<bool(Node*)> visit;

    visit = [&](Node* node) -> bool {
        if (inStack.find(node) != inStack.end())
            return false; // Cycle detected

        if (visited.find(node) != visited.end())
            return true; // Already processed

        inStack.insert(node);

        // Find all nodes connected to outputs of this node
        for (auto& output : node->Outputs)
        {
            for (auto& link : m_Links)
            {
                if (link->StartPinID == output.ID)
                {
                    auto inputPin = FindPin(link->EndPinID);
                    if (inputPin && inputPin->Node)
                    {
                        if (!visit(inputPin->Node))
                            return false;
                    }
                }
            }
        }

        visited.insert(node);
        inStack.erase(node);

        // Add to processing queue in reverse order (we'll reverse it at the end)
        m_ProcessingQueue.push_front(node);

        return true;
        };

    // Start with source nodes
    for (auto node : sourceNodes)
    {
        if (!visit(node))
            return false; // Cycle detected
    }

    // Make sure all nodes are included
    if (visited.size() != m_Nodes.size())
    {
        // There are nodes that aren't connected to any source
        // We can either fail or include them at the end
        for (auto& node : m_Nodes)
        {
            if (visited.find(node.get()) == visited.end())
            {
                m_ProcessingQueue.push_back(node.get());
            }
        }
    }

    return true;
}

void NodeEditorManager::ProcessNodes()
{
    // Calculate processing order
    if (!CalculateProcessingOrder())
        return; // Failed to calculate order (likely due to cycles)

    // Update connection map in the ImageDataManager
    ImageDataManager::GetInstance().UpdateConnections(GetLinks());

    // Process nodes in order
    for (auto node : m_ProcessingQueue)
    {
        if (node->Dirty)
        {
            node->Process();
            node->Dirty = false;
        }
    }
}

void NodeEditorManager::HandleCreation()
{
    if (ed::BeginCreate())
    {
        ed::PinId startPinId, endPinId;
        if (ed::QueryNewLink(&startPinId, &endPinId))
        {
            // Get the pins
            auto startPin = FindPin(startPinId);
            auto endPin = FindPin(endPinId);

            // Check if connection is valid
            if (startPin && endPin)
            {
                // Determine which pin is output and which is input
                Pin* outputPin = nullptr;
                Pin* inputPin = nullptr;

                if (startPin->Kind == PinKind::Output && endPin->Kind == PinKind::Input)
                {
                    outputPin = startPin;
                    inputPin = endPin;
                }
                else if (startPin->Kind == PinKind::Input && endPin->Kind == PinKind::Output)
                {
                    outputPin = endPin;
                    inputPin = startPin;
                }

                if (outputPin && inputPin && IsLinkValid(outputPin, inputPin))
                {
                    // Accept the link
                    if (ed::AcceptNewItem())
                    {
                        CreateLink(outputPin, inputPin);
                    }
                }
                else
                {
                    // Reject the link
                    ed::RejectNewItem();
                }
            }
        }
    }
    ed::EndCreate();
}

void NodeEditorManager::HandleDeletion()
{
    if (ed::BeginDelete())
    {
        // Handle link deletion
        ed::LinkId deletedLinkId;
        while (ed::QueryDeletedLink(&deletedLinkId))
        {
            if (ed::AcceptDeletedItem())
            {
                DeleteLink(deletedLinkId);
            }
        }

        // Handle node deletion
        ed::NodeId deletedNodeId;
        while (ed::QueryDeletedNode(&deletedNodeId))
        {
            if (ed::AcceptDeletedItem())
            {
                DeleteNode(deletedNodeId);
            }
        }
    }
    ed::EndDelete();
}

int NodeEditorManager::GetNextId()
{
    return m_NextId++;
}

std::vector<Link*> NodeEditorManager::GetLinks() const
{
    std::vector<Link*> links;
    links.reserve(m_Links.size());

    for (const auto& link : m_Links)
    {
        links.push_back(link.get());
    }

    return links;
}