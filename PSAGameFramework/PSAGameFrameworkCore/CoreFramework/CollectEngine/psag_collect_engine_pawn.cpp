// psag_collect_engine_pawn.
#include "psag_collect_engine.h"

using namespace std;
using namespace PSAG_LOGGER;

float __COLLECT_ENGINE_TIMESETP::CollectEngineTimeStep = 1.0f;

namespace CollectEnginePawn {

	unordered_map<ImGuiKey, Vector2T<float>> GamePlayerPawn::KeyboardMappings = {
		{ ImGuiKey_W, { 0.0f,  1.0f }},
		{ ImGuiKey_S, { 0.0f, -1.0f }},
		{ ImGuiKey_A, { 1.0f,  0.0f }},
		{ ImGuiKey_D, {-1.0f,  0.0f }}
	};

	GamePlayerPawn::GamePlayerPawn(Vector2T<float> damping) {
		if (damping.vector_x < 1.0f || damping.vector_y < 1.0f) {
			PushLogger(LogError, PSAGM_COLENGINE_PAWN_LABEL, "player pawn: damping.xy < 1.0f");
			return;
		}
		DampingEffectVector = damping;
	}

	bool GamePlayerPawn::MouseButtonPressed_R(bool pulse) {
		return ImGui::IsMouseClicked((ImGuiMouseButton)1, !pulse);
	}

	bool GamePlayerPawn::MouseButtonPressed_L(bool pulse) {
		return ImGui::IsMouseClicked((ImGuiMouseButton)0, !pulse);
	}

	void GamePlayerPawn::PlayerPawnRun(float speed_value) {
		ControlMousePosition = Vector2T<float>(ImGui::GetMousePos().x, ImGui::GetMousePos().y);

		MoveSpeedTarget = Vector2T<float>(0.0f, 0.0f);
		// player control_input.
		for (const auto& [KeyCode, Action] : KeyboardMappings)
			if (ImGui::IsKeyDown(KeyCode))
				MoveSpeedTarget = Vector2T<float>(
					Action.vector_x * speed_value * CollectEngineTimeStep,
					Action.vector_y * speed_value * CollectEngineTimeStep
				);
		// calc_lerp: speed_target => move_vector(speed).
		ControlMoveVector.vector_x += (MoveSpeedTarget.vector_x - ControlMoveVector.vector_x) / DampingEffectVector.vector_x;
		ControlMoveVector.vector_y += (MoveSpeedTarget.vector_y - ControlMoveVector.vector_y) / DampingEffectVector.vector_y;
	}
}