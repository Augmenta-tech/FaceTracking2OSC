#include "ofApp.h"
#include "ofAppBaseWindow.h"


//--------------------------------------------------------------
void ofApp::setup(){
	// Setup face finder
    finder.setup("haarcascade_frontalface_alt2.xml");
	
    // Setup camera
	CAM_WIDTH=  640;
	CAM_HEIGHT= 480;
    
    // Get camera's resolution
	vector< ofVideoDevice > devicesList = cam.listDevices();
	if (devicesList.size()!=0){
		vector< ofVideoFormat > formatsList = devicesList[0].formats;
		if(formatsList.size()!=0){	
			CAM_WIDTH=  formatsList[0].width;
			CAM_HEIGHT= formatsList[0].height;
			cam.initGrabber(formatsList[0].width,formatsList[0].height);
		}
		else{
            cam.initGrabber(CAM_WIDTH,CAM_HEIGHT);
		}
	}
	else{
		cam.initGrabber(CAM_WIDTH,CAM_HEIGHT);
	}
    // Adapt window size to camera resolution
    ofSetWindowShape(CAM_WIDTH, CAM_HEIGHT);
	imgTransform.allocate(CAM_WIDTH,CAM_HEIGHT);
    
    // Setup GUI with default parameters
    gui.setup();
    gui.add(uiFramerate.setup("FPS", framerate));
    gui.add(uiHost.setup("oscHost", "127.0.0.1"));
    gui.add(uiPort.setup("oscPort", ofToString(12000)));
    gui.add(scaleFactor.setup("scaleFactor", 4, 1, 8));
    gui.add(smoothPos.setup("smoothPos Exp", 0.2, 0, 1));
    gui.add(thresholdPos.setup("thresholdPos", 0.2, 0, 0.1));
    gui.add(smoothBound.setup("smoothBound Exp", 0.2, 0, 1));
    gui.add(thresholdBound.setup("thresholdBound", 0.2, 0, 0.1));
    gui.add(finderMinWidth.setup("finderMinWidth", 0, 0, 200));
    gui.add(finderMinHeight.setup("finderMinHeight", 0, 0, 200));
    gui.add(timeout.setup("timeout", 0, 0, 60));
	gui.add(brightness.setup("brightness",50,0,150));
	gui.add(contrast.setup("contrast",50,0,100));
	gui.add(smoothAverage.setup("smoothAverage",10,1,30));

	//Setup OldsfacesVector
	faces.assign(30,ofRectangle(1/2,1/2,0,0));

    // Load autosave (replace default parameters if file exists)
    if(ofFile::doesFileExist("autosave.xml")){
        gui.loadFromFile("autosave.xml");
    }
    
    // Setup OSC sender with parameters defined above in GUI
    host = uiHost.getParameter().toString();
    port = ofToInt(uiPort.getParameter().toString());
    sender.setup(host, port);
    
    // Setup other variables
    pid = 0;
    age = 0;
    face.set(0, 0, 0, 0);
    oldFace.set(0, 0, 0, 0);
	centroid = ofVec3f(CAM_WIDTH/2,CAM_HEIGHT/2,0);
    trackingState = EMPTY;
}

