#include "testApp.h"

ofColor white(255,255,255);
ofColor black(0,0,0);

void fret(double x, double y, double w, double h,double divs, double pos, bool up)
{
	ofShadowRounded(x-4, y-4, w+8, h+8, 10, 5);
  ofSetColor(white*.7);
	ofRoundedRect(x-4, y-4, w+8, h+8, 10);
  ofSetColor(white*.2);
	ofRoundedRect(x, y, w, h, 10);
	for (int i=1; i<divs; i++) {
    ofSetColor(white*.5);
		if(!up) ofRoundedRect(x+w/divs*i, y, 4, h,2);
		else ofRoundedRect(x, y+h/divs*i, w, 4, 2);
	}
	ofSetColor(255, 64, 64);
	if(!up) ofCircle(pos, y+h/2, 15);
	else ofCircle(x+w/2,pos, 15);
}

void frets(double x, double y, double s, double divs, double xpos, double ypos)
{
	fret(x, y+s+40, s, 60, divs, xpos, false);
	fret(x-100, y, 60, s, divs, ypos, true);
	ofSetColor(255, 0, 0,64);
	ofLine(xpos, y+s+50, xpos, ypos);
	ofLine(x-50, ypos, xpos, ypos);
}
	

//--------------------------------------------------------------
void testApp::setup(){
	
	calib=false;
	ofSetDataPathRoot("../Resources/");
	
	maxVal=861;
	minVal=0;
	setMin=100;
	maxValY=861;
	minValY=0;
	setMinY=100;
  
  string serialPort="";
	
	ifstream config(ofToDataPath("config.cfg").c_str());
	while (config.peek()!=EOF) {
		string nextLine;
		getline(config, nextLine);
		vector<string> token=ofSplitString(nextLine, "=");
		if(token.size()){
			if(!token[0].compare("MAX_X_VAL")){
				maxVal = ofToInt(token[1]);
			}
			else if(!token[0].compare("MIN_X_VAL")){
				minVal=ofToInt(token[1]);
			}
			else if(!token[0].compare("MIN_Y_VAL")){
				minValY=ofToInt(token[1]);
			}
			else if(!token[0].compare("MAX_Y_VAL")){
				maxValY=ofToInt(token[1]);
			}
			else if(!token[0].compare("SET_X_VAL")){
				setMin=ofToInt(token[1]);
			}
			else if(!token[0].compare("SET_Y_VAL")){
				setMinY=ofToInt(token[1]);
			}
      else if(!token[0].compare("SERIAL_PORT")){
				serialPort=token[1];
			}
		}
		else {
			maxVal=861;
			minVal=0;
			setMin=100;
			maxValY=861;
			minValY=0;
			setMinY=100;
		}
		
	}
	config.close();
	
	/*maxValY-=setMinY;
	minValY-=setMinY;
	
	maxVal-=setMin;
	minVal-=setMin;*/
	octave=(maxVal-minVal)/numOctaves;
	
	MODE=0;
	complete=false;
	dispLines=true;
	yOffset=200;
	numOctaves=8;
	newTimer=0;
	
	background.loadImage("background.jpg");
	speaker.loadImage("speaker.png");
	
	overlayCnt=0;
	
	height=ofGetHeight();
	width=ofGetWidth();
	sqWid=(height-2*yOffset);
	
	pixOctave=sqWid/numOctaves;
	
	net.update(numOctaves, sqWid,sqWid, (width-sqWid)/2, yOffset);
	
	ofHideCursor();
	ofBackground(0,0,0);

	xpos=net.vertex(0, 0).x;
	ypos=net.vertex(0, 0).y;
	
	//---------------- Sinewave synth -------------
	
	// 2 output channels,
	// 0 input channels
	// 22050 samples per second
	// 256 samples per buffer
	// 4 num buffers (latency)

	sampleRate 			= 44100;
	phase				= 0;
	phaseAdder			= 0.0f;
	phaseAdderTarget 	= 0.0f;
	phase2				= 0;
	phaseAdder2			= 0.0f;
	phaseAdderTarget2 	= 0.0f;
	volume				= 0.1f;
	bNoise 				= false;
	lAudio = new float[256];
	rAudio = new float[256];
	ofSoundStreamSetup(2,0,this, sampleRate,256, 4);

	ofSetFrameRate(60);
	
	//--------------------------------------------
	
	serial.setup(serialPort,115200);
	
	arialHeader.loadFont("arial.ttf", 40);
	arialLabel.loadFont("arial.ttf", 20);
	
	xMeter.setup((width-sqWid)/2-530, height/2, 150, 4);
	yMeter.setup((width+sqWid)/2+300, height/2, 150, 4);
}


