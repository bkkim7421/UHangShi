#pragma warning(disable: 4996)

#include "all_header.h"

extern FILE* Storage[4];
extern bool MainPageFlag;
extern MCI_OPEN_PARMS MainMusic;
extern int MainMusicID;

extern FILE* OpenStorage_i(int n, int rw);
extern void SoundEffect(char filePath[], MCI_OPEN_PARMS* soundEffect, int* dwID, bool load, bool playing, bool repeat);

ManyLayer GameLayer = { NULL, 0, NULL, 0, RGB(255, 255, 255), NULL, NULL, _initialize, _renderAll, NULL, _getBitmapHandle };
MCI_OPEN_PARMS InGameMusic;
int InGameMusicID;

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
	SoundEffect("Musics/BabyElephantWalk60.wav", &MainMusic, &MainMusicID, 0, 0, 0);
	SoundEffect("Musics/InGameBGM.wav", &InGameMusic, &InGameMusicID, 1, 0, 0);
	SoundEffect("Musics/InGameBGM.wav", &InGameMusic, &InGameMusicID, 0, 1, 1);

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
			{ GameLayer.getBitmapHandle(L"InGame/start_bg.bmp"), 0, 0, 2.38, false },
			{ GameLayer.getBitmapHandle(L"InGame/start_bg.bmp"), 2000, 0, 2.38, false },
			{ GameLayer.getBitmapHandle(L"InGame/box.bmp"), 900, 1300, 1.3, false },
		};
		GameLayer.imageCount = 3;

		GameLayer.texts = (Text[]){
			{ L"1번 저장소를 덮어쓰겠습니까?", 1150, 1500, 30, 40, 600, L"둥근모꼴", RGB(0, 0, 0), false },
			{ L"[Y]es", 1400, 1150, 48, 64, 600, L"둥근모꼴", RGB(0, 0, 0), true },
			{ L"[N]o", 2250, 1150, 48, 64, 600, L"둥근모꼴", RGB(0, 0, 0), true },
		};
		GameLayer.textCount = 3;
		Sleep(1000);


	}

	Sleep(10000);
	return;
}