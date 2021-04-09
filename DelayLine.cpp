/*
  ==============================================================================

    DelayLine.cpp
    Created: 8 Mar 2021 3:48:13pm
    Author:  mhhol

  ==============================================================================
*/

#define JUCE_DEBUG 1

#include "DelayLine.h"
//#include <JuceHeader.h>
#include <cassert>
#include <algorithm>

void DelayLine::clear() {
    if (!buffer_) {
        buffer_ = std::make_unique<double[]>(buffer_length_);
    }
    else {
        std::fill(buffer_.get(), buffer_.get() + buffer_length_, 0.0);
    }
    reset();
}

void DelayLine::set_delay(size_t new_size) {

    // protect the user from themselves.
    if (new_size == buffer_length_) return;

    //DBG("DelayLine::set_delay - force setting delay to " << new_size << " samples");

    buffer_length_ = new_size;
    buffer_.reset(nullptr);

    // clear will take care of the details.
    clear();
}

/*
void DelayLine::resize(size_t new_size, DelayLine::ResizeAlgo algo) {

    // protect the user from themselves.
    if (new_size == buffer_length_) return;

    DBG("setting delay to " << new_size << " samples\n");

    if (algo == ResizeAlgo::CLEAR) {
        buffer_length_ = new_size;
        buffer_.reset(nullptr);
        // clear will take care of the details.
        clear();
    }
    else if (algo == ResizeAlgo::COPY) {
        auto new_buffer = std::make_unique<double[]>(new_size);

        // The mental model is that we are increasing/decreasing the delay
        // at the input end. So, samples already in the delay should receive
        // the older delay treatment.
        // This means that if we are decreasing the delay, we drop the newest
        // samples.

        // get_next() returns zeros when it runs out of valid samples. So this is fine.
        for (int i = 0; i < new_size; ++i) {
            new_buffer[i] = get_next();
        }

        std::swap(buffer_, new_buffer);
        buffer_length_ = new_size;
        reset();
    }
    else {
        auto new_buffer = std::make_unique<double[]>(new_size);

        DBG("DelayLine -- resampling");

        // Resampling :
        // Use a much simplified interpolation/decimation algorithm.
        // IF the delay line is going from I samples long to J samples long,
        // act as if we had done the following:
        // Step 1 : interpolate J-2 samples between each sample. This gives us
        //      (I-1)*(J-1)+1 samples because we can't interpolate past the last sample.
        // Step 2: take every (I-1)th sample. This leaves us with J samples.
        //
        // Rather than create that massive intermediate series of samples, we'll
        // note the following:
        //
        // The nth sample in the new series is the n*(I-1) sample in the
        // augmented series.
        // That means it is on or after the div(J-1,n*(I-1)) sample in the old series.
        // It is in the mod(J-1, n*(I-1)) interpolation slot.
        // We can do this one linear interpolation (or whatever algo we choose) to
        // get this value.
        //
        // This algorithm always takes the first and last samples.

        // If were going to do this for realzies we would need to add a low
        // pass filter to get rid of aliasing effects, etc.

        // where we are in the original series
        int current_old_sample_index = -1; 

        // The next two samples in the old series
        double old_values[2];

        old_values[0] = 0 ;
        old_values[1] = get_next();

        // Convenience variable - the J-1 value from the discussion above.
        int new_step = new_size - 1;

        for (int n = 0; n < new_size; ++n) {
            int i = n * (buffer_length_ - 1);
            int f = i / new_step;
            int s = i % new_step;

            if (f > current_old_sample_index) {
                // If new_size is << buffer_length_, we could skip multiple samples.
                old_values[0] = old_values[1];
                ++current_old_sample_index;
                for (;  current_old_sample_index < f; ++current_old_sample_index) {
                    old_values[0] = get_next();
                }
                old_values[1] = get_next();
            }

            // linear interpolation
            double interpolation = (double(new_step - s) * old_values[0] + old_values[1]) / new_step;
            new_buffer[n] = interpolation;

        }


        std::swap(buffer_, new_buffer);
        buffer_length_ = new_size;
        reset();
    }
};

*/

