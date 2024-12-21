// psag_toolkits_engine_camera.
#include "psag_toolkits_engine.h"

using namespace std;
using namespace PSAG_LOGGER;

namespace ToolkitsEngineCamera {
	GamePlayerCameraMP::GamePlayerCameraMP(
		const Vector2T<float>& rect_range, const Vector2T<uint32_t>& window_size, float hardness
	) {
		WindowRectRange  = rect_range;
		WindowResolution = Vector2T<float>((float)window_size.vector_x, (float)window_size.vector_y);
		// check value limit:min.
		if (hardness < 0.0f) {
			PushLogger(LogError, PSAGM_TOOLKITS_CAMERA_LABEL, "player camera(mp): hardness < 0.0f");
			return;
		}
		// lerp_clac: p += (t - p) * speed(hardness) * step.
		CALC_LERP_SCALE = hardness;
	}

	void GamePlayerCameraMP::PlayerCameraLerpValue(float value) {
		if (value < 0.0f || value > 20.0f) 
			return;
		CALC_LERP_SCALE = value;
	}

	void GamePlayerCameraMP::PlayerCameraRun(const Vector2T<float>& window_coord, const Vector2T<float>& actor_speed) {
		// camera_trans [+x] > win_range ? => move camera.
		if (window_coord.vector_x > WindowResolution.vector_x * (0.5f - WindowRectRange.vector_x))
			CameraPositionTarget.vector_x += abs(actor_speed.vector_x) * ToolkitsEngineTimeStep;

		// camera_trans [-x] > win_range ? => move camera.
		if (window_coord.vector_x < WindowResolution.vector_x * (0.5f + WindowRectRange.vector_x))
			CameraPositionTarget.vector_x -= abs(actor_speed.vector_x) * ToolkitsEngineTimeStep;

		// camera_trans [+y] > win_range ? => move camera.
		if (window_coord.vector_y > WindowResolution.vector_y * (0.5f - WindowRectRange.vector_y))
			CameraPositionTarget.vector_y -= abs(actor_speed.vector_y) * ToolkitsEngineTimeStep;

		// camera_trans [-y] > win_range ? => move camera.
		if (window_coord.vector_y < WindowResolution.vector_y * (0.5f + WindowRectRange.vector_y))
			CameraPositionTarget.vector_y += abs(actor_speed.vector_y) * ToolkitsEngineTimeStep;
	}

	void GamePlayerCameraMP::PlayerCameraRunFixed(const Vector2T<float>& actor_position) {
		// camera position = actor position(world).
		CameraPositionTarget = CameraPosition = 
			Vector2T<float>(actor_position.vector_x, -actor_position.vector_y);
	}

	Vector2T<float> GamePlayerCameraMP::GetCameraPosition(const Vector2T<float>& camera_offset) {
		float LERP_VALUE = ToolkitsEngineTimeStep * 0.2f * CALC_LERP_SCALE;
		// calcuate camera_position lerp.
		CameraPosition.vector_x += (CameraPositionTarget.vector_x - CameraPosition.vector_x) * LERP_VALUE;
		CameraPosition.vector_y += (CameraPositionTarget.vector_y - CameraPosition.vector_y) * LERP_VALUE;

		return Vector2T<float>(
			CameraPosition.vector_x + camera_offset.vector_x, CameraPosition.vector_y + camera_offset.vector_y
		);
	}

	GamePlayerCameraGM::GamePlayerCameraGM(const Vector2T<float>& map_limit_min, const Vector2T<float>& map_limit_max) {
		// check range params. min < max ?
		if (map_limit_min.vector_x >= map_limit_max.vector_x || map_limit_min.vector_y >= map_limit_max.vector_y) {
			PushLogger(LogError, PSAGM_TOOLKITS_CAMERA_LABEL, "player camera(gm): map limit min >= max.");
			return;
		}
		CameraPositionRectLimit.vector_x = map_limit_min;
		CameraPositionRectLimit.vector_y = map_limit_max;
	}

	void GamePlayerCameraGM::PlayerCameraLerpValue(float value) {
		if (value < 0.0f || value > 20.0f) 
			return;
		CALC_LERP_SCALE = value;
	}

	void GamePlayerCameraGM::PlayerCameraRun(const Vector2T<float>& mouse_vec_speed, float speed_scale) {
		CameraPositionTarget.vector_x -= mouse_vec_speed.vector_x * 0.25f * speed_scale;
		CameraPositionTarget.vector_y += mouse_vec_speed.vector_y * 0.25f * speed_scale;

		CameraPositionTarget.vector_x = PSAG_IMVEC_CLAMP(
			CameraPositionTarget.vector_x, CameraPositionRectLimit.vector_x.vector_x, CameraPositionRectLimit.vector_y.vector_x);
		CameraPositionTarget.vector_y = PSAG_IMVEC_CLAMP(
			CameraPositionTarget.vector_y, CameraPositionRectLimit.vector_x.vector_y, CameraPositionRectLimit.vector_y.vector_y);
	}

	Vector2T<float> GamePlayerCameraGM::GetCameraPosition(const Vector2T<float>& camera_offset) {
		float LERP_VALUE = ToolkitsEngineTimeStep * 0.1f * CALC_LERP_SCALE;
		// calcuate camera_position lerp.
		CameraPosition.vector_x += (CameraPositionTarget.vector_x - CameraPosition.vector_x) * LERP_VALUE;
		CameraPosition.vector_y += (CameraPositionTarget.vector_y - CameraPosition.vector_y) * LERP_VALUE;

		return Vector2T<float>(
			CameraPosition.vector_x + camera_offset.vector_x, CameraPosition.vector_y + camera_offset.vector_y
		);
	}
}