#pragma warning(disable: 4996)

#include "all_header.h"
#define BOX_IDX 84
#define HUMAN_IDX 85

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
MCI_OPEN_PARMS HitMusic;
int HitMusicID;

COORD human;
int human_frame;
int front;
int BG_cnt;
int Score;
int used[4];
wchar_t* STRING;

bool FlowerFlag = 1;
bool GamePageFlag = 1;

// thread painting layer repeatedly
unsigned int __stdcall GameLayerPaintThread(void* arg)
{
	while (GamePageFlag) {
		GameLayer.renderAll(&GameLayer);
		Sleep(10);
	}
}

// thread making human image move
unsigned int __stdcall HumanThread(void* arg)
{
	while (GamePageFlag) {

		ToggleLayer(&GameLayer, HUMAN_IDX + human_frame, HUMAN_IDX + human_frame, -1, -1);
		GameLayer.images[HUMAN_IDX + human_frame].x = human.X;
		GameLayer.images[HUMAN_IDX + human_frame].y = human.Y;
		ToggleLayer(&GameLayer, HUMAN_IDX + (human_frame + 1) % 4, HUMAN_IDX + (human_frame + 1) % 4, -1, -1);
		GameLayer.images[HUMAN_IDX + (human_frame + 1) % 4].x = human.X;
		GameLayer.images[HUMAN_IDX + (human_frame + 1) % 4].y = human.Y;
		human_frame = (human_frame + 1) % 4;

		Sleep(250);
	}
}

// calculate accuracy
int Judgment(int dif)
{
	if (dif > 75)
		return 0;
	else if (dif > 50)
		return 1;
	else if (dif > 25)
		return 2;
	else
		return 3;
}

// thread making flower showed and calculating score
unsigned int __stdcall FlowerThread(int Flower)
{
	used[Flower] = (used[Flower] + 1) % 20;
	int FlowerIdx = 20 * Flower + used[Flower] + 4;
	int jdg = -1;
	int dif = abs(GameLayer.images[FlowerIdx].x - 165);
	while (GameLayer.images[FlowerIdx].x >= -300) {
		GameLayer.images[FlowerIdx].x -= 2;
		dif = abs(GameLayer.images[FlowerIdx].x - 160);
		if (dif <= 100) {
			if (!Flower && key.D) { // daema
				jdg = Judgment(dif);
				break;
			}
			if (Flower == 1 && key.J) { // fourleaves
				jdg = Judgment(dif);
				break;
			}
			if (Flower == 2 && key.K) { // jangmi
				jdg = Judgment(dif);
				break;
			}
			if (Flower == 3 && key.F) { // yanggwibi
				jdg = Judgment(dif);
				break;
			}
		}
		Sleep(1);
	}

	GameLayer.images[FlowerIdx].x = 3800;
	Score += jdg * (FlowerIdx & 1 ? 2 : 1);

	if (jdg >= 0) { // hit sound effect
		SoundEffect("Musics/hit.wav", &HitMusic, &HitMusicID, 1, 0, 0);
		SoundEffect("Musics/hit.wav", &HitMusic, &HitMusicID, 0, 1, 0);
	}

	if (STRING != NULL)
		free(STRING);
	STRING = (wchar_t*)malloc(20 * sizeof(wchar_t));
	swprintf(STRING, 20, L"Score : %d", Score);
	GameLayer.texts[10].content = STRING;

	GameLayer.texts[6 + jdg].isHidden = false;
	Sleep(500);
	GameLayer.texts[6 + jdg].isHidden = true;
}

// thread making item move
unsigned int __stdcall ItemThread(void* arg)
{
	Sleep(3250);
	while (FlowerFlag) {
		int n = rand() % 4;
		_beginthreadex(NULL, 0, (_beginthreadex_proc_type)FlowerThread, (int)n, 0, NULL);
		Sleep(875);
	}
}

// thread making background move
unsigned int __stdcall BGThread(void* arg)
{
	int mv = 2;
	int hmv = 1;

	while (GamePageFlag) {

		if (BG_cnt < 10) {
			GameLayer.images[1].x -= mv;
			GameLayer.images[0].x -= mv;
		}
		else {
			if (BG_cnt < 11) {
				GameLayer.images[0].x -= mv;
				FlowerFlag = 0;
			}
			GameLayer.images[2].x -= mv;
			GameLayer.images[3].x -= mv;
			if (GameLayer.images[2].x <= -3700) {
				mv = 0;

				human.X += hmv;
				if (human.X >= 1900) {
					hmv = 0;
					_endthreadex(0);
				}
			}
		}
		if (front == 0 && GameLayer.images[1].x <= 0)
			GameLayer.images[0].x = 7570, front = 1, BG_cnt++;
		if (front == 1 && GameLayer.images[0].x <= 0)
			GameLayer.images[1].x = 7570, front = 0, BG_cnt++;

		Sleep(1);
	}
}

