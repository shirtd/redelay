#include "daisysp.h"
#include "src/daisy_hardware.h"
#include "src/daisy_taptempo.h"
#include "src/daisy_revdelayline.h"
#include <string>

#ifndef SAMPLE_RATE
#define SAMPLE_RATE 48014.f
#endif

#define MAX_DELAY static_cast<size_t>(SAMPLE_RATE * 5.f)
#define MIN_DELAY static_cast<size_t>(256)
#define BLOCK_SIZE static_cast<size_t>(4)
#define ATTENUATION 0.7f

using namespace daisy;
using namespace daisysp;

DaisyHardware hw;

ReverseDelayLine<float, MAX_DELAY> DSY_SDRAM_BSS delMem;
TapTempo<static_cast<size_t>(SAMPLE_RATE), BLOCK_SIZE> tapTempo;


struct Delay {
    ReverseDelayLine<float, MAX_DELAY> *del;
    float currentDelay;
    float delayTarget;
    float feedback;

    float Process(float in, bool reverse) {
        //set delay times
        // TODO 
        //  a) coefficient as a parameter?
        //  b) toggle to enable/disable/change smoothing
        fonepole(currentDelay, delayTarget, .0002f); 
        del->SetDelay(currentDelay);
        
        float read = del->Read(); // TODO internalize send/receive to print to delay line
        del->Write((feedback * read) + in);

        return reverse ? del->ReadReverse() : read;
    }
    float Process(float in) { return Process(in, false); }
};


Delay delay;
Parameter param;

float mult; // TODO mult as a pair of integers
float tempo;
float feedback;
float drywet;
float reverse;

bool send;

void ProcessControls();

static void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size) {
    ProcessControls();

    float x;
    for(size_t i = 0; i < size; i++) {
        x = delay.Process(in[0][i], reverse) * ATTENUATION;
        if (send)
            out[1][i] = x, x = in[1][i]; // TODO internalize send/receive to print to delay line
        out[0][i] = drywet * x + (1.0f - drywet) * in[0][i];
    }
        
}

void InitDelay(float samplerate) {
    delMem.Init();
    delay.del = &delMem;
    param.Init(hw.knob[2], MIN_DELAY, MAX_DELAY, Parameter::LOGARITHMIC);
    tapTempo.Init(MIN_DELAY, 3 * MAX_DELAY);
}

int main(void) {
    float samplerate;
    hw.Init();
    hw.SetAudioBlockSize(BLOCK_SIZE);
    samplerate = hw.AudioSampleRate();

    InitDelay(samplerate);

    hw.StartAdc();
    hw.StartAudio(AudioCallback);
    while(1) {
        hw.BlinkLED(1000 * delay.currentDelay / SAMPLE_RATE);
    }
}

float GetMult(float m) {
    float mult = 1.0f / std::pow(2.0f, std::max(std::floor(4.0f - 6.0f * m), -1.0f));
    return hw.switches[7].Pressed() ? 3 * mult / 2 : hw.switches[6].Pressed() ? 2 * mult / 3 : mult;
}

void ProcessControls() {
    hw.ProcessAnalogControls();
    hw.ProcessDigitalControls();
    tapTempo.Process(); // TODO add tapTempo and clockTempo to hw.ProcessDigitalControls; link with LED

    send = hw.switches[2].Pressed();

    if(hw.switches[3].RisingEdge())
        delMem.Reset();

    if (hw.switches[4].RisingEdge() || hw.switches[5].RisingEdge())
        tapTempo.Tap(); // TODO tap vs. clock tempo
    
    if (hw.switches[10].Pressed()) {
        if (tapTempo.Use())
            tempo = tapTempo.GetSamples();
    } else if (hw.switches[11].Pressed()) {
        if (tapTempo.Use())
            tempo = tapTempo.GetSamples();
    } else {
        tempo = param.Process();
        tapTempo.Stop();
    }

    drywet = hw.knob[0].Process();
    delay.feedback = hw.switches[1].Pressed() ? 1.0f : hw.knob[3].Process(); // TODO internalize hold in Delay struct

    reverse = hw.switches[0].Pressed(); // TODO internalize reverse in Delay struct?
    mult = GetMult(hw.knob[5].Process()); // TODO internalize mult in Delay struct?
    delay.delayTarget = tempo * mult;
}
