#include <stdlib.h>

#include "gba.h"
#include "main.h"

#include "images/amongusbg.h"
#include "images/finger.h"
#include "images/crewmatess.h"
#include "images/speechbubble.h"
#include "images/accuracy.h"
#include "images/wpm.h"
#include "images/crewmatedie.h"
#include "images/winscreen.h"
#include "images/losescreen.h"
#include "images/ejectedcrewmate.h"
#include "images/titlescreen.h"
#include "images/jermasus.h"

enum gba_state
{
	TITLE_SCREEN,
	START,
	PLAY,
	WIN,
	WIN_SCREEN,
	LOSE,
	LOSE_SCREEN
};

int fingerRow;
int fingerCol;
int fingerSpeed;
int crewmateFrame;
int crewmateFrameDelayCounter;
int crewmateFrameDelay;
int timerC;
int timer;
int timeLimit;
int numKeysEnabled;
int numKeyPresses;
int numKeysCorrect;
int accuracyNum;
int wpmNum;
int currChar;
int wpmTextDisplayed;
int wpmNumDisplayed;
int accuracyTextDisplayed;
int accuracyNumDisplayed;
int menuItemsDisplayed;
int currMenuItem;
int isTransitioning;
int currTransitionLine;
int currFloatingX;
int floatingY;
int leftBoundsFloating;
int rightBoundsFloating;
int enterNextLevel;
int loseMessageCurrLetter;
int writingLoseMessage;
int loseMessageTimer;
int tooSlow;
int currTitleScreenItem;
int textBlink;
int textBlinkTimer;
u32 timeAtFinish;
u32 previousButtons;
u32 currentButtons;
enum gba_state state;
struct key keys[10] = {
	{26, 11, 91, 197, 1, 174},
	{89, 11, 147, 66, 0, ' '},
	{11, 11, 119, 127, 0, 'j'},
	{11, 11, 119, 85, 0, 'f'},
	{11, 11, 119, 43, 0, 'a'},
	{11, 11, 133, 134, 0, 'm'},
	{11, 11, 105, 149, 0, 'o'},
	{11, 11, 119, 113, 0, 'g'},
	{11, 11, 105, 121, 0, 'u'},
	{11, 11, 119, 57, 0, 's'}};
char *strToType;
char *strToType2;
char *strToType3;
char *strCrewmateMesage;
char *strCrewmateMesage2;
char *loseMessage;
int currLine;

