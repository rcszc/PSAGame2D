// zero_project_gui.
#include "zero_project_gui.h"

using namespace std;
using namespace PSAG_LOGGER;

bool ZPGameGuiMain::LogicInitialization(const Vector2T<uint32_t>& WinSize) {
	PsagManager::SyncLoader::SyncEncDecImage DecodeRawImage;

	// load gui textures.
	for (size_t i = 0; i < 6; ++i) {
		Mptr::DefinePointer<PsagManager::FxView::TextureViewImage> TextureView = {};
		string ImageName = "zpgame_gui/zpgame_gui_b" + to_string(i) + ".png";
		ControlSlotDrawTextures[i] = 
			new PsagManager::FxView::TextureViewImage(
			DecodeRawImage.DecodeImageRawData(
				PsagManager::SyncLoader::FSLD::EasyFileReadRawData(SYSPATH_REF(ImageName))
		));
	}
	return true;
}

void ZPGameGuiMain::LogicCloseFree() {
	// free resource.
	for (auto Texture : ControlSlotDrawTextures)
		delete Texture.second;
}

bool ZPGameGuiMain::LogicEventLoopGame(GameLogic::FrameworkParams& RunningState) {
	ImGuiWindowFlags Flags = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize | 
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar;

	LerpDamageView += (GLO_PlayerDamageCount - LerpDamageView)
		* 0.025f * RunningState.GameRunTimeSTEP;

	ImGui::Text(FMT_TIME_STAMP(chrono::system_clock::now()).c_str());

	ImGui::SetNextWindowSize(ImVec2(330.0f, 180.0f));
	ImGui::SetNextWindowPos(ImVec2(32.0f, ImGui::GetIO().DisplaySize.y - 160.0f));
	ImGui::Begin("##CONTROL", (bool*)NULL, Flags);
	ImGui::SetWindowFontScale(1.8f);
	{
		auto ImGuiImage = IMGUI_TEXTURE_ID(ControlSlotDrawTextures[GLO_PlayerBullets]->GetTextureView());
		ImGui::Image(ImGuiImage, ImVec2(320.0f, 320.0f * 0.2715f));
		ImGui::Text("Damage: %.1f", LerpDamageView);
	}
	ImGui::End();
	return true;
}