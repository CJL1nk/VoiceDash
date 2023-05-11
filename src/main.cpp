#include <iostream>
#include <vector>
#include <Windows.h>
#include <portaudio.h>
#include "extensions_2.h"

constexpr double kSilenceThreshold = 0.0001;  // Adjust this threshold for sensitivity
constexpr unsigned int kBufferSize = 512;     // Adjust buffer size as needed
bool isOn = false;

void sendMouseInput(bool down, bool button) {
    POINT pos{}; GetCursorPos(&pos);

    down ? (button ? SendMessage(GetForegroundWindow(), WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(pos.x, pos.y)) :
        SendMessage(GetForegroundWindow(), WM_RBUTTONDOWN, MK_RBUTTON, MAKELPARAM(pos.x, pos.y))) :
        (button ? SendMessage(GetForegroundWindow(), WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(pos.x, pos.y)) :
            SendMessage(GetForegroundWindow(), WM_RBUTTONUP, MK_RBUTTON, MAKELPARAM(pos.x, pos.y)));
}

void onMicInputStart() {
    if (isOn) { sendMouseInput(true, true); }
}

void onMicInputStop() {
    if (isOn) { sendMouseInput(false, true); }
}

int audioCallback(const void* inputBuffer, void* outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags, void* userData) {
    float* samples = static_cast<float*>(const_cast<void*>(inputBuffer));
    std::vector<float> audioBuffer(samples, samples + framesPerBuffer);

    double energy = 0.0;
    for (const float sample : audioBuffer) {
        energy += sample * sample;
    }
    energy /= framesPerBuffer;

    static bool isInputStarted = false;
    if (!isInputStarted && energy > kSilenceThreshold) {
        isInputStarted = true;
        onMicInputStart();
    }
    else if (isInputStarted && energy <= kSilenceThreshold) {
        isInputStarted = false;
        onMicInputStop();
    }

    return paContinue;
}

void start() {

    PaError err;

    Pa_Initialize();

    PaStream* stream;
    err = Pa_OpenDefaultStream(&stream, 1, 0, paFloat32, 44100, kBufferSize, audioCallback, nullptr);
    if (err != paNoError) {
        Pa_Terminate();
    }

    err = Pa_StartStream(stream);
    if (err != paNoError) {
        Pa_CloseStream(stream);
        Pa_Terminate();
    }
}

void startWindow() {

    MegaHackExt::Window* window = MegaHackExt::Window::Create("VoiceDash");

    MegaHackExt::CheckBox* enable_checkbox = MegaHackExt::CheckBox::Create("Enable");
    enable_checkbox->setCallback([](MegaHackExt::CheckBox* obj, bool a) {

        isOn = !isOn;
        });
    window->add(enable_checkbox);

    MegaHackExt::Client::commit(window);
}

void main_thread(void* instance) {

    startWindow();
    start();
}

BOOL APIENTRY DllMain(void* instance, DWORD reason_for_call, void* reservation) {

    if (reason_for_call != DLL_PROCESS_ATTACH) { return false; }

    CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)main_thread, instance, 0, nullptr));

    return true;
}