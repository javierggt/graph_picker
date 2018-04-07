GraphPicker is (yet another) utility to save graphic plots as a list of points.
This is an ANCIENT project of mine, but I use it from time to time. If I were to do this now... I would most definitely nt use Qt.

There is no documentation apart from this file and I hope it is simple enough to use.

INSTALL:
--------

The installation should be simple:
  qmake
  make

and then copy the executable to wherever you want it.

USAGE:
------

- Start graphPicker. One can optionally give a filename as argument.
- Set the crosshair. The crosshair is made of a vertical and
  horizontal lines. In order to customize it, press the 'reset
  crosshair' button and define both lines by clicking and dragging
  between the start and end points while keeping the 'shift' button
  pressed.
- Calibrate. When the program starts it needs to be calibrated.
  To calibrate, double-click on three points in the image and
  give their coordinates.
- After the calibration is done, you can just add points by double-clicking.
- To add errors, drag from the point to the end of the error bar.
- Save.
