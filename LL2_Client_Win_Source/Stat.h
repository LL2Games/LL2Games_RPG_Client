#pragma once
#include "CommonInclude.h"
#include "playerInfo.h"
#include "PacketData.h"

class Stat
{
public:
	Stat();
	~Stat() {};

public:
	
	void SetStat(const BaseStat& base, const DerivedStat& derived, const ExpStat& expStat, int curHp, int curMp, int remainAp);

	void SetBaseStat(const BaseStat& base);
	void SetDerivedStat(const DerivedStat& derived);

	void SetCurHp(int cur_hp) { m_cur_hp = cur_hp; }
	void SetCurMp(int cur_mp) { m_cur_mp = cur_mp; }
	void SetRemainAp(int remainAp) { m_remain_ap = remainAp; }
	void SetExp(uint64_t exp) { m_expStat.exp = exp; }
	void SetNeedExp(uint64_t needExp) { m_expStat.need_exp = needExp; }
	void SetLevel(int level) { m_expStat.level = level; }

	void HandleExpGain(const ExpUpdateResult& result);

	int GetCurHp() { return m_cur_hp; }
	int GetMaxHp() { return m_derived.maxHp; }
	int GetCurMp() { return m_cur_mp; }
	int GetMaxMp() { return m_derived.maxMp; }

	int GetCurHp() const { return m_cur_hp; }
	int GetMaxHp() const { return m_derived.maxHp; }
	int GetCurMp() const { return m_cur_mp; }
	int GetMaxMp() const { return m_derived.maxMp; }

	int GetStr() const { return m_base.str; }
	int GetDex() const { return m_base.dex; }
	int GetInt() const { return m_base.intel; }
	int GetLuck() const { return m_base.luck; }

	int GetRemainAP() const { return m_remain_ap; }

	int GetLevel() { return m_expStat.level; }
	int64_t GetExp() { return m_expStat.exp; }
	int64_t GetNeedExp() { return m_expStat.need_exp; }
private:

	BaseStat m_base;
	DerivedStat m_derived;
	ExpStat m_expStat;
	int m_cur_hp;
	int m_cur_mp;
	int m_remain_ap;
	



	
};