//--------------------------------------------------------------
void testApp::update(){
	
	
	
	serial.writeByte('a');
	
	//int nRead  = 0;
	//bool full = true;
	
	while (serial.available()) {
		byteRead=serial.readByte();
		switch (MODE) {
			case POSDATA:
			case VOLDATA:
				data[3-waitForData--]=byteRead;
				break;
      case BUTDATA:
        data[1-waitForData--]=byteRead;
        break;

			case 0:
				//cout << " Changing mode to "<< byteRead <<endl;
				if (byteRead==POSDATA||byteRead==VOLDATA) {
					MODE=byteRead;
					waitForData=3;
				}
        if (byteRead==BUTDATA) {
					MODE=byteRead;
					waitForData=1;
				}
				break;
			default:
				break;
		}
		if(waitForData==0) {
			switch (data[0]) {
				case XPOS:{
					float curPos=xpos=data[1]+(data[2]<<8)-setMin;
					int base=(xpos>octave) ? 50 : 50.*xpos/octave;
					targetFrequency = base*pow(2., (curPos/octave))/2;
					targetFrequency = ofClamp(targetFrequency,0,6400);
					phaseAdderTarget = (targetFrequency / (float) sampleRate) * TWO_PI;
				}
					break;
				case YPOS:{
					float curPos=ypos=data[1]+(data[2]<<8)-setMinY;
					int base=(ypos>octave) ? 50 : 50.*ypos/octave;
					targetFrequency2 = base*pow(2., (curPos/octave))/2;
					targetFrequency2=ofClamp(targetFrequency2,0,6400);
					phaseAdderTarget2 = (targetFrequency2 / (float) sampleRate) * TWO_PI;
				}
					break;
				case VOL:{
					int curVol=data[1]+(data[2]<<8);
					volume=double(curVol)/1024.f;
					//cout << volume <<endl;
				}
					break;
        case RESET_BUT:
					newTimer=ofGetElapsedTimeMillis()+3000;
          overlayCnt=0;
					break;
        case ACT_BUT:
          if(++overlayCnt>=5) overlayCnt=0;
					break;

				default:
					break;
			}
			MODE=0;
			complete=false;
			data[0]=data[1]=data[2]=0;
		}

	}
}

static int primes[]={2 , 3 , 5 , 7 , 11 , 13 , 17 , 19 , 23 , 29 , 31 , 37 , 41 , 43 , 47};

void nearest_fraction(int * x, int * y, int size, int num)
{
	//int tempX=x, tempY=y;
	size/=num;
	*x+=size/2;
	*y+=size/2;
	*x/=size;
	*y/=size;
	for (int i=0; i<13; i++) {
		while (((*x)%primes[i]==0&&(*y)%primes[i]==0)&&*x&&*y) {
			(*x)/=primes[i];
			(*y)/=primes[i];
		}
	}
}
	
