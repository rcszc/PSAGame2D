// psag_game_lab.
#include "psag_game_lab.h"

using namespace std;

bool PSAGameFrameworkLab::LogicInitialization(const Vector2T<uint32_t>&WinSize) {

	return true;
}

void PSAGameFrameworkLab::LogicCloseFree() {
	
}

Vector2T<size_t> Blocks[4] = {
	Vector2T<size_t>(1, 1),
	Vector2T<size_t>(2, 1),
	Vector2T<size_t>(1, 2),
	Vector2T<size_t>(2, 2)
};

PsagManager::Tools::Random::NoiseSharedCache TestCache = {};

bool PSAGameFrameworkLab::LogicEventLoopGame(GameLogic::FrameworkParams& RunningState) {

	ImGui::Begin("TEST: NoiseMap");
	{
		if (ImGui::Button("Generate Block")) {
			for (size_t i = 0; i < 4; ++i) {

				PsagManager::Tools::Random::GenNoiseParamsDESC Config = {};
				Config.BlockNumberGrids = 8 * 8;
				Config.CacheIndex = &TestCache;

				PsagManager::Tools::Random::GenNoiseOctavePerlin TestGenMap(
					1024, Blocks[i].vector_x, Blocks[i].vector_y, Config
				);
				Texture[i].CreatePointer(TestGenMap.GenGrayscaleImage(
					[](size_t, size_t, double value) { return value < 8 ? 255 - value : 0; })
				);
				TextureIndex[i] = Texture[i].Get()->GetTextureView();
			}
		}
		ImGui::Image(IMGUI_TEXTURE_ID(TextureIndex[0]), ImVec2(384.0f, 384.0f));
		ImGui::SameLine();
		ImGui::Image(IMGUI_TEXTURE_ID(TextureIndex[1]), ImVec2(384.0f, 384.0f));
		ImGui::Image(IMGUI_TEXTURE_ID(TextureIndex[2]), ImVec2(384.0f, 384.0f));
		ImGui::SameLine();
		ImGui::Image(IMGUI_TEXTURE_ID(TextureIndex[3]), ImVec2(384.0f, 384.0f));
	}
	ImGui::End();

	return true;
}