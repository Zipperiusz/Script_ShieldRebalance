#include "MagicParadeInfo.h"

void MagicParadeInfo::CastSpell() {
    Entity Defender = this->Defender;
    Entity Target = this->Attacker;

    if (!Defender.NPC.IsValid()) {
        return;
    }
    if (!Target.NPC.IsValid()) {
        return;
    }
    bCMatrix Pose = Defender.GetPose();
    Pose.AccessTranslation().AccessY() += 130;
    Entity Spell = Template(this->Spell);

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