// psag_manager_module_maths.
#include "psag_manager_module.hpp"

using namespace std;
using namespace PSAG_LOGGER;

namespace GameManagerCore {
	namespace GameMathsTools {

		float CalcFuncPointsAngle(Vector2T<float> basic_point, Vector2T<float> point) {
			// calc points dx,dy.
			float Dx = point.vector_x - basic_point.vector_x;
			float Dy = point.vector_y - basic_point.vector_y;
			// calc(return): basic_point => point angle.
			return PSAG_M_RADDEG(atan2(Dy, Dx));
		}

		void CalcFuncLerpVec1(float* ahpla, float* target, float speed) {
			*ahpla += (*target - *ahpla) * speed;
		}

		void CalcFuncLerpVec2(Vector2T<float>* ahpla, Vector2T<float>* target, float speed) {
			ahpla->vector_x += (target->vector_x - ahpla->vector_x) * speed;
			ahpla->vector_y += (target->vector_y - ahpla->vector_y) * speed;
		}

		void CalcFuncLerpVec4(Vector4T<float>* ahpla, Vector4T<float>* target, float speed) {
			ahpla->vector_x += (target->vector_x - ahpla->vector_x) * speed;
			ahpla->vector_y += (target->vector_y - ahpla->vector_y) * speed;
			ahpla->vector_z += (target->vector_z - ahpla->vector_z) * speed;
			ahpla->vector_w += (target->vector_w - ahpla->vector_w) * speed;
		}
	}
}