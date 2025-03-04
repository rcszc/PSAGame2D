// psag_toolkits_engine_pawn.
#include "psag_toolkits_engine.h"

using namespace std;
using namespace PSAG_LOGGER;

float __TOOLKITS_ENGINE_TIMESETP::ToolkitsEngineTimeStep = 1.0f;

namespace ToolkitsEnginePawn {
	// clac: (target - value) / damping * scale.
	inline Vector2T<float> PAWN_CALC_LERP_VEC2FUNC(
		const Vector2T<float>& target, const Vector2T<float>& value, const Vector2T<float>& damping, 
		float NoiseScale
	) {
		return Vector2T<float>(
			value.vector_x + (target.vector_x - value.vector_x) / damping.vector_x * NoiseScale,
			value.vector_y + (target.vector_y - value.vector_y) / damping.vector_y * NoiseScale
		);
	}
	// pawn move_vector_list.
	unordered_map<ImGuiKey, Vector2T<float>> GamePlayerPawn::KeyboardMappings = {
		{ ImGuiKey_W, { 0.0f,  1.0f }},
		{ ImGuiKey_S, { 0.0f, -1.0f }},
		{ ImGuiKey_A, { 1.0f,  0.0f }},
		{ ImGuiKey_D, {-1.0f,  0.0f }}
	};

	GamePlayerPawn::GamePlayerPawn(const Vector2T<float>& damping) {
		if (damping.vector_x < 1.0f || damping.vector_y < 1.0f) {
			PushLogger(LogError, PSAGM_TOOLKITS_PAWN_LABEL, "player pawn: damping.xy < 1.0f");
			return;
		}
		DampingEffectVector = damping;
	}

	bool GamePlayerPawn::MouseButtonPressed_R(bool pulse) { return ImGui::IsMouseClicked((ImGuiMouseButton)1, !pulse); }
	bool GamePlayerPawn::MouseButtonPressed_L(bool pulse) { return ImGui::IsMouseClicked((ImGuiMouseButton)0, !pulse); }

	bool GamePlayerPawn::KeyboardPressed_R() { return ImGui::IsKeyPressed(ImGuiKey_R); };
	bool GamePlayerPawn::KeyboardPressed_F() { return ImGui::IsKeyPressed(ImGuiKey_F); };

	void GamePlayerPawn::PlayerPawnRun(float speed_value) {
		ControlMousePosition = Vector2T<float>(ImGui::GetMousePos().x, ImGui::GetMousePos().y);

		MoveSpeedTarget = Vector2T<float>(0.0f, 0.0f);
		// player control_input.
		for (const auto& [KeyCode, Action] : KeyboardMappings)
			if (ImGui::IsKeyDown(KeyCode))
				MoveSpeedTarget = Vector2T<float>(Action.vector_x * speed_value, Action.vector_y * speed_value);

		// calc_lerp: speed_target => move_vector(speed).
		ControlMoveVector = PAWN_CALC_LERP_VEC2FUNC(
			MoveSpeedTarget, ControlMoveVector, DampingEffectVector, ToolkitsEngineTimeStep
		);
	}
}