//--------------------------------------------------------------
void ofApp::update(){
    // Update framerate for GUI
    framerate = ofToString(ofGetFrameRate());
    
    // Update camera
    cam.update();
	if(cam.isFrameNew()) {
        // Get camera image
        ofImage img;	
   		img.setFromPixels(cam.getPixelsRef());
		
		// Use resized image for better performance
        img.resize(CAM_WIDTH/scaleFactor, CAM_HEIGHT/scaleFactor);
		
		// Use of GrayScale image in order to transform more efficiently
		img.setImageType(OF_IMAGE_GRAYSCALE);
		imgTransform.setFromPixels(img.getPixelsRef());
		imgTransform.brightnessContrast(static_cast<float>(brightness)/100,static_cast<float>(contrast)/100);
		imgTransform.contrastStretch();

        // Find face
        finder.findHaarObjects(imgTransform, finderMinWidth/scaleFactor, finderMinHeight/scaleFactor);

		// Update face position
        // Augmenta-like behavior : get state to send in OSC
        // Person entered
        if(blobsNum == 0 && finder.blobs.size() != 0){
            trackingState = PERSON_ENTERED;
            pid++;
            age = 0;
        }
        // Person updated
        else if(blobsNum != 0 && finder.blobs.size() != 0){
            trackingState = PERSON_UPDATED;
            age++;
            // Save the last frame person has been updated for timeout function
            lastUpdated = ofGetFrameNum();
        }
        // Person left
        else if(blobsNum != 0 && finder.blobs.size() == 0){
            // Timeout before person is considered to have really left
            if(timeout == 0){
                trackingState = PERSON_LEFT;
            } else {
                trackingState = PERSON_UPDATED;
            }
        }
        // Timeout
        else if(blobsNum == 0 && finder.blobs.size() == 0){
            // Timeout before person is considered to have really left
            if(ofGetFrameNum()-lastUpdated < timeout){
                trackingState = PERSON_UPDATED;
            } else if(ofGetFrameNum()-lastUpdated == timeout){
                trackingState = PERSON_LEFT;
            } else {
                trackingState = EMPTY;
            }
        }
        
        // Update number of blobs
        blobsNum = finder.blobs.size();
        
        // Update face position
        if(finder.blobs.size() != 0){
            oldFace = face;
            ofRectangle newFace = finder.blobs[0].boundingRect;
            
            // Face data are set between 0 and 1
            newFace.set((float)newFace.getTopLeft().x / ((float)CAM_WIDTH / (float)scaleFactor),
                        (float)newFace.getTopLeft().y / ((float)CAM_HEIGHT / (float)scaleFactor),
                        (float)newFace.getWidth() / ((float)CAM_WIDTH / (float)scaleFactor),
                        (float)newFace.getHeight() / ((float)CAM_HEIGHT / (float)scaleFactor));
            
            float x = oldFace.getTopLeft().x;
            float y = oldFace.getTopLeft().y;
            float width = oldFace.getWidth();
            float height = oldFace.getHeight();
            
            // Threshold ignore new position if it has not changed enough
			if(abs(width-newFace.getWidth())    > thresholdBound) width = newFace.getWidth();
            if(abs(height-newFace.getHeight())  > thresholdBound) height = newFace.getHeight();
			if(abs(x-newFace.getTopLeft().x)    > thresholdPos) x = newFace.getTopLeft().x;
			if(abs(y-newFace.getTopLeft().y)    > thresholdPos) y = newFace.getTopLeft().y;


            // Exponential smooth on new position
            face.set(smoothPos * oldFace.getTopLeft().x + (1-smoothPos) * x,
                     smoothPos * oldFace.getTopLeft().y + (1-smoothPos) * y,
                     smoothBound * oldFace.getWidth() + (1-smoothBound) * width,
                     smoothBound * oldFace.getHeight() + (1-smoothBound) * height);

			// Updating oldsFacesVector
			faces.insert( faces.begin(),face);
			faces.pop_back();
			centroid =	SmoothFaceFromAVerage();
        }
		
	}
    
    // Send face position to osc
    sendDataToOSC();
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofNoFill();
    
	//Draw grayScale image once transformed
	imgTransform.draw(0, 0, CAM_WIDTH, CAM_HEIGHT);
    	
    // Draw area around face if detected
    if(trackingState == PERSON_ENTERED || trackingState == PERSON_UPDATED){
        ofRect(face.getTopLeft().x * CAM_WIDTH,
               face.getTopLeft().y * CAM_HEIGHT,
               face.getWidth() * CAM_WIDTH,
               face.getHeight() * CAM_HEIGHT);
        
		// Draw olds centroids
		drawOldsCentroids();

        // Draw centroid
        drawCentroid();
    }

    // Draw GUI
    uiFramerate.setup("FPS", framerate);
    gui.draw();
}

