#include "utility.h"


GEInt getPowerLevel(Entity& p_entity) {
    Entity player = Entity::GetPlayer();
    GEInt level = p_entity.NPC.GetProperty<PSNpc::PropertyLevel>() + player.NPC.GetProperty<PSNpc::PropertyLevel>();
    if (level > p_entity.NPC.GetProperty<PSNpc::PropertyLevelMax>())
        level = p_entity.NPC.GetProperty<PSNpc::PropertyLevelMax>();
    return level;
}

bCString getProjectile(Entity& p_entity, gEUseType p_rangedWeaponType) {
    GEInt powerLevel = getPowerLevel(p_entity);
    gEPoliticalAlignment aligmnent = p_entity.NPC.GetProperty<PSNpc::PropertyPoliticalAlignment>();
    gESpecies targetSpecies = p_entity.NPC.GetCurrentTarget().NPC.GetProperty<PSNpc::PropertySpecies>();
    GEInt random = Entity::GetRandomNumber(100);

    if (p_rangedWeaponType == gEUseType_CrossBow) {
        if (powerLevel >= 25) {
            return "Bolt_Sharp";
        }
        return "Bolt";
    }
    else if (p_rangedWeaponType == gEUseType_Bow) {
        if (p_entity.GetName() == "Jorn") // And add after quest!
            return "ExplosiveArrow";
        if (targetSpecies == gESpecies_FireGolem || targetSpecies == gESpecies_Golem || targetSpecies == gESpecies_IceGolem || targetSpecies == gESpecies_Skeleton)
            return "BluntArrow";
        if (aligmnent == gEPoliticalAlignment_Ass || aligmnent == gEPoliticalAlignment_Nom)
            return "PoisonArrow";
        if (aligmnent == gEPoliticalAlignment_Nrd)
            return "Arrow_ore";
        if (powerLevel >= 30) {
            if (aligmnent == gEPoliticalAlignment_Orc) {
                if (random <= 20)
                    return "SharpArrow";
                return "GoldArrow";
            }
            if (random <= 20) {
                return "BluntArrow";
            }
            return "FireArrow";
        }
        return "Arrow";
    }
}


GEBool isBigMonster(Entity& p_monster) {
    gCScriptAdmin& ScriptAdmin = GetScriptAdmin();
    if (p_monster == None) {
        return GEFalse;
    }
    if (ScriptAdmin.CallScriptFromScript("IsHumanoid", &p_monster, &None, 0)) {
        return GEFalse;
    }
    switch (p_monster.NPC.GetProperty<PSNpc::PropertySpecies>()) {
    case gESpecies_Demon:
    case gESpecies_Ogre:
    case gESpecies_Troll:
    case gESpecies_Trex:
    case gESpecies_Shadowbeast:
    case gESpecies_Dragon:
    case gESpecies_Gargoyle:
    case gESpecies_ScorpionKing:
    case gESpecies_FireGolem:
    case gESpecies_IceGolem:
    case gESpecies_Golem:
        return GETrue;
    default:
        return GEFalse;
    }


}

GEBool CanBurn(Entity& p_victim, Entity& p_damager) {
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
    if (p_damager == None) return GEFalse;
    // If an Item hit the Victim
    gEDamageType damageType = p_damager.Damage.GetProperty<PSDamage::PropertyDamageType>();
    GEInt random = Entity::GetRandomNumber(100);
    GEU32 itemQuality = p_damager.Item.GetQuality();
    //std::cout << "In CanBurn:\tDamageType: " << damageType << "\trandom: " << random
        //<< "\nProjectile.IsValid: " << p_damager.Projectile.IsValid () << "\titemQuality: " << itemQuality << "\n";
    if (!p_damager.Projectile.IsValid()) {
        
            if (((BYTE)itemQuality & gEItemQuality_Burning) == gEItemQuality_Burning && random < 20) {
                return GETrue;
            }
        
        if (damageType == gEDamageType_Fire) {
            return GETrue;
        }
    }
    // Missile Here
    else {
        if (damageType == gEDamageType_Fire
            && p_damager.Projectile.GetProperty<PSProjectile::PropertyPathStyle>() == gEProjectilePath::gEProjectilePath_Missile)
            return GETrue;
        if (((BYTE)itemQuality & gEItemQuality_Burning) == gEItemQuality_Burning
            && p_damager.Damage.GetProperty<PSDamage::PropertyDamageHitMultiplier>() >= 0.5f) // Bow Tension...
            return GETrue;

    }
    return GEFalse;
}

