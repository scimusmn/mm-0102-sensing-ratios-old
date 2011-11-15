#ifndef _TEST_APP
#define _TEST_APP


#include "ofMain.h"
#include "ofExtended.h"
#include <deque>
#include "freqMeter.h"

#define POSDATA 0x7D
#define XPOS 'x'
#define	YPOS 'y'
#define VOLDATA 0x7E
#define VOL  'v'
#define ENDPACKET 0x7C
#define BUTDATA 0x7F
#define RESET_BUT 'r'
#define ACT_BUT 'a'

class testApp : public ofBaseApp{

	public:


		void setup();
		void update();
		void draw();
		void exit();

		void keyPressed  (int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);

		void audioRequested 	(float * input, int bufferSize, int nChannels);
	
		void overlays(int xint, int yint);
	
		void reducedRatios(int xint, int yint);

	/************************** Audio Handling ********************/
	
		float 	pan;
		int		sampleRate;
	 	bool 	bNoise;
		float 	volume;

		float 	* lAudio;
		float   * rAudio;

		//------------------- for the simple sine wave synthesis
		float 	targetFrequency;
		float 	targetFrequency2;
		float 	phase;
		float	phase2;
		float 	phaseAdder,phaseAdder2;
		float 	phaseAdderTarget;
		float 	phaseAdderTarget2;
	
	/*****************************Talking to the arduino *********************/
	
	ofSerial	serial;
	bool		bSendSerialMessage;			// a flag for sending serial
	unsigned char		bytesRead[255];				// data from serial, we will be trying to read 255
	unsigned char		bytesReadString[256];			// a string needs a null terminator, so we need 256 bytes
	unsigned char bytesReturned[255];
	int			nBytesRead;					// how much did we read?
	int			nTimesRead;					// how many times did we read?
	float		readTime;					// when did we last read?
	long		lastRead;
	int			posCnt;
	
	char		MODE;
	unsigned char		byteRead;
	int			waitForData;
	unsigned char		data[3];
	bool		complete;
	
	double			xpos;
	double			ypos;
	
	/********************** Graphics variables ************************************/
	
	ofImage		background;
	ofImage		speaker;
	bool		clear;
	double			width;
	double			height;
	double		sqWid;
	bool		recY,recX,recV;
	bool		dispLines;
	int			overlayCnt;
	double			yOffset;
	grid		net;
	deque<ofPoint> lines;
	ofTexture	gridArea;
	ofTrueTypeFont arialHeader;
	ofTrueTypeFont arialLabel;
	long		newTimer;
	
	/*********** Slider Predefined values *************/
	
	double		minVal;
	double		maxVal;
	double		minValY;
	double		maxValY;
	double		octave;
	double		pixOctave;
	double			setMin;
	double			setMinY;
	double		numOctaves;
	int count;
	
	freqMeter	xMeter,yMeter;
	
	/*******************Calib*****************/
	
	bool calib;
	
};

#endif
