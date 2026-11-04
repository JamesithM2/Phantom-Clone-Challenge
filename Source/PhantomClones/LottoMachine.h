#ifndef OBJ_LOTTOMACHINE_H
#define OBJ_LOTTOMACHINE_H

#include "GameAPI/Game.h"
#include "Player.h"

// Object Class
typedef struct {
    RSDK_OBJECT
    Hitbox hitboxBottom;
    Hitbox hitboxMotor;
    uint8 activePlayers;
    uint16 sfxPimPom;
    uint16 sfxFail;
    Animator shineAnimator;
    uint16 aniFrames;
} ObjectLottoMachine;

// Entity Class
typedef struct {
    RSDK_ENTITY
    StateMachine(state);
    int32 playerCount;
    EntityPlayer *playerPtrs[PLAYER_COUNT];
    int32 timer;
    int32 spinSpeed;
    int32 playerDir;
    int32 playerTimers[PLAYER_COUNT];
    int32 collectedBallCount;
    int32 collectedBallTypes[4];
    int32 chutePos;
    int32 chuteTargetPos;
    int32 chuteVel;
    uint8 drawGroupHigh;
    int32 playerAngles[PLAYER_COUNT];
    Animator machineAnimator;
    Animator supportAnimator;
    Animator chuteAnimator;
    Animator motorAnimator;
    Animator glassAnimator;
} EntityLottoMachine;

// Object Struct
extern ObjectLottoMachine *LottoMachine;

#endif //! OBJ_LOTTOMACHINE_H
