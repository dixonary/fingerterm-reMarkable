# fingerterm

fingerterm is a terminal emulator designed for touch-based interaction,
specifically for (but not limited to) use on the Nokia N9 and Jolla's
Jolla device.

## Modifications

I have made some modifications to the source code to allow it to work better on the reMarkable.
These include:
* Disabling the visual bell in code
* Keeping the virtual keyboard on screen at all times (except when the menu is open)
* Reducing the text area to fit above the keyboard rather than overlapping, to reduce redraw
* Adjusting the font and UI sizes to fit more comfortably on the screen.

Note that Landscape mode doesn't seem to work - this was true when I downloaded the repo originally from https://github.com/reMarkable/fingerterm .
