///|/ Copyright (c) Prusa Research 2016 - 2020 Vojtěch Bubník @bubnikv, Lukáš Matěna @lukasmatena
///|/
///|/ PrusaSlicer is released under the terms of the AGPLv3 or higher
///|/
#ifndef slic3r_Fill_hpp_
#define slic3r_Fill_hpp_

#include <memory.h>
#include <float.h>
#include <stdint.h>

#include "../libslic3r.h"
#include "../PrintConfig.hpp"

#include "FillBase.hpp"

namespace Slic3r {

class ExtrusionEntityCollection;
class LayerRegion;

// An interface class to Perl, aggregating an instance of a Fill and a FillData.
class Filler
{
public:
    Filler() : fill(nullptr) {}
    ~Filler() { 
        delete fill; 
        fill = nullptr;
    }
    Fill        *fill;
    FillParams   params;
};

} // namespace Slic3r

#endif // slic3r_Fill_hpp_
