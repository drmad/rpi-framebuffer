# rpi-framebuffer
Linux framebuffer tests on the Raspberry Pi

Two demoscene-ish effects using the framebuffer: a fire effect and a rotozoomer. It should work on any Linux machine with framebuffer capable of 8 bits per pixel (for some reason, my desktop doesn't want to change from 32bpp :-P ).

Works *ferpectly* on my Raspberry Pi B+. I wrote them because I wanted to create a low-overhead GUI for any appliance I'll build using with a new 7" touchscreen I bought from China. Now I'm just having some fun :-)

To test them, clone the repo and run `make` inside the directory. To run the programas, you need to add your user to the `video` group, or run it as root.

## Vídeos
Fire:
https://www.youtube.com/watch?v=O2NAr2LjTTM

Rotozoomer:
https://www.youtube.com/watch?v=liOFy33qi1M

## En español
Post en español en mi blog: http://drmad.org/blog/jugando-con-el-framebuffer-de-linux-en-raspberry-pi.html
