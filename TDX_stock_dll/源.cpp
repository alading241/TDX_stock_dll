#include "stdafx.h"
#include "TCalcFuncSets.h"
#include <cassert>
#include <math.h>

//���ɵ�dll���������dll�뿽����ͨ���Ű�װĿ¼��T0002/dlls/����,���ڹ�ʽ���������а�
static int const START_FROM = 13;
typedef enum {
	// �յ�����
	INIT = 0, //�㷨��ʼʱδ֪״̬
	NON_PEAK_BOT = 0, // �� �� �� ֮���K��
	IS_PEAK = 1, //��
	IS_TROUGH = -1  //��
} INFLECTION_POINT;

void TOPRANGE_PERCENT(int DataLen, float* pfOUT, float* HIGH, float* HIGH_ADJUST)
{
	memset(pfOUT, 0, DataLen * sizeof(float));

	for (int i = DataLen - 1; i >= 0; i--)
	{
		float val = HIGH_ADJUST[i];
		int idx = i - 1;
		while (idx >= 0 && val > HIGH[idx])
			idx--;
		pfOUT[i] = i - idx - 1;
	}
}

void LOWRANGE_PERCENT(int DataLen, float* pfOUT, float* LOW, float* LOW_ADJUST)
{
	memset(pfOUT, 0, DataLen * sizeof(float));

	for (int i = DataLen - 1; i >= 0; i--)
	{
		float val = LOW_ADJUST[i];
		int idx = i - 1;
		while (idx >= 0 && val < LOW[idx])
			idx--;
		pfOUT[i] = i - idx - 1;
	}
}

void TROUGH_BARS_JUNXIAN(int DataLen, float* pfOUT, float* VAL, float* JUNXIAN)
{
	// �� ���ơ����� 60�� �����ߣ���Ϊ���ֶ����׵� ��׼�������� ZIG(K,N)���õİ��հٷֱ���ȷ��ת�� 

	memset(pfOUT, -1, DataLen * sizeof(float));

	float curBotVal(0xffffffff), newBotVal(0xffffffff);
	int curBotBars(-1), newBotBars(-1);

	for (int i = START_FROM - 1; i < DataLen; i++)
	{
		static int modify_Done = 0;

		if (VAL[i] <= JUNXIAN[i])
		{
			modify_Done = 0;

			if (VAL[i] < newBotVal)
			{
				newBotVal = VAL[i];
				newBotBars = i;
			}
			if (curBotBars > 0)
			{
				pfOUT[i] = i - curBotBars;
			}
		}
		else
		{
			if (!modify_Done && newBotBars>0)
			{
				for (int j = i; j >= newBotBars; j--)
				{
					pfOUT[j] = j - newBotBars;
				}
				curBotBars = newBotBars;
				curBotVal = newBotVal;
				newBotVal = 0xffffffff;
				newBotBars = -1;

				modify_Done = 1;
			}
			else if (curBotBars >0)
			{
				pfOUT[i] = i - curBotBars;
			}
			//else
			//	assert(curTopBars >0);

		}
	}
}

void PEAK_BARS_JUNXIAN(int DataLen, float* pfOUT, float* VAL, float* JUNXIAN)
{
	// �� ���ơ����� 60�� �����ߣ���Ϊ���ֶ����׵� ��׼�������� ZIG(K,N)���õİ��հٷֱ���ȷ��ת�� 

	memset(pfOUT, -1, DataLen * sizeof(float));

	float curTopVal(-0xffff), newTopVal(-0xffff);
	int curTopBars(-1), newTopBars(-1);

	for (int i = START_FROM -1; i < DataLen; i++)
	{
		static int modify_Done = 0;

		if (VAL[i] >= JUNXIAN[i])
		{
			modify_Done = 0;

			if (VAL[i] > newTopVal)
			{
				newTopVal = VAL[i];
				newTopBars = i;
			}
			if (curTopBars > 0)
			{
				pfOUT[i] = i - curTopBars;
			}
		}
		else
		{
			if (!modify_Done && newTopBars>0)
			{
				for (int j = i; j >= newTopBars; j--)
				{
					pfOUT[j] = j - newTopBars;
				}
				curTopBars = newTopBars;
				curTopVal = newTopVal;
				newTopVal = -0xffff;
				newTopBars = -1;

				modify_Done = 1;
			}
			else if (curTopBars >0)
			{
				pfOUT[i] = i - curTopBars;
			}
			//else
			//	assert(curTopBars >0);

		}
	}
}

