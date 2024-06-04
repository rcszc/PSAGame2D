// framework_sound_player.
#include "framework_sound.h"

using namespace std;
using namespace PSAG_LOGGER;

namespace PSAG_SOUND_PLAYER {
    namespace system {
        ALCdevice*  __PsagSoundDeviceHandle::SoundDevicePtr  = nullptr;
        ALCcontext* __PsagSoundDeviceHandle::SoundContextPtr = nullptr;

        bool __PsagSoundDeviceHandle::CreateSoundDevice() {
            SoundDevicePtr = alcOpenDevice(nullptr); // default device.
            SoundContextPtr = alcCreateContext(SoundDevicePtr, nullptr);
            // set this-thread => context.
            if (alcMakeContextCurrent(SoundContextPtr) == ALC_FALSE) {
                PushLogger(LogError, PSAG_SOUND_LABEL, "sound(openal) global failed create context.");
                return false;
            }
            PushLogger(LogTrace, PSAG_SOUND_LABEL, "sound(openal) global create device & context.");
            return true;
        }

        bool __PsagSoundDeviceHandle::DeleteSoundDevice() {
            alcMakeContextCurrent(nullptr);
            // delete context & close device.
            alcDestroyContext(SoundContextPtr);
            if (alcCloseDevice(SoundDevicePtr) == ALC_FALSE) {
                PushLogger(LogError, PSAG_SOUND_LABEL, "sound(openal) global failed close_device.");
                return false;
            }
            PushLogger(LogTrace, PSAG_SOUND_LABEL, "sound(openal) global free_context & close_device.");
            return true;
        }
    }

    bool PsagSoundDataResource::CreateBufHandle(ALuint& handle) {
        // openal generate handle.
        alGenBuffers(1, &handle);
        // openal handle index > 0.
        if (handle) { PushLogger(LogInfo, PSAG_SOUND_LABEL, "sound(openal) handle create.");           return true; }
        else        { PushLogger(LogWarning, PSAG_SOUND_LABEL, "sound(openal) handle failed create."); return false; }
    }

#define STB_VORBIS_NO_STDIO
#include "stb_vorbis.c"
    
    PsagSoundDataResource::PsagSoundDataResource(const RawSoundStream& rawdata) {
        if (rawdata.empty()) {
            PushLogger(LogError, PSAG_SOUND_LABEL, "sound(audio) raw_data empty.");
            return;
        }
        if (CreateBufHandle(BufferObjectHandle)) {
            int VorbisError = NULL;
            // raw file bin_data => stb read.
            stb_vorbis* VorbisMemory = stb_vorbis_open_memory(rawdata.data(), (int)rawdata.size(), &VorbisError, nullptr);
            if (VorbisMemory == nullptr) {
                PushLogger(LogError, PSAG_SOUND_LABEL, "sound(audio) loader: invalid format.");
                return;
            }
            stb_vorbis_info VorbisInfo = {};
            // get audio params.
            VorbisInfo = stb_vorbis_get_info(VorbisMemory);
            int SampleRate = VorbisInfo.sample_rate;
            int Channels   = VorbisInfo.channels;
            // get total samples number.
            size_t SampleNum = (size_t)stb_vorbis_stream_length_in_samples(VorbisMemory);

            ALenum DataFormat = NULL;
            switch (Channels) {
            case(1): { DataFormat = AL_FORMAT_MONO16;   break; }
            case(2): { DataFormat = AL_FORMAT_STEREO16; break; }
            default: {
                PushLogger(LogError, PSAG_SOUND_LABEL, "sound(audio) decode: non_channels.");
                return;
            }}
            // data(pcm) cache.
            vector<int16_t> AudioDataCache(SampleNum * (size_t)Channels);
            if (stb_vorbis_get_samples_short_interleaved(
                VorbisMemory, Channels, AudioDataCache.data(), (int)AudioDataCache.size()
            ) <= NULL) {
                stb_vorbis_close(VorbisMemory);
                PushLogger(LogError, PSAG_SOUND_LABEL, "sound(audio) decode: failed read pcm_data.");
                return;
            }
            alBufferData(
                BufferObjectHandle, DataFormat, 
                AudioDataCache.data(), 
                (ALsizei)AudioDataCache.size() * sizeof(int16_t),
                (ALsizei)SampleRate
            );
            stb_vorbis_close(VorbisMemory);

            PushLogger(LogInfo, PSAG_SOUND_LABEL, "sound(audio) decode: channels: %d, Rate: %d", Channels, SampleRate);
            PushLogger(LogInfo, PSAG_SOUND_LABEL, "sound(audio) loader: %u samples", AudioDataCache.size());
            return;
        }
        PushLogger(LogError, PSAG_SOUND_LABEL, "sound(audio) failed create.");
    }

