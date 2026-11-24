#pragma once
#ifndef DSY_HARDWARE_H
#define DSY_HARDWARE_H /**< & */

#include "daisy_seed.h"

namespace daisy {
/**
   @brief Helpers and hardware definitions for daisy hardware.
   @ingroup boards
*/
class DaisyHardware
{
   public:
      /** Switches */
      enum Sw {
         SW_1,    /**< Footswitch */
         SW_2,    /**< Footswitch */
         SW_3,    /**< Toggle */
         SW_4,    /**< Toggle */
         SW_5,    /**< Toggle */
         SW_6,    /**< Toggle */
         SW_7,    /**< Toggle */
         SW_8,    /**< Toggle */
         SW_9,    /**< Toggle */
         SW_10,   /**< Toggle */
         SW_11,   /**< Toggle */
         SW_12,   /**< Toggle */
         SW_LAST, /**< Last enum item */
      };

      /** Knobs */
      enum Knob {
         KNOB_1,    /**< & */
         KNOB_2,    /**< & */
         KNOB_3,    /**< & */
         KNOB_4,    /**< & */
         KNOB_5,    /**< & */
         KNOB_6,    /**< & */
         KNOB_LAST, /**< & */
      };

      /** Constructor */
      DaisyHardware() {}
      /** Destructor */
      ~DaisyHardware() {}

      /** Initialize daisy hardware */
      void Init(bool boost = false);

      /** Starts the callback
         \param cb Interleaved callback function
      */
      void StartAudio(AudioHandle::InterleavingAudioCallback cb);

      /** Starts the callback
         \param cb multichannel callback function
      */
      void StartAudio(AudioHandle::AudioCallback cb);

      /** Switch callback functions
         \param cb New interleaved callback function.
      */
      void ChangeAudioCallback(AudioHandle::InterleavingAudioCallback cb);

      /**
         Switch callback functions
         \param cb New multichannel callback function.
      */
      void ChangeAudioCallback(AudioHandle::AudioCallback cb);

      /** Stops the audio if it is running. */
      void StopAudio();

      /** Updates the Audio Sample Rate, and reinitializes.
       ** Audio must be stopped for this to work.
      */
      void SetAudioSampleRate(SaiHandle::Config::SampleRate samplerate);

      /** Returns the audio sample rate in Hz as a floating point number.
       */
      float AudioSampleRate();

      /** Sets the number of samples processed per channel by the audio callback.
         \param size Audio block size
      */
      void SetAudioBlockSize(size_t size);

      /** Returns the number of samples per channel in a block of audio. */
      size_t AudioBlockSize();

      /** Returns the rate in Hz that the Audio callback is called */
      float AudioCallbackRate();

      /** Start analog to digital conversion. */
      void StartAdc();

      /** Stops Transfering data from the ADC */
      void StopAdc();

      /** Call at the same frequency as controls are read for stable readings.*/
      void ProcessAnalogControls();

      /** Process Analog and Digital Controls */
      inline void ProcessAllControls() {
         ProcessAnalogControls();
         ProcessDigitalControls();
      }


      /** Get value per knob.
         \param k Which knob to get
         \return Floating point knob position.
      */
      float GetKnobValue(Knob k);

      /** Process digital controls */
      void ProcessDigitalControls();

      void SetLED(float value);
      void LEDOn();
      void LEDOff();
      void BlinkLED(float ms);

      DaisySeed seed;    /**< & */

      AnalogControl knob[KNOB_LAST]; /**< & */
      Switch switches[SW_LAST] /**< & */;
      Led LED;

   private:
      void SetHidUpdateRates();
      void InitSwitches();
      void InitAnalogControls();
      void InitLED();

    inline uint16_t* adc_ptr(const uint8_t chn) { return seed.adc.GetPtr(chn); }
};
} // namespace daisy
#endif
