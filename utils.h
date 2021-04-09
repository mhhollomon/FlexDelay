/*
  ==============================================================================

    utils.h
    Created: 8 Mar 2021 8:32:42pm
    Author:  mhhol

  ==============================================================================
*/

#pragma once


struct utils {
    
    static double db_to_factor(double db) {
        // dB  = 20*Log10(out/in)
        // out = in * 10^(db/20)
        // so, scale = 10^(db/20)

        return std::pow(10, db/20.0);
    }
};