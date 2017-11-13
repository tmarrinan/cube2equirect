Cubemap to Equirectangular Converter
=======

Converts cubemap data sets to an equirectuangular projection image

##### Cubemap: #####
![Cubemap Image](https://drive.google.com/uc?id=0B3xEf6dg2km5dGNscGZ6c081VkE)

##### Equirectangular: #####
![Equirectangular Image](https://drive.google.com/uc?id=0B3xEf6dg2km5NDlBcGRnczRESzQ)

## Run ##

* `./cube2equirect [options]`
    * options:
        * `-i, --input <DIRECTORY>` directory with cubemap image set sequence
        * `-o, --output <DIRECTORY>` directory to save equirectangular images [Default: 'output/']
        * `-h, --h-resolution <NUMBER>` horizontal resolution of output images [Default: 3840]
        * `-f, --format <IMG_FORMAT>` output image format ('jpg', 'png', or 'mp4') [Default: same as input]
        * `-r, --framerate <NUMBER>` number of images per second (for video output) [Default: 24]
    * cubemap files should be named (JPEG and PNG are both valid):
        * 000000_left.jpg
        * 000000_right.jpg
        * 000000_bottom.jpg
        * 000000_top.jpg
        * 000000_back.jpg
        * 000000_front.jpg
    * if converting a sequence of images, follow above naming convention and increment the leading counter

## Install ##

### Mac OS X ###

* `brew install sdl2`
* `brew install sdl2_image`
* `brew install jpeg`
* `brew install libpng`
* `brew install ffmpeg` (optional - only needed if converting sequences of images to a video)

## Build ##

* `make`

