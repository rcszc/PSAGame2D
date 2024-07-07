// psag_mainevent_support_class.
#include "psag_mainevent_support.h"

using namespace std;
using namespace PSAG_LOGGER;

namespace PsagFrameworkCore {
	namespace PsagManage {

		bool SysBackground::LayerDataPush(const ImageRawData& layer_image) {
			if (layer_image.ImagePixels.empty()) {
				PushLogger(LogWarning, PSAGM_FRAME_COREMAG_LABEL, "background load image_layer data_empty.");
				return false;
			}
			LayersRawImages.push_back(layer_image);
			return true;
		}

		void SysBackground::CreateBackground(const Vector2T<uint32_t>& resolution) {
			if (RendererBackFX != nullptr)
				delete RendererBackFX;
			// (re)create background-shader. 
			RendererBackFX = new GraphicsEngineBackground::PsagGLEngineBackground(
				resolution, LayersRawImages
			);
			// load pointer.
			FrameworkParams.BackShaderParams = RendererBackFX->GetRenderParameters();
			PushLogger(LogInfo, PSAGM_FRAME_COREMAG_LABEL, "background create, layers: %u", LayersRawImages.size());
		}
	}
}