int main(void)
{
	REG_DISPCNT = 0x403;

	initApplication();
	while (1)
	{
		currentButtons = BUTTONS; // Load the current state of the buttons
		waitForVBlank();
		if (KEY_JUST_PRESSED(BUTTON_SELECT, currentButtons, previousButtons))
		{
			initApplication();
		}

		switch (state)
		{
		case TITLE_SCREEN:
			if (isTransitioning)
			{
				drawRectDMA(currTransitionLine, 0, WIDTH, 2, BLACK);
				currTransitionLine += 2;
				if (currTransitionLine > HEIGHT)
				{
					drawTransitionString();
					if (waitForFrames(180))
					{
						continue;
					}
					showStageScreen();
				}
				break;
			}
			currFloatingX++;
			if (currFloatingX > WIDTH)
			{
				currFloatingX = leftBoundsFloating - EJECTEDCREWMATE_WIDTH;
			}
			drawFloatingCrewmate();
			drawTitleOptions();
			if (KEY_JUST_PRESSED(BUTTON_A, currentButtons, previousButtons))
			{
				if (currTitleScreenItem == 0)
				{
					currTransitionLine = 0;
					isTransitioning = 1;
				} else if (currTitleScreenItem == 1) {
					drawImageDMA(109, 2, JERMASUS_WIDTH, JERMASUS_HEIGHT, jermasus);
				} else {
					undrawImageDMA(109, 2, JERMASUS_WIDTH, JERMASUS_HEIGHT, titlescreen);
				}
			}
			if (KEY_JUST_PRESSED(BUTTON_DOWN, currentButtons, previousButtons))
			{
				currTitleScreenItem++;
				currTitleScreenItem %= 3;
			}
			if (KEY_JUST_PRESSED(BUTTON_UP, currentButtons, previousButtons))
			{
				currTitleScreenItem--;
				currTitleScreenItem += 3;
				currTitleScreenItem %= 3;
			}
			break;
		case START:
			handleStartState();
			break;
		case PLAY:
			handlePlayState();
			break;
		case WIN:
			enterWinScreenState();
			break;
		case WIN_SCREEN:
			displayStatistics(0, 0);
			if (!menuItemsDisplayed && (vBlankCounter > (timeAtFinish + 135)))
			{
				drawString(98, 140, "NEXT", CYAN);
				drawString(110, 140, "RETRY", WHITE);
				menuItemsDisplayed = 1;
			}
			if (!isTransitioning && menuItemsDisplayed == 1)
			{
				handleEndScreenMenu();
			}
			if (isTransitioning)
			{
				drawRectDMA(currTransitionLine, 0, WIDTH, 2, BLACK);
				currTransitionLine += 2;
				if (currTransitionLine >= HEIGHT)
				{
					setStageVariables();
					if (enterNextLevel)
					{
						setSecondLevelStrings();
					}
					drawTransitionString();
					waitForFrames(180);
					showStageScreen();
				}
			}
			break;
		case LOSE:
			enterLoseScreenState();
			break;
		case LOSE_SCREEN:
			displayStatistics(180, tooSlow);
			currFloatingX--;
			if (currFloatingX < 0)
			{
				currFloatingX = WIDTH;
			}
			drawFloatingCrewmate();
			if (!menuItemsDisplayed && (vBlankCounter > (timeAtFinish + 315)))
			{
				drawString(98, 140, "RETRY", CYAN);
				drawString(110, 140, "MAIN MENU", WHITE);
				menuItemsDisplayed = 1;
			}
			if (writingLoseMessage)
			{
				typeLoseText();
			}
			if (!isTransitioning && menuItemsDisplayed == 1)
			{
				handleLoseScreenMenu();
			}
			break;
		}
		previousButtons = currentButtons; // Store the current state of the buttons
	}
	return 0;
}

void displayStatistics(int delay, int slow)
{
	if (!wpmTextDisplayed && (vBlankCounter > (timeAtFinish + 15 + delay)))
	{
		drawString(98, 66, "WPM: ", WHITE);
		wpmTextDisplayed = 1;
	}
	if (!wpmNumDisplayed && (vBlankCounter > (timeAtFinish + 45 + delay)))
	{
		int color = GREEN;
		if (slow)
		{
			color = RED;
		}
		drawSmallInt(98, 96, wpmNum, color);
		wpmNumDisplayed = 1;
	}
	if (!accuracyTextDisplayed && (vBlankCounter > (timeAtFinish + 75 + delay)))
	{
		drawString(110, 36, "ACCURACY: ", WHITE);
		accuracyTextDisplayed = 1;
	}
	if (!accuracyNumDisplayed && (vBlankCounter > (timeAtFinish + 105 + delay)))
	{
		int color = GREEN;
		if (accuracyNum < 80)
		{
			color = RED;
		}
		drawPercentage(110, 96, accuracyNum, color);
		accuracyNumDisplayed = 1;
	}
}

void typeLoseText(void)
{
	loseMessageTimer++;
	if (loseMessageTimer > 5)
	{
		loseMessageTimer = 0;
		drawChar(50, 40 + loseMessageCurrLetter * 6, loseMessage[loseMessageCurrLetter], WHITE);
		loseMessageCurrLetter++;
		if (loseMessage[loseMessageCurrLetter] == 0)
		{
			writingLoseMessage = 0;
		}
	}
}

void setSecondLevelStrings(void)
{

	numKeysEnabled = 10;
	strToType = "sus sus amongus vitals ";
	strToType2 = "sussy among us crewmate ";
	strToType3 = "skeld reactor imposter";
	strCrewmateMesage = "WHO GIVES A SHIT";
	strCrewmateMesage2 = "ABOUT THE HOME ROW";
	loseMessage = "Skill issue!";
}