//--------------------------------------------------------------
void testApp::draw(){
	
	if(count++>360) count=0;;
	
  ofRectangle drawRect((ofGetWidth()-sqWid)/2,yOffset,sqWid,sqWid);
	
	ofSetColor(175,175,175);
	background.draw(0, 0,width,height);
	
	int xint=ofMap(xpos, minVal, maxVal, (width-sqWid)/2, (width+sqWid)/2,true);
	int yint=ofMap(maxValY-ypos, minValY, maxValY, yOffset, yOffset+sqWid,true);
	
	ofSetShadowDarkness( .5), ofShadowRounded((width-sqWid)/2-5, yOffset, sqWid+5, sqWid+5, 20);
	ofSetColor(255, 255, 255);
	ofRect((width-sqWid)/2-5, yOffset, sqWid+5, sqWid+5);
	
	net.draw();
	
	ofSetColor(255, 255, 255);
	for (int i=0; i<numOctaves+1; i++) {
		ofPoint k=net.vertex(0, i);
		char num[32];
		sprintf(num, "%i",i);
		arialLabel.drawString(num, k.x-30, k.y+arialLabel.stringHeight(num)/2);
	}
	for (int i=1; i<numOctaves+1; i++) {
		ofPoint k=net.vertex(i, 0);
		char num[32];
		sprintf(num, "%i",i);
		arialLabel.drawString(num, k.x-arialLabel.stringWidth(num)/2, k.y+arialLabel.stringHeight(num)+7);
	}
	
	/************************** Reporting data ********************
	ofSetColor(0xFFFFFF);
	char reportString[255];
	sprintf(reportString, "nread = %i", nBytesRead);
	if (!bNoise) sprintf(reportString, "%s (%fhz)", reportString, targetFrequency);

	ofDrawBitmapString(reportString,80,380);
	
	//arialHeader.drawString(reportString,20, height-50);
	 
	**************************************************************/
	
	overlays(xint,yint);
  
	
	char freq[32];
	sprintf(freq, "%04i",int(targetFrequency2));
	xMeter.draw(freq,((width-sqWid)/2-xMeter.w)/2,ofGetHeight()/2);
	sprintf(freq, "%04i",int(targetFrequency));
	yMeter.draw(freq,(width+sqWid)/2+((ofGetWidth()-(width+sqWid)/2)-yMeter.w)/2,ofGetHeight()/2);
	
	
  ofRaised(.4);
  ofSetColor(white*.9);
	ofCircle_Plus(drawRect.x/2, height/2+200, 100);
	speaker.draw(drawRect.x/2+75, height/2+125,-150,150);
	ofRaised(.4);
  ofSetColor(white*.9);
	ofCircle_Plus((drawRect.x+drawRect.width+ofGetWidth())/2, height/2+200, 100);
	speaker.draw((drawRect.x+drawRect.width+ofGetWidth())/2-75, height/2+125,150,150);
	
	ofSetLineWidth(6);
	ofEnableSmoothing();
	for (unsigned int i=1; i<lines.size(); i++) {
		ofSetColor(65, 134, 200,255-i);
		ofLine(lines[i-1].x, lines[i-1].y, lines[i].x, lines[i].y);
	}
	ofDisableSmoothing();
	
	frets((width-sqWid)/2, yOffset, sqWid, numOctaves, xint, yint);
	ofSetColor(65, 134, 200);
	ofCircle(xint, yint, 7);
	
	lines.push_front(ofPoint(xint,yint));
	if (lines.size()>255) {
		lines.pop_back();
	}
	
	if (overlayCnt==4) {
		reducedRatios(xint, yint);
	}
	
	if(newTimer>ofGetElapsedTimeMillis()){
		int mW=ofGetWidth();
		int mH=ofGetHeight();
		string rep="Trace the lines using the sliders.\nPress the New Activities button to access\nmore screens.";
		ofRectangle s=arialLabel.getStringBoundingBox(rep, 0, 0);
		ofSetColor(64, 64,64);
		ofRoundedRect((mW-(s.width+40))/2, (mH-(s.height+40))/2, s.width+40, 40+s.height, 20);
		ofSetColor(255, 255, 255);
		arialLabel.drawString(rep, (mW-s.width)/2, (mH-s.height)/2+20);
	}
	
	if (calib) {
		ofSetColor(0,0,0,128);
		ofRect(0,0,ofGetWidth(),ofGetHeight());
		ofSetColor(128, 128, 128);
		ofRoundedRect(150, 100, ofGetWidth()-300, ofGetHeight()-200, 20);
		
		char freq[32];
		sprintf(freq, "%f",ypos+setMinY);
		
		char reportString[255];
		sprintf(reportString, "<-- yslider  xslider -->");
		int w= arialHeader.stringWidth(reportString);
		int h= arialHeader.stringHeight(reportString);
		ofSetColor(255, 255, 255);
		arialHeader.drawString(reportString,(width-w)/2, ofGetHeight()/4*2);
		
		arialHeader.drawString(freq,200, ofGetHeight()/4*2);
		sprintf(freq, "%f",xpos+setMin);
		
		arialHeader.drawString(freq,ofGetWidth()-(200+arialHeader.stringWidth(freq)), ofGetHeight()/4*2);
		
		
		sprintf(freq, "%f",ypos);
		
		sprintf(reportString, "<-- yoff  from calibrated values  xoff -->");
		w= arialHeader.stringWidth(reportString);
		h= arialHeader.stringHeight(reportString);
		ofSetColor(255, 255, 255);
		arialHeader.drawString(reportString,(width-w)/2, ofGetHeight()/4*3);
		
		arialHeader.drawString(freq,200, ofGetHeight()/4*3);
		sprintf(freq, "%f",xpos);
		
		arialHeader.drawString(freq,ofGetWidth()-(200+arialHeader.stringWidth(freq)), ofGetHeight()/4*3);
		
		if(maxVal+setMin<1024&&maxValY+setMinY<1024){
			sprintf(reportString, "To re-calibrate, pull back sliders and press 'r'.");
			w= arialHeader.stringWidth(reportString);
			h= arialHeader.stringHeight(reportString);
			ofSetColor(255, 255, 255);
			arialHeader.drawString(reportString,(width-w)/2, ofGetHeight()/4);
		}
		else {
			sprintf(reportString, "Out of calibration range; tune with screwdriver.");
			w= arialHeader.stringWidth(reportString);
			h= arialHeader.stringHeight(reportString);
			ofSetColor(255, 255, 255);
			arialHeader.drawString(reportString,(width-w)/2, ofGetHeight()/4);
		}

	}
}

