// framework_window_imgui.
#include "framework_window.h"

using namespace std;
using namespace PSAG_LOGGER;

#define SYSTEM_DEFAULT_FONTS_SC 0.42f
namespace PSAG_WINDOW_IMGUI {

    void SpcaImGuiEvent::ImGuiInit(GLFWwindow* window_object, ImGuiConfig cfgdata) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& GUIIO = ImGui::GetIO(); (void)GUIIO;
        GUIIO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // enable keyboard controls.
        GUIIO.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // enable gamepad controls.

        if (!std::filesystem::exists(cfgdata.FontsFilepath))
            PushLogger(LogWarning, PSAG_IMGUI_LABEL, "failed imgui_read: fonts file.");
        else
            PushLogger(LogInfo, PSAG_IMGUI_LABEL, "imgui_read: fonts file: %s", cfgdata.FontsFilepath.c_str());

        // setup imgui style.
        ImGui::StyleColorsDark();
        {
            ImGuiStyle* ConfigStyle = &ImGui::GetStyle();
            ImVec4* ConfigTextColor = ConfigStyle->Colors;

            ConfigTextColor[ImGuiCol_Text] = ImVec4(
                cfgdata.FontsGlobalColor.vector_x,
                cfgdata.FontsGlobalColor.vector_y,
                cfgdata.FontsGlobalColor.vector_z,
                cfgdata.FontsGlobalColor.vector_w
            );
        }
        // init set font.
        auto ConfigFonts = ImGui::GetIO().Fonts;
        ConfigFonts->AddFontFromFileTTF(
            cfgdata.FontsFilepath.c_str(),
            cfgdata.FontsGlobalSize,
            NULL,
            ConfigFonts->GetGlyphRangesChineseFull()
        );
        ImGui::GetIO().FontGlobalScale = SYSTEM_DEFAULT_FONTS_SC;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, cfgdata.WindowRounding);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, cfgdata.FrameRounding);

        // setup platform / renderer backends.
        ImGui_ImplGlfw_InitForOpenGL(window_object, true);
        ImGui_ImplOpenGL3_Init(cfgdata.ShaderVersionStr.c_str());
    }

    void SpcaImGuiEvent::ImGuiFree() {
        // end close imgui.
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        PushLogger(LogInfo, PSAG_IMGUI_LABEL, "imgui_free: imgui context.");
    }

    void SpcaImGuiEvent::RenderGuiContextA() {
        // start the imgui frame.
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void SpcaImGuiEvent::RenderGuiContextB() {
        // render imgui.
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
}