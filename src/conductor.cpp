/*
 *  conductor.cpp
 *  Miditron
 *
 *  Created by Exhibits on 4/1/2554.
 *  Copyright 2011 Science Museum of Minnesota. All rights reserved.
 *
 */

#include "conductor.h"

void midiSequencer::setup(double nMeasures, double secondsPerMeasure, double pixelsPerSec, double vSize)
{
	mark.w=mark.h=20;
	h=30;
	divsPerMeasure=3;
	w=vSize;
	label.loadFont("Arial.ttf");
	label.setSize(12);
	midiConductor::setup(nMeasures*secondsPerMeasure,pixelsPerSec);
	pps=pixelsPerSec;
	numMeasures=nMeasures;
	measureLength=secondsPerMeasure*pps;
	bar.setup(20, vSize, OF_HOR);
	bar.registerArea(vSize, playTime*pps);
	playBut.setup(64, 64, "images/play.png","images/pause.png");
	rewindBut.setup( 48, 48, "images/rewind.png");
	loopBut.setup(48, 48, "images/repeat.png");
	display.setup(200, 6);
}

void midiSequencer::setTimeSignature(int beatsPerMeasure)
{
  divsPerMeasure=beatsPerMeasure;
}

void midiSequencer::setTempo(double secondsPerMeasure)
{
  
}

void midiSequencer::registerPlayback(bandBar * bnd)
{
	band=bnd;
	bar.x=band->w;
	setScrollPos(0);
}

void midiSequencer::loadSong(string filename)
{
  ofXML xml;
  xml.loadFile(filename);
  bandBar & bnd=*band;
  bnd.clear();
  xml.setCurrentTag(";band");
  ofTag & tag=xml.getCurrentTag();
  for (unsigned int i=0; i<tag.size(); i++) {
    if(tag[i].getLabel()=="instrument"){
      string which=tag[i].getAttribute("name");
      for (unsigned int j=0; j<bnd.size(); j++) {
        cout << bnd[j].title << endl;
        if(bnd[j].title==which){
          for (unsigned int k=0; k<tag[i].size(); k++) {
            if(tag[i][k].getLabel()=="strike"){
              int beat=ofToInt(tag[i][k].getAttribute("beat"));
              int length=ofToInt(tag[i][k].getAttribute("length"));
              double divLength=measureLength/divsPerMeasure;
              if(beat<numMeasures*divsPerMeasure){
                bnd[j].blocks.push_back(dragBlock(beat*divLength+x,length*divLength,bnd[j].base));
              }
            }
          }
        }
      }
    }
  }
}

void midiSequencer::saveSong(string filename)
{
  ofXML xml;
  bandBar & bnd=*band;
  xml.newCurrentTag("band");
  for (unsigned int i=0; i<bnd.size(); i++) {
    xml.setCurrentTag(";band");
    xml.newCurrentTag("instrument");
    xml.addAttribute("name", bnd[i].title);
    for (unsigned int j=0; j<bnd[i].size(); j++) {
      xml.newCurrentTag("strike");
      double divLength=measureLength/divsPerMeasure-1;
      int bt=(bnd[i][j].x-x)/divLength;
      int lng=bnd[i][j].w/divLength;
      xml.addAttribute("beat", ofToString(bt));
      xml.addAttribute("length", ofToString(lng));
      xml.popTag();
    }
  }
  xml.writeFile(filename);
}

void midiSequencer::drawMark()
{
	ofSetColor(255, 0, 0);
	ofTriangle(mark.x-mark.w/2, mark.y, mark.x+mark.w/2, mark.y, mark.x, mark.y+mark.h);
	ofSetColor(255, 0, 0,128);
	ofLine(mark.x, y, mark.x, ofGetHeight());
}