void testApp::overlays(int xint, int yint)
{
	if(overlayCnt==0){
		char reportString[255];
		sprintf(reportString, "Try to Trace the Lines!");
		int w= arialHeader.stringWidth(reportString);
		int h= arialHeader.stringHeight(reportString);
		ofRectangle box=arialHeader.getStringBoundingBox(reportString, (width-w)/2, 75);
    ofSetColor(.6*255);
    ofRaised(.2);
		ofRoundedRect(box.x-5, box.y-5, box.width+10, box.height+10, 10);
    ofSetColor(.2*255);
		ofRoundedRect(box.x-3, box.y-3, box.width+6, box.height+6, 10);
		ofSetColor(255, 255, 255);
		arialHeader.drawString(reportString,(width-w)/2, 75);
		ofSetLineWidth(3);
		ofSetColor(255, 64, 64);
		for (int i=0; i< net.numdivs; i++) {
			ofLine(net.vertex(i, i),net.vertex(i+1, i));
			ofLine(net.vertex(i+1, i),net.vertex(i+1, i+1));
		}
		
		ofSetColor(128, 64, 64);
		ofPoint disp=net.vertex(numOctaves/2, numOctaves/2);
		string report="Unison";
		ofRectangle g=arialLabel.getStringBoundingBox(report, 0, 0);
		arialLabel.drawString(report, disp.x-g.width-50,disp.y-100);
		
		ofSetLineWidth(1);
		ofEnableSmoothing();
		ofLine(disp.x-48, disp.y-98, disp.x, disp.y);
		ofDisableSmoothing();
		
		
		ofSetColor(128, 64, 64);
		disp=net.vertex(numOctaves/2, numOctaves/2-1);
		report="Octave Difference";
		g=arialLabel.getStringBoundingBox(report, 0, 0);
		arialLabel.drawString(report, disp.x+50,disp.y+g.height+50);
		
		ofSetLineWidth(1);
		ofEnableSmoothing();
		ofLine(disp.x+50, disp.y+50, disp.x, disp.y);
		ofDisableSmoothing();
		//reducedRatios(xint, yint);
	}
	if(overlayCnt==1){
		char reportString[255];
		sprintf(reportString, "Constant Ratios");
		int w= arialHeader.stringWidth(reportString);
		int h= arialHeader.stringHeight(reportString);
		ofRectangle box=arialHeader.getStringBoundingBox(reportString, (width-w)/2, 75);
    ofSetColor(.6*255);
    ofRaised(.2);
		ofRoundedRect(box.x-5, box.y-5, box.width+10, box.height+10, 10);
    ofSetColor(.2*255);
		ofRoundedRect(box.x-3, box.y-3, box.width+6, box.height+6, 10);
		ofSetColor(255, 255, 255);
		arialHeader.drawString(reportString,(width-w)/2, 75);
		ofSetLineWidth(3);
		ofSetColor(255, 64, 64);
		ofLine(net.vertex(0,0),net.vertex(numOctaves/2, numOctaves));
		ofLine(net.vertex(0,0),net.vertex(numOctaves, numOctaves/2));
		//arialLabel.drawString("2:1", net.vertex(2,4).x, net.vertex(2,4).y);
		ofPushMatrix();
		ofTranslate(net.vertex(2,4).x-10, net.vertex(2,4).y,0);
		ofRotate(-63, 0,0,1);
		ofSetColor(178, 64, 64);
		arialLabel.drawString("2:1 ratio", 0,0);
		ofPopMatrix();
		ofPushMatrix();
		ofTranslate(net.vertex(4,2).x-15, net.vertex(4,2).y,0);
		ofRotate(-27, 0,0,1);
		ofSetColor(178, 64, 64);
		arialLabel.drawString("1:2 ratio", 0,0);
		ofPopMatrix();
	}
	
	if(overlayCnt==2){
		char reportString[255];
		sprintf(reportString, "Constant Differences");
		int w= arialHeader.stringWidth(reportString);
		int h= arialHeader.stringHeight(reportString);
		ofRectangle box=arialHeader.getStringBoundingBox(reportString, (width-w)/2, 75);
    ofSetColor(.6*255);
    ofRaised(.2);
		ofRoundedRect(box.x-5, box.y-5, box.width+10, box.height+10, 10);
    ofSetColor(.2*255);
		ofRoundedRect(box.x-3, box.y-3, box.width+6, box.height+6, 10);
		ofSetColor(255, 255, 255);
		arialHeader.drawString(reportString,(width-w)/2, 75);
		ofSetLineWidth(3);
		ofSetColor(255, 64, 64);
		ofLine(net.vertex(0,0),net.vertex(numOctaves, numOctaves));
		ofLine(net.vertex(0,1),net.vertex(numOctaves-1, numOctaves));
		//arialLabel.drawString("2:1", net.vertex(2,4).x, net.vertex(2,4).y);
		ofPushMatrix();
		ofTranslate(net.vertex(3,4).x-10, net.vertex(3,4).y,0);
		ofRotate(-45, 0,0,1);
		ofSetColor(178, 64, 64);
		arialLabel.drawString("Octave Difference", 0,0);
		ofPopMatrix();
		ofPushMatrix();
		ofTranslate(net.vertex(3,3).x-15, net.vertex(3,3).y,0);
		ofRotate(-45, 0,0,1);
		ofSetColor(178, 64, 64);
		arialLabel.drawString("Unison", 0,0);
		ofPopMatrix();
	}
	
	if(overlayCnt==3){
		char reportString[255];
		sprintf(reportString, "Can you trace the circle?");
		int w= arialHeader.stringWidth(reportString);
		int h= arialHeader.stringHeight(reportString);
		ofRectangle box=arialHeader.getStringBoundingBox(reportString, (width-w)/2, 75);
    ofSetColor(.6*255);
    ofRaised(.2);
		ofRoundedRect(box.x-5, box.y-5, box.width+10, box.height+10, 10);
    ofSetColor(.2*255);
		ofRoundedRect(box.x-3, box.y-3, box.width+6, box.height+6, 10);
		ofSetColor(255, 255, 255);
		arialHeader.drawString(reportString,(width-w)/2, 75);
		ofSetLineWidth(3);
		ofSetColor(255, 64, 64);
		ofNoFill();
		ofRect(net.vertex(1, 1),pixOctave*(numOctaves-2),-pixOctave*(numOctaves-2));
		ofSetCircleResolution(60);
		ofCircle(net.vertex(numOctaves/2, numOctaves/2), pixOctave*(numOctaves-2)/2);
		ofSetCircleResolution(20);
		ofFill();
	}
	if(overlayCnt==4){
		//reducedRatios(xint,yint);
		
		char reportString[255];
		sprintf(reportString, "Ratios");
		int w= arialHeader.stringWidth(reportString);
		int h= arialHeader.stringHeight(reportString);
		ofRectangle box=arialHeader.getStringBoundingBox(reportString, (width-w)/2, 75);
		ofSetColor(.6*255);
    ofRaised(.2);
		ofRoundedRect(box.x-5, box.y-5, box.width+10, box.height+10, 10);
    ofSetColor(.2*255);
		ofRoundedRect(box.x-3, box.y-3, box.width+6, box.height+6, 10);
		ofSetColor(255, 255, 255);
		arialHeader.drawString(reportString,(width-w)/2, 75);
		
		if(dispLines){
			int xm=xint-(width-sqWid)/2,ym=sqWid-(yint-yOffset);
			nearest_fraction(&xm, &ym,sqWid,numOctaves);
			
			for(int i=0; i<numOctaves+1; i++){
				ofPoint k=net.vertex(xm*i, ym*i);
				if(abs(xint-k.x)<20&&abs(yint-k.y)<20){
					double xp=xm,yp=ym;
					
					if(xp<8&&yp<8){
						if(xp>yp) yp*=(8/xp), xp=8;
						else if(yp>xp) xp*=(8/yp), yp=8;
						else if(yp=xp) xp=yp=8;
					}
					ofEnableSmoothing();
					ofSetColor(255, 64, 64);
					ofSetLineWidth(3);
					ofLine(net.vertex(0, 0),net.vertex(xp, yp));
					ofDisableSmoothing();
				}
			}
		}
	}
}