void TROUGH_BARS_ZIG(int DataLen, float* pfOUT, float* VAL, float* _ZIG_PERCENT, float* criticalPoint)
{
	// criticalPoint��ָ�� �ж�ZIG�յ� ���Ǹ����ߵ㣬Ҳ���Ǳȹյ� Ҫ��һ����Ǹ�λ�ã�ʵ����5����Ĺյ����
	bool criticalPnt = ((int)*criticalPoint == 1);
	float ZIG_PERCENT = *_ZIG_PERCENT ? *_ZIG_PERCENT : 5;

	memset(pfOUT, -1, DataLen * sizeof(float));

	float possibleBot(VAL[0]), possibleTop(VAL[0]);
	int posBotBars(0), posTopBars(0);
	int curBotBars(-1), curTopBars(-1);

	enum {UNSURE, SEARCHING_TOP, SEARCHING_BOT} goal = UNSURE;

	for (int i = 1; i < DataLen; i++)
	{
		static int modify_Done = 0;

		if (goal == UNSURE)
		{
			if (VAL[i] < possibleBot)
			{
				possibleBot = VAL[i];
				posBotBars = i;
			}
			else if (VAL[i] >= possibleBot*(1 + ZIG_PERCENT / 100))
			{
				curBotBars = posBotBars;
				posBotBars = -1;
				possibleTop = VAL[i];
				posTopBars = i;
				goal = SEARCHING_TOP;
				continue;
			}


			if (VAL[i] > possibleTop)
			{
				possibleTop = VAL[i];
				posTopBars = i;
			}
			else if (VAL[i] <= possibleTop*(1 - ZIG_PERCENT / 100))
			{
				curTopBars = posTopBars;
				posTopBars = -1;
				possibleBot = VAL[i];
				posBotBars = i;
				goal = SEARCHING_BOT;
				continue;
			}
		}
		else if (goal == SEARCHING_TOP)
		{
			/*
			ÿ��ȷ���˵ײ��󣨵�ǰλ�ý�֮ǰ�Ǹ��͵�posBotBars���� ZIG_PERCENT)����posBotBars��ΪcurBotBars�� Ȼ��Ϳ�ʼSEARCHING_TOP��
			SEARCHING_TOP�޷����������
			(1) ���ϴ����¸�(����û�лس�����ZIG_PERCENT)�����ϸ���posTopBars
			(2) �����˻س������posTopBars�ﵽ��ZIG_PERCENT����ʱposTopBars��ΪcurTopBars��Ȼ��ͽ�����SEARCHING_BOT��
			*/
			if (VAL[i] > possibleTop)
			{
				posTopBars = i;
				possibleTop = VAL[i];
				if (!criticalPnt)
					pfOUT[i] = i - curBotBars;
			}
			else 
			{
				if (!criticalPnt)
					pfOUT[i] = i - curBotBars;
				if (VAL[i] <= possibleTop*(1 - ZIG_PERCENT / 100))
				{
					curTopBars = posTopBars;
					posTopBars = -1;
					posBotBars = i;
					possibleBot = VAL[i];
					goal = SEARCHING_BOT;
					continue;
				}
			}
		}
		else
		{ // SEARCHING_BOT
			if (VAL[i] < possibleBot)
			{
				posBotBars = i;
				possibleBot = VAL[i];
				if (!criticalPnt)
					pfOUT[i] = i - curBotBars;
			}
			else
			{
				
				if (VAL[i] >= possibleBot*(1 + ZIG_PERCENT / 100))
				{
					if (criticalPnt)
						pfOUT[i] = 0;
					else 
					{
						for (int j = i; j >= posBotBars; j--)
						{
							pfOUT[j] = j - posBotBars;
						}
					}

					curBotBars = posBotBars;
					posBotBars = -1;
					possibleTop = VAL[i];
					posTopBars = i;
					goal = SEARCHING_TOP;
					continue;
				}
				else
				{
					if (!criticalPnt)
						pfOUT[i] = i - curBotBars;
				}

			}
		}
	}
}