void midiSequencer::drawDivs(bool full)
{
	for (int i=0; i<numMeasures; i++) {
		if(!full){
			//ofSetColor(0xE0C39B);
#if F_YEAH_WOOD
			ofSetColor(0, 0, 0);
#else
			ofSetColor(0xFFFFFF);
#endif
			label.drawString(ssprintf("%i",i), x+measureLength*i+1-bar.getScrollPosition(), y+label.stringHeight(ssprintf("%i",i)));
		}
		for (int j=0; j<divsPerMeasure; j++) {
#if F_YEAH_WOOD
			if(j%2) ofSetColor(0x60,0x43,0x1B,64);
			else ofSetColor(0x60,0x43,0x1B,128);
#else
			if(j%2) ofSetColor(0x77,0x77,0,64);
			else ofSetColor(0x66,0x66,0,128);
#endif
			ofLine(x+measureLength*i+measureLength*j/divsPerMeasure-bar.getScrollPosition(), \
				   (full)?y:(abs(j-double(divsPerMeasure)/2)<=.5)?y+10:(j==0)?y+5:y+15,\
				   x+measureLength*i+measureLength*j/divsPerMeasure-bar.getScrollPosition(), (!full)?y+h:ofGetHeight());
		}
	}
	ofSetColor(0xE0C39B);
	label.drawString(ofToString(int(numMeasures)), x+measureLength*numMeasures+1-bar.getScrollPosition(), y+label.stringHeight(ssprintf("%i",numMeasures)));
	ofSetColor(0x60,0x43,0x1B,128);
	ofLine(x+measureLength*numMeasures-bar.getScrollPosition(),y+5, x+measureLength*numMeasures-bar.getScrollPosition(), (!full)?y+h:ofGetHeight());
}

void midiSequencer::draw(int _x, int _y)
{
	x=_x;
	w=ofGetWidth()-x;
	y=_y;
	mark.y=y+10;
	
	band->drawBackground();
	//Draw measure lines
	
	drawDivs(true);
	
	band->drawInstruments();
	
#if F_YEAH_WOOD
	ofSetColor(0xA0835B);
#else
	ofSetColor(0x333333);
#endif
	ofRect(x, y, w, h);
	ofShade(x, y+h, 15, w, OF_UP, .4);
	ofShade(x, y+h, 5, w, OF_DOWN, .3);
	
	drawDivs(false);
	
	drawMark();
	
	// Bottom scroll bar
	ofShade(_x, band->getBottomPos(), 5, w, OF_UP, .3);
	
	int botY=band->getBottomPos();
	if(bar.available()){
		bar.draw(_x, band->getBottomPos());
		botY=bar.y+bar.h+2;
	}
	
	int botH=ofGetHeight()-botY;
	//Task bar on bottom
#if F_YEAH_WOOD
	ofSetColor(0xA0835B);
#else
	ofSetColor(0x777777);
#endif
	ofShadeBox(x,botY, w, botH, OF_DOWN, .3);
	//ofShade(x, bar.y+bar.h+2, 5, w, OF_UP, .3);
	ofShade(x,botY, 10, w, OF_DOWN, .3);
	
	double indent=8;
	label.setMode(OF_FONT_CENTER);
	ofRoundShadow(x+w/2-playBut.w/2-indent/2, botY+(botH/2-playBut.h/2)-indent/2, playBut.w+indent, playBut.h+indent, playBut.w/2+indent/2, 1);
	playBut.draw(x+w/2-playBut.w/2, botY+(botH-playBut.h)/2);
	label.drawString("play", playBut.x+playBut.w/2, playBut.y+playBut.h+15);
	ofRoundShadow(x+w/2-playBut.w/2-indent/2-50, botY+(botH/2-rewindBut.h/2)-indent/2,\
				  rewindBut.w+indent, rewindBut.h+indent, rewindBut.w/2+indent/2, 1);
	rewindBut.draw(x+w/2-playBut.w/2-50, botY+(botH-rewindBut.h)/2);
	label.drawString("reset", rewindBut.x+rewindBut.w/2, rewindBut.y+rewindBut.h+15);
	ofRoundShadow(x+w/2-playBut.w/2-indent/2-100, botY+(botH/2-loopBut.h/2)-indent/2, loopBut.w+indent, loopBut.h+indent, loopBut.w/2+indent/2, 1);
	loopBut.draw(x+w/2-playBut.w/2-100, botY+(botH-loopBut.h)/2);
	label.drawString("loop", loopBut.x+loopBut.w/2, loopBut.y+loopBut.h+15);
	
	ofRoundShadow(display.x-10-indent/2, display.y-10-indent/2,\
				  display.w+20+indent, display.h+20+indent, 5, 1);
	ofSetColor(0, 0, 0);
	ofRoundBox(display.x-10, display.y-10, display.w+20, display.h+20, 5, .2);
	ofSetColor(0, 128, 200);
	label.setMode(OF_FONT_LEFT);
	int secs=metronome.getElapsed();
	//display.draw(ssprintf("%02i:%02i.%02i",(secs/1000/60),(secs/1000)%60,(secs%1000/10)), x+w/2+playBut.w/2+20, botY+(botH-display.h)/2);
}

