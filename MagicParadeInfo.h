#pragma once
#include "util/Memory.h"
#include "util/ScriptUtil.h"

class MagicParadeInfo {
public:
	GEBool ParadeSpell;
	bCString Spell;
	Entity Attacker;
	Entity Defender;


	void CastSpell();
};



