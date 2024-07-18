#pragma once
#include "util/Memory.h"
#include "util/ScriptUtil.h"

class MagicParadeInfo {
public:
	GEBool ParadeSpell;
	bCString spell;
	Entity entity;


    void castSpell() {
        Entity Defender = this->entity;
        if (!Defender.NPC.IsValid()) {
            return;
        }
        Entity Target = Defender.NPC.GetCurrentTarget();
        bCMatrix Pose = Defender.GetPose();
        Pose.AccessTranslation().AccessY() += 130;
        Entity Spell = Template(this->spell);

        Entity Spawn = Spell.Magic.GetSpawn();
        Entity SpawnedEntity = Spawn.Spawn(Spawn.GetTemplate(), Pose);

        SpawnedEntity.Interaction.SetOwner(Defender);
        SpawnedEntity.Interaction.SetSpell(Spell);

        SpawnedEntity.EnableCollisionWith(SpawnedEntity, GEFalse);
        SpawnedEntity.EnableCollisionWith(Defender, GEFalse);
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
};



