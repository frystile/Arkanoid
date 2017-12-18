#pragma once

#include "resource.h"
#define PI 3.14159265
#define LIFE 2;
#define WALL_WIDTH 15 //15
#define WALL_HEIGHT 6;  //6
#define START_STICK_POSITION 0.5
#define START_BALL_VECTOR -90;
#define NULL_XY -100 ;
#define STICK_SPEED 1.75;
#define BALL_SPEED 10;
#define BONUS_PROBABILITY 4;
#define BONUS_LIFE_PROBABILITY 6;

RECT getBrick(int );
void createBricks();
int getRadius();
int DrawFrame(HWND );
void startPosition();
void Rebuild();
void moveStick(float );
void showMessage(LPWSTR );
int APIENTRY wWinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int);
ATOM MyRegisterClass(HINSTANCE );
BOOL InitInstance(HINSTANCE , int );
LRESULT CALLBACK WndProc(HWND , UINT , WPARAM , LPARAM );
INT_PTR CALLBACK About(HWND , UINT , WPARAM , LPARAM );