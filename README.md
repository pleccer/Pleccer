# IMPORTANT!! This slicer is in beta version and it is still under a lot of development. A stable version is planned for release in March 2023.

# Pleccer

### **A slicer focused on supporting waste-free multicolor 3D printing based on PrusaSlicer**

![splash_logo](https://user-images.githubusercontent.com/124889495/219878352-059f453c-721a-4bdd-929b-c02ebb01fe76.png)


Pleccer incorporates features to make it easier to make a colored 3d model and slice it for a multicolor mixing or non-mixing 3D printer. It is part of the PLEC (purgeless extruding colorer) project; which develops the needs to upgrade your/any printer to a mixing color 3D printer optimized for avoiding waste. You can find out more about PLEC on the [community site](https://plec.community).

## What are Pleccer's special features?

* Mixing color g-code generation based on calibrated data and the color you pick
* Improved color picker (thanks to [colorgrab](https://github.com/nielssp/colorgrab))
* Mixing color calibration setting (requires a (diy) color measurer)
* Enhanced smart retraction and gradient change calibration and automatic model updating for purge-free printing (no prime tower/purge)
* Includes Arc overhang infill ["Multiplex"-version](https://github.com/stmcculloch/arc-overhang/issues/10) for support-free printing with also a spiral-mode for pedestal-connected overhangs
* Includes smart island-only tree support setting for use with arc infill overhangs to have only the really required supports printed
* Includes Lightning infill for faster printing
* Includes Arachne g-code generation
* It has also all the current PrusaSlicer's features

### What are the main features?

* **Multi-platform** (Linux/Mac/Win) and packaged as standalone-app with no dependencies required
* Complete **command-line interface** to use it without GUI
* Multi-material **(multiple extruders)** object printing
* Multiple G-code flavors supported (RepRap, Makerbot, Mach3, Machinekit, etc.)
* Ability to plate **multiple objects having distinct print settings**
* **Multithread** processing
* **STL auto-repair** (tolerance for broken models)
* Wide automated unit testing

Other major features are:

* Combine infill every 'n' perimeters layer & varying density to speed up printing
* **3D preview** (including multi-material files)
* **Multiple layer heights** in a single print
* **Spiral vase** mode for bumpless vases
* Fine adjustment of speed, acceleration, and extrusion width
* Several infill patterns including honeycomb, spirals, Hilbert curves, gyroid
* Support material, raft, brim, skirt
* **Standby temperature** and automatic wiping for multi-extruder printing
* [Customizable **G-code macros**](https://help.prusa3d.com/article/list-of-placeholders_205643) and output filename with variable placeholders
* Support for **post-processing scripts**
* **Cooling logic** controlling fan speed and dynamic print speed
* Flushing transition-filament into infill/object during filament change
* Support STEP format


## How to compile
Following platforms are currently supported to compile:
- Windows 64-bit, [Compile Guide](https://github.com/pleccer/Pleccer/wiki/Windows-Compile-Guide)
- Mac 64-bit, [Compile Guide](https://github.com/pleccer/Pleccer/wiki/Mac-Compile-Guide)
- Linux 64-bit, [Compile Guide](https://github.com/pleccer/Pleccer/wiki/Linux-Compile-Guide)

## Report issue
You can add an issue to the [github tracker](https://github.com/pleccer/Pleccer/issues) if **it isn't already present.**

## License
Pleccer is licensed under the GNU Affero General Public License, version 3. 
Pleccer is based on PrusaSlicer by PrusaResearch.

PrusaSlicer is licensed under the GNU Affero General Public License, version 3. PrusaSlicer is owned by Prusa Research. PrusaSlicer is originally based on Slic3r by Alessandro Ranellucci.

Slic3r is licensed under the GNU Affero General Public License, version 3. Slic3r was created by Alessandro Ranellucci with the help of many other contributors.

The GNU Affero General Public License, version 3 ensures that if you use any part of this software in any way (even behind a web server), your software must be released under the same license.
