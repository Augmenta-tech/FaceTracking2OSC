# FaceTracking2OSC

Small app that sends facetracking data to OSC (standard OSC and Augmenta OSC)
The OSC port is define in ofApp.cpp (default = 12000) 

## Deploy

OSX 10.9
Windows 7 with Visual Studio 2012 Express
OF 0.8.4

### GUI

ScaleFactor:	resize of the image send to the HaarDetection.
smoothPos Exp:	increase the fluidity of the deplcament of the position(depending of the last position)
thresholdPos:	the position won't change if the distance between the old nd the new point is not greater than the thresHoldPos value  
smoothBoundExp:	same as smoothPosExp but with the rectangle instead of the position
thresholdBound:	same as thresholdBound but with the rectangle instead of the position
finderMinWidth:	allows you to set the minimum width of the face 
finderMinHeight:allows you to set the minimum height of the face 
timeout:	Time before person is considered to have really left
brightness:	set the brightness of the scene
contrast:	set the contrast of the scene
smoothAverage:	smooth the position thanks to olds positions, depending on how long this position is past. 

Addons :

- ofxGui
- ofxOpenCv
- ofxOsc
