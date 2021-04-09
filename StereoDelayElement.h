/*
  ==============================================================================

    StereoDelayElement.h
    Created: 8 Mar 2021 8:37:45pm
    Author:  mhhol

  ==============================================================================
*/

#pragma once

#include "DelayLine.h"

#include <array>

class StereoDelayElement {
public:
    // These two force a hard reset on the delay lines. All data is cleared.
    void set_delay(double msec, double sample_rate = -1);
    void set_sample_rate(double sample_rate);

    // This tries to do something graceful with the change
    void change_delay(double new_msec);

    void do_delay(int channel, const std::vector<double>& input, std::vector<double>& output);

private:
    static constexpr int CHANNEL_COUNT = 2;

    std::array<DelayLine, CHANNEL_COUNT> delays;
    double sample_rate_ = 44100.0;
    double delay_msec_[CHANNEL_COUNT];

    double target_msec_ = 200.0;

    void recalc_delays(double new_rate, double new_msec);
    int msec_to_sample(double msec) { return int(sample_rate_ * .001 * msec); }
    double sample_to_msec(int samples) { return double(samples) * 1000.0 / sample_rate_; }

};