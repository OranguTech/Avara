#include "Beeper.h"
#include "BasePath.h"
#include <SDL3/SDL.h>
#include <stdint.h>
#include <stdio.h>

SDL_AudioSpec wavSpec;
uint32_t wavLength;
uint8_t *wavBuffer;
SDL_AudioStream *beepDevice = NULL;
bool beepLoaded = false;

void Beep() {
    if (!beepLoaded) {
        // TODO: backslash on Windows
        char wavPath[256];
        snprintf(wavPath, 256, "%srsrc/%s", GetBasePath().c_str(), "sosumi.wav");
        SDL_memset(&wavSpec, 0, sizeof(wavSpec));
        if (SDL_LoadWAV(wavPath, &wavSpec, &wavBuffer, &wavLength)) {
            beepLoaded = true;
        }
        else {
            SDL_Log("Could not load beep: %s\n", SDL_GetError());
        }
    }
    
    if (!beepDevice) { //SDL3 TODO REVIEW
        //beepDevice = SDL_OpenAudioDevice(NULL, 0, &wavSpec, NULL, 0);
        beepDevice = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &wavSpec, NULL, NULL);
        SDL_ResumeAudioDevice(SDL_GetAudioStreamDevice(beepDevice));
    }

    if (beepLoaded && beepDevice) {
        //SDL_QueueAudio(beepDevice, wavBuffer, wavLength);
        SDL_PutAudioStreamData(beepDevice, wavBuffer, wavLength);
    }
}
