#include "daisy_hardware.h"

using namespace daisy;

#ifndef SAMPLE_RATE
//#define SAMPLE_RATE DSY_AUDIO_SAMPLE_RATE
#define SAMPLE_RATE 48014.f
#endif

// Hardware related defines.
// Switches

constexpr Pin LED_PIN = seed::D2;       // delay time

constexpr Pin SW_1_PIN = seed::D0;      //  0: reverse
constexpr Pin SW_2_PIN = seed::D1;      //  1: hold
constexpr Pin SW_3_PIN = seed::D3;      //  2: send on
constexpr Pin SW_4_PIN = seed::D4;      //  3: reset
constexpr Pin SW_5_PIN = seed::D5;      //  4: tap
constexpr Pin SW_6_PIN = seed::D6;      //  5: clock
constexpr Pin SW_7_PIN = seed::D7;      //  6: tri
constexpr Pin SW_8_PIN = seed::D8;      //  7: dot
constexpr Pin SW_9_PIN = seed::D9;      //  8: fx 1
constexpr Pin SW_10_PIN = seed::D10;    //  9: fx 2
constexpr Pin SW_11_PIN = seed::D11;    // 10: tap on
constexpr Pin SW_12_PIN = seed::D12;    // 11: clock on

// Knobs
constexpr Pin PIN_KNOB_1 = seed::A5;    // 0: mix
constexpr Pin PIN_KNOB_2 = seed::A4;    // 1: fx 1
constexpr Pin PIN_KNOB_3 = seed::A3;    // 2: tempo
constexpr Pin PIN_KNOB_4 = seed::A2;    // 3: feedback
constexpr Pin PIN_KNOB_5 = seed::A1;    // 4: fx 2
constexpr Pin PIN_KNOB_6 = seed::A0;    // 5: mult


void DaisyHardware::Init(bool boost) {
    seed.Configure();
    seed.Init(boost);
    InitSwitches();
    InitAnalogControls();
    InitLED();
    SetAudioBlockSize(48);
}

void DaisyHardware::SetHidUpdateRates() {
    for(size_t i = 0; i < KNOB_LAST; i++)
        knob[i].SetSampleRate(AudioCallbackRate());
}

void DaisyHardware::StartAudio(AudioHandle::InterleavingAudioCallback cb) { seed.StartAudio(cb); }
void DaisyHardware::StartAudio(AudioHandle::AudioCallback cb) { seed.StartAudio(cb); }
void DaisyHardware::StopAudio() { seed.StopAudio(); }

void DaisyHardware::ChangeAudioCallback(AudioHandle::InterleavingAudioCallback cb) { seed.ChangeAudioCallback(cb); }
void DaisyHardware::ChangeAudioCallback(AudioHandle::AudioCallback cb) { seed.ChangeAudioCallback(cb); }

size_t DaisyHardware::AudioBlockSize() { return seed.AudioBlockSize(); }
float DaisyHardware::AudioSampleRate() { return seed.AudioSampleRate(); }
float DaisyHardware::AudioCallbackRate() { return seed.AudioCallbackRate(); }

void DaisyHardware::SetAudioBlockSize(size_t size) { 
    seed.SetAudioBlockSize(size); 
    SetHidUpdateRates();
}
void DaisyHardware::SetAudioSampleRate(SaiHandle::Config::SampleRate samplerate) { 
    seed.SetAudioSampleRate(samplerate); 
    SetHidUpdateRates(); 
}

void DaisyHardware::StartAdc() { seed.adc.Start(); }
void DaisyHardware::StopAdc() { seed.adc.Stop(); }

void DaisyHardware::ProcessAnalogControls() {
    for(size_t i = 0; i < KNOB_LAST; i++)
        knob[i].Process();
}

float DaisyHardware::GetKnobValue(Knob k) {
    size_t idx;
    idx = k < KNOB_LAST ? k : KNOB_1;
    return knob[idx].Value();
}

void DaisyHardware::ProcessDigitalControls() {
    for(size_t i = 0; i < SW_LAST; i++)
        switches[i].Debounce();
}

void DaisyHardware::InitSwitches() {
    constexpr Pin pin_numbers[SW_LAST] = {
        SW_1_PIN,
        SW_2_PIN,
        SW_3_PIN,
        SW_4_PIN,
        SW_5_PIN,
        SW_6_PIN,
        SW_7_PIN,
        SW_8_PIN,
        SW_9_PIN,
        SW_10_PIN,
        SW_11_PIN,
        SW_12_PIN,
    };

    for(size_t i = 0; i < SW_LAST; i++)
        switches[i].Init(pin_numbers[i]);
}

void DaisyHardware::InitAnalogControls() {
    AdcChannelConfig cfg[KNOB_LAST];

    cfg[KNOB_1].InitSingle(PIN_KNOB_1);
    cfg[KNOB_2].InitSingle(PIN_KNOB_2);
    cfg[KNOB_3].InitSingle(PIN_KNOB_3);
    cfg[KNOB_4].InitSingle(PIN_KNOB_4);
    cfg[KNOB_5].InitSingle(PIN_KNOB_5);
    cfg[KNOB_6].InitSingle(PIN_KNOB_6);

    seed.adc.Init(cfg, KNOB_LAST + 1);

    for(int i = 0; i < KNOB_LAST; i++)
        knob[i].Init(seed.adc.GetPtr(i), AudioCallbackRate());
} 

void DaisyHardware::InitLED() {
    LED.Init(LED_PIN, false);
}

void DaisyHardware::SetLED(float value) {
    LED.Set(value);
    LED.Update();
}

void DaisyHardware::LEDOn() { SetLED(1.0f); }
void DaisyHardware::LEDOff() { SetLED(0.0f); }
void DaisyHardware::BlinkLED(float ms) {
    LEDOn(); System::Delay(ms / 2);
    LEDOff(); System::Delay(ms / 2);
}
