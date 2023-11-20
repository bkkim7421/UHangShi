#pragma warning(disable: 4996)

#include "all_header.h"

extern FILE* Storage[4];
extern bool MainPageFlag;

extern FILE* OpenStorage_i(int n, int rw);

ManyLayer GameLayer = { NULL, 0, NULL, 0, RGB(255, 255, 255), NULL, NULL, _initialize, _renderAll, NULL, _getBitmapHandle };

bool GamePageFlag =1;

void GameBGThread()
{
	while(GamePageFlag){
		GameLayer.renderAll(&GameLayer);
		Sleep(10);
	}
	return;
}

void WritePG(int N, int PG)
{
	Storage[N] = OpenStorage_i(N, 1);
	fprintf(Storage[N], "%d\n", PG);
	fclose(Storage[N]);
	Storage[N] = NULL;
}

extern void StartGame(int N, int PG)
{
	GameLayer.initialize(&GameLayer);
	if(!GamePageFlag){
		GamePageFlag = 1;
		_beginthreadex(NULL, 0, (_beginthreadex_proc_type)GameBGThread, 0, 0, NULL);
	}

	MainPageFlag = 0;
	switch (PG) {
	case 0:
		WritePG(N, 0);
		GameLayer.images = (Image[]){
			{ GameLayer.getBitmapHandle(L"start_bg.bmp"), 0, 0, 2.38, false },
			{ GameLayer.getBitmapHandle(L"start_bg.bmp"), 2000, 0, 2.38, false }
		};
		GameLayer.imageCount = 2;
	}

	Sleep(10000);
	return;
}