#/|/ Copyright (c) Prusa Research 2018 Vojtěch Bubník @bubnikv
#/|/ Copyright (c) Slic3r 2011 - 2015 Alessandro Ranellucci @alranel
#/|/
#/|/ PrusaSlicer is released under the terms of the AGPLv3 or higher
#/|/
package Slic3r::Point;
use strict;
use warnings;

sub new_scale {
    my $class = shift;
    return $class->new(map Slic3r::Geometry::scale($_), @_);
}

package Slic3r::Pointf;
use strict;
use warnings;

sub new_unscale {
    my $class = shift;
    return $class->new(map Slic3r::Geometry::unscale($_), @_);
}

package Slic3r::Pointf3;
use strict;
use warnings;

sub new_unscale {
    my $class = shift;
    return $class->new(map Slic3r::Geometry::unscale($_), @_);
}

1;
