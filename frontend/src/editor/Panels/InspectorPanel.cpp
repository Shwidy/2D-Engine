#include "InspectorPanel.h"
#include "imgui.h"
#include <cstring>

void DrawInspectorPanel(SceneState& sceneState, EditorState& editorState)
{
    ImGui::Begin("Inspector");

    static int lastSelectedIndex = -1;
    static char textureBuffer[128] = "";

    int index = editorState.selectedObjectIndex;
    if (index >= 0 && index < static_cast<int>(sceneState.objects.size())) {
        auto& obj = sceneState.objects[index];

        if (lastSelectedIndex != index) {
            strncpy(textureBuffer, obj.texturePath.c_str(), sizeof(textureBuffer));
            textureBuffer[sizeof(textureBuffer) - 1] = '\0';
            lastSelectedIndex = index;
        }

        ImGui::Text("Selected Object");
        ImGui::Separator();
        ImGui::Text("Name: %s", obj.name.c_str());
        ImGui::Text("ID: %d", obj.id);

        if (ImGui::InputText("Texture Path", textureBuffer, sizeof(textureBuffer))) {
            obj.texturePath = textureBuffer;
        }

        ImGui::Text("Texture: %s", obj.texturePath.c_str());
        ImGui::InputFloat2("Position", obj.position);
        ImGui::InputFloat2("Scale", obj.scale);
    }
    else {
        ImGui::Text("No object selected");
        lastSelectedIndex = -1;
    }

    ImGui::End();
}