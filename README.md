This utility is a MOSlet (i.e. intended to be placed in /mos/ and called directly via its name) that will load, parse, and then upload .BMP files as RGBA2222 buffers.

As a MOSlet it can be called from within BASIC using the OSCLI function.

Usage is as follows:

* `bmpb <filename.bmp>`: by default, parse, upload and then show the specified .bmp file in the centre of the screen.
* `-b X`: parse and upload the specified .bmp to buffer id X. This will not show the image (unless `-s` is added) and is the most likely use when called from within BASIC.
* `-i`: prints information about the .bmp file.
* `-s`: displays the uploaded .bmp file.
* `-x`: specifies the position on the X axis at which to display the uploaded bitmap (does nothing without `-s`)
* `-y`: specifies the position on the Y axis at which to display the uploaded bitmap (does nothing without `-s`)
* `-c`: specifies the number of columns for a tilesheet. If not provided and `-r` is, assumed to be 1.
* `-r`: specifies the number of rows for a tilesheet. If not provided and `-c` is, assumed to be 1. `r` x `c` tiles will be stored left-to-right and then top-to-bottom from the buffer specified (or 0).