void PEAK_BARS_ZIG(int DataLen, float* pfOUT, float* VAL, float* _ZIG_PERCENT, float* criticalPoint)
{
	// criticalPoint��ָ�� �ж�ZIG�յ� ���Ǹ����ߵ㣬Ҳ���Ǳȹյ� Ҫ��һ����Ǹ�λ�ã�ʵ����5����Ĺյ����
	bool criticalPnt = ((int)*criticalPoint == 1);
	float ZIG_PERCENT = *_ZIG_PERCENT ? *_ZIG_PERCENT : 5;


	memset(pfOUT, -1, DataLen * sizeof(float));

	float possibleBot(VAL[0]), possibleTop(VAL[0]);
	int posBotBars(0), posTopBars(0);
	int curBotBars(-1), curTopBars(-1);

	enum { UNSURE, SEARCHING_TOP, SEARCHING_BOT } goal = UNSURE;

	for (int i = 1; i < DataLen; i++)
	{
		static int modify_Done = 0;

		if (goal == UNSURE)
		{
			if (VAL[i] < possibleBot)
			{
				possibleBot = VAL[i];
				posBotBars = i;
			}
			else if (VAL[i] >= possibleBot*(1 + ZIG_PERCENT / 100))
			{
				curBotBars = posBotBars;
				posBotBars = -1;
				possibleTop = VAL[i];
				posTopBars = i;
				goal = SEARCHING_TOP;
				continue;
			}


			if (VAL[i] > possibleTop)
			{
				possibleTop = VAL[i];
				posTopBars = i;
			}
			else if (VAL[i] <= possibleTop*(1 - ZIG_PERCENT / 100))
			{
				curTopBars = posTopBars;
				posTopBars = -1;
				possibleBot = VAL[i];
				posBotBars = i;
				goal = SEARCHING_BOT;
				continue;
			}
		}
		else if (goal == SEARCHING_TOP)
		{
			if (VAL[i] > possibleTop)
			{
				posTopBars = i;
				possibleTop = VAL[i];
				if (!criticalPnt)
					pfOUT[i] = i - curTopBars;
			}
			else
			{
				if (VAL[i] <= possibleTop*(1 - ZIG_PERCENT / 100))
				{
					if (criticalPnt)
						pfOUT[i] = 0;
					else
					{
						for (int j = i; j >= posTopBars; j--)
						{
							pfOUT[j] = j - posTopBars;
						}
					}

					curTopBars = posTopBars;
					posTopBars = -1;
					posBotBars = i;
					possibleBot = VAL[i];
					goal = SEARCHING_BOT;
					continue;
				}
				else 
				{
					if (!criticalPnt)
						pfOUT[i] = i - curTopBars;
				}
			}
		}
		else
		{ // SEARCHING_BOT
			if (!criticalPnt)
				pfOUT[i] = i - curTopBars;
			if (VAL[i] < possibleBot)
			{
				posBotBars = i;
				possibleBot = VAL[i];
			}
			else
			{
				if (VAL[i] >= possibleBot*(1 + ZIG_PERCENT / 100))
				{
					curBotBars = posBotBars;
					posBotBars = -1;
					possibleTop = VAL[i];
					posTopBars = i;
					goal = SEARCHING_TOP;
					continue;
				}

			}
		}
	}
}

