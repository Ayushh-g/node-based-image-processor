#include "ImageEditorApp.h"
#include <vector>
#include <string>


void ImageEditorApp::OnStart()
{
    // Initialize the node editor manager
    ed::Config config;
    config.SettingsFile = "temppp.json";
    m_Context = ed::CreateEditor(&config);
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

    // Show demo windows if enabled
    if (m_ShowImGuiDemoWindow)
        ImGui::ShowDemoWindow(&m_ShowImGuiDemoWindow);
}

void ImageEditorApp::OnStop()
{
    // Cleanup resources
    ed::DestroyEditor(m_Context);
}

void ImageEditorApp::ShowMainMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New Graph"))
            {
                // Reset node editor
                ed::DestroyEditor(m_Context);
                ed::Config config;
                config.SettingsFile = "Widgets.json";
                m_Context = ed::CreateEditor(&config);
            }
            if (ImGui::MenuItem("Open Image"))  {}
            if (ImGui::MenuItem("Save Output")) {}
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) Close();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Create"))
        {
            if (ImGui::MenuItem("Input Node")) {}
            if (ImGui::MenuItem("Output Node")) {}
            ImGui::Separator();
            if (ImGui::BeginMenu("Processing Nodes"))
            {
                if (ImGui::MenuItem("Brightness/Contrast"))
                {
                    //  will do ...................
                }
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
    if (!m_Context)
        return;

    auto& io = ImGui::GetIO();

    ImGui::Text("FPS: %.2f (%.2gms)", io.Framerate, io.Framerate ? 1000.0f / io.Framerate : 0.0f);

    ImGui::Separator();

    ed::SetCurrentEditor(m_Context);
    ed::Begin("My Editor", ImVec2(0.0, 0.0f));
    int uniqueId = 1;
    // Start drawing nodes.
    ed::BeginNode(uniqueId++);
    ImGui::Text("Node A");
    ed::BeginPin(uniqueId++, ed::PinKind::Input);
    ImGui::Text("-> In");
    ed::EndPin();
    ImGui::SameLine();
    ed::BeginPin(uniqueId++, ed::PinKind::Output);
    ImGui::Text("Out ->");
    ed::EndPin();
    ed::EndNode();
    ed::End();
    ed::SetCurrentEditor(nullptr);


}

void ImageEditorApp::ShowPropertiesPanel()
{
    ImGui::BeginChild("PropertiesPanel", ImVec2(0, 0), true);
    ImGui::Text("Properties");
    ImGui::Separator();

    // Get selected node from the node editor
    if (m_Context)
    {
        ImGui::Text("No node selected");
    }

    ImGui::EndChild();
}



