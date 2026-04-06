#include "ScenePanel.h"
#include "imgui.h"
#include "../../../../backend/SceneSerializer.h"
#include <algorithm>
#include <cstdint>

static std::string sceneStatus = "No scene operation yet";

namespace {

constexpr float kSceneObjectBaseSize = 64.0f;

bool PointInRect(float x, float y, float left, float top, float width, float height) {
    return x >= left && y >= top && x <= left + width && y <= top + height;
}

bool MapScreenToScenePoint(const EditorState& editorState, const ImVec2& screenPos, float& sceneX, float& sceneY) {
    if (editorState.sceneViewportScreenWidth <= 0.0f || editorState.sceneViewportScreenHeight <= 0.0f ||
        editorState.sceneViewportWidth <= 0.0f || editorState.sceneViewportHeight <= 0.0f) {
        return false;
    }

    if (!PointInRect(
        screenPos.x,
        screenPos.y,
        editorState.sceneViewportScreenX,
        editorState.sceneViewportScreenY,
        editorState.sceneViewportScreenWidth,
        editorState.sceneViewportScreenHeight)) {
        return false;
    }

    const float normalizedX = (screenPos.x - editorState.sceneViewportScreenX) / editorState.sceneViewportScreenWidth;
    const float normalizedY = (screenPos.y - editorState.sceneViewportScreenY) / editorState.sceneViewportScreenHeight;
    sceneX = normalizedX * editorState.sceneViewportWidth;
    sceneY = normalizedY * editorState.sceneViewportHeight;
    return true;
}

int FindTopmostObjectAt(const SceneState& sceneState, float sceneX, float sceneY) {
    for (int index = static_cast<int>(sceneState.objects.size()) - 1; index >= 0; --index) {
        const GameObject& object = sceneState.objects[index];
        const float width = kSceneObjectBaseSize * std::max(object.scale[0], 0.0f);
        const float height = kSceneObjectBaseSize * std::max(object.scale[1], 0.0f);
        if (PointInRect(sceneX, sceneY, object.position[0], object.position[1], width, height)) {
            return index;
        }
    }

    return -1;
}

void AssignTextureAssetToObject(SceneState& sceneState, EditorState& editorState, int objectIndex, const AssetRecord& asset) {
    if (objectIndex < 0 || objectIndex >= static_cast<int>(sceneState.objects.size())) {
        editorState.assetStatus = "Drop a texture onto a scene object or select one first";
        return;
    }

    sceneState.objects[objectIndex].textureResourceId = asset.id;
    sceneState.objects[objectIndex].texturePath = !asset.relativePath.empty() ? asset.relativePath : asset.sourcePath;
    editorState.selectedObjectIndex = objectIndex;
    editorState.assetStatus = "Bound texture via drag: " + asset.name;
}

void AssignScriptAssetToObject(SceneState& sceneState, EditorState& editorState, int objectIndex, const AssetRecord& asset) {
    if (objectIndex < 0 || objectIndex >= static_cast<int>(sceneState.objects.size())) {
        editorState.assetStatus = "Drop a script onto a scene object or select one first";
        return;
    }

    sceneState.objects[objectIndex].scriptResourceId = asset.id;
    sceneState.objects[objectIndex].scriptPath = asset.sourcePath;
    editorState.selectedObjectIndex = objectIndex;
    editorState.assetStatus = "Bound script via drag: " + asset.name;
}

void DrawActionButtonRow(SceneState& sceneState, EditorState& editorState, int index) {
    const float width = ImGui::GetContentRegionAvail().x;
    const int columns = width < 780.0f ? 2 : 4;
    const float spacing = ImGui::GetStyle().ItemSpacing.x;
    const float buttonWidth = (width - spacing * static_cast<float>(columns - 1)) / static_cast<float>(columns);

    auto drawButton = [&](const char* label, auto&& fn) {
        if (ImGui::Button(label, ImVec2(buttonWidth, 0.0f))) {
            fn();
        }
    };

    drawButton("Reset Position", [&]() {
        if (index >= 0 && index < static_cast<int>(sceneState.objects.size())) {
            sceneState.objects[index].position[0] = 0.0f;
            sceneState.objects[index].position[1] = 0.0f;
        }
    });

    if (columns > 1) ImGui::SameLine();
    drawButton("Reset Scale", [&]() {
        if (index >= 0 && index < static_cast<int>(sceneState.objects.size())) {
            sceneState.objects[index].scale[0] = 1.0f;
            sceneState.objects[index].scale[1] = 1.0f;
        }
    });

    if (columns > 2) {
        ImGui::SameLine();
    } else {
        ImGui::NewLine();
    }
    drawButton("Reset Rotation", [&]() {
        if (index >= 0 && index < static_cast<int>(sceneState.objects.size())) {
            sceneState.objects[index].rotation = 0.0f;
        }
    });

    if (columns > 3) {
        ImGui::SameLine();
    } else {
        ImGui::NewLine();
    }
    drawButton("Add Object", [&]() {
        GameObject newObject;
        newObject.id = static_cast<int>(sceneState.objects.size());
        newObject.name = "New Object " + std::to_string(newObject.id);
        newObject.position[0] = 0.0f;
        newObject.position[1] = 0.0f;
        newObject.scale[0] = 1.0f;
        newObject.scale[1] = 1.0f;
        newObject.rotation = 0.0f;
        newObject.textureResourceId = 0;
        newObject.texturePath = "pillar.png";
        newObject.scriptResourceId = 0;
        newObject.scriptPath.clear();
        sceneState.objects.push_back(newObject);
        editorState.selectedObjectIndex = static_cast<int>(sceneState.objects.size()) - 1;
    });

    if (columns > 4) {
        ImGui::SameLine();
    } else {
        ImGui::NewLine();
    }
    drawButton("Delete Selected", [&]() {
        if (index >= 0 && index < static_cast<int>(sceneState.objects.size())) {
            sceneState.objects.erase(sceneState.objects.begin() + index);
            editorState.isDraggingSceneObject = false;
            editorState.draggingObjectIndex = -1;
            editorState.sceneDragOffsetX = 0.0f;
            editorState.sceneDragOffsetY = 0.0f;

            if (sceneState.objects.empty()) {
                editorState.selectedObjectIndex = -1;
            }
            else if (index >= static_cast<int>(sceneState.objects.size())) {
                editorState.selectedObjectIndex = static_cast<int>(sceneState.objects.size()) - 1;
            }
        }
    });
}

}  // namespace

