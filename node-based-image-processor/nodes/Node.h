#pragma once

#include <imgui_node_editor.h>
#include <string>
#include <vector>
#include <memory>

namespace ed = ax::NodeEditor;

// Forward declarations 
class Pin;

// Enum for pin types
enum class PinType {
    Image,      // For image data
    Int,        // For integer parameters
    Float,      // For float parameters
    Bool,       // For boolean parameters
    String,     // For string parameters
    Color,      // For color parameters
    Channel     // For individual color channels
};

// Enum for pin direction
enum class PinKind {
    Input,
    Output
};

// Pin class for inputs/outputs
class Pin {
public:
    Pin(int id, const char* name, PinType type, PinKind kind);
    virtual ~Pin() = default;

    ed::PinId ID;
    class Node* Node; // pointer to Node class of name Node with forward declaration to class Node
    std::string Name;
    PinType Type;
    PinKind Kind;
    
    bool IsConnected() const; // const so that it can't change Pin's members
    ImColor GetColor() const;
};

// Base Node class
class Node {
	// abstract class, no object of this class will be made but will use derived classes through node*
public:
    Node(int id, const char* name, ImColor color = ImColor(255, 255, 255));
    virtual ~Node() = default;

    // Virtual methods for node operations
	virtual void Process() = 0; // pure virtual function creates abstract class
    virtual void DrawNodeContent() = 0;


    ed::NodeId ID;
    std::string Name;
    std::vector<Pin> Inputs;
    std::vector<Pin> Outputs;
    ImColor Color;
    ImVec2 Size;
    bool Dirty;  // Flag to indicate if node needs reprocessing

    // Add pins
    void AddInputPin(const char* name, PinType type);
    void AddOutputPin(const char* name, PinType type);

    // Get pin data
    Pin* FindPin(ed::PinId id);
    Pin* GetInputPin(int index);
    Pin* GetOutputPin(int index);

    // Setup connections
    void AddLink(ed::LinkId id, Pin* output, Pin* input);
    void RemoveLink(ed::LinkId id);

protected:

    std::vector<ed::LinkId> m_Links;
};

// Factory class to create specific node types
class NodeFactory {
public:
    static Node* CreateNode(int nodeType, int id);
};