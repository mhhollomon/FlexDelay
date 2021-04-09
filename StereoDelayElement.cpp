/*
  ==============================================================================

    StereoDelayElement.cpp
    Created: 8 Mar 2021 8:37:45pm
    Author:  mhhol

  ==============================================================================
*/

#include "StereoDelayElement.h"

void StereoDelayElement::set_sample_rate(double sample_rate) {
    if (sample_rate != sample_rate_) {
        recalc_delays(sample_rate, delay_msec_[0]);
    }
}


void StereoDelayElement::set_delay(double msec, double sample_rate) {
    if (sample_rate < 0) sample_rate = sample_rate_;

    recalc_delays(sample_rate, msec);
}

void StereoDelayElement::change_delay(double new_msec) {
    target_msec_ = new_msec;
}

constexpr double DELTA_FACTOR = 0.3;

void StereoDelayElement::do_delay(int channel, const std::vector<double>& input, std::vector<double>& output) {

    if (target_msec_ != delay_msec_[channel]) {

        auto old_msec = delay_msec_[channel];
        auto old_delay_samples = msec_to_sample(old_msec);

        auto new_msec = target_msec_;
        auto new_delay_samples = msec_to_sample(new_msec);
        
        auto delta = new_delay_samples - old_delay_samples;
        if (std::abs(delta) > (DELTA_FACTOR * input.size())) {
            int sign = (delta > 0) - (delta < 0);
            new_delay_samples = old_delay_samples + int(DELTA_FACTOR * input.size() * sign);
            new_msec = sample_to_msec(new_delay_samples);
        }
        delay_msec_[channel] = new_msec;

        delays[channel].do_delay(input, output, new_delay_samples);
    }
    else {
        delays[channel].do_delay(input, output, -1);
    }
}


void StereoDelayElement::recalc_delays(double new_rate, double new_msec) {

    // Hard reset the delay lines to the new (possibly the same) values.

    sample_rate_ = new_rate;

    for (auto& x : delay_msec_) {
        x = new_msec;
    }

    auto delay_samples = new_rate * new_msec * 0.001;

    for (auto& d : delays) {
        d.set_delay(delay_samples);
    }

}