void testApp::reducedRatios(int xint, int yint){
	int xm=xint-(width-sqWid)/2,ym=sqWid-(yint-yOffset);
	nearest_fraction(&xm, &ym,sqWid,numOctaves);
	
	for(int i=0; i<numOctaves+1; i++){
		ofPoint k=net.vertex(xm*i, ym*i);
		if(abs(xint-k.x)<20&&abs(yint-k.y)<20){
			char report[255];
			sprintf(report, "%i:%i octave ratio",xm,ym);
			ofRectangle box=arialLabel.getStringBoundingBox(report, k.x+10, k.y-10);
			ofSetColor(.6*255);
      ofRaised(.2);
      ofRoundedRect(box.x-5, box.y-5, box.width+10, box.height+10, 10);
      ofSetColor(.2*255);
      ofRoundedRect(box.x-3, box.y-3, box.width+6, box.height+6, 10);
			ofSetColor(255,255,255);
			arialLabel.drawString(report, k.x+10,k.y-10);
			ofSetColor(255, 64, 64);
			ofSetLineWidth(3);
		}
	}
	
	/*double xp=xm,yp=ym;
	 
	 if(xp<8&&yp<8){
	 if(xp>yp) yp*=(8/xp), xp=8;
	 else if(yp>xp) xp*=(8/yp), yp=8;
	 else if(yp=xp) xp=yp=8;
	 }
	 ofLine(net.vertex(0, 0),net.vertex(xp, yp));*/
}

