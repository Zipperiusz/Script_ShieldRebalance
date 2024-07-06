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
	GEInt iFreezeTime;
	GEInt ProcChance = 10;
	GEInt SharpDamage = 10;
	GEInt RandomDisease = randomizer.Random(0, 100);
	GEInt RandomPoison = randomizer.Random(0, 100);
	GEInt RandomBurn = randomizer.Random(0, 100);
	GEInt RandomFreeze = randomizer.Random(0, 100);
	GEInt RandomSharp = randomizer.Random(0, 100);
	GEInt RandomBless = randomizer.Random(0, 100);
	GEInt RandomForged = randomizer.Random(0, 100);

	//Proc original function and get result
	gEAction OriginalResult = Hook_AssessHit.GetOriginalFunction(&AssessHit)(a_pSPU, a_pSelfEntity, a_pOtherEntity, a_iArgs);

	if (Attacker == None && Damager.Navigation.IsValid())
	{
		Attacker = Damager;
	}
	if (Victim != Entity::GetPlayer()) return OriginalResult;
	if (Attacker == Entity::GetPlayer()) return OriginalResult;
	//Check if attack was blocked

	if (OriginalResult == gEAction_None) {
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
				}
				//Diesase enchant
				if (
					RandomDisease < ProcChance &&
					(ShieldQuality & gEItemQuality_Diseased) == gEItemQuality_Diseased &&
					CanProcDisease(Attacker, ScriptAdmin)
					) {
					//It's still WIP need idea what to do

					Attacker.NPC.EnableStatusEffects(gEStatusEffect_Poisoned, GETrue);
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
					iFreezeTime = 5;
					//if (BetterShieldPerk) iFreezeTime = 10;
					Attacker.Routine.SetTask("ZS_Freeze");
					Attacker.Routine.AccessProperty<PSRoutine::PropertyTaskPosition>() = 12 * iFreezeTime;
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
				}
				//Forged enchant
				if (
					RandomForged < ProcChance &&
					(ShieldQuality & gEItemQuality_Forged) == gEItemQuality_Forged &&
					CanProcForged(Attacker)
					) {
					//Dunno what to do
					spy->Send("Wykute moje");
				}
			}
		}
	}
	return OriginalResult;
}




extern "C" __declspec(dllexport)
gSScriptInit const* GE_STDCALL ScriptInit(void)
{
	GetScriptAdmin().LoadScriptDLL("Script_Game.dll");
	GetScriptAdmin().LoadScriptDLL("Script_AssessHit.dll");
	GetScriptAdmin().LoadScriptDLL("ScriptNewBalance.dll");

	if (!GetScriptAdmin().IsScriptDLLLoaded("Script_AssessHit.dll") && !GetScriptAdmin().IsScriptDLLLoaded("Script_NewBalance.dll")) {
		GE_FATAL_ERROR_EX("Script_ShieldRebalance", "Missing Script_AssessHit.dll or Script_NewBalance.dll file.");
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