GEBool CanFreeze(Entity& p_victim, Entity& p_damager) {
    gESpecies victimSpecies = p_victim.NPC.GetProperty<PSNpc::PropertySpecies>();
    //std::cout << "Projectilename: " << p_damager.GetName ( ) << std::endl;
    if (p_damager.GetName() == "Mis_IceBlock")
        return GETrue;
    switch (victimSpecies) {
    case gESpecies_Golem:
    case gESpecies_Demon:
    case gESpecies_Troll:
    case gESpecies_FireGolem:
    case gESpecies_IceGolem:
    case gESpecies_Dragon:
        return GEFalse;
    }
    if (p_victim == Entity::GetPlayer() && p_victim.Inventory.IsSkillActive(Template("Perk_ResistCold")))
        return GEFalse;
    if (p_damager == None) return GEFalse;
    // If an Item hit the Victim
    gEDamageType damageType = p_damager.Damage.GetProperty<PSDamage::PropertyDamageType>();
    GEInt random = Entity::GetRandomNumber(100);
    GEU32 itemQuality = p_damager.Item.GetQuality();
    if (!p_damager.Projectile.IsValid()) {
        
            if (((BYTE)itemQuality & gEItemQuality_Frozen) == gEItemQuality_Frozen && random < 20) {
                return GETrue;
            }
        
        if (damageType == gEDamageType_Ice) {
            return GETrue;
        }
    }
    // Missile Here
    else {
        if (damageType == gEDamageType_Ice
            && p_damager.Projectile.GetProperty<PSProjectile::PropertyPathStyle>() == gEProjectilePath::gEProjectilePath_Missile)
            return GETrue;
        if (((BYTE)itemQuality & gEItemQuality_Frozen) == gEItemQuality_Frozen
            && p_damager.Damage.GetProperty<PSDamage::PropertyDamageHitMultiplier>() >= 0.5f) // Bow Tension...
            return GETrue;

    }
    return GEFalse;
}

GEBool IsNormalProjectile(Entity& p_damager) {
    //std::cout << "Projectilename: " << p_damager.GetName ( ) << std::endl;
    //std::cout << "Projectile?: " << p_damager.Projectile.IsValid() << std::endl;
    return p_damager.Projectile.IsValid() &&
        p_damager.Interaction.GetSpell() == None;
}

GEBool IsSpellContainer(Entity& p_damager) {
    //std::cout << "Spell?: " << p_damager.Interaction.GetSpell ( ).GetName ( )
    //    << "\n" << p_damager.Interaction.GetOwner ( ).GetName() << std::endl;
    return p_damager.Interaction.GetSpell() != None;
}

GEBool IsMagicProjectile(Entity& p_damager) {
    //std::cout << "Projectilename: " << p_damager.GetName ( ) << std::endl;
    //std::cout << "Projectile?: " << p_damager.Projectile.IsValid ( ) << std::endl;
    return p_damager.Projectile.IsValid() &&
        p_damager.Interaction.GetSpell() != None;
}

GEBool CheckHandUseTypes(gEUseType p_lHand, gEUseType p_rHand, Entity& entity) {
    //std::cout << "Left: " << p_lHand << "Right: " << p_rHand
       // << "\nItem Left: " << entity.Inventory.GetItemFromSlot ( gESlot_LeftHand ).GetName ( )
       // << "\nItem Right" << entity.Inventory.GetItemFromSlot ( gESlot_RightHand ).GetName ( ) << std::endl;
    return (entity.Inventory.GetItemFromSlot(gESlot_LeftHand)
        .Interaction.GetUseType() == p_lHand &&
        entity.Inventory.GetItemFromSlot(gESlot_RightHand)
        .Interaction.GetUseType() == p_rHand);
}