//--------------------------------------------------------------
void testApp::keyPressed  (int key){
	if (key == '-'){
		volume -= 0.05;
		volume = MAX(volume, 0);
	} else if (key == '+'){
		volume += 0.05;
		volume = MIN(volume, 1);
	}
	else if(key=='q'){
		serial.close();
		
	}
	else if(key=='n'){
		numOctaves--;
		net.update(numOctaves,sqWid,sqWid, (width-sqWid)/2, yOffset);
		octave=(maxVal-minVal)/numOctaves;
		pixOctave=sqWid/numOctaves;
	}
	else if(key=='m'){
		numOctaves++;
		net.update(numOctaves,sqWid,sqWid, (width-sqWid)/2, yOffset);
		octave=(maxVal-minVal)/numOctaves;
		pixOctave=sqWid/numOctaves;
	}
	else if(key=='l'){
		dispLines=!dispLines;
	}
	else if(key=='z'){
		newTimer=ofGetElapsedTimeMillis()+3000;
		overlayCnt=0;
	}
	else if(key==' '){
		if(++overlayCnt>=5) overlayCnt=0;
	}
	
	else if (key=='C'){
		calib=!calib;
	}
	
	else if (key=='r'&&calib){
		if(maxVal+setMin<1024&&maxValY+setMinY<1024){
			setMinY+=ypos;
			setMin+=xpos;
			ofstream config(ofToDataPath("config.cfg").c_str());
			config << "MAX_X_VAL=" << maxVal <<endl; 
			config << "MIN_X_VAL=" << minVal <<endl;
			config << "MIN_Y_VAL=" << minValY << endl;
			config << "MAX_Y_VAL=" << maxValY << endl;
			config << "SET_X_VAL=" << setMin << endl;
			config << "SET_Y_VAL=" << setMinY << endl;
			
			
			config.close();
		}
	}
}