void drawTransitionString(void)
{
	drawString(70, 35, "FINISH IN ", WHITE);
	drawString(70, 95, "00:45", GREEN);
	drawString(70, 131, "WITH AT LEAST", WHITE);
	drawString(82, 89, "80%", GREEN);
	drawString(82, 114, "ACCURACY!", WHITE);
}

int waitForFrames(int frames)
{
	for (int i = 0; i < frames; i++)
	{

		currentButtons = BUTTONS; // Load the current state of the buttons
		if (KEY_JUST_PRESSED(BUTTON_SELECT, currentButtons, previousButtons))
		{
			initApplication();
			return 1;
		}
		waitForVBlank();
		previousButtons = currentButtons; // Store the current state of the buttons
	}
	return 0;
}

void handleEndScreenMenu(void)
{
	if (KEY_JUST_PRESSED(BUTTON_UP, currentButtons, previousButtons) || KEY_JUST_PRESSED(BUTTON_DOWN, currentButtons, previousButtons))
	{
		currMenuItem++;
		currMenuItem %= 2;
		if (currMenuItem)
		{
			drawString(98, 140, "NEXT", WHITE);
			drawString(110, 140, "RETRY", CYAN);
		}
		else
		{
			drawString(98, 140, "NEXT", CYAN);
			drawString(110, 140, "RETRY", WHITE);
		}
	}
	if (KEY_JUST_PRESSED(BUTTON_A, currentButtons, previousButtons))
	{
		switch (currMenuItem)
		{
		case 1:
			setStageVariables();
			showStageScreen();
			break;
		case 0:
			if (!isTransitioning)
			{
				currTransitionLine = 0;
				enterNextLevel = 1;
			}
			isTransitioning = 1;
			break;
		}
	}
}

void handleLoseScreenMenu(void)
{
	if (KEY_JUST_PRESSED(BUTTON_UP, currentButtons, previousButtons) || KEY_JUST_PRESSED(BUTTON_DOWN, currentButtons, previousButtons))
	{
		currMenuItem++;
		currMenuItem %= 2;
		if (currMenuItem)
		{
			drawString(98, 140, "RETRY", WHITE);
			drawString(110, 140, "MAIN MENU", CYAN);
		}
		else
		{
			drawString(98, 140, "RETRY", CYAN);
			drawString(110, 140, "MAIN MENU", WHITE);
		}
	}
	if (KEY_JUST_PRESSED(BUTTON_A, currentButtons, previousButtons))
	{
		switch (currMenuItem)
		{
		case 1:
			initApplication();
			break;
		case 0:
			setStageVariables();
			showStageScreen();
			break;
		}
	}
}

void handleStartState(void)
{
	if (KEY_JUST_PRESSED(BUTTON_A, currentButtons, previousButtons))
	{
		drawImageDMA(30, 10, CREWMATESS_WIDTH, 23, crewmatess);
		undrawImageDMA(20, 39, SPEECHBUBBLE_WIDTH, SPEECHBUBBLE_HEIGHT, amongusbg);
		drawImageDMA(60, 7, WPM_WIDTH, WPM_HEIGHT, wpm);
		drawRectDMA(36, 55, 160, 1, BLACK);
		drawRectDMA(56, 55, 160, 1, BLACK);
		drawRectDMA(76, 55, 160, 1, BLACK);
		drawString(26, 61, strToType, LIGHT_GRAY);
		drawString(46, 61, strToType2, LIGHT_GRAY);
		drawString(66, 61, strToType3, LIGHT_GRAY);
		drawRectDMA(36, 61, 6, 1, CYAN);
		state = PLAY;
	}
	crewmateFrameDelayCounter++;
	if (crewmateFrameDelayCounter > crewmateFrameDelay)
	{
		crewmateFrame++;
		crewmateFrame %= 7;
		crewmateFrameDelayCounter = 0;
		drawImageDMA(30, 10, CREWMATESS_WIDTH, 23, crewmatess + 575 * crewmateFrame);
	}
}