static INFLECTION_POINT PEAK_TROUGH_BARS_KCOUNT_STEP1(int DataLen, float* pfOUT, float* HIGH, float* LOW, float* _decidePoint)
{
	/*
	������ͨ��pfOUT��������������ṩ��
	  ��ǰ�㵽��һ���յ㣨���� ���� ���ȣ��ľ��루k����)��
	  �����һ���յ��ǲ��壬������Ǹ������������µģ�
	  �����һ���յ��ǲ��ȣ���������������������ϵ�)
	����������ֵ����һ��bar���ǲ��� ���� ���ȣ�
	*/

	// criticalPoint��ָ�� �ж�ZIG�յ� ���Ǹ����ߵ㣬Ҳ���Ǳȹյ� Ҫ��һ����Ǹ�λ�ã�ʵ����5����Ĺյ����

	memset(pfOUT, 0, DataLen * sizeof(float));
	bool outputDecidePoint = (int)*_decidePoint != 0;

	float possibleBot(LOW[0]), possibleTop(HIGH[0]);
	int posBotBars(0), posTopBars(0);  // pos : possible
	int curBotBars(-1), curTopBars(-1); // cur : current

	enum { UNSURE, SEARCHING_TOP, SEARCHING_BOT } goal = UNSURE;
	INFLECTION_POINT lastStatus = INIT, firstBarIs = INIT;

	for (int i = 1; i < DataLen; i++)
	{
		if (goal == UNSURE)
		{
			if (LOW[i] <= possibleBot)
			{
				if (HIGH[i] < possibleTop)
				{
					if (outputDecidePoint)
						pfOUT[i] = 0;
					else if (lastStatus != IS_PEAK)
					{
						for (int j = posTopBars - 1; j > curBotBars; j--)
						{
							pfOUT[j] = j - curBotBars;
						}
						curTopBars = posTopBars;
						posTopBars = -1;
						lastStatus = IS_PEAK;
						if (firstBarIs == INIT)
							firstBarIs = IS_PEAK;
					}

					posBotBars = i;
					possibleBot = LOW[i];
					possibleTop = HIGH[i];
					goal = SEARCHING_BOT;
				}
				else {
					possibleBot = LOW[i];
					possibleTop = HIGH[i];
					posTopBars = posBotBars = i;
				}
			}
			else if (HIGH[i] > possibleTop)
			{
				if (outputDecidePoint)
					pfOUT[i] = 0;
				else if (lastStatus != IS_TROUGH)
				{
					for (int j = posBotBars - 1; j > curTopBars; j--)
					{
						pfOUT[j] = -(j - curTopBars);
					}
					curBotBars = posBotBars;
					posBotBars = -1;
					lastStatus = IS_TROUGH;
					if (firstBarIs == INIT)
						firstBarIs = IS_TROUGH;
				}
				posTopBars = i;
				possibleBot = LOW[i];
				possibleTop = HIGH[i];
				goal = SEARCHING_TOP;
				continue;
			}else
				continue;
		}
		else if (goal == SEARCHING_TOP)
		{
			if (HIGH[i] >= possibleTop)
			{
				if (LOW[i] <= possibleBot) {
					// ���ǰ
				}
				else
				{
					possibleBot = LOW[i];				
				}
				possibleTop = HIGH[i];
				posTopBars = i;
			}
			else if (LOW[i] < possibleBot)
			{
				if (outputDecidePoint)
					pfOUT[i] = 0;
				else
				{
					for (int j = posTopBars - 1; j > curBotBars; j--)
					{
						pfOUT[j] = j - curBotBars;
					}
				}

				curTopBars = posTopBars;
				posTopBars = -1;
				posBotBars = i;
				possibleBot = LOW[i];
				possibleTop = HIGH[i];
				goal = SEARCHING_BOT;
				lastStatus = IS_PEAK;
			}
			else
			{
				// ǰ���󣬴��������ϵ
				possibleBot = LOW[i];
			}
		}
		else
		{
			if (LOW[i] <= possibleBot)
			{
				if (HIGH[i] >= possibleTop)
				{
					// ���ǰ�����������ϵ
				}
				else
				{
					possibleTop = HIGH[i];
				}
				possibleBot = LOW[i];
				posBotBars = i;
			}
			else if (HIGH[i] > possibleTop)
			{
				if (outputDecidePoint)
					pfOUT[i] = 0;
				else
				{
					for (int j = posBotBars - 1; j > curTopBars; j--)
					{
						pfOUT[j] = -(j - curTopBars);
					}
				}

				curBotBars = posBotBars;
				posBotBars = -1;
				posTopBars = i;
				possibleBot = LOW[i];
				possibleTop = HIGH[i];
				goal = SEARCHING_TOP;
				lastStatus = IS_TROUGH;
			}
			else
			{
				// ǰ����
				possibleTop = HIGH[i];
			}

		}
	}
	return firstBarIs;
}

