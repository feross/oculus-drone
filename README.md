Oculus Drone
============

![Oculus Drone project](https://raw.github.com/feross/oculus-drone/master/slide.png)

Control a quadcopter with an Oculus Rift. This is virtual reality piloting---you see through the drone's camera. When you turn, it turns; when you nod, it tilts. Kind of like flying a jetpack.

## How it works

The Oculus has a C++ SDK. We made a server in C++ that gets orientation readings from the Oculus and sends them out over TCP. 

The main command+control was a Node.js application. It connects to the Oculus server. There's a great Node.js SDK for the AR Drone 2.0 -- written by users, by the way, the OEM's SDK is in C++ and is really bad. 

The Node app does the following:
* Reads headset orientation from the Oculus server
* Runs a PID controller to make the drone orientation match that of the headset, with some modifications
* Pulls the video feed from the drone, converts it, and serves it as an HTTP stream
* Serves a tiny web page

The web page runs in full screen. (To clarify: the Oculus is plugged in both via USB, for orientation readings, and HDMI, for video.) The web page receives the video stream and uses WebGL to render two copies of the video, side by side, for the right and left eyes.

## Authors

- [Feross Aboukhadijeh](http://feross.org)
- John Hiesey
- [Daniel Posch](http://dcpos.ch/)
- Abi Raja

## Greylock Hackfest 2013

Originally written in 24 hours for the Greylock Hackfest, where we got 2nd place. Code is currently a mess, as expected. Will improve over the coming weeks.

## How to get this running

This project is still very much hackathon quality code, so it's a bit tricky to get it running. We never had a chance to go back and fix it, so these instructions will have to suffice.

There's an Xcode project in there somewhere that you need to build and run first. It gathers data points from the oculus headset and sends them over a TCP socket to anyone who connects. Then you run the node server which connects to the first server and uses the data to control the drone over wifi. You need to already be connected to the drone's wifi hotspot when you run the node server. Then you visit http://localhost:3000, I believe, and you get the live video feed there. Fullscreen your web browser to fill up the Oculus's display and you're good to go.

Use up/down (or maybe W/S) keys to control elevation since the oculus only has six axes. Once you're at a good elevation, then use the oculus for all control. Tilt extra far in one direction to do a flip in that direction.

It works best in huge, wide open spaces.

## Contributors

**Contributions welcome!**


## The MIT License

Copyright (c) 2013

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
