#ifndef GLOBALOBJECTS_H
#define GLOBALOBJECTS_H

#include "GameAPI/Game.h"
#include "Player.h"

// Enums
typedef enum {
    EXPLOSION_ITEMBOX,
    EXPLOSION_ENEMY,
    EXPLOSION_BOSS,
    EXPLOSION_BOSSPUFF,
} ExplosionTypes;

typedef enum {
    TRACK_NONE       = -1,
    TRACK_STAGE      = 0,
    TRACK_INVINCIBLE = 1,
    TRACK_SNEAKERS   = 2,
    TRACK_MINIBOSS   = 3,
    TRACK_HBHBOSS    = 4,
    TRACK_EGGMAN1    = 5,
    TRACK_EGGMAN2    = 6,
    TRACK_ACTCLEAR   = 7,
    TRACK_DROWNING   = 8,
    TRACK_GAMEOVER   = 9,
    TRACK_SUPER      = 10,
#if MANIA_USE_PLUS
    TRACK_HBHMISCHIEF = 11,
    TRACK_SOUNDTEST   = 12,
#else
    TRACK_SOUNDTEST   = 11,
    TRACK_HBHMISCHIEF = 12,
#endif
    TRACK_1UP = 13,

    // Aliases/Reused slots
    TRACK_METALSONIC   = TRACK_EGGMAN1,
    TRACK_RUBYPRESENCE = TRACK_EGGMAN1,
    TRACK_BUDDYBEAT    = TRACK_EGGMAN2,
    TRACK_ERZBOSS      = TRACK_SUPER,
} MusicTracks;

typedef enum {
    RING_TYPE_NORMAL,
    RING_TYPE_BIG,
    RING_TYPE_SPARKLE1,
    RING_TYPE_SPARKLE2,
    RING_TYPE_SPARKLE3,
} RingTypes;

typedef enum {
    RING_MOVE_FIXED,
    RING_MOVE_LINEAR,
    RING_MOVE_CIRCLE,
    RING_MOVE_TRACK,
    RING_MOVE_PATH,
} RingMoveTypes;

// Object Classes
typedef struct {
    RSDK_OBJECT
} ObjectDebris;

typedef struct {
    RSDK_OBJECT
    uint16 aniFrames;
    uint16 sfxDestroy;
} ObjectExplosion;

typedef struct {
    RSDK_OBJECT
    char trackNames[16][32];
    uint32 trackLoops[16];
    int32 trackStartPos;
    int32 channelID;
    int32 activeTrack;
#if MANIA_USE_PLUS
    int32 nextTrack;
    int32 restartTrackID;
#else
    int32 prevTrack;
    int32 queuedTrack;
    int32 nextTrack;
    bool32 playingRegularTrack;
    bool32 playingDrownTrack;
    bool32 playing1UPTrack;
#endif
    uint16 aniFrames;
} ObjectMusic;

typedef struct {
    RSDK_OBJECT
    Hitbox hitbox;
    int32 pan;
    uint16 aniFrames;
    uint16 sfxRing;
} ObjectRing;

typedef struct {
    RSDK_OBJECT
    int32 actID;
    StateMachine(stageFinishCallback);
    bool32 shouldRecoverPlayers; // a little misleading, forces the player on-screen before an act transition if enabled
    StateMachine(vsSwapCB[0x10]);
    int32 vsSwapCBCount;
#if MANIA_USE_PLUS
    int32 playerSwapEnabled[PLAYER_COUNT];
    uint8 swapPlayerID;
    uint8 swapPlayerCount;
    uint8 preSwapPlayerIDs[PLAYER_COUNT];
    uint8 swappedPlayerIDs[PLAYER_COUNT];
#else
    bool32 playerSwapEnabled;
#endif
    int32 listPos;
    int32 prevListPos;
    int32 ringFrame;
    int32 timer;
    int32 persistentTimer;
    int32 cameraBoundsL[PLAYER_COUNT];
    int32 cameraBoundsR[PLAYER_COUNT];
    int32 cameraBoundsT[PLAYER_COUNT];
    int32 cameraBoundsB[PLAYER_COUNT];
    int32 playerBoundsL[PLAYER_COUNT];
    int32 playerBoundsR[PLAYER_COUNT];
    int32 playerBoundsT[PLAYER_COUNT];
    int32 playerBoundsB[PLAYER_COUNT];
    int32 deathBoundary[PLAYER_COUNT];
    int32 playerBoundActiveL[PLAYER_COUNT];
    int32 playerBoundActiveR[PLAYER_COUNT];
    int32 playerBoundActiveT[PLAYER_COUNT];
    int32 playerBoundActiveB[PLAYER_COUNT];
    int32 autoScrollSpeed;
    bool32 setATLBounds;
    bool32 gotTimeOver;
    StateMachine(timeOverCallback);
    uint16 collisionLayers;
    uint16 fgLayer[2]; // { lowPriority, highPriority }
    uint16 moveLayer;
#if MANIA_USE_PLUS
    uint16 scratchLayer;
#endif
    uint16 fgLayerMask[2]; // { lowPriority, highPriority }
    uint16 moveLayerMask;
    uint8 fgDrawGroup[2];     // { lowPriority, highPriority }
    uint8 objectDrawGroup[2]; // { lowPriority, highPriority }
    uint8 playerDrawGroup[2]; // { lowPriority, highPriority }
    uint8 hudDrawGroup;
    uint16 sfxFail;
#if MANIA_USE_PLUS
    EntityBase entityStorage[16];
    int32 screenPosX[PLAYER_COUNT];
    int32 screenPosY[PLAYER_COUNT];
    bool32 swapGameMode;
    bool32 teleportActionActive;
    int32 randSeed;
#endif
} ObjectZone;

// Entity Classes
typedef struct {
    RSDK_ENTITY
    StateMachine(state);
    int32 timer;
    int32 gravityStrength;
    int32 rotSpeed;
    Vector2 scaleSpeed;
    int32 delay;
    Animator animator;
} EntityDebris;

typedef struct {
    RSDK_ENTITY
    Animator animator;
    int32 planeFilter;
} EntityExplosion;

typedef struct {
    RSDK_ENTITY
    StateMachine(state);
    StateMachine(stateDraw);
    RingTypes type;
    ManiaPlaneFilterTypes planeFilter;
    int32 ringAmount;
    int32 timer;
    int32 maxFrameCount;
    int32 sparkleType;
    EntityPlayer *storedPlayer;
    RingMoveTypes moveType;
    Vector2 amplitude;
    int32 speed;
    Vector2 drawPos;
    Animator animator;
} EntityRing;

// Object Structs
extern ObjectDebris *Debris;
extern ObjectExplosion *Explosion;
extern ObjectMusic *Music;
extern ObjectZone *Zone;

// Extra Entity Functions
extern void (*HUD_EnableRingFlash)(void);

// States
extern StateMachine(Debris_State_Move);
extern StateMachine(Ring_State_Lost);

// Draw States
extern StateMachine(Ring_Draw_Normal);

#endif //! GLOBALOBJECTS_H