void PEAK_TROUGH_BARS_KCOUNT_STEP2(int DataLen, float* pfOUT, float* HIGH, float* LOW, float* _KCOUNT_AND_decidePoint)
{
	/*
	������ͨ��pfOUT������߷��أ� �����յ�
	*/
	memset(pfOUT, 0, DataLen * sizeof(float));

	// criticalPoint��ָ�� �ж�ZIG�յ� ���Ǹ����ߵ㣬Ҳ���Ǳȹյ� Ҫ��һ����Ǹ�λ�ã�ʵ����5����Ĺյ����
	float PERCENT = 20;
	float decidePoint = 0;

	decidePoint = ((int)*_KCOUNT_AND_decidePoint < 0) ? 1 : 0;
	int KCOUNT = abs((int)*_KCOUNT_AND_decidePoint) ? abs((int)*_KCOUNT_AND_decidePoint) : 5;

	float*  peaks_bottoms_distance = new float[DataLen];

	INFLECTION_POINT firstBarIs = PEAK_TROUGH_BARS_KCOUNT_STEP1(DataLen, peaks_bottoms_distance, HIGH, LOW, &decidePoint);
	INFLECTION_POINT alternate = firstBarIs;
	for (int i = 0; i < DataLen; i++)
	{
		if (peaks_bottoms_distance[i] != 0)
			pfOUT[i] = NON_PEAK_BOT;
		else {
			pfOUT[i] = alternate;
			alternate = (alternate == IS_PEAK) ? IS_TROUGH : IS_PEAK;
		}
	}
	delete[] peaks_bottoms_distance;
}

void PEAK_TROUGH_BARS_KCOUNT_STEP3(int DataLen, float* pfOUT, float* HIGH, float* LOW, float* _ZIG_PERCENT)
{
	memset(pfOUT, NON_PEAK_BOT, DataLen * sizeof(float));

	float *peaks_and_bottoms = new float[DataLen];
	float KCOUNT_and_DecidePoint = 5;
	float ZIG_PERCENT = *_ZIG_PERCENT ? *_ZIG_PERCENT : 2;

	PEAK_TROUGH_BARS_KCOUNT_STEP2(DataLen, peaks_and_bottoms, HIGH, LOW, &KCOUNT_and_DecidePoint);
	
	float possibleBot(peaks_and_bottoms[0] == IS_TROUGH ? LOW[0] : HIGH[0]);
	float possibleTop(peaks_and_bottoms[0] == IS_PEAK ? HIGH[0] : LOW[0]);
	int posBotBars(peaks_and_bottoms[0] == IS_TROUGH ? 0 : -1); // pos: possible
	int posTopBars(peaks_and_bottoms[0] == IS_PEAK ? 0:-1); // pos: possible
	int curBotBars(-1), curTopBars(-1); // cur: current
	INFLECTION_POINT  lastStatus = INIT;

	enum { UNSURE, SEARCHING_TOP, SEARCHING_BOT } goal = UNSURE;

	for (int i = 1; i < DataLen; i++)
	{
		if (peaks_and_bottoms[i] == NON_PEAK_BOT) {
			pfOUT[i] = NON_PEAK_BOT;
			continue;
		}

		if (goal == UNSURE)
		{
			if (peaks_and_bottoms[i] == IS_PEAK)
			{
				if (possibleBot*(1 + ZIG_PERCENT / 100) <= HIGH[i])
				{
					curBotBars = posBotBars;
					posBotBars = -1;
					possibleTop = HIGH[i];
					posTopBars = i;
					goal = SEARCHING_TOP;
					pfOUT[curBotBars] = IS_TROUGH;
					lastStatus = IS_TROUGH;
				}
				else if (HIGH[i] > possibleTop) {
					posTopBars = i;
					possibleTop = HIGH[i];
				}
			}
			else {
				//if (LOW[i] <= possibleTop*(1 - ZIG_PERCENT / 100))
				if (LOW[i]* (1 + ZIG_PERCENT / 100) <= possibleTop)
				{
					curTopBars = posTopBars;
					posTopBars = -1;
					possibleBot = LOW[i];
					posBotBars = i;
					goal = SEARCHING_BOT;
					pfOUT[curTopBars] = IS_PEAK;
					lastStatus = IS_PEAK;
				}
				else if (LOW[i] < possibleBot)
				{
					posBotBars = i;
					possibleBot = LOW[i];
				}
			}
		}
		else if (goal == SEARCHING_TOP)
		{
			if (peaks_and_bottoms[i] == IS_PEAK) {
				if (HIGH[i] > possibleTop) {
					possibleTop = HIGH[i];
					posTopBars = i;
				}
			}
			else {
				//if (LOW[i] <= possibleTop*(1 - ZIG_PERCENT / 100)) {
				if (LOW[i]* (1 + ZIG_PERCENT / 100) <= possibleTop) {
					curTopBars = posTopBars;
					posTopBars = -1;
					posBotBars = i;
					possibleBot = LOW[i];
					goal = SEARCHING_BOT;
					pfOUT[curTopBars] = IS_PEAK;
					lastStatus = IS_PEAK;
				}
				else if (LOW[i] < LOW[curBotBars]) {
					/* 
					TODO:
					������õ��ǣ�if (LOW[i] <= possibleTop*(1 - ZIG_PERCENT / 100)) ����ܳ���LOW[i] < LOW[curBotBars]
					������õ��ǣ�if (LOW[i]* (1 + ZIG_PERCENT / 100) <= possibleTop)���򲻻���� LOW[i] < LOW[curBotBars]
					*/
				}
			}
		}
		else
		{ // SEARCHING_BOT
			if (peaks_and_bottoms[i] == IS_TROUGH) {
				if (LOW[i] < possibleBot) {
					possibleBot = LOW[i];
					posBotBars = i;
				}
			}
			else {
				if (possibleBot*(1 + ZIG_PERCENT / 100) <= HIGH[i]) {
					curBotBars = posBotBars;
					posBotBars = -1;
					posTopBars = i;
					possibleTop = HIGH[i];
					goal = SEARCHING_TOP;
					pfOUT[curBotBars] = IS_TROUGH;
					lastStatus = IS_TROUGH;
				}
			}
		}
	}

	if (lastStatus == IS_TROUGH)
	{
		pfOUT[posTopBars] = IS_PEAK;
	}
	else if (lastStatus == IS_PEAK)
	{
		pfOUT[posBotBars] = IS_TROUGH;
	}

	delete[] peaks_and_bottoms;
}

