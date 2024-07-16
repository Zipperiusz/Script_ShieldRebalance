#include "Script_ShieldRebalance.h"

#include "util\Memory.h"
#include "util\Logging.h"
#include "util\Hook.h"
#include "util/Util.h"
#include "util/ScriptUtil.h"
#include "checks.h"
#include "utility.h"
#include "Script.h"
Entity damager;
void castSpell() {
	Entity Player = Entity::GetPlayer();
	Entity Target = Player.NPC.GetCurrentTarget();
	bCMatrix Pose = Player.GetPose();
	Pose.AccessTranslation().AccessY() += 130;
	Entity Spell = Template(spellInfo->spell);

	Entity Spawn = Spell.Magic.GetSpawn();
	Entity SpawnedEntity = Spawn.Spawn(Spawn.GetTemplate(), Pose);

	SpawnedEntity.Interaction.SetOwner(Player);
	SpawnedEntity.Interaction.SetSpell(Spell);

	SpawnedEntity.EnableCollisionWith(SpawnedEntity, GEFalse);
	SpawnedEntity.EnableCollisionWith(Player, GEFalse);
	bCVector Vec = Pose.AccessZAxis();
	Vec.AccessY() += 0.01;
	Vec.Normalize();
	SpawnedEntity.CollisionShape.CreateShape(eECollisionShapeType_Point, eEShapeGroup_Projectile, bCVector(0, 0, 0), bCVector(0, 0, 0));

	SpawnedEntity.Projectile.SetTarget(Target);
	SpawnedEntity.Projectile.AccessProperty<PSProjectile::PropertyPathStyle>() = gEProjectilePath_Missile;

	SpawnedEntity.Projectile.AccessProperty<PSProjectile::PropertyTargetDirection>() = Vec;
	SpawnedEntity.Damage.AccessProperty<PSDamage::PropertyManaUsed>() = Spell.Magic.GetProperty<PSMagic::PropertyMaxManaCost>();
	SpawnedEntity.Projectile.Shoot();

}

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
		EvilDamage = randomizer.Random(75, 100) * ScriptAdmin.CallScriptFromScript("GetIntelligence", &Victim, &None, 0) / 100.0f;
		break;
	case EDifficulty_Hard:
		FreezeTime = 4;
		ProcChance = 10;
		SharpDamage = 10;
		EvilDamage = randomizer.Random(25, 50) * ScriptAdmin.CallScriptFromScript("GetIntelligence", &Victim, &None, 0) / 100.0f;
		break;
	default:
		FreezeTime = 7;
		ProcChance = 15;
		SharpDamage = 15;
		EvilDamage = randomizer.Random(50, 75) * ScriptAdmin.CallScriptFromScript("GetIntelligence", &Victim, &None, 0) / 100.0f;
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
	if (OriginalResult == gEAction_MagicParade) {
		spellInfo->ParadeSpell = GETrue;
		spellInfo->spell = Damager.Interaction.GetSpell().GetName();
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
	static bCAccessorCreator ShieldTest(bTClassName<ShieldTest>::GetUnmangled());

	return &GetScriptInit();
}

ShieldTest::ShieldTest(void) {
	spellInfo = new SpellInfo();
	spellInfo->ParadeSpell = GEFalse;
	eCModuleAdmin::GetInstance().RegisterModule(*this);
}
ShieldTest::~ShieldTest(void) {
	if (spellInfo != nullptr) {
		delete spellInfo;
		spellInfo = nullptr;
	}

}
bTPropertyObject<ShieldTest, eCEngineComponentBase> ShieldTest::ms_PropertyObjectInstance_ShieldTest(GETrue);

void ShieldTest::Process() {
	if (spellInfo->ParadeSpell) {
		castSpell();
		spellInfo->ParadeSpell = GEFalse;
	}
}




BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		::DisableThreadLibraryCalls(hModule);
		AllocConsole();
		freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
