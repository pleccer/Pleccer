#/|/ Copyright (c) Prusa Research 2018 Vojtěch Bubník @bubnikv
#/|/ Copyright (c) Slic3r 2011 - 2014 Alessandro Ranellucci @alranel
#/|/ Copyright (c) 2012 Mark Hindess
#/|/
#/|/ PrusaSlicer is released under the terms of the AGPLv3 or higher
#/|/
package Slic3r::Polyline;
use strict;
use warnings;

use Slic3r::Geometry qw(X Y);

sub new_scale {
    my $class = shift;
    my @points = map { ref($_) eq 'Slic3r::Point' ? $_->pp : $_ } @_;
    return $class->new(map [ Slic3r::Geometry::scale($_->[X]), Slic3r::Geometry::scale($_->[Y]) ], @points);
}

1;
