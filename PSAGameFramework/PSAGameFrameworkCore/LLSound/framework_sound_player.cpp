// framework_sound_player.
#include "framework_sound.h"

using namespace std;
using namespace PSAG_LOGGER;

namespace PSAG_AUDIO_PLAYER {
    namespace system {
        ALCdevice*  PsagAudioSystemDevice::HDPTR_OPENAL_DEVICE  = nullptr;
        ALCcontext* PsagAudioSystemDevice::HDPTR_OPENAL_CONTEXT = nullptr;

        bool PsagAudioSystemDevice::CreateSoundDevice() {
            HDPTR_OPENAL_DEVICE  = alcOpenDevice(nullptr); // default device.
            HDPTR_OPENAL_CONTEXT = alcCreateContext(HDPTR_OPENAL_DEVICE, nullptr);
            // set this-thread => context.
            if (alcMakeContextCurrent(HDPTR_OPENAL_CONTEXT) == ALC_FALSE) {
                PushLogger(LogError, PSAG_AUDIO_LABEL, "audio(openal) global failed create context.");
                return false;
            }
            PushLogger(LogTrace, PSAG_AUDIO_LABEL, "audio(openal) global create device & context.");
            return true;
        }

        bool PsagAudioSystemDevice::DeleteSoundDevice() {
            alcMakeContextCurrent(nullptr);
            // delete context & close device.
            alcDestroyContext(HDPTR_OPENAL_CONTEXT);
            if (alcCloseDevice(HDPTR_OPENAL_DEVICE) == ALC_FALSE) {
                PushLogger(LogError, PSAG_AUDIO_LABEL, "audio(openal) global failed close_device.");
                return false;
            }
            PushLogger(LogTrace, PSAG_AUDIO_LABEL, "audio(openal) global free_context & close_device.");
            return true;
        }
    }

    bool PsagAudioDataResource::CreateBufferHandle(ALuint& handle) {
        // openal generate handle.
        alGenBuffers(1, &handle);
        // openal handle index > 0.
        if (handle != OPENAL_INVALID_HANDEL) {
            PushLogger(LogInfo, PSAG_AUDIO_LABEL, "audio(openal) handle create.");
            return true;
        }
        PushLogger(LogWarning, PSAG_AUDIO_LABEL, "audio(openal) handle failed create."); 
        return false;
    }

#define STB_VORBIS_NO_STDIO
#include "stb_vorbis.c"
    
