/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Jeroen Domburg <jeroen@spritesmods.com> wrote this file. As long as you retain 
 * this notice you can do whatever you want with this stuff. If we meet some day, 
 * and you think this stuff is worth it, you can buy me a beer in return. 
 * ----------------------------------------------------------------------------
 */
#include "mouse.h"

static volatile int posX = 320;
static volatile int posY = 240;
static volatile int posDirty = 0;
static volatile int posBtn = 0;

void mouseMove(int dx, int dy, int btn) {
	int x = posX + dx;
	int y = posY + dy;
	if (x < 0) x = 0;
	if (y < 0) y = 0;
	if (x > 639) x = 639;
	if (y > 479) y = 479;
	posX = x;
	posY = y;
	posBtn = btn ? 1 : 0;
	if (dx || dy) posDirty = 1;
}

int mouseButton(void) {
	return posBtn;
}

int mouseTakePos(int *x, int *y) {
	*x = posX;
	*y = posY;
	if (!posDirty) return 0;
	posDirty = 0;
	return 1;
}
