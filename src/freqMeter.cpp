/*
 *  freqMeter.cpp
 *  SensingRatios
 *
 *  Created by Exhibits on 11/16/11.
 *  Copyright 2011 Science Museum of Minnesota. All rights reserved.
 *
 */


#include "freqMeter.h"

ofImage speaker;
bool imgLoaded=false;

fMeter::~fMeter(){
  if(imgLoaded) speaker.clear();
}

void fMeter::setup(float & dNum, int _w, bool isLeftMeter)
{
  dispNum=&dNum;
  w=_w;
  title.loadFont("Avenir-Medium.otf");
  title.setSize(21);
  title.setMode(OF_FONT_TOP);
  display.setup(3*_w/5, 4);
  
  cout << ofToDataPath("speaker.png")<< endl;
  if(!imgLoaded) speaker.loadImage("speaker.png"),imgLoaded=true,cout << "loaded\n";
  pad=5;
  h=title.stringHeight("Kjhg")*2+pad*4+display.h*1.1;
  bLeftMet=isLeftMeter;
}

void fMeter::draw(int _x, int _y)
{
  int strH=title.stringHeight("Frequency");
  x=_x, y=_y;
  ofSetShadowDarkness(.5);
  ofShadowRounded(x, y, w, h, pad, 10);
  ofRaised(.2);
  ofSetColor(white*.7);
  ofRoundedRect(x, y, w, h, pad);
  ofSetShadowDarkness(1);
  ofShadowRounded(display.x-display.w*.05, display.y-display.h*.05, display.w*1.1, display.h*1.1, display.h/16, 3);
  ofSetColor(black);
  ofRect(display.x-display.w*.05,display.y-display.h*.05,display.w*1.1, display.h*1.1);
  ofSetColor(blue);
  display.draw(ssprintf("%04i",int(*dispNum)),((!bLeftMet)?x+w/10:x+w-(display.w*1.1+w/10)), y+strH+pad+display.h/4);
  ofSetColor(black);
  title.drawString("Frequency", display.x-display.w*.05, y+pad);
  title.drawString("Frecuencia", display.x-display.w*.05, y+h-(pad+strH));
  ofSetColor(white);
  if(!bLeftMet) speaker.draw(x+9*w/10, y+(h-w/5)/2,-w/5,w/5);
  else speaker.draw(x+w/10, y+(h-w/5)/2,w/5,w/5);
  ofFlat();
}

void drawTitleBox(ofRectangle eng, string text, ofFont & title, ofColor color){
  ofSetShadowDarkness(.5);
  ofShadowRounded(eng.x, eng.y, eng.width, eng.height, eng.height/4, 5);
  ofSetColor(white);
  ofRoundedRect(eng.x, eng.y, eng.width, eng.height, eng.height/4);
  ofSetColor(black);
  ofNoFill();
  ofRoundedRect(eng.x, eng.y, eng.width, eng.height, eng.height/4);
  ofFill();
  ofSetColor(color);
  title.drawString(text, eng.x+eng.width/2, eng.y+eng.height/6);
}

void titleBar::setup(){
  title.loadFont("Avenir-Medium.otf");
  title.setSize(30);
  title.setMode(OF_FONT_TOP);
  title.setMode(OF_FONT_CENTER);
  eng.height=esp.height=title.stringHeight("Kjhg")*1.5;
  esp.width=eng.width=2*ofGetWidth()/7.;
}

void titleBar::draw(string english, string espanol, int y){
  eng.x=(ofGetWidth()/2-eng.width)/2;
  esp.x=(3*ofGetWidth()/2-esp.width)/2;
  eng.y=y;
  esp.y=y;
  
  drawTitleBox(eng,english,title,red);
  drawTitleBox(esp,espanol,title,blue);
}