    PsagAudioDataResource::PsagAudioDataResource(const RawAudioStream& rawdata) {
        if (rawdata.empty()) {
            PushLogger(LogError, PSAG_AUDIO_LABEL, "audio raw_data empty.");
            return;
        }
        if (CreateBufferHandle(HANDLE_AUDIO_BUFFER)) {
            int VorbisError = NULL;
            // raw file bin_data => stb read.
            stb_vorbis* VorbisMemory = stb_vorbis_open_memory(rawdata.data(), (int)rawdata.size(), &VorbisError, nullptr);
            if (VorbisMemory == nullptr) {
                PushLogger(LogError, PSAG_AUDIO_LABEL, "audio loader: invalid format.");
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
                PushLogger(LogError, PSAG_AUDIO_LABEL, "audio decode: non_channels.");
                return;
            }}
            // data(pcm) cache.
            vector<int16_t> AudioDataCache(SampleNum * (size_t)Channels);
            if (stb_vorbis_get_samples_short_interleaved(
                VorbisMemory, Channels, AudioDataCache.data(), (int)AudioDataCache.size()
            ) <= NULL) {
                stb_vorbis_close(VorbisMemory);
                PushLogger(LogError, PSAG_AUDIO_LABEL, "audio decode: failed read pcm_data.");
                return;
            }
            alBufferData(
                HANDLE_AUDIO_BUFFER, DataFormat, 
                AudioDataCache.data(), 
                (ALsizei)AudioDataCache.size() * sizeof(int16_t),
                (ALsizei)SampleRate
            );
            stb_vorbis_close(VorbisMemory);

            PushLogger(LogInfo, PSAG_AUDIO_LABEL, "audio decode: channels: %d, Rate: %d", Channels, SampleRate);
            PushLogger(LogInfo, PSAG_AUDIO_LABEL, "audio loader: %u samples", AudioDataCache.size());
            return;
        }
        PushLogger(LogError, PSAG_AUDIO_LABEL, "audio failed create.");
    }

    PsagAudioDataResource::~PsagAudioDataResource() {
        alDeleteBuffers(1, &HANDLE_AUDIO_BUFFER);
        PushLogger(LogInfo, PSAG_AUDIO_LABEL, "audio loader: delete buffer.");
    }

    PsagAudioDataPlayer::PsagAudioDataPlayer(PsagAudioDataResource* loader) {
        ResourceLoader = loader;
        if (ResourceLoader == nullptr) {
            PushLogger(LogError, PSAG_AUDIO_LABEL, "audio system: loader error, ptr = null.");
            return;
        }
        HANDLE_AUDIO_BUFFER = ResourceLoader->_MS_GETRES();
        // invalid handle.
        if (HANDLE_AUDIO_BUFFER == NULL)
            PushLogger(LogError, PSAG_AUDIO_LABEL, "audio system: loader error, hd = null.");
        
        // create source => bind buffer.
        alGenSources(1, &HANDLE_AUDIO_OBJECT);
        if (HANDLE_AUDIO_OBJECT == NULL) {
            PushLogger(LogError, PSAG_AUDIO_LABEL, "audio system: failed create source.");
            return;
        }
        alSourcei(HANDLE_AUDIO_OBJECT, AL_BUFFER, HANDLE_AUDIO_BUFFER);
        alSourcei(HANDLE_AUDIO_OBJECT, AL_LOOPING, AL_TRUE);
    }

    PsagAudioDataPlayer::~PsagAudioDataPlayer() {
        if (ResourceLoader != nullptr)
            delete ResourceLoader;
        alDeleteSources(1, &HANDLE_AUDIO_OBJECT);
        PushLogger(LogInfo, PSAG_AUDIO_LABEL, "audio system: delete player.");
    };

    void PsagAudioDataPlayer::SoundPlayer() { return alSourcePlay(HANDLE_AUDIO_OBJECT); }
    void PsagAudioDataPlayer::SoundPause()  { return alSourcePause(HANDLE_AUDIO_OBJECT); }

    void PsagAudioDataPlayer::SoundBeginPosition() {
        alSourceRewind(HANDLE_AUDIO_OBJECT);
        PushLogger(LogInfo, PSAG_AUDIO_LABEL, "audio system: set pos_begin.");
    }

    bool PsagAudioDataPlayer::PlayerEndedFlag() {
        ALint StateTemp = NULL;
        alGetSourcei(HANDLE_AUDIO_OBJECT, AL_SOURCE_STATE, &StateTemp);
        return StateTemp == AL_STOPPED ? true : false;
    }

    void PsagAudioDataPlayer::SoundSet3DPosition(const Vector3T<float>& position) {
        alSource3f(HANDLE_AUDIO_OBJECT, AL_POSITION, position.vector_x, position.vector_y, position.vector_z);
    }
    void PsagAudioDataPlayer::SoundSet3DDirection(const Vector3T<float>& direction) {
        alSource3f(HANDLE_AUDIO_OBJECT, AL_DIRECTION, direction.vector_x, direction.vector_y, direction.vector_z);
    }
    void PsagAudioDataPlayer::SoundSet3DGain(float gain) {
        alSourcef(HANDLE_AUDIO_OBJECT, AL_GAIN, gain);
    }

    void PsagAudioDataPlayer::SoundSet3DVelocity(const Vector3T<float>& velocity) {
        alSource3f(HANDLE_AUDIO_OBJECT, AL_VELOCITY, velocity.vector_x, velocity.vector_y, velocity.vector_z);
    }
    void PsagAudioDataPlayer::SoundSet3DVelocityListener(const Vector3T<float>& listener_v) {
        alListener3f(AL_VELOCITY, listener_v.vector_x, listener_v.vector_y, listener_v.vector_z);
    }
}