char *getLine(void)
{
	switch (currLine)
	{
	case 2:
		return strToType3;
	case 1:
		return strToType2;
	default:
		return strToType;
	}
}

void enterWinScreenState(void)
{
	undrawImageDMA(fingerRow, fingerCol, FINGER_WIDTH, FINGER_HEIGHT, amongusbg);
	fingerCol = 0;
	fingerRow = 150;
	for (int i = 0; i < numKeysEnabled; i++)
	{
		drawKey(keys[i], WHITE);
	}
	drawImageDMA(32, 27, WINSCREEN_WIDTH, WINSCREEN_HEIGHT, winscreen);
	state = WIN_SCREEN;
}

void enterLoseScreenState(void)
{
	undrawImageDMA(fingerRow, fingerCol, FINGER_WIDTH, FINGER_HEIGHT, amongusbg);
	fingerCol = 0;
	fingerRow = 150;
	for (int i = 0; i < numKeysEnabled; i++)
	{
		drawKey(keys[i], WHITE);
	}
	drawImageDMA(32, 27, LOSESCREEN_WIDTH, LOSESCREEN_HEIGHT, losescreen);
	leftBoundsFloating = 28;
	rightBoundsFloating = 26 + LOSESCREEN_WIDTH;
	floatingY = 72;
	currFloatingX = -10;
	writingLoseMessage = 1;
	state = LOSE_SCREEN;
}

void drawPercentage(int row, int col, int val, int color)
{
	if (val < 10)
	{
		char s[] = {val + 48, '%', '\0'};
		drawString(row, col, s, color);
	}
	else if (val < 100)
	{
		char s[] = {val / 10 + 48, (val % 10) + 48, '%', '\0'};
		drawString(row, col, s, color);
	}
	else
	{
		char s[] = "100%";
		drawString(row, col, s, color);
	}
}

void enterWinState(void)
{
	if (accuracyNum >= 80)
	{
		state = WIN;
	}
	else
	{
		state = LOSE;
	}
	timeAtFinish = vBlankCounter;
}

int checkIfKeyPressed(struct key key)
{
	if (!isWithinKey(key, fingerRow, fingerCol))
	{
		return 0;
	}

	if (key.isBackspace)
	{
		backspacePressed();
		return 1;
	}

	drawRectDMA(9, 65, 24, 8, RED);
	numKeyPresses++;
	if (getLine()[currChar] == key.character)
	{
		correctKeyPressed(key);
	}
	else
	{
		incorrectKeyPressed();
	}

	accuracyNum = (numKeysCorrect * 100) / numKeyPresses;
	drawPercentage(9, 65, accuracyNum, WHITE);
	currChar++;
	if (getLine()[currChar] == 0)
	{
		currLine++;
		currChar = 0;
	}
	if (currLine >= 3)
	{
		enterWinState();
		return 1;
	}
	drawRectDMA(36 + currLine * 20, 61 + currChar * 6, 6, 1, CYAN);
	return 1;
}

void backspacePressed(void)
{
	if (currChar == 0 && currLine == 0)
	{
		return;
	}
	drawRectDMA(36 + currLine * 20, 61 + currChar * 6, 6, 1, BLACK);
	currChar--;
	if (currChar < 0)
	{
		currChar = 0;
		currLine--;
		while (getLine()[currChar] != 0)
		{
			currChar++;
		}
		currChar--;
	}
	drawChar(26 + currLine * 20, 61 + currChar * 6, getLine()[currChar], LIGHT_GRAY);
	drawRectDMA(36 + currLine * 20, 61 + currChar * 6, 6, 1, CYAN);
}

void correctKeyPressed(struct key key)
{
	drawImageDMA(30, 10, CREWMATESS_WIDTH, 23, crewmatess);
	numKeysCorrect++;
	if (key.character != ' ')
	{
		drawChar(26 + currLine * 20, 61 + currChar * 6, getLine()[currChar], BLACK);
	}
	drawRectDMA(36 + currLine * 20, 61 + currChar * 6, 6, 1, BLACK);
}