GEInt GetSkillLevels(Entity& p_entity) {
    if (p_entity != Entity::GetPlayer()) {
        GEU32 npcLevel = p_entity.NPC.GetProperty<PSNpc::PropertyLevel>();
        //std::cout << "Entity: " << p_entity.GetName ( ) << "\tLevel: " << npcLevel << std::endl;
        if (npcLevel > 44)
            return 3;
        if (npcLevel > 29)
            return 2;
        if (npcLevel > 14)
            return 1;
        return 0;
    }

    GEInt level = 0;
    GEInt playerRightHandStack = p_entity.Inventory.FindStackIndex(gESlot_RightHand);
    gEUseType playerUseType = p_entity.Inventory.GetUseType(playerRightHandStack);

    //std::cout << "Player in GetSkillLevels" << std::endl;
    // maybe even use a switch case here...

    switch (playerUseType) {
    case gEUseType_1H:
        if (p_entity.Inventory.IsSkillActive(Template("Perk_1H_3")))
            level = 2;
        else if (p_entity.Inventory.IsSkillActive(Template("Perk_1H_2")))
            level = 1;
        if (CheckHandUseTypes(gEUseType_1H, gEUseType_1H, p_entity)
            && p_entity.Inventory.IsSkillActive(Template("Perk_1H1H_2")))
            level += 1;
        break;
    case gEUseType_2H:
    case gEUseType_Axe:
    case gEUseType_Halberd:
        if (p_entity.Inventory.IsSkillActive(Template("Perk_Axe_3")))
            level = 2;
        else if (p_entity.Inventory.IsSkillActive(Template("Perk_Axe_2")))
            level = 1;
        break;
    case gEUseType_Staff:
        if (p_entity.Inventory.IsSkillActive(Template("Perk_Staff_3")))
            level = 2;
        if (p_entity.Inventory.IsSkillActive(Template("Perk_Staff_2")))
            level = 1;
        break;
    case gEUseType_Cast:
        GEInt playerInt = p_entity.PlayerMemory.GetIntelligence();
        if (playerInt > 199)
            level = 2;
        else if (playerInt > 99)
            level = 1;
        break;
    }
    if (GetScriptAdmin().CallScriptFromScript("GetStrength", &p_entity, &None, 0) >= 250) {
        level += 1;
    }
    /*
    if ( playerUseType == gEUseType_1H ) {
        if ( p_entity.Inventory.IsSkillActive ( Template ( "Perk_1H_3" ) ) )
            level = 2;
        else if ( p_entity.Inventory.IsSkillActive ( Template ( "Perk_1H_2" ) ) )
            level = 1;
        if ( CheckHandUseTypes ( gEUseType_1H , gEUseType_1H , p_entity )
            && p_entity.Inventory.IsSkillActive ( Template ( "Perk_1H1H_2" ) ) )
            level += 1;
        return level;
    }
    else if ( playerUseType == gEUseType_2H || playerUseType == gEUseType_Axe || playerUseType == gEUseType_Halberd ) {
        if ( p_entity.Inventory.IsSkillActive ( Template ( "Perk_Axe_3" ) ) )
            level = 2;
        else if ( p_entity.Inventory.IsSkillActive ( Template ( "Perk_Axe_2" ) ) )
            level = 1;
        return level;
    }
    else if ( playerUseType == gEUseType_Staff ) {
        if ( p_entity.Inventory.IsSkillActive ( Template ( "Perk_Staff_3" ) ) )
            level = 2;
        if ( p_entity.Inventory.IsSkillActive ( Template ( "Perk_Staff_2" ) ) )
            level = 1;
        return level;
    }
    // Addition
    else if ( playerUseType == gEUseType_Cast ) {
        GEInt playerInt = p_entity.PlayerMemory.GetIntelligence();
        if ( playerInt > 199 ) {
            return 2;
        }
        if ( playerInt > 99 ) {
            return 1;
        }
    }*/
    return level; // or level
}

GEInt GetActionWeaponLevel(Entity& p_damager, gEAction p_action) {
    GEInt level = 0;
    switch (p_action) {
    case gEAction_Attack:
    case gEAction_SimpleWhirl:
    case gEAction_WhirlAttack:
    case gEAction_PierceAttack:
    case gEAction_GetUpAttack:
        level = 2;
        break;
    case gEAction_PowerAttack:
    case gEAction_SprintAttack:
        if (CheckHandUseTypes(gEUseType_1H, gEUseType_1H, p_damager)) {
            level = 3 - (GEU32)p_damager.Routine.GetProperty<PSRoutine::PropertyStatePosition>();
            break;
        }
    case gEAction_HackAttack:
        level = 4;
        break;
    case gEAction_QuickAttack:
    case gEAction_QuickAttackR:
    case gEAction_QuickAttackL:
        level = 0;
        break;
    }
    return level + GetSkillLevels(p_damager);
}

