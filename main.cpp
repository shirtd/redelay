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
#define BLOCK_SIZE static_cast<size_t>(32)
#define ATTENUATION 0.7f

#define MIN_CUTOFF 100.0f
#define MAX_CUTOFF 20000.0f

#define MIN_RES 0.0f
#define MAX_RES 0.95f

using namespace daisy;
using namespace daisysp;

DaisyHardware hw;

MoogLadder filter;
ReverseDelayLine<float, MAX_DELAY> DSY_SDRAM_BSS delMem;
TapTempo<static_cast<size_t>(SAMPLE_RATE), BLOCK_SIZE> tapTempo;
TapTempo<static_cast<size_t>(SAMPLE_RATE), BLOCK_SIZE> clockTempo;

struct DelaySample {
    float read;
    float write;
};

struct Delay {
    ReverseDelayLine<float, MAX_DELAY> *del;
    float currentDelay;
    float delayTarget;
    float feedback;
    bool reverse;
    bool hold;
    bool send;

    DelaySample Process(float in, float receive) {
        // TODO 
        //  a) coefficient as a parameter?
        //  b) toggle to enable/disable/change smoothing
        fonepole(currentDelay, delayTarget, .0002f); 
        del->SetDelay(currentDelay);

        float read = del->Read();
        float write = ((hold ? 1.0f : feedback) * read) + in;
        del->Write(send ? receive : write);

        return DelaySample {reverse ? del->ReadReverse() : read, write};
    }
};


Delay delay;
Parameter delay_time;
Parameter filter_cutoff;
Parameter filter_res;

float mult; // TODO mult as a pair of integers
float tempo;
float drywet;
float cutoff;
int filtPos;

void ProcessControls();

static void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size) {
    ProcessControls();

    float x, y;
    DelaySample sample;
    for(size_t i = 0; i < size; i++) {
        x = in[0][i];
        if (filtPos < 0)
            x = filter.Process(x);
        sample = delay.Process(x, in[1][i]);
        y = drywet * sample.read + (1.0f - drywet) * x;
        if (filtPos > 0)
            y = filter.Process(y);
        out[0][i] = y, out[1][i] = sample.write;
    }  
}

void InitDelay(float samplerate) {
    delMem.Init();
    delay.del = &delMem;
    filter.Init(samplerate);
    delay_time.Init(hw.knob[2], MIN_DELAY, MAX_DELAY, Parameter::LOGARITHMIC);
    filter_cutoff.Init(hw.knob[1], MIN_CUTOFF, MAX_CUTOFF, Parameter::LOGARITHMIC);
    filter_res.Init(hw.knob[4], MIN_RES, MAX_RES, Parameter::LINEAR);
    tapTempo.Init(MIN_DELAY, 2 * MAX_DELAY);
    clockTempo.Init(MIN_DELAY, 2 * MAX_DELAY);
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

float GetMult(float m, bool tri, bool dot) {
    float mult = 1.0f / std::pow(2.0f, std::max(std::floor(4.0f - 6.0f * m), -1.0f));
    return tri ? 3 * mult / 2 : dot ? 2 * mult / 3 : mult;
}

void ProcessControls() {
    hw.ProcessAnalogControls();
    hw.ProcessDigitalControls();

    tapTempo.Process();
    clockTempo.Process();

    if(hw.switches[3].RisingEdge())
        delMem.Reset();

    if (hw.switches[4].RisingEdge())
        tapTempo.Tap();
    
    if (hw.switches[5].RisingEdge())
        clockTempo.Tap();
    
    if (hw.switches[10].Pressed())
        tempo = tapTempo.GetSamples();
    else if (hw.switches[11].Pressed())
        tempo = clockTempo.GetSamples();
    else
        tempo = delay_time.Process();

    filter.SetFreq(filter_cutoff.Process());
    filter.SetRes(filter_res.Process());

    filtPos = hw.switches[8].Pressed() ? 0 : hw.switches[9].Pressed() ? 1 : -1;
    delay.reverse = hw.switches[0].Pressed();
    delay.hold = hw.switches[1].Pressed();
    delay.send = hw.switches[2].Pressed();
    delay.feedback = hw.knob[3].Process();

    drywet = hw.knob[0].Process();
    mult = GetMult(hw.knob[5].Process(),
                   hw.switches[7].Pressed(),
                   hw.switches[6].Pressed());

    delay.delayTarget = tempo * mult;
}