void incorrectKeyPressed(void)
{
	drawImageDMA(30, 10, CREWMATEDIE_WIDTH, CREWMATEDIE_HEIGHT, crewmatedie);
	if (getLine()[currChar] != ' ')
	{
		drawChar(26 + currLine * 20, 61 + currChar * 6, getLine()[currChar], RED);
	}
	drawRectDMA(36 + currLine * 20, 61 + currChar * 6, 6, 1, RED);
}

void handlePlayState(void)
{
	undrawImageDMA(fingerRow, fingerCol, FINGER_WIDTH, FINGER_HEIGHT, amongusbg);
	tickTimer();
	if (KEY_JUST_PRESSED(BUTTON_A, currentButtons, previousButtons))
	{
		for (int i = 0; i < numKeysEnabled; i++)
		{
			if (checkIfKeyPressed(keys[i]))
			{
				break;
			}
		}
	}
	if (KEY_DOWN(BUTTON_LEFT, currentButtons))
	{
		fingerCol -= fingerSpeed;
		if (fingerCol < 0)
		{
			fingerCol = 0;
		}
	}
	if (KEY_DOWN(BUTTON_RIGHT, currentButtons))
	{
		fingerCol += fingerSpeed;
		if (fingerCol > WIDTH - FINGER_WIDTH)
		{
			fingerCol = WIDTH - FINGER_WIDTH;
		}
	}
	if (KEY_DOWN(BUTTON_UP, currentButtons))
	{
		fingerRow -= fingerSpeed;
		if (fingerRow < 88)
		{
			fingerRow = 88;
		}
	}
	if (KEY_DOWN(BUTTON_DOWN, currentButtons))
	{
		fingerRow += fingerSpeed;
		if (fingerRow > HEIGHT - FINGER_HEIGHT + 5)
		{
			fingerRow = HEIGHT - FINGER_HEIGHT + 5;
		}
	}
	for (int i = 0; i < numKeysEnabled; i++)
	{
		checkKeyAndDraw(keys[i], fingerRow, fingerCol);
	}
	drawFinger(fingerRow, fingerCol);
}

void drawFloatingCrewmate(void)
{
	int leftCol = currFloatingX;
	if (leftCol < leftBoundsFloating)
	{
		leftCol = leftBoundsFloating;
	}
	int rightCol = currFloatingX + EJECTEDCREWMATE_WIDTH;
	if (rightCol > rightBoundsFloating)
	{
		rightCol = rightBoundsFloating;
	}
	if (leftCol >= rightCol || (leftCol - rightCol) > EJECTEDCREWMATE_WIDTH || (rightCol - leftCol) > EJECTEDCREWMATE_WIDTH)
	{
		return;
	}
	for (int i = 0; i < EJECTEDCREWMATE_HEIGHT; i++)
	{
		DMA[3].src = ejectedcrewmate + i * EJECTEDCREWMATE_WIDTH + (leftCol - currFloatingX);
		DMA[3].dst = videoBuffer + (floatingY + i) * WIDTH + leftCol;
		DMA[3].cnt = DMA_ON | DMA_16 | DMA_DESTINATION_INCREMENT | DMA_SOURCE_INCREMENT | (rightCol - leftCol);
	}
}

