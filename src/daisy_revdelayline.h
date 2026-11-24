/*
Copyright (c) 2020 Electrosmith, Corp

Use of this source code is governed by an MIT-style
license that can be found in the LICENSE file or at
https://opensource.org/licenses/MIT.
*/

#pragma once
#include <stdlib.h>
#include <stdint.h>
namespace daisysp
{
/** Simple Delay line.
November 2019

Converted to Template December 2019

declaration example: (1 second of floats)

RevDelayLine<float, SAMPLE_RATE> del;

By: shensley

Modified for Reverse Delay September 2026
by shirtd
*/

template <typename T, size_t max_size>
class ReverseDelayLine
{
  public:
    /** Constructor */
    ReverseDelayLine() {}
    /** Destructor */
    ~ReverseDelayLine() {}

    /** initializes the delay line by clearing the values within, and setting delay to 1 sample.
    */
    void Init() { Reset(); }
    /** clears buffer, sets write ptr to 0, and delay to 1 sample.
    */
    void Reset() {
        std::fill(&line_[0], &line_[max_size], T(0));
        write_ptr_ = 0;
        phase_ = 0.0f;
    }

    /** sets the delay time in samples
     *  If a float is passed in, a fractional component will be calculated for interpolating the delay line.
     *  \param delay delay time in samples  
    */
    inline void SetDelay(size_t delay) {
        frac_  = 0.0f;
        delay_ = delay < max_size ? delay : max_size - 1;
    }
    inline void SetDelay(float delay) {
        int32_t int_delay = static_cast<int32_t>(delay);
        frac_ = delay - static_cast<float>(int_delay);
        delay_ = static_cast<size_t>(int_delay) < max_size ? int_delay : max_size - 1;
    }

    inline size_t GetDelay() { return delay_; }
    inline float GetFrac() { return frac_; }

    /** writes the sample of type T to the delay line, and advances the write ptr
     * \param sample the sample to write
    */
    inline void Write(const T sample) {
        line_[write_ptr_] = sample;
        write_ptr_ = (write_ptr_ - 1 + max_size) % max_size;
    }

    /** read delay line in reverse
     *  \param delay_samples length of the reverse window (must be <= max_delay - 4)
     *  \param speed 1.0f for normal reverse, 2.0f for octave-up reverse, etc.
    */
    inline const T ReadReverse(float delay_samples, float speed = 1.0f) {
        delay_samples = delay_samples < 4.0f ? 4.0f : delay_samples;
        if (delay_samples > static_cast<float>(max_size - 4))
            delay_samples = static_cast<float>(max_size - 4);
            
        // Advance phase normalized to [0.0, 1.0) across the window
        phase_ += (1.0f + speed) / delay_samples;
        if (phase_ >= 1.0f) 
            phase_ -= 1.0f;

        // Two read heads offset by 0.5 (180 degrees) to crossfade seamlessly
        float phase_a = phase_, phase_b = phase_ + 0.5f;
        if (phase_b >= 1.0f)
            phase_b -= 1.0f;

        // Triangle window: 0.0 at wrap boundaries (0.0 and 1.0), 1.0 at center (0.5)
        float gain_a = 1.0f - fabsf(2.0f * phase_a - 1.0f), gain_b = 1.0f - fabsf(2.0f * phase_b - 1.0f);
        return ReadHermite(phase_a * delay_samples) * gain_a + ReadHermite(phase_b * delay_samples) * gain_b;
    }

    inline const T ReadReverse() { return ReadReverse(delay_); }

    /** returns the next sample of type T in the delay line, interpolated if necessary.
     *  \param reverse boolean flag indicating the delay line should be read in reverse
    */
    inline const T Read() const {
        T a = line_[(write_ptr_ + delay_) % max_size];
        T b = line_[(write_ptr_ + delay_ + 1) % max_size];
        return a + (b - a) * frac_;
    }

    /** Read from a set location
     *  \param delay location to read from 
    */
    inline const T Read(float delay) const {
        int32_t delay_integral   = static_cast<int32_t>(delay);
        float   delay_fractional = delay - static_cast<float>(delay_integral);
        const T a = line_[(write_ptr_ + delay_integral) % max_size];
        const T b = line_[(write_ptr_ + delay_integral + 1) % max_size];
        return a + (b - a) * delay_fractional;
    }
    /** Read from a set location using hermite interpolation
     *  \param delay location to read from 
    */
    inline const T ReadHermite(float delay) const {
        int32_t delay_integral   = static_cast<int32_t>(delay);
        float   delay_fractional = delay - static_cast<float>(delay_integral);

        int32_t     t     = (write_ptr_ + delay_integral + max_size);
        const T     xm1   = line_[(t - 1) % max_size];
        const T     x0    = line_[(t) % max_size];
        const T     x1    = line_[(t + 1) % max_size];
        const T     x2    = line_[(t + 2) % max_size];
        const float c     = (x1 - xm1) * 0.5f;
        const float v     = x0 - x1;
        const float w     = c + v;
        const float a     = w + v + (x2 - x0) * 0.5f;
        const float b_neg = w + a;
        const float f     = delay_fractional;
        return (((a * f) - b_neg) * f + c) * f + x0;
    }

    inline const T Allpass(const T sample, size_t delay, const T coefficient) {
        T read  = line_[(write_ptr_ + delay) % max_size];
        T write = sample + coefficient * read;
        Write(write);
        return -write * coefficient + read;
    }

  private:
    float  frac_; 
    float phase_;
    size_t write_ptr_;
    size_t delay_;
    T line_[max_size];
};
}