// write progress in text file
void WritePG(int N, int PG)
{
	Storage[N] = OpenStorage_i(N, 1);
	fprintf(Storage[N], "%d\n", PG);
	fclose(Storage[N]);
	Storage[N] = NULL;
}

// create in-game screen
void StartGame(int N, int PG)
{
	SoundEffect("Musics/BabyElephantWalk60.wav", &MainMusic, &MainMusicID, 0, 0, 0);
	MainPageFlag = 0;

	human = (COORD){ 0, 800 };
	human_frame = 0;
	front = 0;
	BG_cnt = 0;
	for (int i = 0; i < 4; i++)
		used[i] = 0;
	if (!GamePageFlag) {
		GameLayer.initialize(&GameLayer);

		GameLayer.images = (Image[]){
			{ GameLayer.getBitmapHandle(L"InGame/start_bg.bmp"), 0, 0, 2.38, false },
			{ GameLayer.getBitmapHandle(L"InGame/start_bg.bmp"), 7610, 0, 2.38, false },
			{ GameLayer.getBitmapHandle(L"InGame/final_bg.bmp"), 7610, 0, 2.38, false },
			{ GameLayer.getBitmapHandle(L"InGame/final_bg_school.bmp"), 7610, 0, 2.38, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/daema.bmp"), 3800, 1200, 0.5, false }, // 4
			{ GameLayer.getBitmapHandle(L"InGame/flowers/daema_on.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/daema.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/daema_on.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/daema.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/daema_on.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/daema.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/daema_on.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/daema.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/daema_on.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/daema.bmp"), 3800, 1200, 0.5, false }, // 14
			{ GameLayer.getBitmapHandle(L"InGame/flowers/daema_on.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/daema.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/daema_on.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/daema.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/daema_on.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/daema.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/daema_on.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/daema.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/daema_on.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/fourleaves.bmp"), 3800, 1200, 0.5, false }, // 24
			{ GameLayer.getBitmapHandle(L"InGame/flowers/fourleaves_on.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/fourleaves.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/fourleaves_on.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/fourleaves.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/fourleaves_on.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/fourleaves.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/fourleaves_on.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/fourleaves.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/fourleaves_on.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/fourleaves.bmp"), 3800, 1200, 0.5, false }, // 34
			{ GameLayer.getBitmapHandle(L"InGame/flowers/fourleaves_on.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/fourleaves.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/fourleaves_on.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/fourleaves.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/fourleaves_on.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/fourleaves.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/fourleaves_on.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/fourleaves.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/fourleaves_on.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/jangmi.bmp"), 3800, 1200, 0.5, false }, // 44
			{ GameLayer.getBitmapHandle(L"InGame/flowers/jangmi_on.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/jangmi.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/jangmi_on.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/jangmi.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/jangmi_on.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/jangmi.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/jangmi_on.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/jangmi.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/jangmi_on.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/jangmi.bmp"), 3800, 1200, 0.5, false }, // 54
			{ GameLayer.getBitmapHandle(L"InGame/flowers/jangmi_on.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/jangmi.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/jangmi_on.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/jangmi.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/jangmi_on.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/jangmi.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/jangmi_on.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/jangmi.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/jangmi_on.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/yanggwibi.bmp"), 3800, 1200, 0.5, false }, // 64
			{ GameLayer.getBitmapHandle(L"InGame/flowers/yanggwibi_on.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/yanggwibi.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/yanggwibi_on.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/yanggwibi.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/yanggwibi_on.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/yanggwibi.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/yanggwibi_on.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/yanggwibi.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/yanggwibi_on.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/yanggwibi.bmp"), 3800, 1200, 0.5, false }, // 74
			{ GameLayer.getBitmapHandle(L"InGame/flowers/yanggwibi_on.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/yanggwibi.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/yanggwibi_on.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/yanggwibi.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/yanggwibi_on.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/yanggwibi.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/yanggwibi_on.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/yanggwibi.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/flowers/yanggwibi_on.bmp"), 3800, 1200, 0.5, false },
			{ GameLayer.getBitmapHandle(L"InGame/box.bmp"), 900, 1300, 1.3, true }, // 84
			{ GameLayer.getBitmapHandle(L"InGame/human/walking_human1.bmp"), human.X, human.Y, 1.3, human_frame != 0},
			{ GameLayer.getBitmapHandle(L"InGame/human/walking_human2.bmp"), human.X, human.Y, 1.3, human_frame != 1},
			{ GameLayer.getBitmapHandle(L"InGame/human/walking_human3.bmp"), human.X, human.Y, 1.3, human_frame != 2},
			{ GameLayer.getBitmapHandle(L"InGame/human/walking_human4.bmp"), human.X, human.Y, 1.3, human_frame != 3},
			{ GameLayer.getBitmapHandle(L"InGame/result.bmp"), 0, 0, 2.38, true }
		};
		GameLayer.imageCount = 100;

		GameLayer.texts = (Text[]){
			{ L"<space>", 1800, 1800, 30, 40, 600, L"둥근모꼴", RGB(0, 0, 0), true },
			{ L"등교시간이다..", 1150, 1500, 30, 40, 600, L"둥근모꼴", RGB(0, 0, 0), true},
			{ L"학교가기 싫다..", 1150, 1500, 30, 40, 600, L"둥근모꼴", RGB(0, 0, 0), true},
			{ L"뜬금없지만 꽃이나 주워볼까..?", 1150, 1500, 30, 40, 600, L"둥근모꼴", RGB(0, 0, 0), true},
			{ L"[D]:대마초 / [F]:양귀비 / [J]:클로버 / [K]:장미", 1100, 1500, 30, 40, 600, L"둥근모꼴", RGB(0, 0, 0), true},
			{ L"Miss", 500, 700, 45, 60, 600, L"둥근모꼴", RGB(100, 100, 100), true},
			{ L"Bad", 500, 700, 45, 60, 600, L"둥근모꼴", RGB(191, 253, 159), true},
			{ L"Good", 500, 700, 45, 60, 600, L"둥근모꼴", RGB(121, 237, 255), true},
			{ L"Great!", 500, 700, 45, 60, 600, L"둥근모꼴", RGB(250, 199, 232), true},
			{ L"Perfect!!", 500, 700, 45, 60, 600, L"둥근모꼴", RGB(139, 0, 255), true},
			{ L"Score : 0", 3000, 200, 45, 60, 600, L"둥근모꼴", RGB(139, 0, 255), false},
		};
		GameLayer.textCount = 11;

		GamePageFlag = 1;
		_beginthreadex(NULL, 0, (_beginthreadex_proc_type)GameLayerPaintThread, (void*)NULL, 0, NULL);
	}

	switch (PG) {
	case 0:
		WritePG(N, 0);
		Sleep(1000);

		ToggleLayer(&GameLayer, BOX_IDX, BOX_IDX, -1, -1);

		for (int i = 1; i <= 4; i++) {
			Sleep(300);
			ToggleLayer(&GameLayer, -1, -1, i, i);
			Sleep(1000);
			ToggleLayer(&GameLayer, -1, -1, 0, 0);
			while (!key.SPACE);
			ToggleLayer(&GameLayer, -1, -1, i, i);
			ToggleLayer(&GameLayer, -1, -1, 0, 0);
		}

		ToggleLayer(&GameLayer, BOX_IDX, BOX_IDX, -1, -1);

	case 1: // main game
		WritePG(N, 1);
		SoundEffect("Musics/InGameBGM.wav", &InGameMusic, &InGameMusicID, 1, 0, 0);
		SoundEffect("Musics/InGameBGM.wav", &InGameMusic, &InGameMusicID, 0, 1, 0);

		_beginthreadex(NULL, 0, (_beginthreadex_proc_type)HumanThread, (void*)NULL, 0, NULL);
		_beginthreadex(NULL, 0, (_beginthreadex_proc_type)BGThread, (void*)NULL, 0, NULL);
		_beginthreadex(NULL, 0, (_beginthreadex_proc_type)ItemThread, (void*)NULL, 0, NULL);
		Sleep(90000);

	case 2: // game result
		Storage[N] = OpenStorage_i(N, 0);
		int PG;
		fscanf(Storage[N], "%d", &PG);
		printf("%d\n", PG);
		if (PG == 2) {
			fscanf(Storage[N], "%d", &Score);
			fclose(Storage[N]);
			Storage[N] = NULL;
		}
		else {
			fclose(Storage[N]);
			Storage[N] = NULL;
			WritePG(N, 2);
			Storage[N] = OpenStorage_i(N, 2);
			fprintf(Storage[N], "%d\n", Score);
			fclose(Storage[N]);
			Storage[N] = NULL;
		}

		if (STRING != NULL)
			free(STRING);
		STRING = (wchar_t*)malloc(20 * sizeof(wchar_t));
		swprintf(STRING, 20, L"Score : %d", Score);
		GameLayer.texts[10].content = STRING;

		GameLayer.texts[10].isHidden = true;
		GameLayer.images[89].isHidden = false;
		Sleep(1000);

		GameLayer.texts[10].x = 500;
		GameLayer.texts[10].y = 400;
		GameLayer.texts[10].height = 150;
		GameLayer.texts[10].width = 200;
		GameLayer.texts[10].isHidden = false;
		Sleep(1000);

		GameLayer.texts[0].color = RGB(255, 255, 255);
		GameLayer.texts[0].isHidden = false;
		while (!key.SPACE);
	}


	return;
}