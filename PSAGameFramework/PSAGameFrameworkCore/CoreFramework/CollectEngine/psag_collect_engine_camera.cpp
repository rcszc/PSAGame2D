// psag_collect_engine_camera.
#include "psag_collect_engine.h"

using namespace std;
using namespace PSAG_LOGGER;

namespace CollectEngineCamera {
	GamePlayerComaeraMP::GamePlayerComaeraMP(
		const Vector2T<float>& rect_range, const Vector2T<uint32_t>& window_size, float hardness
	) {
		WindowRectRange  = rect_range;
		WindowResolution = 
			Vector2T<float>((float)window_size.vector_x, (float)window_size.vector_y);
		
		if (hardness < 0.0f) {
			PushLogger(LogError, PSAGM_COLENGINE_CAMERA_LABEL, "player camera(mp): hardness < 0.0f");
			return;
		}
		// lerp_clac: p += (t - p) * speed(hardness) * step.
		CALC_LERP_SCALE = hardness;
	}

	void GamePlayerComaeraMP::PlayerCameraRun(const Vector2T<float>& window_coord, const Vector2T<float>& actor_speed) {
		// clac camera x+-,y+- > win_range ? => move camera.
		if (window_coord.vector_x > WindowResolution.vector_x * (0.5f - WindowRectRange.vector_x))
			CameraPositionTarget.vector_x -= abs(actor_speed.vector_x) * CollectEngineTimeStep;

		if (window_coord.vector_x < WindowResolution.vector_x * (0.5f + WindowRectRange.vector_x))
			CameraPositionTarget.vector_x += abs(actor_speed.vector_x) * CollectEngineTimeStep;

		if (window_coord.vector_y > WindowResolution.vector_y * (0.5f - WindowRectRange.vector_y))
			CameraPositionTarget.vector_y += abs(actor_speed.vector_y) * CollectEngineTimeStep;

		if (window_coord.vector_y < WindowResolution.vector_y * (0.5f + WindowRectRange.vector_y))
			CameraPositionTarget.vector_y -= abs(actor_speed.vector_y) * CollectEngineTimeStep;

		CameraPosition.vector_x += (CameraPositionTarget.vector_x - CameraPosition.vector_x) * 0.05f * CALC_LERP_SCALE;
		CameraPosition.vector_y += (CameraPositionTarget.vector_y - CameraPosition.vector_y) * 0.05f * CALC_LERP_SCALE;
	}

	Vector2T<float> GamePlayerComaeraMP::GetCameraPosition(const Vector2T<float>& camera_offset) {
		return Vector2T<float>(
			CameraPosition.vector_x + camera_offset.vector_x,
			CameraPosition.vector_y + camera_offset.vector_y
		);
	}
}