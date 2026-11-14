#include "PhantomClone.h"
#include "GlobalObjects.h"
#include "LottoMachine.h"
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
    ObjectRing *Ring           = Mod.FindObject("Ring");
    ObjectZone *Zone           = Mod.FindObject("Zone");

    // get info from universal trail frames and set them to the clone
    self->trailOffset++;
    if (self->trailOffset >= CLONE_MAX_FRAMECOUNT)
        self->trailOffset = 0;

    int32 offset = self->trailOffset;

    self->position  = PhantomClone->frame[offset].position;
    self->direction = PhantomClone->frame[offset].direction;
    self->rotation  = PhantomClone->frame[offset].rotation;
    self->scale     = PhantomClone->frame[offset].scale;
    self->drawGroup = PhantomClone->frame[offset].drawGroup;
    self->visible   = !self->processState ? PhantomClone->frame[offset].visible : false;
    memcpy(&self->animator, &PhantomClone->frame[offset].animator, sizeof(PhantomClone->frame[offset].animator));
    
    self->tailDirection = PhantomClone->frame[offset].tailDirection;
    self->tailRotation  = PhantomClone->frame[offset].tailRotation;
    memcpy(&self->tailAnimator, &PhantomClone->frame[offset].tailAnimator, sizeof(PhantomClone->frame[offset].tailAnimator));

    // process clone states
    switch (self->processState) {
        case PROCESS_TYPE_ACTIVE:
            PhantomClone_CheckProcessState();
            PhantomClone_HandleParticleFX();

            // delay before the clone actually shows up
            if (self->spawnDelay) {
                self->spawnDelay--;
                if (!self->spawnDelay) {
                    RSDK.PlaySfx(PhantomClone->sfxSpawn, false, 0xFF);

                    EntityExplosion *explosion = CREATE_ENTITY(Explosion, INT_TO_VOID(EXPLOSION_ENEMY), self->position.x, self->position.y);
                    explosion->drawGroup       = Zone->objectDrawGroup[1];
                }
            }
            else {
                // player interaction
                Hitbox *cloneHitbox  = RSDK.GetHitbox(&self->animator, 0);
                EntityPlayer *player = RSDK_GET_ENTITY(SLOT_PLAYER1, Player);

                if (Player_CheckValidState(player) && player->state != Player_State_Static && !player->invincibleTimer && !player->blinkTimer) {
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
            break;

        case PROCESS_TYPE_VANISH_TEMP:
            if (!self->spawnDelay) {
                if (RSDK.GetEntitySlot(self) == SLOT_CLONESTACK_START)
                    RSDK.PlaySfx(PhantomClone->sfxVanish, false, 0xFF);

                CREATE_ENTITY(Explosion, INT_TO_VOID(EXPLOSION_ENEMY), self->position.x, self->position.y);
            }

            RSDK.StopSfx(PhantomClone->sfxAppear);
            self->processState = PROCESS_TYPE_FROZEN;
            break;

        case PROCESS_TYPE_VANISH_PERM:
            if (RSDK.GetEntitySlot(self) == SLOT_CLONESTACK_START)
                RSDK.PlaySfx(PhantomClone->sfxSpawn, false, 0xFF);

            CREATE_ENTITY(Explosion, INT_TO_VOID(EXPLOSION_ENEMY), self->position.x, self->position.y);

            int32 count = 3;
            int32 angle = 16 * (12 - (count >> 1));
            for (int32 i = 0; i < count; ++i) {
                EntityRing *ring = CREATE_ENTITY(Ring, INT_TO_VOID(1), self->position.x, self->position.y);
                ring->velocity.x = RSDK.Cos256(angle) << 9;
                ring->velocity.y = RSDK.Sin256(angle) << 9;
                ring->timer          = 32;
                ring->collisionPlane = RSDK_GET_ENTITY(SLOT_PLAYER1, Player)->collisionPlane;
                ring->inkEffect      = INK_ALPHA;
                ring->alpha          = 256;
                ring->state          = Ring_State_Lost;
                ring->animator.speed = 512;
                ring->stateDraw      = Ring_Draw_Normal;
                angle += 16;
            }

            destroyEntity(self);
            break;

        case PROCESS_TYPE_FROZEN:
            if (!PhantomClone_CheckRespawn()) {
                if (RSDK.GetEntitySlot(self) == SLOT_CLONESTACK_START)
                    RSDK.PlaySfx(PhantomClone->sfxAppear, false, 0xFF);

                self->spawnDelay = 180;
                self->processState = PROCESS_TYPE_ACTIVE;
            }
            break;
    }
}

void PhantomClone_StaticUpdate(void) 
{
    // copy player info
    if (SceneInfo->timeEnabled) {
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

        if (player->tailFrames != (uint16)-1) {
            PhantomClone->frame[buffer].tailDirection = player->tailDirection;
            PhantomClone->frame[buffer].tailRotation  = player->tailRotation;
            memcpy(&PhantomClone->frame[buffer].tailAnimator, &player->tailAnimator, sizeof(player->tailAnimator));
        }

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

    if (!self->spawnDelay) {
        int32 rotStore  = self->rotation;
        int32 dirStore  = self->direction;
        self->rotation  = self->tailRotation;
        self->direction = self->tailDirection;
        RSDK.DrawSprite(&self->tailAnimator, NULL, false);

        self->rotation  = rotStore;
        self->direction = dirStore;

        RSDK.DrawSprite(&self->animator, NULL, false);
    }

    // my awful attempt at tinting the player sprite
    // techincally I could get away with using a tint lookup table but idk how that works LOL
    uint32 colorStore[2][0x80];
    for (int32 c = 0; c < 0x80; ++c) {
        colorStore[0][c] = RSDK.GetPaletteEntry(0, c);
        colorStore[1][c] = RSDK.GetPaletteEntry(1, c);
        RSDK.SetPaletteEntry(0, c, 0xE000E0);
        RSDK.SetPaletteEntry(1, c, 0xE000E0);
    }

    self->inkEffect = INK_ADD;
    self->alpha     = 0xA0;

    if (!self->spawnDelay) {
        int32 rotStore  = self->rotation;
        int32 dirStore  = self->direction;
        self->rotation  = self->tailRotation;
        self->direction = self->tailDirection;
        RSDK.DrawSprite(&self->tailAnimator, NULL, false);

        self->rotation  = rotStore;
        self->direction = dirStore;

        RSDK.DrawSprite(&self->animator, NULL, false);
    }

    self->alpha     = 0;
    self->inkEffect = INK_NONE;

    for (int32 c = 0; c < 0x80; ++c) {
        RSDK.SetPaletteEntry(0, c, colorStore[0][c]);
        RSDK.SetPaletteEntry(1, c, colorStore[1][c]);
    }
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
    if (PhantomClone->cloneCount < 1 || PhantomClone->cloneCount > 3)
        PhantomClone->cloneCount = 1;

    PhantomClone->sfxAppear = RSDK.GetSfx("Global/CloneAppear.wav");
    PhantomClone->sfxSpawn  = RSDK.GetSfx("Global/CloneSpawn.wav");
    PhantomClone->sfxHit    = RSDK.GetSfx("Global/CloneHit.wav");
    PhantomClone->sfxVanish = RSDK.GetSfx("Global/CloneVanish.wav");

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

void PhantomClone_CheckProcessState()
{
    RSDK_THIS(PhantomClone);

    ObjectMusic *Music               = Mod.FindObject("Music");
    ObjectPlayer *Player             = Mod.FindObject("Player");
    ObjectLottoMachine *LottoMachine = Mod.FindObject("LottoMachine");

    EntityPlayer *player = RSDK_GET_ENTITY(SLOT_PLAYER1, Player);

    bool32 bossTrack = false;
    if (Music->activeTrack == TRACK_MINIBOSS || Music->activeTrack == TRACK_HBHBOSS || Music->activeTrack == TRACK_EGGMAN1
        || Music->activeTrack == TRACK_EGGMAN2 || Music->activeTrack == TRACK_HBHMISCHIEF)
        bossTrack = true;

    // checks circumstances where clones should temporaily disappear:
    if (player->superState || player->classID != Player->classID || (LottoMachine && ((1 << player->playerID) & LottoMachine->activePlayers)))
        self->processState = PROCESS_TYPE_VANISH_TEMP;

    // checks if the clones should vanish for the rest of the stages duration
    if ((bossTrack && !config.allowInBosses) || !SceneInfo->timeEnabled)
        self->processState = PROCESS_TYPE_VANISH_PERM;
}

bool32 PhantomClone_CheckRespawn() 
{
    RSDK_THIS(PhantomClone);

    // checks for respawn after temporary vanish
    ObjectMusic *Music               = Mod.FindObject("Music");
    ObjectPlayer *Player             = Mod.FindObject("Player");
    ObjectLottoMachine *LottoMachine = Mod.FindObject("LottoMachine");

    EntityPlayer *player = RSDK_GET_ENTITY(SLOT_PLAYER1, Player);

    if (player->superState || player->classID != Player->classID || (LottoMachine && ((1 << player->playerID) & LottoMachine->activePlayers)))
        return true;

    return false;
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
    if ((RSDK.CheckSceneFolder("LRZ3") || RSDK.CheckSceneFolder("TMZ3")) && !config.allowInBosses)
        return true;

    return false;
}

#if GAME_INCLUDE_EDITOR
void PhantomClone_EditorDraw(void) {}

void PhantomClone_EditorLoad(void) {}
#endif

void PhantomClone_Serialize(void) {}
