Cubemap to Equirectangular Converter
=======

Converts cubemap data sets to an equirectuangular projection image

##### Cubemap: #####
![Cubemap Image](https://lh3.googleusercontent.com/drive-viewer/AFGJ81oQZ9Qt74gGEUdpwO9a5igSgZGkoACXNFozawYTmGAIZgEvrgM2ulOAtP2nkyRDotsBsoLJfZxjq2rQcxS6Vn7QHkqGZA_vHyolAFWKkitU-OFV3u69k7dwaT0c=s1600)

##### Equirectangular: #####
![Equirectangular Image](https://lh3.googleusercontent.com/drive-viewer/AFGJ81oSG0hI7OJjEW9jgdEvxO7K3QS1wSN3ENKL9pdjLyZ66YP6ICfDuhxWqxfw_c5JC4y9VBXWrQey7PWvfyZDtYi2Ea35Nt4AbkGXAELWb-K0DiO8d6DuhY9IWYlz=s1600)

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

### Ubuntu ###

* `apt install mesa-common-dev`
* `apt install libegl-dev`
* `apt install ffmpeg` (optional - only needed if converting sequences of images to a video)

## Build ##

* `make`