//--------------------------------------------------------------
void ofApp::exit(){
	//Save of the parameters
	gui.saveToFile("autosave.xml");
}

//--------------------------------------------------------------
void ofApp::sendDataToOSC(){
	ofxOscMessage m;
    
    // Send face-tracker data
    m.setAddress("/head");
    m.addFloatArg(face.getCenter().x);
    m.addFloatArg(face.getCenter().y);
    m.addFloatArg(face.getWidth());
    m.addFloatArg(face.getHeight());
    sender.sendMessage(m);
    
    // Send Augmenta-simulated data
    m.clear();
    
    switch(trackingState){
        case PERSON_ENTERED:
            m.setAddress("/au/personEntered");
            break;
            
        case PERSON_UPDATED:
            m.setAddress("/au/personUpdated");
            break;
            
        case PERSON_LEFT:
            m.setAddress("/au/personWillLeave");
            break;
            
        default:
            break;
    }
    if(trackingState != EMPTY){
        m.addIntArg(pid);       // pid
        m.addIntArg(0);         // oid
        m.addIntArg(age);       // age
        m.addFloatArg(face.getCenter().x);                              // centroid.x
        m.addFloatArg(1-face.getHeight());                              // centroid.y
        m.addFloatArg(face.getCenter().x - oldFace.getCenter().x);      // velocity.x
        m.addFloatArg((1-face.getHeight()) - (1-oldFace.getHeight()));  // velocity.y
        m.addFloatArg(0.5f);                                            // depth
        m.addFloatArg(face.getTopLeft().x);                             // boundingRect.x
        m.addFloatArg((1-face.getHeight())-face.getHeight()/2);         // boundingRect.y
        m.addFloatArg(face.getWidth());                                 // boundingRect.width
        m.addFloatArg(face.getHeight());                                // boundingRect.height
        m.addFloatArg(face.getCenter().x);                              // highest.x
        m.addFloatArg(1-face.getHeight());                              // highest.y
        m.addFloatArg(face.getCenter().y);                              // highest.z
        sender.sendMessage(m);
    }
}

//--------------------------------------------------------------
void ofApp::drawCentroid(){
	ofPushStyle();
    ofSetColor(255,0,0);
	ofFill();
    ofCircle(centroid.x, centroid.y, 10);
	ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::drawOldsCentroids(){
	for(int i = smoothAverage-1; i > 0; --i){
		ofPushStyle();
		ofSetColor(25*i,25*i,255);
		ofFill();
		ofCircle(faces[i].getCenter().x * CAM_WIDTH, faces[i].getCenter().y * CAM_HEIGHT, 10);
		ofPopStyle();
	}
}

//--------------------------------------------------------------
//Returning the position of the centroid depending of the olds positions using a weighted average
//the more the position is old the more is coefficient is weak, using indice as coefficient
//--------------------------------------------------------------
ofVec3f ofApp::SmoothFaceFromAVerage(){
	ofVec3f temporaryFace= ofVec3f(0,0,0);
	float divide = 0;
	for(int i = 0 ; i < smoothAverage ; i++){
		temporaryFace.x = temporaryFace.x + faces[i].getCenter().x * (static_cast<float>(smoothAverage - i) / smoothAverage) * CAM_WIDTH;
		temporaryFace.y = temporaryFace.y + faces[i].getCenter().y * (static_cast<float>(smoothAverage - i) / smoothAverage) * CAM_HEIGHT;
		temporaryFace.z = temporaryFace.z + ((face.getHeight() * CAM_HEIGHT + face.getWidth() * CAM_WIDTH)/2) *
											((static_cast<float>(smoothAverage - i) / smoothAverage));
		divide = divide + (static_cast<float>(smoothAverage - i) / smoothAverage);
	}
	temporaryFace.x = temporaryFace.x / divide ;
	temporaryFace.y = temporaryFace.y / divide ;
	temporaryFace.z = temporaryFace.z / divide ;

	return temporaryFace;
}