void DelayLine::do_delay(const std::vector<double>& input, std::vector<double>& output, int target_delay) {

    output.clear();

    if (target_delay < 0 || target_delay == buffer_length_) {
        // The delay isn't changing, so we just need to copy from
        // the buffer to the output.
        for (int i = 0; i < input.size(); ++i) {
            output.push_back(get_next());
            add(input[i]);
        }
        return;
    }

    // We assume that the target delay is "reasonably" close to our
    // current delay.
    // The idea is that we take just enough samples out of the delay line so that
    // when we add back in the input, the delay line winds up being the new
    // length. The samples are then up(or down) sampled to fit the size that
    // we are expected to return.

    // delta should be positive if we are making the delay smaller.
    // This is because we need to take more samples off the buffer.
    int delta = buffer_length_ - target_delay;

    assert(std::abs(delta) < input.size()-2);

    int temp_buffer_size = input.size() + delta;

    std::vector<double> temp_buffer;

    int inp_index = 0;
    for (int i = 0; i < temp_buffer_size; ++i) {
        temp_buffer.push_back(get_next());
        // Add the input in case the current delay is so short that
        // we need part of the input to feed the output.
        if (inp_index < input.size())
            add(input[inp_index++]);
    }

    // Create the new buffer and fill it with anything left in the delay line 
    // as well as anything more from the input (in that order).
    auto new_buffer = std::make_unique<double[]>(target_delay);
    int write_pos = 0;
    while (valid_sample_count_ > 0) {
        new_buffer[write_pos] = get_next();
        ++write_pos;
    }

    for (; inp_index < input.size(); ++inp_index) {
        new_buffer[write_pos] = input[inp_index];
        ++write_pos;
    }
    assert(valid_sample_count_ <= target_delay);

    // patch up all our invariants
    std::swap(buffer_, new_buffer);
    buffer_length_ = target_delay;
    reset();

    // Resampling :
    // Use a much simplified interpolation/decimation algorithm.
    // If the delay line is going from I samples long to J samples long,
    // act as if we had done the following:
    // Step 1 : interpolate J-2 samples between each sample. This gives us
    //      (I-1)*(J-1)+1 samples because we can't interpolate past the last sample.
    // Step 2: take every (I-1)th sample. This leaves us with J samples.
    //
    // Rather than create that massive intermediate series of samples, we'll
    // note the following:
    //
    // The nth sample in the new series is the n*(I-1) sample in the
    // augmented series.
    // That means it is on or after the div(J-1,n*(I-1)) sample in the old series.
    // It is in the mod(J-1, n*(I-1)) interpolation slot.
    // We can do this one linear interpolation (or whatever algo we choose) to
    // get this value.
    //
    // This algorithm always takes the first and last samples.

    // If were going to do this for realzies we would need to add a low
    // pass filter to get rid of aliasing effects, etc.

    // For this I (current length) = length of the temp buffer we filled.
    //          J (new length)     = length of the input.

    // Cubic interpolation : https://www.paulinternet.nlk/?page=bicubic with
    //  hints from http://paulbourke.net/miscellaneous/interpolation

    auto old_size = temp_buffer.size();
    auto new_size = input.size();

    // Values from the original series used in the interpolation. We will be interpolating 
    // in the interval between base_values[1] and base_values[2]
    double base_values[4];

    // Convenience variable - the J-1 value from the discussion above.
    auto new_step = new_size - 1;

    for (size_t n = 0; n < new_size; ++n) {
        // n = index in the "new" series
   
        // i = index in the augmented series
        auto i = n * (old_size - 1);

        // f = index in the original series
        auto f = i / new_step;

        // s = "slot" along the line from f to f+1
        auto s = i % new_step;


        double interpolation;

        if ((s == 0) || (f >= (old_size-1))) {
            interpolation = temp_buffer[f];
        }
        else {
            if (f == 0) {
                // we will make this nicer later
                base_values[0] = temp_buffer[f];
            }
            else {
                base_values[0] = temp_buffer[f - 1];
            }

            base_values[1] = temp_buffer[f];
            base_values[2] = temp_buffer[f + 1];

            if (f == old_size - 2) {
                // we will make this nicer later
                base_values[3] = temp_buffer[f+1];
            }
            else {
                base_values[3] = temp_buffer[f + 2];
            }

            double a, b, c, d;
            a = -0.5 * base_values[0] + 1.5 * base_values[1] - 1.5 * base_values[2] + 0.5 * base_values[3];
            b =        base_values[0] - 2.5 * base_values[1] + 2   * base_values[2] - 0.5 * base_values[3];
            c = -0.5 * base_values[0]                        + 0.5 * base_values[2];
            d =                               base_values[1];

            auto mu = double(s) / double(new_size);

            interpolation = a * mu*mu*mu + b*mu*mu + c*mu + d;
        }

        output.push_back(interpolation);
    }
    
}
