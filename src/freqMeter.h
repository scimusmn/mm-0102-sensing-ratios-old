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

extern ofColor white, black, blue, red;

class fMeter : public ofInterObj {
protected:
  ofFont title;
  float * dispNum;
  digitDisplay display;
  float pad;
  bool bLeftMet;
public:
  ~fMeter();
  void setup(float & dNum, int _w, bool isLeftMeter);
  
  void draw(int _x, int _y);
};

class titleBar {
protected:
  ofRectangle eng;
  ofRectangle esp;
  ofFont title;
public:
  void setup();
  void draw(string english, string espanol, int y);
};

#endif