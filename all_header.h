#pragma once
#ifndef ALL_HEADER_H
#define ALL_HEADER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <windows.h>
#include <process.h>
#include <mmsystem.h>
#include <conio.h>
#pragma comment (lib, "winmm.lib")
#include <Digitalv.h>
#include "ManyLayer.h"


typedef struct KEY_INFO {
	bool LEFT, RIGHT, UP, DOWN, ENTER, SPACE, ESC, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z;
} KEY_INFO;

#endif