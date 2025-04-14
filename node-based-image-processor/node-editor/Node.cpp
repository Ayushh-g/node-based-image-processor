#include "Node.h"
#include "NodeEditorManager.h"
#include "nodes/InputNode.h"
#include "nodes/OutputNode.h"
#include "nodes/BrightnessContrastNode.h"
#include "nodes/ColorChannelSplitterNode.h"
#include "nodes/BlurNode.h"
#include "nodes/ThresholdNode.h"
#include "nodes/BlendNode.h"
#include "nodes/EdgeDetectionNode.h"

// Pin implementation
Pin::Pin(int id, const char* name, PinType type, PinKind kind)
    : ID(id), Node(nullptr), Name(name), Type(type), Kind(kind)
{
}

bool Pin::IsConnected() const
{
    // Use the global NodeEditorManager instance to check if this pin is connected
    if (g_NodeEditorManager)
    {
        return g_NodeEditorManager->IsPinLinked(ID);
    }
    return false;
}

ImColor Pin::GetColor() const
{
    // Return color based on pin type
    switch (Type)
    {
    case PinType::Image:   return ImColor(255, 128, 128);
    case PinType::Int:     return ImColor(68, 201, 156);
    case PinType::Float:   return ImColor(147, 226, 74);
    case PinType::Bool:    return ImColor(220, 48, 48);
    case PinType::String:  return ImColor(124, 21, 153);
    case PinType::Color:   return ImColor(51, 150, 215);
    case PinType::Channel: return ImColor(218, 0, 183);
    default:               return ImColor(255, 255, 255);
    }
}

// Node implementation
Node::Node(int id, const char* name, ImColor color)
    : ID(id), Name(name), Color(color), Size(0, 0), Dirty(true)
{
}

void Node::OnSelected()
{
    // Default implementation does nothing
}

void Node::OnDeselected()
{
    // Default implementation does nothing
}

void Node::AddInputPin(const char* name, PinType type)
{
    // Create an input pin with a unique ID and add it to inputs
    Inputs.emplace_back(Inputs.size() + 1000 * (int)ID.Get(), name, type, PinKind::Input);
    Inputs.back().Node = this;
}

void Node::AddOutputPin(const char* name, PinType type)
{
    // Create an output pin with a unique ID and add it to outputs
    Outputs.emplace_back(Outputs.size() + 2000 * (int)ID.Get(), name, type, PinKind::Output);
    Outputs.back().Node = this;
}

Pin* Node::FindPin(ed::PinId id)
{
    // Search in inputs
    for (auto& pin : Inputs)
    {
        if (pin.ID == id)
            return &pin;
    }

    // Search in outputs
    for (auto& pin : Outputs)
    {
        if (pin.ID == id)
            return &pin;
    }

    return nullptr;
}

Pin* Node::GetInputPin(int index)
{
    if (index >= 0 && index < Inputs.size())
        return &Inputs[index];
    return nullptr;
}

Pin* Node::GetOutputPin(int index)
{
    if (index >= 0 && index < Outputs.size())
        return &Outputs[index];
    return nullptr;
}

// Node factory - this will be expanded later with more node types
Node* NodeFactory::CreateNode(int nodeType, int id)
{
    // Create different node types based on nodeType
    switch (nodeType)
    {
    case 0:  // Image Input Node
        return new InputNode(id);

    case 1:  // Output Node
        return new OutputNode(id);

    case 2:  // Brightness/Contrast Node
        return new BrightnessContrastNode(id);

    case 3:  // Color Channel Splitter Node
        return new ColorChannelSplitterNode(id);

    case 4:  // Blur Node
        return new BlurNode(id);

    case 5:  // Threshold Node
        return new ThresholdNode(id);

    case 6:  // Edge Detection Node
        return new EdgeDetectionNode(id);

	case 7:  // Blend Node
		return new BlendNode(id);
        // Additional node types will be added as they are implemented

    default:
        return nullptr;
    }
}