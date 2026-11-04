#include "PhantomClone.h"
#include "GlobalObjects.h"
#include "ModConfig.h"

ObjectPhantomClone *PhantomClone;

void PhantomClone_Update(void) 
{
    RSDK_THIS(PhantomClone);

    RSDK.ProcessAnimation(&self->animator);
}

void PhantomClone_LateUpdate(void) 
{
    RSDK_THIS(PhantomClone);

    ObjectExplosion *Explosion = Mod.FindObject("Explosion");
    ObjectPlayer *Player       = Mod.FindObject("Player");
    ObjectZone *Zone           = Mod.FindObject("Zone");

    // delay before the clone actually shows up
    if (self->spawnDelay) {
        self->spawnDelay--;
        if (!self->spawnDelay) {
            RSDK.PlaySfx(PhantomClone->sfxSpawn, false, 0xFF);

            EntityExplosion *explosion = CREATE_ENTITY(Explosion, INT_TO_VOID(EXPLOSION_ENEMY), self->position.x, self->position.y);
            explosion->drawGroup       = Zone->objectDrawGroup[1];
        }
    }

    PhantomClone_HandleParticleFX();

    // get info from universal trail frames and set them to the clone
    self->trailOffset++;
    if (self->trailOffset >= CLONE_MAX_FRAMECOUNT)
        self->trailOffset = 0;

    int32 offset = self->trailOffset;

    self->position  = PhantomClone->frame[offset].position;
    self->direction = PhantomClone->frame[offset].direction;
    self->rotation  = PhantomClone->frame[offset].rotation;
    self->visible   = PhantomClone->frame[offset].visible;
    self->scale     = PhantomClone->frame[offset].scale;
    self->drawGroup = PhantomClone->frame[offset].drawGroup;
    memcpy(&self->animator, &PhantomClone->frame[offset].animator, sizeof(PhantomClone->frame[offset].animator));

    // player interaction
    Hitbox *cloneHitbox = RSDK.GetHitbox(&self->animator, 0);

    EntityPlayer *player = RSDK_GET_ENTITY(SLOT_PLAYER1, Player);

    if (!self->spawnDelay) {
        if (Player_CheckValidState(player) && !player->invincibleTimer && !player->blinkTimer) {
            if (Player_CheckCollisionTouch(player, self, cloneHitbox)) {
                if (++self->timer >= 7) {
                    if (player->rings) {
                        player->rings--;
                        RSDK.PlaySfx(PhantomClone->sfxHit, false, 0xFF);
                    }
                    else
                        Player_Hurt(player, self);

                    self->timer = 0;
                }

                if (player->rings)
                    HUD_EnableRingFlash();
            }
        }
    }
}

void PhantomClone_StaticUpdate(void) 
{
    if (PhantomClone_CheckVanish()) {
        // handle clone vanishing
        ObjectExplosion *Explosion = Mod.FindObject("Explosion");
        ObjectZone *Zone           = Mod.FindObject("Zone");

        foreach_active(PhantomClone, clone)
        {
            if (clone->spawnDelay) {
                RSDK.StopSfx(PhantomClone->sfxAppear);
                destroyEntity(clone);
            }
            else {
                if (RSDK.GetEntitySlot(clone) == SLOT_CLONESTACK_START)
                    RSDK.PlaySfx(PhantomClone->sfxSpawn, false, 0xFF);

                EntityExplosion *explosion = CREATE_ENTITY(Explosion, INT_TO_VOID(EXPLOSION_ENEMY), clone->position.x, clone->position.y);
                explosion->drawGroup       = Zone->objectDrawGroup[1];

                PhantomClone_SpawnRings(clone, 3, 0);

                destroyEntity(clone);
            }
        }

        // reset info
        PhantomClone->trailFrame = 0;
        PhantomClone->spawnTimer = 0;
        PhantomClone->spawnCount = 0;

        for (int32 f = 0; f < CLONE_MAX_FRAMECOUNT; ++f) {
            PhantomClone->frame[f].position.x = 0;
            PhantomClone->frame[f].position.y = 0;
            PhantomClone->frame[f].direction  = FLIP_NONE;
            PhantomClone->frame[f].rotation   = 0;
            PhantomClone->frame[f].visible    = false;
            PhantomClone->frame[f].scale.x    = 512;
            PhantomClone->frame[f].scale.y    = 512;
            PhantomClone->frame[f].drawGroup  = 0;
            memcpy(&PhantomClone->frame[f].animator, NULL, 0);
        }
    }
    else {
        // copy player info
        PhantomClone->trailFrame++;
        if (PhantomClone->trailFrame >= CLONE_MAX_FRAMECOUNT)
            PhantomClone->trailFrame = 0;

        int32 buffer = PhantomClone->trailFrame;

        ObjectPlayer *Player = Mod.FindObject("Player");
        EntityPlayer *player = RSDK_GET_ENTITY(SLOT_PLAYER1, Player);

        PhantomClone->frame[buffer].position  = player->position;
        PhantomClone->frame[buffer].direction = player->direction;
        PhantomClone->frame[buffer].rotation  = player->rotation;
        PhantomClone->frame[buffer].visible   = player->visible;
        PhantomClone->frame[buffer].scale     = player->scale;
        PhantomClone->frame[buffer].drawGroup = player->drawGroup;
        memcpy(&PhantomClone->frame[buffer].animator, &player->animator, sizeof(player->animator));

        // spawn clones
        PhantomClone->spawnTimer++;
        if (PhantomClone->spawnTimer == 60) {
            // spawn at most three clones at a time
            if (PhantomClone->spawnCount < PhantomClone->cloneCount) {
                EntityPhantomClone *clone = RSDK_GET_ENTITY(SLOT_CLONESTACK_START + PhantomClone->spawnCount, PhantomClone);
                RSDK.ResetEntity(clone, PhantomClone->classID, NULL);

                // if this is the first clone we're spawning, play appear sfx
                if (RSDK.GetEntitySlot(clone) == SLOT_CLONESTACK_START)
                    RSDK.PlaySfx(PhantomClone->sfxAppear, false, 0xFF);

                PhantomClone->spawnCount++;
                PhantomClone->spawnTimer = 0;
            }
        }
    }
}

