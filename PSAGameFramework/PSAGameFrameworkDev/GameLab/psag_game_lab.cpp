// psag_game_lab.
#include "psag_game_lab.h"

using namespace std;

bool PSAGameFrameworkLab::LogicInitialization(const Vector2T<uint32_t>&WinSize) {

	return true;
}

void PSAGameFrameworkLab::LogicCloseFree() {
	Texture[0].DeletePointer();
	Texture[1].DeletePointer();
	Texture[2].DeletePointer();
	Texture[3].DeletePointer();
}
Vector2T<size_t> Blocks[4] = {
	Vector2T<size_t>(1, 1),
	Vector2T<size_t>(2, 1),
	Vector2T<size_t>(1, 2),
	Vector2T<size_t>(2, 2)
};

PsagManager::NoiseMap::SharedCache TestCache = {};

bool PSAGameFrameworkLab::LogicEventLoopGame(GameLogic::FrameworkParams& RunningState) {
	ImGui::Begin("TEST: NoiseMap");
	{
		if (ImGui::Button("Generate Block")) {
			for (size_t i = 0; i < 4; ++i) {

				PsagManager::NoiseMap::NoiseBlockDESC Config = {};
				Config.BlockNumberGrids = 8 * 8;
				Config.CacheIndex = &TestCache;

				PsagManager::NoiseMap::NoiseBlock TestGenMap(
					512, Blocks[i].vector_x, Blocks[i].vector_y, Config
				);
				Texture[i].CreatePointer(TestGenMap.GenerateNoiseDataImage());
				TextureIndex[i] = Texture[i].Get()->GetTextureView();
			}
		}
		ImVec2 Coord = ImGui::GetCursorPos();
		ImGui::Image(IMGUI_TEXTURE_ID(TextureIndex[0]), ImVec2(384.0f, 384.0f));
		ImGui::SetCursorPos(ImVec2(Coord.x + 384.0f, Coord.y));
		ImGui::Image(IMGUI_TEXTURE_ID(TextureIndex[1]), ImVec2(384.0f, 384.0f));

		ImGui::SetCursorPos(ImVec2(Coord.x, Coord.y + 384.0f));
		ImGui::Image(IMGUI_TEXTURE_ID(TextureIndex[2]), ImVec2(384.0f, 384.0f));
		ImGui::SetCursorPos(ImVec2(Coord.x + 384.0f, Coord.y + 384.0f));
		ImGui::Image(IMGUI_TEXTURE_ID(TextureIndex[3]), ImVec2(384.0f, 384.0f));
	}
	ImGui::End();
	return true;
}