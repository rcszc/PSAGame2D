// framework_sound_player.
#include "framework_sound.h"

using namespace std;
using namespace PSAG_LOGGER;

bool PSAG_FUNC_BASS_GLOBAL_INIT() {
    // init config bass(global).
    if (!BASS_Init(PSAG_BASS_CONFIG_DEVICE, PSAG_BASS_CONFIG_FREQ, NULL, nullptr, nullptr)) {
        PushLogger(LogError, PSAG_SOUND_PLAYER::PSAG_SOUND_LABEL, "failed bass global_initialize.");
        return false;
    }
    return true;
}

bool PSAG_FUNC_BASS_GLOBAL_FREE() {
    // free system bass(global).
    if (!BASS_Free()) {
        PushLogger(LogError, PSAG_SOUND_PLAYER::PSAG_SOUND_LABEL, "failed bass global_free.");
        return false;
    }
    return true;
}

namespace PSAG_SOUND_PLAYER {

    inline bool BassHanderRepeat(const HSTREAM& handel) {
        if (handel != NULL) {
            PushLogger(LogWarning, PSAG_SOUND_LABEL, "bass handel_loader: repeat.");
            return true;
        }
        return false;
    }

    bool PsagSoundHandleLoader::SoundLoaderRawStream(const RawSoundStream& rawdata) {
        if (BassHanderRepeat(BassStreamObject))
            return false;
        // create mem_stream => load_stream.
        BassStreamObject = BASS_StreamCreateFileUser(STREAMFILE_NOBUFFER, BASS_SAMPLE_FLOAT, nullptr, nullptr);
        BASS_StreamPutData(BassStreamObject, rawdata.data(), (DWORD)rawdata.size());
        if (BassStreamObject == NULL)
            PushLogger(LogError, PSAG_SOUND_LABEL, "bass handel_loader: load_raw_data error.");
        return BassStreamObject != NULL;
    }

    bool PsagSoundHandleLoader::SoundLoaderFile(const string& filename) {
        if (BassHanderRepeat(BassStreamObject))
            return false;
        // create file_loader => load_file.
        BassStreamObject = BASS_StreamCreateFile(FALSE, filename.c_str(), 0, 0, BASS_SAMPLE_FLOAT);
        if (BassStreamObject == NULL)
            PushLogger(LogError, PSAG_SOUND_LABEL, "bass handel_loader: load_file error.");
        return BassStreamObject != NULL;
    }

    HSTREAM PsagSoundHandleLoader::_MS_GETRES() { return BassStreamObject; }

    PsagSoundPlayerHandle::PsagSoundPlayerHandle(PsagSoundHandleLoader& loader, const Sound3DConfig& s3d) {
        BassStreamObject = loader._MS_GETRES();
        // loader handle = null : invalid_hd.
        if (BassStreamObject == NULL)
            PushLogger(LogError, PSAG_SOUND_LABEL, "bass system: loader error, hd = null.");

        if (s3d.EnableFlag) {
            BASS_ChannelSet3DAttributes(
                BassStreamObject, BASS_3DMODE_NORMAL, s3d.SoundLimit.vector_x, s3d.SoundLimit.vector_y, -1, -1, -1
            ) == (int)true ? 
                PushLogger(LogInfo, PSAG_SOUND_LABEL, "bass system: enabel 3d_attrib.") : 
                PushLogger(LogError, PSAG_SOUND_LABEL, "bass system: enabel 3d_attrib error.");
        }
        Sound3DEnabelFlag = s3d.EnableFlag;
    }

    inline bool BassFuncProcess(bool flag, const char* info) {
        flag == (int)false ? 
            PushLogger(LogError, PSAG_SOUND_LABEL, "failed loader(sound): %s.", info) :
            PushLogger(LogInfo, PSAG_SOUND_LABEL, "loader(sound): %s.", info);
        return flag;
    }

    bool PsagSoundPlayerHandle::PlayerSound() { return BassFuncProcess(BASS_ChannelPlay(BassStreamObject, TRUE), "player_sound"); }
    bool PsagSoundPlayerHandle::PauseSound()  { return BassFuncProcess(BASS_ChannelPause(BassStreamObject), "pause_sound");        }

    bool PsagSoundPlayerHandle::SetPlayerPosition(float second) {
        QWORD BytePosition = BASS_ChannelSeconds2Bytes(BassStreamObject, (double)second);
        // second => byte_pos => set player_pos.
        if (!BASS_ChannelSetPosition(BassStreamObject, BytePosition, BASS_POS_BYTE)) {
            PushLogger(LogError, PSAG_SOUND_LABEL, "failed loader(sound): set player_pos.");
            return true;
        }
        PushLogger(LogInfo, PSAG_SOUND_LABEL, "loader(sound): set player_pos: %.2f s", second);
        return false;
    }

    bool PsagSoundPlayerHandle::PlayerEndedFlag() {
        // playback ended, true: end, false: playing...
        return (BASS_ChannelIsActive(BassStreamObject) != BASS_ACTIVE_PLAYING);
    }

    bool PsagSoundPlayerHandle::Set3DSoundListenerPosition(const Vector3T<float>& pos) {
        BASS_3DVECTOR ListenerPos = {};
        memcpy(&ListenerPos.x, &pos.vector_x, sizeof(float) * 3);

        BASS_3DVECTOR ListenerVEL   = { 0.0f, 0.0f, 0.0f };  // listener speed.
        BASS_3DVECTOR ListenerFRONT = { 0.0f, 0.0f, -1.0f }; // listener front.
        BASS_3DVECTOR ListenerTOP   = { 0.0f, 1.0f, 0.0f };  // listener top.

        return BASS_Set3DPosition(&ListenerPos, &ListenerVEL, &ListenerFRONT, &ListenerTOP);
    }

    PsagSoundPlayerHandle::~PsagSoundPlayerHandle() {
        if (!BASS_StreamFree(BassStreamObject))
            PushLogger(LogError, PSAG_SOUND_LABEL, "bass system: free error.");
    };
}