//--------------------------------------------------------------
void testApp::keyReleased  (int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
	int width = ofGetWidth();
	pan = (float)x / (float)width;
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
	bNoise = true;
}


//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
	bNoise = false;
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){
	height=ofGetHeight();
	width=ofGetWidth();
	sqWid=(height-2*yOffset);
	net.update(numOctaves,sqWid,sqWid, (width-sqWid)/2, yOffset);
	octave=(maxVal-minVal)/numOctaves;
	pixOctave=sqWid/numOctaves;
}
//--------------------------------------------------------------
void testApp::audioRequested 	(float * output, int bufferSize, int nChannels){
	pan = 0.5f;
	float leftScale = 1 - pan;
	float rightScale = pan;

	// sin (n) seems to have trouble when n is very large, so we
	// keep phase in the range of 0-TWO_PI like this:
	while (phase > TWO_PI){
		phase -= TWO_PI;
	}
	while (phase2 > TWO_PI){
		phase2 -= TWO_PI;
	}

	if ( bNoise == true){
		// ---------------------- noise --------------
		for (int i = 0; i < bufferSize; i++){
			lAudio[i] = output[i*nChannels    ] = ofRandomf() * volume * leftScale;
			rAudio[i] = output[i*nChannels + 1] = ofRandomf() * volume * rightScale;
		}
	} else {
		
		phaseAdder = 0.95f * phaseAdder + 0.05f * phaseAdderTarget;
		phaseAdder2 = 0.95f * phaseAdder2 + 0.05f * phaseAdderTarget2;
		for (int i = 0; i < bufferSize; i++){
			phase += phaseAdder;
			phase2 += phaseAdder2;
			float sample = 0;
			for (double j=1; j<5; j++) {
				sample+=(sin(phase*j))/j;
			}
			double t=(2*xpos/maxVal);
			t=ofClamp(t, 0, 1);
			t=pow(t, .5);
			sample=(sample*(1-t)+sin(phase)*(t));
			float sample2 = 0;
			for (double j=1; j<5; j++) {
				sample2+=(sin(phase2*j))/j;
			}
			t=(2*ypos/maxVal);
			t=ofClamp(t, 0, 1);
			t=pow(t, .5);
			sample2=(sample2*(1-t)+sin(phase2)*t);
			//sample2=ofClamp(sample2*1.2, -1, 1);
			//sample=ofClamp(sample*1.2, -1, 1);
			//float sample2=(sample>.75)?1:(sample<-.75)?-1:0;
			lAudio[i] = output[i*nChannels    ] = sample2 * volume * leftScale;
			rAudio[i] = output[i*nChannels + 1] = sample * volume * rightScale;
		}
	}

}

void testApp::exit(){
	serial.close();
	//ofSoundStreamClose();
}

