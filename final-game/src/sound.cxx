#include <SDL2/SDL.h>
#include <AL/al.h>
#include <AL/alc.h>
#include "headers/engine.hxx"
#include <iostream>
#include <assert.h>

namespace CHL {
sound::sound(const std::string& file)
    : device_id(0), buffer(nullptr), buffer_size(0) {
    if (!load(file))
        throw std::runtime_error("can't load sound");

    alGenBuffers(1, &al_buffer);
    alGenSources(1, &al_source);
    alBindBuffer(file, al_buffer);
}

bool sound::load(const std::string& file) {
    SDL_AudioSpec wavSpec;

    if (SDL_LoadWAV(file.c_str(), &wavSpec, &buffer, &buffer_size) == NULL)
        return false;

    device_id = SDL_OpenAudioDevice(NULL, 0, &wavSpec, NULL, 0);
    if (!device_id)
        return false;

    /*openal*/

    vec3 sourcePos(0, 0, 2);    // source position
    vec3 sourceVel(0, 0, 0);    // source velocity (used for doppler
    // effect)

    alSourcei(al_source, AL_BUFFER, al_buffer);
    alSourcef(al_source, AL_PITCH, 1.0f);
    alSourcef(al_source, AL_GAIN, 1.0f);
    alSource3f(al_source, AL_POSITION, sourcePos.x, sourcePos.y, sourcePos.z);
    alSource3f(al_source, AL_VELOCITY, sourceVel.x, sourceVel.y, sourcePos.z);
    alSourcei(al_source, AL_LOOPING, AL_TRUE);

    alSourcePlay(al_source);
    /*end openal*/

    return true;
}
void sound::play() const {
    SDL_QueueAudio(device_id, buffer, buffer_size);
    SDL_PauseAudioDevice(device_id, 0);
}
void sound::play_always() const {
    constexpr uint32_t max_play = 20;
    for (uint32_t i = 0; i < max_play; ++i)
        SDL_QueueAudio(device_id, buffer, buffer_size);

    SDL_PauseAudioDevice(device_id, 0);
}
void sound::stop() const {
    SDL_PauseAudioDevice(device_id, 1);
}

int sound::alBindBuffer(const std::string& source, int id) {
    int error;

    SDL_AudioSpec wavspec;
    uint32_t wavlen;
    uint8_t* wavbuf;

    if (!SDL_LoadWAV(source.c_str(), &wavspec, &wavbuf, &wavlen))
        return false;

    // map wav header to openal format
    ALenum format;
    switch (wavspec.format) {
        case AUDIO_U8:
        case AUDIO_S8:
            format = wavspec.channels == 2 ? AL_FORMAT_STEREO8
                                           : AL_FORMAT_MONO8;
            break;
        case AUDIO_U16:
        case AUDIO_S16:
            format = wavspec.channels == 2 ? AL_FORMAT_STEREO16
                                           : AL_FORMAT_MONO16;
            break;
        default:
            SDL_FreeWAV(wavbuf);
            return CHL_FAILURE;
    }

    alBufferData(id, format, wavbuf, wavlen, wavspec.freq);
    if ((error = alGetError()) != AL_NO_ERROR) {
        std::cerr << "openal error: " << error << std::endl;
        return CHL_FAILURE;
    }

    SDL_FreeWAV(wavbuf);

    std::cout << "cool!" << std::endl;
    return CHL_SUCCESS;
}

sound::~sound() {
    SDL_CloseAudioDevice(device_id);
    SDL_FreeWAV(buffer);
}

}    // namespace CHL