GEInt GetShieldLevelBonus(Entity& p_entity) {
    //std::cout << "Name in GetShieldLevelBonus: " << p_entity.GetName ( ) << std::endl;
    if (p_entity.Routine.GetProperty<PSRoutine::PropertyAction>() == gEAction::gEAction_GetUpParade) {
        return 2;
    }
    GEInt level = GetSkillLevels(p_entity);
    GEInt stackIndex = p_entity.Inventory.FindStackIndex(gESlot::gESlot_LeftHand);
    gEUseType useType = p_entity.Inventory.GetUseType(stackIndex);

    if (useType == gEUseType_Shield) {
        if (p_entity == Entity::GetPlayer()) {
            if (p_entity.Inventory.IsSkillActive(Template("Perk_Shield_2")))
                level += 1;
        }
    }
    return level;
}

// return an Enum of vulnaribility
VulnerabilityStatus DamageTypeEntityTest(Entity& p_victim, Entity& p_damager) {  // 0: Immunity, 1: Regular Damage, 2: Double Damage 3 Half Damage
    if (p_victim == None || p_damager == None)
        return VulnerabilityStatus::VulnerabilityStatus_IMMUNE;
    gEDamageType damageType = p_damager.Damage.GetProperty<PSDamage::PropertyDamageType>();
    gESpecies victimSpecies = p_victim.NPC.GetProperty<PSNpc::PropertySpecies>();
    //std::cout << "Damager in DamageTypeEntityTest: " << p_damager.GetName ( ) << "\tvictimspecies: " << victimSpecies 
        //<< "\tDamageType: " << damageType << std::endl;
    switch (damageType) {
    case gEDamageType_None:
        break;
    case gEDamageType_Impact:
        if (victimSpecies == gESpecies::gESpecies_Skeleton || victimSpecies == gESpecies::gESpecies_Golem)
            return VulnerabilityStatus_WEAK;
        return VulnerabilityStatus_REGULAR;
    case gEDamageType_Blade:
        if (victimSpecies == gESpecies::gESpecies_Golem)
            return VulnerabilityStatus_STRONG;
        return VulnerabilityStatus_REGULAR;
    case gEDamageType_Missile:
        switch (victimSpecies) {
        case gESpecies_Skeleton:
        case gESpecies_IceGolem:
        case gESpecies_Troll:
            if (p_damager.GetName().Contains("Fire", 0) || p_damager.GetName().Contains("Explosive", 0))
                return VulnerabilityStatus_REGULAR;
            if (victimSpecies == gESpecies_Troll) {
                return VulnerabilityStatus_STRONG;
            }
            return VulnerabilityStatus_IMMUNE;
        case gESpecies_Golem:
        case gESpecies_FireGolem:
            return VulnerabilityStatus_IMMUNE;
        default:
            return VulnerabilityStatus_REGULAR;
        }
    case gEDamageType_Fire:
        switch (victimSpecies) {
        case gESpecies_FireVaran:
        case gESpecies_FireGolem:
        case gESpecies_Dragon:
            return VulnerabilityStatus_IMMUNE;
        case gESpecies_IceGolem:
        case gESpecies_Zombie:
            return VulnerabilityStatus_WEAK;
        case gESpecies_Demon:
            return VulnerabilityStatus_STRONG;
        default:
            return VulnerabilityStatus_REGULAR;
        }
    case gEDamageType_Ice:
        if (p_victim.NPC.HasStatusEffects(gEStatusEffect::gEStatusEffect_Frozen))
            return VulnerabilityStatus_IMMUNE;
        switch (victimSpecies) {
        case gESpecies_FireGolem:
            return VulnerabilityStatus_WEAK;
        case gESpecies_Zombie:
            return VulnerabilityStatus_STRONG;
        case gESpecies_IceGolem:
            return VulnerabilityStatus_IMMUNE;
        default:
            return VulnerabilityStatus_REGULAR;
        }
    case gEDamageType_Lightning:
        if (victimSpecies == gESpecies_Golem)
            return VulnerabilityStatus_WEAK;
        return VulnerabilityStatus_REGULAR;
    default:
        return VulnerabilityStatus_REGULAR;
    }

    return VulnerabilityStatus_IMMUNE;
}

GEInt GetHyperActionBonus(gEAction p_action)
{
    switch (p_action) {
    case gEAction_Summon:
    case gEAction_FlameSword:
        return 2;
    case gEAction_PowerAttack:
    case gEAction_SprintAttack:
    case gEAction_HackAttack:
        return 0; // Change someday
    case gEAction_Heal:
        return 0;
    default:
        return 0;
    }

}