    PsagSoundDataResource::~PsagSoundDataResource() {
        alDeleteBuffers(1, &BufferObjectHandle);
        PushLogger(LogInfo, PSAG_SOUND_LABEL, "sound(audio) loader: delete buffer.");
    }

    PsagSoundDataPlayer::PsagSoundDataPlayer(PsagSoundDataResource& loader) {
        BufferObjectHandle = loader._MS_GETRES();
        // invalid handle.
        if (BufferObjectHandle == NULL)
            PushLogger(LogError, PSAG_SOUND_LABEL, "sound system: loader error, hd = null.");
        
        // create source => bind buffer.
        alGenSources(1, &SourceObjectHandle);
        if (SourceObjectHandle == NULL) {
            PushLogger(LogError, PSAG_SOUND_LABEL, "sound system: failed create source.");
            return;
        }
        alSourcei(SourceObjectHandle, AL_BUFFER, BufferObjectHandle);
        alSourcei(SourceObjectHandle, AL_LOOPING, AL_TRUE);
    }

    PsagSoundDataPlayer::~PsagSoundDataPlayer() {
        alDeleteSources(1, &SourceObjectHandle);
        PushLogger(LogInfo, PSAG_SOUND_LABEL, "sound system: delete player.");
    };

    void PsagSoundDataPlayer::SoundPlayer() { return alSourcePlay(SourceObjectHandle); }
    void PsagSoundDataPlayer::SoundPause()  { return alSourcePause(SourceObjectHandle); }

    void PsagSoundDataPlayer::SetPlayerBegin() {
        alSourceRewind(SourceObjectHandle);
        PushLogger(LogInfo, PSAG_SOUND_LABEL, "sound system: set pos_begin.");
    }

    bool PsagSoundDataPlayer::PlayerEndedFlag() {
        ALint StateTemp = NULL;
        alGetSourcei(SourceObjectHandle, AL_SOURCE_STATE, &StateTemp);
        return StateTemp == AL_STOPPED ? true : false;
    }

    void PsagSoundDataPlayer::SoundSet3DPosition(const Vector3T<float>& position) {
        alSource3f(SourceObjectHandle, AL_POSITION, position.vector_x, position.vector_y, position.vector_z);
    }
    void PsagSoundDataPlayer::SoundSet3DDirection(const Vector3T<float>& direction) {
        alSource3f(SourceObjectHandle, AL_DIRECTION, direction.vector_x, direction.vector_y, direction.vector_z);
    }
    void PsagSoundDataPlayer::SoundSet3DGain(float gain) {
        alSourcef(SourceObjectHandle, AL_GAIN, gain);
    }

    void PsagSoundDataPlayer::SoundSet3DVelocity(const Vector3T<float>& velocity) {
        alSource3f(SourceObjectHandle, AL_VELOCITY, velocity.vector_x, velocity.vector_y, velocity.vector_z);
    }
    void PsagSoundDataPlayer::SoundSet3DVelocityListener(const Vector3T<float>& listener_v) {
        alListener3f(AL_VELOCITY, listener_v.vector_x, listener_v.vector_y, listener_v.vector_z);
    }
}