void PhantomClone_Draw(void)
{
    RSDK_THIS(PhantomClone);

    if (!self->spawnDelay)
        RSDK.DrawSprite(&self->animator, NULL, false);

    // my awful attempt at tinting the player sprite
    // techincally I could get away with using a tint lookup table but idk how that works LOL
    uint32 colorStore[0x80];
    for (int32 c = 0; c < 0x80; ++c) {
        colorStore[c] = RSDK.GetPaletteEntry(0, c);
        RSDK.SetPaletteEntry(0, c, 0xE000E0);
    }

    self->inkEffect = INK_ADD;
    self->alpha     = 0xA0;

    if (!self->spawnDelay)
        RSDK.DrawSprite(&self->animator, NULL, false);

    self->alpha     = 0;
    self->inkEffect = INK_NONE;

    for (int32 c = 0; c < 0x80; ++c) RSDK.SetPaletteEntry(0, c, colorStore[c]);
}

void PhantomClone_Create(void *data) 
{
    RSDK_THIS(PhantomClone);

    if (!SceneInfo->inEditor) {
        self->active     = ACTIVE_NORMAL;
        self->visible    = false;
        self->drawGroup  = 4;
        self->drawFX     = FX_FLIP | FX_ROTATE | FX_SCALE;
        self->spawnDelay = 180;
    }
}

void PhantomClone_StageLoad(void)
{
    PhantomClone->aniFrames = RSDK.LoadSpriteAnimation("Phantom/CloneFX.bin", SCOPE_STAGE);

    PhantomClone->trailFrame = 0;
    PhantomClone->spawnTimer = 0;
    PhantomClone->spawnCount = 0;

    PhantomClone->cloneCount = config.cloneCount;
    if (PhantomClone->cloneCount < 1)
        PhantomClone->cloneCount = 1;

    if (PhantomClone->cloneCount > 3)
        PhantomClone->cloneCount = 1;

    PhantomClone->sfxAppear = RSDK.GetSfx("Global/CloneAppear.wav");
    PhantomClone->sfxSpawn  = RSDK.GetSfx("Global/CloneSpawn.wav");
    PhantomClone->sfxHit    = RSDK.GetSfx("Global/CloneHit.wav");

    // reset all copied player info
    for (int32 f = 0; f < CLONE_MAX_FRAMECOUNT; ++f) {
        PhantomClone->frame[f].position.x = 0;
        PhantomClone->frame[f].position.y = 0;
        PhantomClone->frame[f].direction  = FLIP_NONE;
        PhantomClone->frame[f].rotation   = 0;
        PhantomClone->frame[f].visible    = false;
        PhantomClone->frame[f].scale.x    = 512;
        PhantomClone->frame[f].scale.y    = 512;
        PhantomClone->frame[f].drawGroup  = 0;
        memcpy(&PhantomClone->frame[f].animator, NULL, 0);
    }

    // under certain circumstances, no need to run logic at all
    if (PhantomClone_CheckSceneExceptions() || !isMainGameMode())
        PhantomClone->active = ACTIVE_NEVER;
}

