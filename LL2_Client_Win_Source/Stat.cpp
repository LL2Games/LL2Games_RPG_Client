#include "Stat.h"

Stat::Stat() : m_base({ 0, }), m_derived({ 0, }), m_expStat({0,}), m_cur_hp(0), m_cur_mp(0), m_remain_ap(0)
{
}

void Stat::SetStat(const BaseStat& base, const DerivedStat& derived, const ExpStat& expStat, int curHp, int curMp, int remainAp)
{
	m_base.str = base.str;
	m_base.dex = base.dex;
	m_base.intel = base.intel;
	m_base.luck = base.luck;

	m_derived.maxHp = derived.maxHp;
	m_derived.maxMp = derived.maxMp;

	m_cur_hp = curHp;
	m_cur_mp = curMp;
	m_remain_ap = remainAp;

	m_expStat.level = expStat.level;
	m_expStat.exp = expStat.exp;
	m_expStat.need_exp = expStat.need_exp;

}

void Stat::SetBaseStat(const BaseStat& base)
{
	m_base.str = base.str;
	m_base.dex = base.dex;
	m_base.intel = base.intel;
	m_base.luck = base.luck;
}

void Stat::SetDerivedStat(const DerivedStat& derived)
{
	m_derived.maxHp = derived.maxHp;
	m_derived.maxMp = derived.maxMp;
}

void Stat::HandleExpGain(const ExpUpdateResult& result)
{
	m_expStat.level = result.newLevel;
	m_expStat.exp = result.curExp;
	m_expStat.need_exp = result.needExp;
}
