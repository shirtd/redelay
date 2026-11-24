#pragma once
#ifndef DSY_TAPTEMPO
#define DSY_TAPTEMPO
#include <stdlib.h>
#include <stdint.h>
namespace daisy {
/**
   @brief Tap tempo for daisy hardware.
   @ingroup boards
*/
template<size_t sample_rate_, size_t block_size_>
class TapTempo
{
    public:
        /** Constructor */
        TapTempo() {}
        /** Destructor */
        ~TapTempo() {}

        /** Initialize tap tempo
         *  \param min_count minimum samples
         *  \param max_count maximum samples
         */
        void Init(size_t min_count, size_t max_count) {
            min_count_ = min_count;
            max_count_ = max_count;
            tapping_ = false;
            averaging_ = false;
            use_tt_ = false;
            count_ = 0;
            count_avg_ = min_count;
        }

        /** Called once every audio block. 
         *  Increments counter and resets values when max_count time is exceeded
         */
        void Process() {
            if(tapping_)
                count_ = count_ + block_size_;
            // After max_count samples without a tap, reset values
            if(count_ >= max_count_) {  
                count_ = 0;
                tapping_ = false;
                averaging_ = false;
            }
        }

        /** Called when a tap occurs. 
         *  Initiates counting and calculates count average
         */
        void Tap() {
            if(tapping_) {
                if(count_ > min_count_) {
                    // 3rd plus tap
                    if (averaging_) { // Weighted Averaging
                        count_avg_ = 0.6*(count_avg_) + 0.4*(count_); 
                    // 2nd tap
                    } else {
                        count_avg_ = count_;
                        averaging_ = true;
                        use_tt_ = true;
                    }
                    count_ = 0;
                } 
            // 1st tap
            } else {
                tapping_ = true;
            }
        }

        /** Returns True while the counting audio blocks between taps 
         *  \return is tapping
        */
        bool Tapping(){ return tapping_; } 

        /** Use tap tempo. 
         *  Used to override externally set value with tap tempo value.
         *  Must be manually deasserted with Stop_tt()
         *  \return currently using tap tempo
         */
        bool Use() { return use_tt_; }

        /** Deassert tap tempo overide */
        void Stop() { use_tt_ = false; }

        /** Returns the average tap time in samples 
         *  \return average tap time in samples
        */
        size_t GetSamples() { return count_avg_; }


        /** Returns the average tap time in milliseconds
         *  \return average tap time in milliseconds
        */
        float GetMS() { return 1000.0 * (float)count_avg_ / (float)sample_rate_; }

    private:

        bool tapping_;      // True when user is tapping
        bool averaging_;    // True after 2nd tap
        bool use_tt_;       // Use tap tempo. Used to override externally set value with tap tempo value. Must be manually deasserted with Stop_tt()

        size_t min_count_;  // minimum samples
        size_t max_count_;  // maximum samples
        size_t count_;      // counter to count time in samples between taps
        size_t count_avg_;  // average count of time in samples between taps
};
}

#endif