void PhantomClone_SpawnParticleFX(int32 x, int32 y) 
{
    int32 velX = RSDK.Rand(-0xC000, 0xC000);
    int32 velY = RSDK.Rand(-0xC000, 0xC000);

    ObjectDebris *Debris = Mod.FindObject("Debris");
    ObjectZone *Zone     = Mod.FindObject("Zone");

    EntityDebris *particles = CREATE_ENTITY(Debris, Debris_State_Move, x, y);
    particles->drawGroup    = Zone->objectDrawGroup[0];
    particles->drawFX       = FX_FLIP;
    particles->direction    = RSDK.Rand(0, 4);
    particles->velocity.x   = velX;
    particles->velocity.y   = velY;
    particles->inkEffect    = INK_SUB;
    particles->alpha        = 0xE0;
    RSDK.SetSpriteAnimation(PhantomClone->aniFrames, RSDK.Rand(0, 2), &particles->animator, true, 0);
    particles->timer = 3 * particles->animator.frameCount;

    particles             = CREATE_ENTITY(Debris, Debris_State_Move, x, y);
    particles->velocity.x = velX;
    particles->velocity.y = velY;
    particles->drawGroup  = Zone->objectDrawGroup[0];
    particles->inkEffect  = INK_ADD;
    particles->alpha      = 0xFF;
    RSDK.SetSpriteAnimation(PhantomClone->aniFrames, RSDK.Rand(0, 2) + 2, &particles->animator, true, 0);
    particles->timer = 3 * particles->animator.frameCount;
}

void PhantomClone_HandleParticleFX(void) 
{
    RSDK_THIS(PhantomClone);

    ObjectZone *Zone = Mod.FindObject("Zone");

    if (!(Zone->timer & 7)) {
        PhantomClone_SpawnParticleFX(self->position.x + RSDK.Rand(-0x150000, -0x100000), self->position.y + RSDK.Rand(-0x80000, 0));
        PhantomClone_SpawnParticleFX(self->position.x + RSDK.Rand(0x100000, 0x150000), self->position.y + RSDK.Rand(0, 0x80000));
    }
}

void PhantomClone_SpawnRings(EntityPhantomClone *clone, int32 rings, uint8 cPlane)
{
    RSDK_THIS(PhantomClone);

    ObjectRing *Ring = Mod.FindObject("Ring");

    int32 innerRingCount = CLAMP(rings, 0, 16);

    int32 angleStart = 0xC0 - 8 * (innerRingCount & -2);
    int32 angle      = angleStart + 8;
    if (!(innerRingCount & 1))
        angle = angleStart - 8;

    for (int32 i = 0; i < innerRingCount; ++i) {
        EntityRing *ring     = CREATE_ENTITY(Ring, clone, clone->position.x, clone->position.y);
        ring->velocity.x     = RSDK.Cos256(angle) << 9;
        ring->velocity.y     = RSDK.Sin256(angle) << 9;
        ring->animator.speed = 0x200;
        ring->collisionPlane = cPlane;
        ring->inkEffect      = INK_ALPHA;
        ring->alpha          = 0x100;
        ring->isPermanent    = true;
        ring->state          = Ring_State_Lost;
        ring->stateDraw      = Ring_Draw_Normal;
        ring->drawGroup      = self->drawGroup;
        ring->moveType       = RING_MOVE_LINEAR;
        angle += 0x10;
    }
}

bool32 PhantomClone_CheckSceneExceptions(void) 
{ 
    // function for checking specific scenes in which clone(s) should NEVER appear
    // this mainly checks for scenes that are only used for cutscenes

    if (RSDK.CheckSceneFolder("AIZ") || RSDK.CheckSceneFolder("GHZCutscene") || RSDK.CheckSceneFolder("MSZCutscene")
        || RSDK.CheckSceneFolder("TimeTravel") || RSDK.CheckSceneFolder("Ending") || RSDK.CheckSceneFolder("MSZEnding"))
        return true;

    // in the case of LRZ3/TMZ3, these are both boss acts where the clone(s) would only be around for a couple of seconds
    // so there's not really a good reason to spawn them in here
    if (RSDK.CheckSceneFolder("LRZ3") || RSDK.CheckSceneFolder("TMZ3"))
        return true;

    return false;
}

bool32 PhantomClone_CheckVanish() 
{ 
    RSDK_THIS(PhantomClone);

    ObjectMusic *Music   = Mod.FindObject("Music");
    ObjectPlayer *Player = Mod.FindObject("Player");

    // checks circumstances where clones should disappear such as:

    // if the player entity isn't using its class ID (i.e. Debug Mode)
    EntityPlayer *player = RSDK_GET_ENTITY(SLOT_PLAYER1, Player);
    if (player->classID != Player->classID)
        return true;

    // if the player is super (clones can't hurt the player anyways, plus there's no good way to handle super palettes between clones)
    if (player->superState)
        return true;

    // if the stage timer isn't active
    if (!SceneInfo->timeEnabled)
        return true;

    // if any boss music is playing
    int32 track = Music->activeTrack;

    if (track == TRACK_MINIBOSS || track == TRACK_HBHBOSS || track == TRACK_EGGMAN1 || track == TRACK_EGGMAN2 || track == TRACK_HBHMISCHIEF)
        return true;

    return false;
}

#if GAME_INCLUDE_EDITOR
void PhantomClone_EditorDraw(void) {}

void PhantomClone_EditorLoad(void) {}
#endif

void PhantomClone_Serialize(void) {}