void drawFinger(int row, int col)
{
	drawImageDMA(row, col + 1, 2, 1, finger + 1);
	drawImageDMA(row + 1, col, 4, 1, finger + FINGER_WIDTH);
	drawImageDMA(row + 2, col + 1, 4, 1, finger + 2 * FINGER_WIDTH + 1);
	drawImageDMA(row + 2, col + 7, 3, 1, finger + 2 * FINGER_WIDTH + 7);
	drawImageDMA(row + 3, col + 2, 9, 1, finger + 3 * FINGER_WIDTH + 2);
	drawImageDMA(row + 4, col + 3, 9, 1, finger + 4 * FINGER_WIDTH + 3);
	drawImageDMA(row + 5, col + 4, 8, 1, finger + 5 * FINGER_WIDTH + 4);
	drawImageDMA(row + 6, col + 4, 9, 1, finger + 6 * FINGER_WIDTH + 4);
	drawImageDMA(row + 7, col + 4, 10, 1, finger + 7 * FINGER_WIDTH + 4);
	drawImageDMA(row + 8, col + 5, 9, 1, finger + 8 * FINGER_WIDTH + 5);
	drawImageDMA(row + 9, col + 6, 7, 1, finger + 9 * FINGER_WIDTH + 6);
	drawImageDMA(row + 10, col + 8, 4, 1, finger + 10 * FINGER_WIDTH + 8);
	drawImageDMA(row + 11, col + 9, 2, 1, finger + 11 * FINGER_WIDTH + 9);
}

int isWithinKey(struct key key, int row, int col)
{
	return key.row < row && key.col < col && (row < key.row + key.height) && (col < key.col + key.width);
}

void checkKeyAndDraw(struct key key, int row, int col)
{
	int dRow = key.row - row;
	int dCol = key.col - col;
	if (-dRow > FINGER_HEIGHT || dRow > FINGER_HEIGHT || -dCol > FINGER_WIDTH + key.width || dCol > FINGER_WIDTH)
	{
		return;
	}
	int color = WHITE;
	if (isWithinKey(key, row, col))
	{
		color = BLUE;
	}
	drawKey(key, color);
}

void initApplication(void)
{
	state = TITLE_SCREEN;
	reset();
	showTitleScreen();
}

void reset(void)
{
	setStageVariables();
	enterNextLevel = 0;
	strToType = "ffff jjjj fjfj jfjf ffj ";
	strToType2 = "jjf jj ff jf fj f j f j ";
	strToType3 = "jfjjff fjjfjf";
	strCrewmateMesage = "LET'S LEARN ABOUT OUR";
	strCrewmateMesage2 = "TASKS (HOME ROW)!";
	loseMessage = "RED needs practice!";
}

void setStageVariables(void)
{
	fingerRow = 100;
	fingerCol = 100;
	fingerSpeed = 1;
	crewmateFrame = 0;
	crewmateFrameDelayCounter = 0;
	crewmateFrameDelay = 3;
	timerC = 0;
	timer = 0;
	timeLimit = 45;
	numKeyPresses = 0;
	numKeysCorrect = 0;
	wpmNum = 0;
	numKeysEnabled = 4;
	accuracyNum = 0;
	currChar = 0;
	currLine = 0;
	accuracyNumDisplayed = 0;
	accuracyTextDisplayed = 0;
	menuItemsDisplayed = 0;
	wpmNumDisplayed = 0;
	wpmTextDisplayed = 0;
	timeAtFinish = 0;
	currTransitionLine = 0;
	currFloatingX = -120;
	floatingY = 70;
	leftBoundsFloating = 2;
	rightBoundsFloating = 120;
	isTransitioning = 0;
	loseMessageCurrLetter = 0;
	loseMessageTimer = 0;
	writingLoseMessage = 0;
	tooSlow = 0;
	textBlink = 0;
	currTitleScreenItem = 0;
	textBlinkTimer = 0;
}

void showTitleScreen(void)
{
	drawFullScreenImageDMA(titlescreen);
	drawTitleOptions();
	drawFloatingCrewmate();
}

void drawTitleOptions(void)
{
	drawBlinkingText(34, 144, "PLAY GAME", 0);
	drawBlinkingText(75, 144, "REVEAL JEREMY", 1);
	drawBlinkingText(116, 144, "REMOVE STAIN", 2);
}

void drawBlinkingText(int row, int col, char *str, int id)
{
	if (currTitleScreenItem == id)
	{
		textBlinkTimer++;
		if (textBlinkTimer < 4)
		{
			return;
		}
		textBlinkTimer = 0;
		int color = COLOR(25, 25, 25);
		if (textBlink)
		{
			textBlink = 0;
			color = YELLOW;
		}
		else
		{
			textBlink = 1;
		}
		drawString(row, col, str, color);
		return;
	}
	drawString(row, col, str, WHITE);
}

