/*
  ==============================================================================

    DelayLine.h
    Created: 8 Mar 2021 3:48:13pm
    Author:  mhhol

  ==============================================================================
*/

#pragma once
#include <memory>
#include <vector>

class DelayLine {
public:

    DelayLine(size_t buffer_length = 100) : buffer_length_(buffer_length)
    {
        clear();
    }

    size_t get_delay() const { return buffer_length_; }

    // Changes the delay to the new size samples.
    // Does a hard reset on the delay line - clearing all data.
    void set_delay(size_t new_size);

    // Add a sample to the end of the buffer.
    // Currently fails to fail if the buffer is full.
    void add(double sample) {
        valid_sample_count_ += 1;

        if (last_insert_pos_ >= (buffer_length_-1)) {
            last_insert_pos_ = 0;
        }
        else {
            ++last_insert_pos_;

        }
        buffer_[last_insert_pos_] = sample;
    }

    // Return the next value at the start of the buffer.
    // If the buffer is empty, it will return 0.0
    double get_next() {

        if (valid_sample_count_ == 0) return 0.0;

        auto ret_val = buffer_[next_return_pos_];
        --valid_sample_count_;
        ++next_return_pos_;
        if (next_return_pos_ >= buffer_length_) next_return_pos_ = 0;

        return ret_val;
    }

    void do_delay(const std::vector<double>& input, std::vector<double>&output, int target_delay=-1);

private:
    std::unique_ptr<double[]> buffer_;
    size_t last_insert_pos_;
    size_t next_return_pos_ = 0;
    int valid_sample_count_ = 0;
    size_t buffer_length_;

    void reset() {
        valid_sample_count_ = buffer_length_;
        last_insert_pos_ = buffer_length_;
        next_return_pos_ = 0;
    }

    void clear(); 
};