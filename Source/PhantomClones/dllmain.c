#include "GameAPI/Game.h"
#include "PhantomClone.h"
#include "Player.h"
#include "GlobalObjects.h"
#include "ModConfig.h"

ModConfig config;

StateMachine(Debris_State_Move) = NULL;

void (*HUD_EnableRingFlash)(void) = NULL;

bool32 (*Player_CheckCollisionTouch)(EntityPlayer *player, void *entity, Hitbox *entityHitbox) = NULL;
bool32 (*Player_Hurt)(EntityPlayer *player, void *e)                                           = NULL;
bool32 (*Player_CheckValidState)(EntityPlayer *player)                                         = NULL;

StateMachine(Ring_State_Lost)  = NULL;
StateMachine(Ring_Draw_Normal) = NULL;

#if RETRO_USE_MOD_LOADER
DLLExport bool32 LinkModLogic(EngineInfo *info, const char *id);

#define ADD_PUBLIC_FUNC(func) Mod.AddPublicFunction(#func, (void *)(func))
#endif

void InitModAPI(void) 
{ 
    // Get public funcs
    Debris_State_Move          = Mod.GetPublicFunction(NULL, "Debris_State_Move");
    HUD_EnableRingFlash        = Mod.GetPublicFunction(NULL, "HUD_EnableRingFlash");
    Player_CheckCollisionTouch = Mod.GetPublicFunction(NULL, "Player_CheckCollisionTouch");
    Player_CheckValidState     = Mod.GetPublicFunction(NULL, "Player_CheckValidState");
    Player_Hurt                = Mod.GetPublicFunction(NULL, "Player_Hurt");
    Ring_State_Lost            = Mod.GetPublicFunction(NULL, "Ring_State_Lost");
    Ring_Draw_Normal           = Mod.GetPublicFunction(NULL, "Ring_Draw_Normal");

    // Register global obj
    RSDK_REGISTER_OBJECT(PhantomClone);

    // Set mod config
    config.cloneCount = Mod.GetSettingsInteger("", "Clone:CloneCount", 1);
    
    Mod.SetSettingsInteger("Clone:CloneCount", config.cloneCount);
    Mod.SaveSettings();

    // Add public funcs
    ADD_PUBLIC_FUNC(PhantomClone_SpawnParticleFX);
    ADD_PUBLIC_FUNC(PhantomClone_HandleParticleFX);
    ADD_PUBLIC_FUNC(PhantomClone_SpawnRings);
    ADD_PUBLIC_FUNC(PhantomClone_CheckSceneExceptions);
    ADD_PUBLIC_FUNC(PhantomClone_CheckVanish);
}

#if RETRO_USE_MOD_LOADER
void InitModAPI(void);

bool32 LinkModLogic(EngineInfo *info, const char *id)
{
#if MANIA_USE_PLUS
    LinkGameLogicDLL(info);
#else
    LinkGameLogicDLL(*info);
#endif

    globals = Mod.GetGlobals();

    modID = id;

    InitModAPI();

    return true;
}
#endif