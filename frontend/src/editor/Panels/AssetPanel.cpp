#include "AssetPanel.h"
#include "imgui.h"

void DrawAssetPanel(SceneState& sceneState, EditorState& editorState)
{
    ImGui::Begin("Project");

    ImGui::Text("Resource List");
    ImGui::Text("Click an asset to assign it to the selected object.");
    ImGui::Separator();

    const char* assets[] = { "test.png", "player.png", "enemy.png" };

    int index = editorState.selectedObjectIndex;
    bool hasSelection = (index >= 0 && index < static_cast<int>(sceneState.objects.size()));

    if (hasSelection) {
        ImGui::Text("Selected Object: %s", sceneState.objects[index].name.c_str());
        ImGui::Text("Current Texture: %s", sceneState.objects[index].texturePath.c_str());
    }
    else {
        ImGui::Text("No object selected");
    }

    ImGui::Separator();

    if (!hasSelection) {
        ImGui::BeginDisabled();
    }

    for (const char* asset : assets) {
        bool selected = false;
        if (hasSelection) {
            selected = (sceneState.objects[index].texturePath == asset);
        }

        if (ImGui::Selectable(asset, selected)) {
            if (hasSelection) {
                sceneState.objects[index].texturePath = asset;
            }
        }
    }

    if (!hasSelection) {
        ImGui::EndDisabled();
    }

    ImGui::Separator();
    ImGui::Text("Preview");
    if (hasSelection) {
        ImGui::BulletText("Texture file: %s", sceneState.objects[index].texturePath.c_str());
    }
    else {
        ImGui::BulletText("No texture selected");
    }

    ImGui::End();
}