void DrawScenePanel(SceneState& sceneState, EditorState& editorState, SDL_Texture* sceneTexture)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);

    ImGui::Begin("Scene");

    int index = editorState.selectedObjectIndex;
    ImGui::TextUnformatted("Viewport");

    ImVec2 availableRegion = ImGui::GetContentRegionAvail();
    float viewportHeight = std::clamp(availableRegion.y * 0.34f, 140.0f, 420.0f);
    ImVec2 viewportSize(availableRegion.x, viewportHeight);

    editorState.sceneViewportWidth = viewportSize.x;
    editorState.sceneViewportHeight = viewportSize.y;
    editorState.sceneViewportScreenWidth = 0.0f;
    editorState.sceneViewportScreenHeight = 0.0f;

    ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(0, 0, 0, 255));
    ImGui::BeginChild("SceneViewport", viewportSize, true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    const ImVec2 childScreenPos = ImGui::GetCursorScreenPos();
    editorState.sceneViewportScreenX = childScreenPos.x;
    editorState.sceneViewportScreenY = childScreenPos.y;
    editorState.sceneViewportScreenWidth = viewportSize.x;
    editorState.sceneViewportScreenHeight = viewportSize.y;

    if (sceneTexture) {
        float textureWidth = 1.0f;
        float textureHeight = 1.0f;
        if (!SDL_GetTextureSize(sceneTexture, &textureWidth, &textureHeight)) {
            textureWidth = viewportSize.x;
            textureHeight = viewportSize.y;
        }

        float scale = std::min(viewportSize.x / textureWidth, viewportSize.y / textureHeight);
        scale = (scale > 0.0f) ? scale : 1.0f;

        ImVec2 imageSize(textureWidth * scale, textureHeight * scale);
        ImVec2 cursor = ImGui::GetCursorPos();
        float offsetX = (viewportSize.x - imageSize.x) * 0.5f;
        float offsetY = (viewportSize.y - imageSize.y) * 0.5f;
        ImGui::SetCursorPos(ImVec2(cursor.x + std::max(0.0f, offsetX), cursor.y + std::max(0.0f, offsetY)));
        const ImVec2 imageScreenPos = ImGui::GetCursorScreenPos();
        editorState.sceneViewportScreenX = imageScreenPos.x;
        editorState.sceneViewportScreenY = imageScreenPos.y;
        editorState.sceneViewportScreenWidth = imageSize.x;
        editorState.sceneViewportScreenHeight = imageSize.y;
        ImGui::Image((ImTextureID)(intptr_t)sceneTexture, imageSize);

        if (ImGui::BeginDragDropTarget()) {
            int dropTargetIndex = editorState.selectedObjectIndex;
            float sceneX = 0.0f;
            float sceneY = 0.0f;
            if (MapScreenToScenePoint(editorState, ImGui::GetIO().MousePos, sceneX, sceneY)) {
                const int hoveredObject = FindTopmostObjectAt(sceneState, sceneX, sceneY);
                if (hoveredObject >= 0) {
                    dropTargetIndex = hoveredObject;
                }
            }

            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_TEXTURE_ID")) {
                const std::uint64_t assetId = *static_cast<const std::uint64_t*>(payload->Data);
                if (const AssetRecord* asset = editorState.assetRegistry.findById(assetId)) {
                    AssignTextureAssetToObject(sceneState, editorState, dropTargetIndex, *asset);
                }
            }

            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_SCRIPT_ID")) {
                const std::uint64_t assetId = *static_cast<const std::uint64_t*>(payload->Data);
                if (const AssetRecord* asset = editorState.assetRegistry.findById(assetId)) {
                    AssignScriptAssetToObject(sceneState, editorState, dropTargetIndex, *asset);
                }
            }

            ImGui::EndDragDropTarget();
        }
    }
    else {
        ImGui::GetWindowDrawList()->AddText(
            ImVec2(childScreenPos.x + 12.0f, childScreenPos.y + 12.0f),
            IM_COL32(220, 220, 220, 255),
            "Scene viewport is initializing..."
        );
    }

    ImGui::EndChild();
    ImGui::PopStyleColor();

    ImGui::Separator();
    DrawActionButtonRow(sceneState, editorState, index);
    ImGui::Spacing();
    const bool hasProject = !editorState.sceneFilePath.empty();

    if (!hasProject) {
        ImGui::BeginDisabled();
    }

    if (ImGui::Button("Save Scene")) {
        std::string sceneName = "UntitledScene";
        if (index >= 0 && index < static_cast<int>(sceneState.objects.size())) {
            sceneName = sceneState.objects[index].name;
        } else if (!editorState.projectName.empty()) {
            sceneName = editorState.projectName;
        }

        if (SaveSceneToFile(sceneState, sceneName, editorState.sceneFilePath)) {
            sceneStatus = "Scene saved successfully";
        }
        else {
            sceneStatus = "Failed to save scene";
        }
    }

    if (!hasProject) {
        ImGui::EndDisabled();
        ImGui::TextWrapped("Create or open a project before saving the scene.");
    }
    else {
        ImGui::TextWrapped("Scene File: %s", editorState.sceneFilePath.c_str());
    }
    ImGui::TextWrapped("Status: %s", sceneStatus.c_str());
    ImGui::TextWrapped("Scene Input: click to select, drag to move, drop texture or script assets onto objects.");

    ImGui::End();

    ImGui::PopStyleVar(2);
}
