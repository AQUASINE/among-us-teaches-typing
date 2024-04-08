#ifndef MAIN_H
#define MAIN_H

#include "gba.h"

struct key {	
	int width;
	int height;
	int row;
	int col;
	int isBackspace;
	char character;	
};

void drawFinger(int row, int col);
void checkKeyAndDraw(struct key key, int row, int col);
void drawKey(struct key key, int color);
void initApplication(void);
void handleStartState(void);
void handlePlayState(void);
void tickTimer(void);
char *getLine(void);
void enterWinScreenState(void);
void enterWinState(void);
void drawPercentage(int row, int col, int val, int color);
void drawTime(int row, int col, int seconds, int color);
void drawSmallInt(int row, int col, int val, int color);
void drawSmallIntCentered(int row, int col, int val, int color);
void incorrectKeyPressed(void);
void correctKeyPressed(struct key key);
void backspacePressed(void);
void handleEndScreenMenu(void);
void handleLoseScreenMenu(void);
void drawFloatingCrewmate(void);
void showTitleScreen(void);
void showStageScreen(void);
void setStageVariables(void);
int waitForFrames(int frames);
void drawTransitionString(void);
void setSecondLevelStrings(void);
void reset(void);
void typeLoseText(void);
void enterLoseScreenState(void);
void drawTitleOptions(void);
void drawBlinkingText(int row, int col, char * str, int id);
void displayStatistics(int delay, int slow);
int isWithinKey(struct key key, int row, int col);

#endif
