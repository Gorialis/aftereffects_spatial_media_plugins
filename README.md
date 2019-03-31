# After Effects Inverse Equirectangular Plug-in

This is a plug-in that can convert regular perspective footage (from e.g. a camera) to equirectangular patches that can be used to produce a full equirectangular image/video:

![Example of an equirectangular projection](demo.png)

## Installing

Check the [releases page](https://github.com/Gorialis/aftereffects_inverse_equirectangular/releases) and place the .aex file into the `Plug-ins` folder of your install, e.g. at `C:\Program Files\Adobe\Adobe After Effects CC 2018\Support Files\Plug-ins\`.

The effect is called "Inverse Equirectangular" and will appear in the "Devon's Plug-ins" subcategory.

## Using

This plugin works by mapping the individual bits of footage onto a larger 2-by-1 equirectangular master layer. The effect itself should be applied to this master layer.

For example, create a 2000x1000 solid, apply the effect to it, and select the footage to use for a patch as the "Source Layer" attribute.

The FOV is the vertical FOV of the footage, and the yaw and pitch are relative to the center of the equirectangular image.

If you don't want the color of the solid to show through as the background, untick "Preserve layer" and anything not drawn by the patch will become transparent.

You can apply all of your footage patches to the master solid by adding the effect multiple times, and keeping "Preserve layer" ticked on all but the first effect.

## Building

Building this plug-in relies on proprietary Adobe code that I cannot distribute under a free software license.

To make the plug-in build properly, download the [Adobe After Effects SDK](https://www.adobe.io/apis/creativecloud/aftereffects.html) and clone this repository into the `Templates` folder.

From there, the project can be built from the `InvEqui.sln` in the `Win` folder.

If you do not use CC 2018, or your After Effects is installed in a non-default location, you may need to open the `InvEqui` solution properties and edit the `General > Output directory` and `Debug > Command` entries to suit your install.

From there, building the solution should build and install the plug-in and launch After Effects for debugging.
