#ifndef OBJ_PHANTOMCLONE_H
#define OBJ_PHANTOMCLONE_H

#include "GameAPI/Game.h"
#include "Player.h"

#define CLONE_MAX_FRAMECOUNT (256)
#define CLONE_MAX_CLONECOUNT (3)

// don't really have to do this but it looks neater
typedef enum {
    SLOT_CLONESTACK_START = 50,
    SLOT_CLONESTACK_END   = 53,
} ReservedEntitySlots;

typedef enum {
    PROCESS_TYPE_ACTIVE, // active, process normally
    PROCESS_TYPE_VANISH_TEMP, // only vanish temporarily, then spawn back later
    PROCESS_TYPE_VANISH_PERM, // vanish permanently for the rest of the player's time in the current stage (also spawns rings)
    PROCESS_TYPE_FROZEN, // hold clone until it can respawn again
} CloneProcessStates;

typedef struct {
    Vector2 position;
    uint8 direction;
    int32 rotation;
    uint32 visible;
    Vector2 scale;
    uint16 drawGroup;
    Animator animator;
} CloneFrame;

// Object Class
typedef struct {
    RSDK_OBJECT
    CloneFrame frame[CLONE_MAX_FRAMECOUNT];
    int32 trailFrame;
    int32 spawnTimer;
    int32 spawnCount;
    int32 cloneCount;
    uint16 aniFrames;
    uint16 sfxAppear;
    uint16 sfxSpawn;
    uint16 sfxHit;
    uint16 sfxVanish;
} ObjectPhantomClone;

// Entity Class
typedef struct {
    RSDK_ENTITY
    int32 processState;
    int32 trailOffset;
    int32 spawnDelay;
    int32 timer;
    Animator animator;
} EntityPhantomClone;

// Object Entity
extern ObjectPhantomClone *PhantomClone;

// Standard Entity Events
void PhantomClone_Update(void);
void PhantomClone_LateUpdate(void);
void PhantomClone_StaticUpdate(void);
void PhantomClone_Draw(void);
void PhantomClone_Create(void *data);
void PhantomClone_StageLoad(void);
#if GAME_INCLUDE_EDITOR
void PhantomClone_EditorDraw(void);
void PhantomClone_EditorLoad(void);
#endif
void PhantomClone_Serialize(void);

// Extra Entity Functions
void PhantomClone_SpawnParticleFX(int32 x, int32 y);
void PhantomClone_HandleParticleFX(void);
void PhantomClone_CheckProcessState();
bool32 PhantomClone_CheckSceneExceptions(void);
bool32 PhantomClone_CheckRespawn();

#endif //! OBJ_PHANTOMCLONE_H
