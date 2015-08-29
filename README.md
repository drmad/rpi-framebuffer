# rpi-framebuffer
Linux framebuffer tests on the Raspberry Pi

For now, there's only a simple demoscene fire effect using the framebuffer. It should work on any Linux machine with framebuffer capable of 8 bits per pixel (for some reason, my desktop doesn't want to change from 32bpp :-P ), but I tested on a Raspberry Pi B+, because I want to create a low-overhead GUI for any appliance I'll build using with a new 7" touchscreen I just bought from China :)

Video here: https://www.youtube.com/watch?v=O2NAr2LjTTM

To test it, just clone the repo, and run:

```
cd rpi-framebuffer
make
./fire8
```

Post en español en mi blog: http://drmad.org/blog/jugando-con-el-framebuffer-de-linux-en-raspberry-pi.html
