#/|/ Copyright (c) Prusa Research 2017 - 2022 Vojtěch Bubník @bubnikv
#/|/ Copyright (c) Slic3r 2011 - 2015 Alessandro Ranellucci @alranel
#/|/ Copyright (c) 2013 Jose Luis Perez Diez
#/|/ Copyright (c) 2013 Anders Sundman
#/|/ Copyright (c) 2013 Jesse Vincent
#/|/ Copyright (c) 2012 Mike Sheldrake @mesheldrake
#/|/ Copyright (c) 2012 Mark Hindess
#/|/
#/|/ PrusaSlicer is released under the terms of the AGPLv3 or higher
#/|/
package Slic3r::Geometry;
use strict;
use warnings;

require Exporter;
our @ISA = qw(Exporter);

# Exported by this module. The last section starting with convex_hull is exported by Geometry.xsp
our @EXPORT_OK = qw(
    PI epsilon 

    scale
    unscale
    scaled_epsilon

    X Y Z
    convex_hull
    deg2rad
    rad2deg
);

use constant PI => 4 * atan2(1, 1);
use constant A => 0;
use constant B => 1;
use constant X1 => 0;
use constant Y1 => 1;
use constant X2 => 2;
use constant Y2 => 3;

sub epsilon () { 1E-4 }
sub scaled_epsilon () { epsilon / &Slic3r::SCALING_FACTOR }

sub scale   ($) { $_[0] / &Slic3r::SCALING_FACTOR }
sub unscale ($) { $_[0] * &Slic3r::SCALING_FACTOR }

1;
