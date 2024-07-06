#include <iostream>
#include "util/Memory.h"
#include "util/Logging.h"
#include "util/Hook.h"
#include "checks.h"

#include "Script.h"

GEBool CanProcDisease(Entity& p_victim, gCScriptAdmin& scriptAdmin) {
	return static_cast<GEBool>(scriptAdmin.CallScriptFromScript("CanBeDiseased", &p_victim, &None, 0));
}

GEBool CanProcPoison(Entity& p_victim, gCScriptAdmin& scriptAdmin) {
	return static_cast<GEBool>(scriptAdmin.CallScriptFromScript("CanBePoisoned", &p_victim, &None, 0));

}

GEBool CanProcFreeze(Entity& p_victim) {

	gESpecies VictimSpecies = p_victim.NPC.GetProperty<PSNpc::PropertySpecies>();

	switch (VictimSpecies)
	{
	case gESpecies_Golem:
	case gESpecies_Demon:
	case gESpecies_Troll:
	case gESpecies_FireGolem:
	case gESpecies_IceGolem:
	case gESpecies_Dragon:
		return GEFalse;
	}

	return GETrue;
}

GEBool CanProcBurn(Entity& p_victim)
{
	gESpecies victimSpecies = p_victim.NPC.GetProperty<PSNpc::PropertySpecies>();
	switch (victimSpecies) {
	case gESpecies_Golem:
	case gESpecies_Demon:
	case gESpecies_Troll:
	case gESpecies_FireGolem:
	case gESpecies_IceGolem:
	case gESpecies_Dragon:
		return GEFalse;
	}
	if (p_victim == Entity::GetPlayer() && p_victim.Inventory.IsSkillActive(Template("Perk_ResistHeat")))
		return GEFalse;

	return GETrue;
}

GEBool CanProcSharp(Entity& p_victim) {
	gESpecies victimSpecies = p_victim.NPC.GetProperty<PSNpc::PropertySpecies>();
	switch (victimSpecies) {
	case gESpecies_Golem:
	case gESpecies_FireGolem:
	case gESpecies_IceGolem:
	case gESpecies_Skeleton:
		return GEFalse;
	}
	return GETrue;
}

GEBool CanProcBless(Entity& p_victim, gCScriptAdmin& scriptAdmin) {
	return static_cast<GEBool>(scriptAdmin.CallScriptFromScript("IsEvil", &p_victim, &None, 0));
}

GEBool CanProcForged(Entity& p_victim) {
	return GEFalse;
}