static void merge(int DataLen, float* pfOUT, float* VAL1, float* VAL2)
{
	for (int i = 0; i < DataLen; i++)
	{
		if (VAL1[i] == 0 && VAL2[i] == 0)
		{
			pfOUT[i] = 1;
		}
		else
		{
			pfOUT[i] = 0;
		}
	}
}

void FENBI(int DataLen, float* pfOUT, float* LOW, float* HIGH, float* JUNXIAN)
{
	float inflectionPoint = 0;
	float ZIG_PERCENT = 5;

	float* MID = new float[DataLen] {0};
	for (int i = 0; i < DataLen; i++)
	{
		MID[i] = (LOW[i] + HIGH[i]) / 2;
	}

	float* peaks_junxian = new float[DataLen] {0};
	float* peaks_zig = new float[DataLen] {0};
	float* peaks = new float[DataLen] {0};

	PEAK_BARS_ZIG(DataLen, peaks_zig, MID, &ZIG_PERCENT, &inflectionPoint);
	PEAK_BARS_JUNXIAN(DataLen, peaks_junxian, MID, JUNXIAN);
	merge(DataLen, peaks, peaks_zig, peaks_junxian);

	float* trough_junxian = new float[DataLen] {0};
	float* trough_zig = new float[DataLen] {0};
	float* troughs = new float[DataLen] {0};

	TROUGH_BARS_ZIG(DataLen, trough_zig, MID, &ZIG_PERCENT, &inflectionPoint);
	TROUGH_BARS_JUNXIAN(DataLen, trough_junxian, MID, JUNXIAN);
	merge(DataLen, troughs, trough_zig, trough_junxian);

	memset(pfOUT, 0, DataLen * sizeof(float));
	for (int i = 0; i < DataLen; i++)
	{
		assert(((int)peaks[i] & (int)troughs[i]) == 0);
		pfOUT[i] = (peaks[i] ? 1 : (troughs[i] ? -1 : 0));
		
	}


	delete[] peaks, peaks_zig, peaks_junxian;
	delete[] troughs, trough_zig, trough_junxian;
	delete[] MID;
}

