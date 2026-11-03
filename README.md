<div align="center">

<picture>
  <img alt="AnycubicSlicerNext logo" src="resources/images/AnycubicSlicer.png" width="15%" height="15%">
</picture>

# AnycubicSlicerNext 
</div>
AnycubicSlicerNext is an open source Next-Gen Slicing Software for Precision 3D Prints.  
Optimize your prints with ultra-fast slicing, intelligent support generation, and seamless printer compatibility—engineered for perfection.

AnycubicSlicerNext is an open-source slicing software developed based on OrcaSlicer. It delivers powerful slicing performance and an exceptional user experience.  
OrcaSlicer was originally forked from Bambu Studio, it was previously known as BambuStudio-SoftFever.  
[Bambu Studio](https://github.com/bambulab/BambuStudio) is forked from [PrusaSlicer](https://github.com/prusa3d/PrusaSlicer) by Prusa Research, which is from [Slic3r](https://github.com/Slic3r/Slic3r) by Alessandro Ranellucci and the RepRap community.  

See the [wiki](doc/Wiki.md) for more information.
<h3>

## [How to compile](doc/How-to-build.md) 

build instructions for Windows, macOS, and Linux 

AnycubicSlicerNext is an open-source slicing software developed based on OrcaSlicer. It delivers powerful slicing performance and an exceptional user experience.  
OrcaSlicer was originally forked from Bambu Studio, it was previously known as BambuStudio-SoftFever.  
[Bambu Studio](https://github.com/bambulab/BambuStudio) is forked from [PrusaSlicer](https://github.com/prusa3d/PrusaSlicer) by Prusa Research, which is from [Slic3r](https://github.com/Slic3r/Slic3r) by Alessandro Ranellucci and the RepRap community.  

See the [wiki](doc/Wiki.md) for more information.
<h3>

## [How to compile](doc/How-to-build.md) 

build instructions for Windows, macOS, and Linux 

## Optional 3MF sanitization

When opening a 3MF project or importing its settings, choose **Yes** to sanitize
before loading, **No** to load unchanged, or **Cancel** to stop the import.
Each file has its own choice. Geometry-only imports, recovery and silent loads
do not show this prompt.

The two compatibility fixes from [3MF Sanitizer](https://github.com/myles1663/3mf-Sanitizer)
replace `-1` (including the string `"-1"`) with `0` for
`raft_first_layer_expansion` and `tree_support_wall_count` in the extracted JSON
project settings and embedded presets. The original 3MF is not overwritten,
and G-code is not modified.

## Telegram support

The Telegram button in the top bar (Windows and Linux) and **Help > Telegram
Support** (all platforms) open [t.me/anycubic](https://t.me/anycubic) in the
default web browser.

# License

- **AnycubicSlicerNext** is licensed under the GNU Affero General Public License, version 3. AnycubicSlicerNext is based on OrcaSlicer.

- **OrcaSlicer** is licensed under the GNU Affero General Public License, version 3. OrcaSlicer is based on Bambu Studio by BambuLab.
- **Bambu Studio** is licensed under the GNU Affero General Public License, version 3. Bambu Studio is based on PrusaSlicer by PrusaResearch.
- **PrusaSlicer** is licensed under the GNU Affero General Public License, version 3. PrusaSlicer is owned by Prusa Research. PrusaSlicer is originally based on Slic3r by Alessandro Ranellucci.
- **Slic3r** is licensed under the GNU Affero General Public License, version 3. Slic3r was created by Alessandro Ranellucci with the help of many other contributors.
- The **GNU Affero General Public License**, version 3 ensures that if you use any part of this software in any way (even behind a web server), your software must be released under the same license.
- OrcaSlicer includes a **pressure advance calibration pattern test** adapted from Andrew Ellis' generator, which is licensed under GNU General Public License, version 3. Ellis' generator is itself adapted from a generator developed by Sineos for Marlin, which is licensed under GNU General Public License, version 3.
- The **Bambu networking plugin** is based on non-free libraries from BambuLab. It is optional to the OrcaSlicer and provides extended functionalities for Bambulab printer users.
