#pragma warning(disable: 4996)

#include "all_header.h"
#define HUMAN_IDX 3

extern FILE* Storage[4];
extern bool MainPageFlag;
extern MCI_OPEN_PARMS MainMusic;
extern int MainMusicID;
extern KEY_INFO key;

extern FILE* OpenStorage_i(int n, int rw);
extern void SoundEffect(char filePath[], MCI_OPEN_PARMS* soundEffect, int* dwID, bool load, bool playing, bool repeat);
extern void ToggleLayer(ManyLayer* ML, int img_s, int img_e, int txt_s, int txt_e);

ManyLayer GameLayer = { NULL, 0, NULL, 0, RGB(255, 255, 255), NULL, NULL, _initialize, _renderAll, NULL, _getBitmapHandle };
MCI_OPEN_PARMS InGameMusic;
int InGameMusicID;

COORD human;
int human_frame;

bool GamePageFlag =1;

void GameBGThread()
{
	while(GamePageFlag){
		GameLayer.renderAll(&GameLayer);
		Sleep(10);
	}
	return;
}

void HumanThread()
{
	while (GamePageFlag) {

		ToggleLayer(&GameLayer, HUMAN_IDX + human_frame, HUMAN_IDX + human_frame, -1, - 1);
		ToggleLayer(&GameLayer, HUMAN_IDX + (human_frame+1)%4, HUMAN_IDX + (human_frame+1)%4, -1, - 1);
		human_frame = (human_frame + 1) % 4;

		printf("%d", human_frame);
		Sleep(400);
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
	MainPageFlag = 0;

	human = (COORD){0, 800};

	GameLayer.initialize(&GameLayer);

	GameLayer.images = (Image[]){
		{ GameLayer.getBitmapHandle(L"InGame/start_bg.bmp"), 0, 0, 2.38, false },
		{ GameLayer.getBitmapHandle(L"InGame/start_bg.bmp"), 2000, 0, 2.38, false },
		{ GameLayer.getBitmapHandle(L"InGame/box.bmp"), 900, 1300, 1.3, true },
		{ GameLayer.getBitmapHandle(L"InGame/human/walking_human1.bmp"), human.X, human.Y, 1.3, human_frame != 0},
		{ GameLayer.getBitmapHandle(L"InGame/human/walking_human2.bmp"), human.X, human.Y, 1.3, human_frame != 1},
		{ GameLayer.getBitmapHandle(L"InGame/human/walking_human3.bmp"), human.X, human.Y, 1.3, human_frame != 2},
		{ GameLayer.getBitmapHandle(L"InGame/human/walking_human4.bmp"), human.X, human.Y, 1.3, human_frame != 3},
	};
	GameLayer.imageCount = 10;

	GameLayer.texts = (Text[]){
		{ L"<space>", 1800, 1800, 30, 40, 600, L"µÕ±Ù¸ð²Ã", RGB(0, 0, 0), true },
		{ L"plot 1", 1150, 1500, 30, 40, 600, L"µÕ±Ù¸ð²Ã", RGB(0, 0, 0), true},
		{ L"plot 2", 1150, 1500, 30, 40, 600, L"µÕ±Ù¸ð²Ã", RGB(0, 0, 0), true},
		{ L"plot 3", 1150, 1500, 30, 40, 600, L"µÕ±Ù¸ð²Ã", RGB(0, 0, 0), true},
		{ L"plot 4", 1150, 1500, 30, 40, 600, L"µÕ±Ù¸ð²Ã", RGB(0, 0, 0), true},
	};
	GameLayer.textCount = 5;

	if(!GamePageFlag){
		GamePageFlag = 1;
		_beginthreadex(NULL, 0, (_beginthreadex_proc_type)GameBGThread, 0, 0, NULL);
		_beginthreadex(NULL, 0, (_beginthreadex_proc_type)HumanThread, 0, 0, NULL);
	}

	switch (PG) {
	case 0:
		WritePG(N, 0);
		Sleep(1000);

		ToggleLayer(&GameLayer, 2, 2, -1, -1);

		for (int i = 1; i <= 4; i++) {
			Sleep(300);
			ToggleLayer(&GameLayer, -1, -1, i, i);
			Sleep(1000);
			ToggleLayer(&GameLayer, -1, -1, 0, 0);
			while (!key.SPACE);
			ToggleLayer(&GameLayer, -1, -1, i, i);
			ToggleLayer(&GameLayer, -1, -1, 0, 0);
		}

		ToggleLayer(&GameLayer, 2, 2, -1, -1);
		
	case 1:
		WritePG(N, 1);
		SoundEffect("Musics/InGameBGM.wav", &InGameMusic, &InGameMusicID, 1, 0, 0);
		SoundEffect("Musics/InGameBGM.wav", &InGameMusic, &InGameMusicID, 0, 1, 1);

	}

	Sleep(5000);
	return;
}