// framework_window_imgui.
#include "framework_window.h"

using namespace std;
using namespace PSAG_LOGGER;

#define SYSTEM_DEFAULT_FONTS_SC 0.6025f
namespace PSAG_WINDOW_IMGUI {

    bool PsagImGuiContextEvent::ImGuiContextInit(GLFWwindow* window_object, ImGuiConfig im_config) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& GUIIO = ImGui::GetIO(); (void)GUIIO;
        GUIIO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // enable keyboard controls.
        GUIIO.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // enable gamepad controls.

        if (!std::filesystem::exists(im_config.FontsFilepath)) {
            PushLogger(LogWarning, PSAG_IMGUI_LABEL, "failed imgui_read: fonts file.");
            return false;
        }
        PushLogger(LogInfo, PSAG_IMGUI_LABEL, "imgui_read: fonts file: %s", im_config.FontsFilepath.c_str());

        // setup imgui style.
        ImGui::StyleColorsDark();
        {
            ImGuiStyle* ConfigStyle = &ImGui::GetStyle();
            ImVec4* ConfigTextColor = ConfigStyle->Colors;

            ConfigTextColor[ImGuiCol_Text] = ImVec4(
                im_config.FontsGlobalColor.vector_x,
                im_config.FontsGlobalColor.vector_y,
                im_config.FontsGlobalColor.vector_z,
                im_config.FontsGlobalColor.vector_w
            );
        }
        // init config fonts.
        auto ConfigFonts = ImGui::GetIO().Fonts;
        ConfigFonts->AddFontFromFileTTF(
            im_config.FontsFilepath.c_str(),
            im_config.FontsGlobalSize,
            NULL,
            ConfigFonts->GetGlyphRangesChineseFull()
        );
        ImGui::GetIO().FontGlobalScale = SYSTEM_DEFAULT_FONTS_SC;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, im_config.WindowRounding);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, im_config.FrameRounding);

        // setup platform / renderer backends.
        ImGui_ImplGlfw_InitForOpenGL(window_object, true);
        ImGui_ImplOpenGL3_Init(im_config.ShaderVersionStr.c_str());

        PushLogger(LogInfo, PSAG_IMGUI_LABEL, "imgui_init: imgui_opengl3 context.");
        return true;
    }

    void PsagImGuiContextEvent::ImGuiContextFree() {
        // close imgui.
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        // free imgui context.
        ImGui::DestroyContext();
        PushLogger(LogInfo, PSAG_IMGUI_LABEL, "imgui_free: imgui_opengl3 context.");
    }

    void PsagImGuiContextEvent::RenderGuiContextA() {
        // start the imgui frame.
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void PsagImGuiContextEvent::RenderGuiContextB() {
        // render imgui.
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
}