void showStageScreen(void)
{
	state = START;
	currTransitionLine = 0;
	isTransitioning = 0;
	drawFullScreenImageDMA(amongusbg);
	drawImageDMA(20, 39, SPEECHBUBBLE_WIDTH, SPEECHBUBBLE_HEIGHT, speechbubble);
	drawImageDMA(7, 9, ACCURACY_WIDTH, ACCURACY_HEIGHT, accuracy);
	drawImageDMA(60, 7, WPM_WIDTH, WPM_HEIGHT, wpm);
	drawString(9, 65, "100%", WHITE);
	drawString(30, 53, strCrewmateMesage, BLACK);
	drawString(42, 53, strCrewmateMesage2, BLACK);
	drawString(9, 200, "00:45", BLACK);
	drawCenteredString(75, 14, 24, 8, "0", BLACK);
	drawImageDMA(30, 10, CREWMATESS_WIDTH, 23, crewmatess);
	for (int i = 0; i < numKeysEnabled; i++)
	{
		drawKey(keys[i], WHITE);
	}
	previousButtons = BUTTONS;
	currentButtons = BUTTONS;
}

void drawKey(struct key key, int color)
{
	drawRectDMA(key.row, key.col, key.width, key.height, color);
	drawChar(key.row + 1, key.col + 1, key.character, BLACK);
}

void drawTime(int row, int col, int seconds, int color)
{
	char ones = (seconds % 10) + 48;
	char tens = (seconds / 10) % 6 + 48;
	int minute = seconds / 60;
	char hund = (minute % 10) + 48;
	char thou = (minute / 10) % 10 + 48;
	char str[] = {thou, hund, ':', tens, ones, '\0'};
	drawString(row, col, str, color);
}

void drawSmallIntCentered(int row, int col, int val, int color)
{
	if (val < 10)
	{
		char str[] = {val + 48, '\0'};
		drawCenteredString(row, col, 24, 8, str, color);
	}
	else if (val < 100)
	{
		char str[] = {val / 10 + 48, val % 10 + 48, '\0'};
		drawCenteredString(row, col, 24, 8, str, color);
	}
	else
	{
		char str[] = {(val / 100) % 10 + 48, (val / 10) % 10 + 48, val % 10 + 48, '\0'};
		drawCenteredString(row, col, 24, 8, str, color);
	}
}

void drawSmallInt(int row, int col, int val, int color)
{
	if (val < 10)
	{
		char str[] = {val + 48, '\0'};
		drawString(row, col, str, color);
	}
	else if (val < 100)
	{
		char str[] = {val / 10 + 48, val % 10 + 48, '\0'};
		drawString(row, col, str, color);
	}
	else
	{
		char str[] = {(val / 100) % 10 + 48, (val / 10) % 10 + 48, val % 10 + 48, '\0'};
		drawString(row, col, str, color);
	}
}

void tickTimer(void)
{
	timerC++;
	if (timerC == 60)
	{
		timer++;
		wpmNum = numKeyPresses * 12 / timer;
		drawRectDMA(75, 14, 24, 8, WHITE);
		drawSmallIntCentered(75, 14, wpmNum, BLACK);
		timerC = 0;
		drawRectDMA(9, 200, 30, 8, WHITE);
		int timerSub = timeLimit - timer;
		if (timerSub < 0)
		{
			timerSub = 0;
			tooSlow = 1;
			state = LOSE;
			timeAtFinish = vBlankCounter;
		}
		drawTime(9, 200, timerSub, BLACK);
	}
	/*
	if (timerC % 15 == 0) {
		drawRectDMA(50, 50, 30, 8, BLACK);
		int c = (timer * 4 + timerC / 15) % 256;
		char str[] = {c, (c / 100) % 10 + 48, (c / 10) % 10 + 48, c % 10 + 48, '\0'};
		drawString(50, 50, str, GREEN);
	} */
}
