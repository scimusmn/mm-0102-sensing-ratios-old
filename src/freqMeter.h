/*
 *  freqMeter.h
 *  audioOutputExample
 *
 *  Created by Exhibits on 1/26/2554.
 *  Copyright 2011 Science Museum of Minnesota. All rights reserved.
 *
 */

#ifndef _freqMeter
#define _freqMeter

#include "ofMain.h"
#include "ofExtended.h"

extern ofColor white, black;

class freqMeter : public ofInterObj {
public:
	//vector<numberTemplate> numTemp;
  digitDisplay display;
	ofTrueTypeFont arialLabel;
	ofTrueTypeFont arialHeader;
	int num;
	float s,r,g,b;
	freqMeter():ofInterObj(){}
	void setup(int _x, int _y,double _w, double numDigs){
		arialLabel.loadFont("arial.ttf", 14);
		arialHeader.loadFont("arial.ttf", 20);
		num=numDigs;
		s=(_w)/(numDigs*40);
		cSetup(_x, _y, _w, 60*s);
		r=32,g=176,b=255;
		/*for (int i=0; i<numDigs; i++) {
			numTemp.push_back(numberTemplate(x+40*s*i,y,s,.25,.6,.9));
		}*/
    x=_x,y=_y; 
    display.setup(_w,numDigs);
    cSetup(_x, _y, _w, display.h);
	}
	~freqMeter(){
	}
	void draw(string time,int _x, int _y){
    x=_x, y=_y;
		ofSetShadowDarkness( .75), ofShadowRounded(x-20, y-40, w+55, h+60, 20);
    ofRaised(.2);
    ofSetColor(white*.7);
		ofRoundedRect(x-20, y-40, w+55, h+60, 10);
    ofRaised(.2);
    ofSetColor(white*.1);
		ofRoundedRect(x-10, y-10, w+35, h+20, 10);
    ofSetColor(255*.25,255*.6,255*.9);
		display.draw(time, x, y);
		arialLabel.drawString("hz",display.x+display.w+10,display.y+display.h);
		ofSetColor(0, 0, 0);
		arialHeader.drawString("Frequency",x, y-20);
	}
};

#endif