void midiSequencer::update()
{
	band->update(-getBarPosition(),OF_HOR);
	if(isPlaying())
		band->checkActives(cursor()+band->w);
	if(loopBut.pressed()&&cursor()>band->farthestPoint()-x)
		reset();
	if (cursor()>=numMeasures*measureLength+x) {
		reset();
	}
	if(metronome.justExpired())
		reset();
	midiConductor::update();
	double temp=metronome.getElapsedf()*pps-w/2;
	if (bPlaying&&cursorPos>=w/2+bar.getScrollPosition()&&bar.setScrollPosition(temp));
	mark.x=cursor()+x;
	//bar.update();
}

bool midiSequencer::clickDown(int _x, int _y)
{
	if(mark.clickDown(_x, _y));
	else if(_y<mark.y+mark.h){
		mark.setPressed(true);
		setCursorPosition(_x-x+getBarPosition());
		mark.x=cursorPos+x;
	}
	else if(bar.clickDown(_x, _y));
	else if(playBut.toggle(_x, _y)){
		bPlaying=!playBut.pressed();
		if(isPlaying()) midiConductor::pause(),band->stopAll();
		else midiConductor::play();
	}
	else if(rewindBut.clickDown(_x, _y)) reset(), setScrollPos(0);
	else if(loopBut.toggle(_x, _y));
		
}

void midiSequencer::play()
{
	playBut.setPressed(true);
	midiConductor::play();
}

void midiSequencer::pause()
{
	playBut.setPressed(false);
	midiConductor::pause();
}

bool midiSequencer::clickUp()
{
	mark.clickUp();
	rewindBut.clickUp();
	bar.clickUp();
}

bool midiSequencer::mouseMotion(int _x, int _y)
{
	if(mark.pressed()){
		int newPos=_x-x+getBarPosition();
		if(_x<x) newPos=0;
		else if(_x>ofGetWidth()) newPos=ofGetWidth();
		setCursorPosition(newPos);
		mark.x=cursorPos+x;
	}
	else if(bar.mouseMotion(_x, _y));
}

void midiSequencer::snapTo(double & num)
{
	for (int i=0; i<numMeasures; i++) {
		for (int j=0; j<divsPerMeasure; j++) {
			if (abs((x+measureLength*i+measureLength*j/divsPerMeasure)-num)<=measureLength/(divsPerMeasure*2)) {
				num=x+measureLength*i+measureLength*j/divsPerMeasure;
			}
		}
	}
}

void midiSequencer::snapTo(dragBlock & last)
{
	double & num = last.x;
	double & wid = last.w;
	for (int i=0; i<numMeasures; i++) {
		for (int j=0; j<divsPerMeasure; j++) {
			if (abs((x+measureLength*i+measureLength*j/divsPerMeasure)-num)<=measureLength/(divsPerMeasure*2)) {
				num=x+measureLength*i+measureLength*j/divsPerMeasure;
			}
			if (abs((x+measureLength*i+measureLength*j/divsPerMeasure)-(num+wid))<=measureLength/(divsPerMeasure*2)) {
				wid=x+measureLength*i+measureLength*j/divsPerMeasure-num;
			}
			if(num>x+measureLength*numMeasures){
				num=x+measureLength*numMeasures-wid;
			}
			if(num+wid>x+measureLength*numMeasures){
				//num=x+measureLength*numMeasures-wid;
				wid=x+measureLength*numMeasures-num;
			}
		}
	}
}

double midiSequencer::getBarPosition()
{
	return bar.getScrollPosition();
}

double midiSequencer::cursor()
{
	return cursorPos-getBarPosition();
}

void midiSequencer::setScrollPos(double pos)
{
	bar.setScrollPosition(pos);
}