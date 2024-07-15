#include "Script_ShieldRebalance.h"

#include "util\Memory.h"
#include "util\Logging.h"
#include "util\Hook.h"
#include "util/Util.h"
#include "util/ScriptUtil.h"
#include "checks.h"
#include "utility.h"
#include "zSpy.h"
#include "Script.h"

gSScriptInit& GetScriptInit()
{
	static gSScriptInit s_ScriptInit;
	return s_ScriptInit;
}

static mCFunctionHook Hook_AssessHit;
gEAction GE_STDCALL AssessHit(gCScriptProcessingUnit* a_pSPU, Entity* a_pSelfEntity, Entity* a_pOtherEntity, GEU32 a_iArgs) {
	INIT_SCRIPT_EXT(Victim, Damager);
	gCScriptAdmin& ScriptAdmin = GetScriptAdmin();
	Entity Attacker = Damager.Interaction.GetOwner();
	GEBool BetterShieldPerk = Victim.Inventory.IsSkillActive("Perk_Shield_2");
	GEInt FreezeTime;
	GEInt ProcChance;
	GEInt SharpDamage;
	GEInt EvilDamage;
	GEInt RandomDisease = randomizer.Random(0, 100);
	GEInt RandomPoison = randomizer.Random(0, 100);
	GEInt RandomBurn = randomizer.Random(0, 100);
	GEInt RandomFreeze = randomizer.Random(0, 100);
	GEInt RandomSharp = randomizer.Random(0, 100);
	GEInt RandomBless = randomizer.Random(0, 100);
	GEInt RandomForged = randomizer.Random(0, 100);

	switch (Entity::GetCurrentDifficulty()) {
	case EDifficulty_Easy:
		FreezeTime = 10;
		ProcChance = 25;
		SharpDamage = 20;
		EvilDamage = 100;
		break;
	case EDifficulty_Hard:
		FreezeTime = 4;
		ProcChance = 10;
		SharpDamage = 10;
		EvilDamage = 50;
		break;
	default:
		FreezeTime = 7;
		ProcChance = 15;
		SharpDamage = 15;
		EvilDamage = 75;
		break;
	}

	//Proc original function and get result
	gEAction OriginalResult = Hook_AssessHit.GetOriginalFunction(&AssessHit)(a_pSPU, a_pSelfEntity, a_pOtherEntity, a_iArgs);

	if (Attacker == None && Damager.Navigation.IsValid())
	{
		Attacker = Damager;
	}
	if (Victim != Entity::GetPlayer() || Attacker == Entity::GetPlayer()) return OriginalResult;
	//Check if attack was blocked	
	if (IsParade(OriginalResult)) {
		//Check if player has shield in left hand and 1H in right
		if (CheckHandUseTypes(gEUseType_Shield, gEUseType_1H, Victim)) {
			GEU32 ShieldQuality = Victim.Inventory.GetItemFromSlot(gESlot_LeftHand).Item.GetQuality();
			//Check if there are any enchants on shield.
			if (ShieldQuality > 0) {
				//Take enchants from shield
				//Shield perk level 2 increases proc chance and sharp damage
				if (BetterShieldPerk) {
					ProcChance *= 1.5f;
					SharpDamage *= 1.5f;
					FreezeTime *= 1.5f;
				}
				//Diesase enchant
				if (
					RandomDisease < ProcChance &&
					(ShieldQuality & gEItemQuality_Diseased) == gEItemQuality_Diseased &&
					CanProcDisease(Attacker, ScriptAdmin)
					) {
					Attacker.NPC.EnableStatusEffects(gEStatusEffect_Diseased, GETrue);
				}
				//Poison enchant
				if (
					RandomPoison < ProcChance &&
					(ShieldQuality & gEItemQuality_Poisoned) == gEItemQuality_Poisoned &&
					CanProcPoison(Attacker, ScriptAdmin)
					) {
					Attacker.NPC.EnableStatusEffects(gEStatusEffect_Poisoned, GETrue);
				}
				//Burn enchant
				if (
					RandomBurn < ProcChance &&
					(ShieldQuality & gEItemQuality_Burning) == gEItemQuality_Burning &&
					CanProcBurn(Attacker)
					) {
					Attacker.NPC.EnableStatusEffects(gEStatusEffect_Burning, GETrue);
					Attacker.Effect.StartRuntimeEffect("eff_magic_firespell_target_01");
				}
				//Freeze enchant
				if (
					RandomFreeze < ProcChance &&
					(ShieldQuality & gEItemQuality_Frozen) == gEItemQuality_Frozen &&
					CanProcFreeze(Attacker)
					) {
					Attacker.Routine.SetTask("ZS_Freeze");
					Attacker.Routine.AccessProperty<PSRoutine::PropertyTaskPosition>() = 12 * FreezeTime;
				}
				//Sharp enchant 
				if (
					RandomSharp < ProcChance &&
					(ShieldQuality & gEItemQuality_Sharp) == gEItemQuality_Sharp &&
					CanProcSharp(Attacker)
					) {
					ScriptAdmin.CallScriptFromScript("AddHitPoints", &Attacker, &None, -SharpDamage);
					Attacker.NPC.SetLastAttacker(Victim);
					if (ScriptAdmin.CallScriptFromScript("IsHumanoid", &Attacker, &None, 0)) {
						if (ScriptAdmin.CallScriptFromScript("GetHitPoints", &Attacker, &None, 0) <= 0) {
							if (ScriptAdmin.CallScriptFromScript("GetAttitude", &Attacker, &Victim, 0) == gEAttitude_Hostile ||
								ScriptAdmin.CallScriptFromScript("GetAttitude", &Attacker, &Victim, 0) == gEAttitude_Panic) {
								Attacker.Routine.FullStop();
								Attacker.Routine.SetTask("ZS_RagDollDead");
								return OriginalResult;
							} else {
								Attacker.Routine.FullStop();
								Attacker.Routine.SetTask("ZS_Unconscious");
								return OriginalResult;
							}
						} else {
							Attacker.Routine.FullStop();
							Attacker.Routine.SetTask("ZS_PierceStumble");
						}
					} else {
						if (ScriptAdmin.CallScriptFromScript("GetHitPoints", &Attacker, &None, 0) <= 0) {
							Attacker.Routine.SetTask("ZS_RagDollDead");
							return OriginalResult;
						}
					}
				}
				//Bless enchant
				if (
					RandomBless < ProcChance &&
					(ShieldQuality & gEItemQuality_Blessed) == gEItemQuality_Blessed &&
					CanProcBless(Attacker, ScriptAdmin)
					) {
					// Smash enemy with banish evil spell
					spy->Send("Pomiot beliara");
					EffectSystem::StartEffect("eff_magic_destroyevil_impact_01", Attacker);
					ScriptAdmin.CallScriptFromScript("AddHitPoints", &Attacker, &None, -EvilDamage);
					Attacker.NPC.SetLastAttacker(Victim);
					if (ScriptAdmin.CallScriptFromScript("GetHitPoints", &Attacker, &None, 0) <= 0) {
						Attacker.Routine.SetTask("ZS_RagDollDead");
						return OriginalResult;
					}

				}
				//Forged enchant
				/*if (
					RandomForged < ProcChance &&
					(ShieldQuality & gEItemQuality_Forged) == gEItemQuality_Forged) {
				}*/
			}
		}
	}
	return OriginalResult;
}




extern "C" __declspec(dllexport)
gSScriptInit const* GE_STDCALL ScriptInit(void)
{
	GetScriptAdmin().LoadScriptDLL("Script_Game.dll");
	GetScriptAdmin().LoadScriptDLL("Script_NewBalance.dll");

	if (!GetScriptAdmin().IsScriptDLLLoaded("Script_NewBalance.dll")) {
		GE_FATAL_ERROR_EX("Script_ShieldRebalance", "Missing Script_NewBalance.dll.");
	}
	Hook_AssessHit.Hook(GetScriptAdminExt().GetScript("AssessHit")->m_funcScript, &AssessHit, mCBaseHook::mEHookType_OnlyStack);
	spy = new zSpy();
	spy->Send("ShieldRebalance script loaded.");
	return &GetScriptInit();
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		::DisableThreadLibraryCalls(hModule);
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