void DEBUG_BARS_JUNXIAN(int DataLen, float* pfOUT, float* LOW, float* HIGH, float* JUNXIAN)
{
	float* MID = new float[DataLen] {0};
	for (int i = 0; i < DataLen; i++)
	{
		MID[i] = (LOW[i] + HIGH[i]) / 2;
	}

	float* peaks_junxian = new float[DataLen] {0};
	float* trough_junxian = new float[DataLen] {0};

	PEAK_BARS_JUNXIAN(DataLen, peaks_junxian, MID, JUNXIAN);
	TROUGH_BARS_JUNXIAN(DataLen, trough_junxian, MID, JUNXIAN);

	memset(pfOUT, 0, DataLen * sizeof(float));
	for (int i = 0; i < DataLen; i++)
	{
		if (peaks_junxian[i] == 0)
			assert(trough_junxian[i] != 0);
		if (trough_junxian[i] == 0)
			assert(peaks_junxian[i] != 0);

		pfOUT[i] = (peaks_junxian[i] == 0 ? 1 : (trough_junxian[i] == 0 ? -1 : 0));
	}

	delete[] MID, peaks_junxian, trough_junxian;

}
void DEBUG_FENBI(int DataLen, float* pfOUT, float* LOW, float* HIGH, float* JUNXIAN)
{
	float* peaks_junxian = new float[DataLen] {0};
	float* trough_junxian = new float[DataLen] {0};

	PEAK_BARS_JUNXIAN(DataLen, peaks_junxian, HIGH, JUNXIAN);
	TROUGH_BARS_JUNXIAN(DataLen, trough_junxian, LOW, JUNXIAN);

	memset(pfOUT, 0, DataLen * sizeof(float));
	for (int i = 0; i < DataLen; i++)
	{
		if (peaks_junxian[i] == 0)
			assert(trough_junxian[i] != 0);
		if (trough_junxian[i] == 0)
			assert(peaks_junxian[i] != 0);

		pfOUT[i] = (peaks_junxian[i] == 0 ? 1 : (trough_junxian[i] == 0 ? -1 : 0));
	}

	delete[]  peaks_junxian, trough_junxian;

}

void DEBUG_BARS_ZIG(int DataLen, float* pfOUT, float* VAL, float* _ZIG_PERCENT, float* criticalPoint)
{
	float* peaks_zig = new float[DataLen] {0};
	float* trough_zig = new float[DataLen] {0};

	PEAK_BARS_ZIG(DataLen, peaks_zig, VAL, _ZIG_PERCENT, criticalPoint);
	TROUGH_BARS_ZIG(DataLen, trough_zig, VAL, _ZIG_PERCENT, criticalPoint);

	memset(pfOUT, 0, DataLen * sizeof(float));
	for (int i = 0; i < DataLen; i++)
	{
		pfOUT[i] = (peaks_zig[i] == 0 ? 1 : (trough_zig[i] == 0 ? -1 : 0));
	}


	delete[] peaks_zig, trough_zig;
}

//���صĺ���
PluginTCalcFuncInfo g_CalcFuncSets[] =
{
	{ 1,(pPluginFUNC)&TOPRANGE_PERCENT },
	{ 2,(pPluginFUNC)&LOWRANGE_PERCENT },

	{ 3,(pPluginFUNC)&PEAK_BARS_JUNXIAN },
	{ 4,(pPluginFUNC)&TROUGH_BARS_JUNXIAN },

	{ 5,(pPluginFUNC)&PEAK_BARS_ZIG },
	{ 6,(pPluginFUNC)&TROUGH_BARS_ZIG },

	{ 7,(pPluginFUNC)&FENBI },

	{ 8,(pPluginFUNC)&DEBUG_BARS_JUNXIAN },
	{ 9,(pPluginFUNC)&DEBUG_FENBI },
	{ 10,(pPluginFUNC)&DEBUG_BARS_ZIG },

	{ 11,(pPluginFUNC)&PEAK_TROUGH_BARS_KCOUNT_STEP2 },
	{ 12,(pPluginFUNC)&PEAK_TROUGH_BARS_KCOUNT_STEP3 },
	

	{ 0,NULL },
};

//������TCalc��ע�ắ��
BOOL RegisterTdxFunc(PluginTCalcFuncInfo** pFun)
{
	if (*pFun == NULL)
	{
		(*pFun) = g_CalcFuncSets;
		return TRUE;
	}
	return FALSE;
}
