# QOI: Simple file format for fast & lossless images
https://qoiformat.org/

## Methods to reduce file size

### 1.Indexing
QOI uses a 64 pixel running array of previous pixels, if a pixel is already in the array we can store the pixel in 1 byte using index.
### 2.Running pixels
If the same color repeats for maximum of 62 pixels, we can store the pixels in 1 byte.
### 3.Small Difference
If the pixel is very close to previous pixel, we can store the pixel in 1 byte.
### 4.Bigger Difference
If the pixel is close to previous pixel, we can store the pixel in 2 bytes.
### 4.RGB & RGBA
If we can't use any other method, we can store the pixel in 4 or 5 bytes depending on alpha channel.

## Why it works?
Since the colors on an image tend to change smooth (especially in real life images) we have a lot of chances to actually use one of the methods above and store the pixels in a more compressed way. Surely one can create an image where qoi format takes more space than the original raw data but this is only a problem for hand-crafted sentetic images.