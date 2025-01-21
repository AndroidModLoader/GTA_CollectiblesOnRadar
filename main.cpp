#include <mod/amlmod.h>
#include <mod/logger.h>
#include <mod/config.h>
#include <time.h>
#include "isautils.h"
ISAUtils* sautils = NULL;

#ifdef AML32
    #include "GTASA_STRUCTS.h"
    #define BYVER(__for32, __for64) (__for32)
#else
    #include "GTASA_STRUCTS_210.h"
    #define BYVER(__for32, __for64) (__for64)
#endif
#define sizeofA(__aVar)  ((int)(sizeof(__aVar)/sizeof(__aVar[0])))

MYMOD(net.kong78.rusjj.cor, Collectibles on Radar, 1.3, kong78 & RusJJ)
BEGIN_DEPLIST()
    ADD_DEPENDENCY_VER(net.rusjj.aml, 1.0.2.2)
END_DEPLIST()

uintptr_t pGTASA;
void* hGTASA;

// Dont ask me, dont be dumb, use google
#include "configs_part.inl"

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
//////      Variables
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
#define HEIGHT_DELTA 1.7f
MobileMenu *gMobileMenu;
float *m_radarRange;
int *ms_numTags;
tTagDesc *ms_tagDesc;
CPool<CStuntJump> **mp_poolStuntJumps;
CPickup *aPickUps;
CPlayerInfo *WorldPlayers;

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
//////      Functions
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
CPlayerPed* (*FindPlayerPed)(int);
void (*ShowRadarTraceWithHeight)(float fXPos, float fYPos, UInt32 Size, UInt8 R, UInt8 G, UInt8 B, UInt8 A, uint8 h);
void (*TransformRealWorldPointToRadarSpace)(CVector2D &,CVector2D const&);
void (*LimitRadarPoint)(CVector2D &);
void (*TransformRadarPointToScreenSpace)(CVector2D &,CVector2D const&);

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
//////      Stocks
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
inline void DrawRadarTrace(float screenX, float screenY, float heightDifference, rgba_t color)
{
    int mode = RADAR_TRACE_LOW;
    if (heightDifference <= (HEIGHT_DELTA))
    {
        if (heightDifference < (-2 * HEIGHT_DELTA)) mode = RADAR_TRACE_HIGH;
        else mode = RADAR_TRACE_NORMAL;
    }
    ShowRadarTraceWithHeight(screenX, screenY, 1, color.r, color.g, color.b, color.a, mode);
}
inline void DrawRadarTrace(float screenX, float screenY, rgba_t color)
{
    ShowRadarTraceWithHeight(screenX, screenY, 1, color.r, color.g, color.b, color.a, RADAR_TRACE_NORMAL);
}

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
//////      Tags
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
inline void Radar_DrawTags(const CVector& center)
{
    static CVector2D radarSpace, screenSpace;
    bool drawTag = true;
    float nearestTag = 9999999.f;
    int indexTag = -1, tagsCount = *ms_numTags;
    const float radarRange = *m_radarRange;

    for (int i = 0; i < tagsCount; ++i)
    {
        if(ms_tagDesc[i].m_pEntity == NULL || ms_tagDesc[i].m_alpha > 228) continue;

        const CVector& tagPos = ms_tagDesc[i].m_pEntity->m_placement.m_vPosn;
        const float distance = DistanceBetweenPoints(center.m_vec2D, tagPos.m_vec2D);

        if (distance < radarRange)
        {
            TransformRealWorldPointToRadarSpace(radarSpace, tagPos.m_vec2D);
            LimitRadarPoint(radarSpace);
            TransformRadarPointToScreenSpace(screenSpace, radarSpace);

            drawTag = false;
            DrawRadarTrace(screenSpace.x, screenSpace.y, tagPos.z - center.z, TagsColor);
        }
        else if (drawTag && ShowNearest && distance < nearestTag)
        {
            nearestTag = distance;
            indexTag = i;
        }
    }

    if(!ShowNearest) return;

    if (drawTag && indexTag >= 0)
    {
        if(ms_tagDesc[indexTag].m_pEntity == NULL) return;
        const CVector& tagPos = ms_tagDesc[indexTag].m_pEntity->m_placement.m_vPosn;

        TransformRealWorldPointToRadarSpace(radarSpace, tagPos.m_vec2D);
        LimitRadarPoint(radarSpace);
        TransformRadarPointToScreenSpace(screenSpace, radarSpace);

        DrawRadarTrace(screenSpace.x, screenSpace.y, tagPos.z - center.z, TagsColor);
    }
}
inline void Map_DrawTags(const CVector& center, bool showHeight)
{
    static CVector2D radarSpace, screenSpace;
    int tagsCount = *ms_numTags;

    for (int i = 0; i < tagsCount; ++i)
    {
        if(ms_tagDesc[i].m_pEntity == NULL || ms_tagDesc[i].m_alpha > 228) continue;

        const CVector& tagPos = ms_tagDesc[i].m_pEntity->m_placement.m_vPosn;

        TransformRealWorldPointToRadarSpace(radarSpace, tagPos.m_vec2D);
        TransformRadarPointToScreenSpace(screenSpace, radarSpace);

        if(showHeight) DrawRadarTrace(screenSpace.x, screenSpace.y, tagPos.z - center.z, TagsColor);
        else DrawRadarTrace(screenSpace.x, screenSpace.y, TagsColor);
    }
}

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
//////      Stunt Jumps
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
inline void Radar_DrawStuntJumps(const CVector& center)
{
    static CVector2D radarSpace, screenSpace;
    bool drawTag = true;
    float nearestTag = 9999999.f;
    auto stuntJumpsPool = (*mp_poolStuntJumps);
    int indexTag = -1, tagsCount = stuntJumpsPool->m_nSize;
    const float radarRange = *m_radarRange;

    for (int i = 0; i < tagsCount; ++i)
    {
        const CStuntJump& stuntjump = stuntJumpsPool->m_pObjects[i];
        if(stuntJumpsPool->m_byteMap[i].bEmpty || stuntjump.m_bPassed) continue;

        const CVector& tagPos = {
            0.5f * (stuntjump.m_bbStart.m_vecMin.x + stuntjump.m_bbStart.m_vecMax.x),
            0.5f * (stuntjump.m_bbStart.m_vecMin.y + stuntjump.m_bbStart.m_vecMax.y),
            stuntjump.m_bbStart.m_vecMin.z
        };
        const float distance = DistanceBetweenPoints(center.m_vec2D, tagPos.m_vec2D);

        if (distance < radarRange)
        {
            TransformRealWorldPointToRadarSpace(radarSpace, tagPos.m_vec2D);
            LimitRadarPoint(radarSpace);
            TransformRadarPointToScreenSpace(screenSpace, radarSpace);

            drawTag = false;
            DrawRadarTrace(screenSpace.x, screenSpace.y, tagPos.z - center.z, StuntJumpsColor);
        }
        else if (drawTag && ShowNearest && distance < nearestTag)
        {
            nearestTag = distance;
            indexTag = i;
        }
    }

    if(!ShowNearest) return;

    if (drawTag && indexTag >= 0)
    {
        const CStuntJump& stuntjump = stuntJumpsPool->m_pObjects[indexTag];
        if(stuntJumpsPool->m_byteMap[indexTag].bEmpty || stuntjump.m_bPassed) return;

        const CVector& tagPos = {
            0.5f * (stuntjump.m_bbStart.m_vecMin.x + stuntjump.m_bbStart.m_vecMax.x),
            0.5f * (stuntjump.m_bbStart.m_vecMin.y + stuntjump.m_bbStart.m_vecMax.y),
            stuntjump.m_bbStart.m_vecMin.z
        };

        TransformRealWorldPointToRadarSpace(radarSpace, tagPos.m_vec2D);
        LimitRadarPoint(radarSpace);
        TransformRadarPointToScreenSpace(screenSpace, radarSpace);

        DrawRadarTrace(screenSpace.x, screenSpace.y, tagPos.z - center.z, StuntJumpsColor);
    }
}
inline void Map_DrawStuntJumps(const CVector& center, bool showHeight)
{
    static CVector2D radarSpace, screenSpace;
    auto stuntJumpsPool = (*mp_poolStuntJumps);
    int tagsCount = stuntJumpsPool->m_nSize;

    for (int i = 0; i < tagsCount; ++i)
    {
        const CStuntJump& stuntjump = stuntJumpsPool->m_pObjects[i];
        if(stuntJumpsPool->m_byteMap[i].bEmpty || stuntjump.m_bPassed) continue;

        const CVector& tagPos = {
            0.5f * (stuntjump.m_bbStart.m_vecMin.x + stuntjump.m_bbStart.m_vecMax.x),
            0.5f * (stuntjump.m_bbStart.m_vecMin.y + stuntjump.m_bbStart.m_vecMax.y),
            stuntjump.m_bbStart.m_vecMin.z
        };

        TransformRealWorldPointToRadarSpace(radarSpace, tagPos.m_vec2D);
        TransformRadarPointToScreenSpace(screenSpace, radarSpace);

        if(showHeight) DrawRadarTrace(screenSpace.x, screenSpace.y, tagPos.z - center.z, StuntJumpsColor);
        else DrawRadarTrace(screenSpace.x, screenSpace.y, StuntJumpsColor);
    }
}
inline bool IsAllowedCollectible(int model, int type)
{
    if(type == PICKUP_ONCE)
    {
        return (ShowOysters && model == 953) || (ShowHorseshoes && model == 954);
    }
    return (ShowSnapshots && model == 1253 && type == PICKUP_SNAPSHOT);
}
inline bool IsAllowedPickup(int model, int type)
{
    if(type != PICKUP_ON_STREET && type != PICKUP_ON_STREET_SLOW) return false;
    return (ShowBribes && model == 1247) || (ShowHealths && model == 1240) || (ShowArmours && model == 1242) || (ShowWeapons && model >= 321 && model <= 372 && model != 370);
}

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
//////      PickUps
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
inline void Radar_DrawPickups(const CVector& center)
{
    static CVector2D radarSpace, screenSpace;
    
    bool drawOyster = true, drawHorse = true, drawSnap = true;
    float nearestOyster = 9999999.f, nearestHorse = 9999999.f, nearestSnap = 9999999.f;
    int indexOyster = -1, indexHorse = -1, indexSnap = -1;
    const float radarRange = *m_radarRange;

    for (int i = 0; i < 620; ++i)
    {
        CPickup& pickup = aPickUps[i];
        if(pickup.m_nFlags.bDisabled || pickup.m_vecPos.z >= 7680) continue;

        const short model = pickup.m_nModelIndex;
        const unsigned char type = pickup.m_nPickupType;

        if(IsAllowedCollectible(model, type) || IsAllowedPickup(model, type))
        {
            const CVector pickupPos(UncompressLargeVector(pickup.m_vecPos));
            const float distance = DistanceBetweenPoints(center.m_vec2D, pickupPos.m_vec2D);
            if(distance < radarRange)
            {
                TransformRealWorldPointToRadarSpace(radarSpace, pickupPos.m_vec2D);
                LimitRadarPoint(radarSpace);
                TransformRadarPointToScreenSpace(screenSpace, radarSpace);

                float heightDiff = pickupPos.z - center.z;
                switch(model)
                {
                    case 953:
                        drawOyster = false;
                        DrawRadarTrace(screenSpace.x, screenSpace.y, heightDiff, OystersColor);
                        break;
                    case 954:
                        drawHorse = false;
                        DrawRadarTrace(screenSpace.x, screenSpace.y, heightDiff, HorseshoesColor);
                        break;
                    case 1253:
                        drawSnap = false;
                        DrawRadarTrace(screenSpace.x, screenSpace.y, heightDiff, SnapshotsColor);
                        break;
                    case 1247:
                        DrawRadarTrace(screenSpace.x, screenSpace.y, heightDiff, BribesColor);
                        break;
                    case 1242:
                        DrawRadarTrace(screenSpace.x, screenSpace.y, heightDiff, ArmoursColor);
                        break;
                    case 1240:
                        DrawRadarTrace(screenSpace.x, screenSpace.y, heightDiff, HealthsColor);
                        break;
                    default:
                        DrawRadarTrace(screenSpace.x, screenSpace.y, heightDiff, WeaponsColor);
                        break;
                }
            }
            else if(ShowNearest)
            {
                switch(model)
                {
                    case 953:
                        if(drawOyster && distance < nearestOyster)
                        {
                            nearestOyster = distance;
                            indexOyster = i;
                        }
                        break;
                    case 954:
                        if(drawHorse && distance < nearestHorse)
                        {
                            nearestHorse = distance;
                            indexHorse = i;
                        }
                        break;
                    case 1253:
                        if(drawSnap && distance < nearestSnap)
                        {
                            nearestSnap = distance;
                            indexSnap = i;
                        }
                        break;
                }
            }
        }
    }

    if(!ShowNearest) return;

    if (drawOyster && indexOyster >= 0)
    {
        CPickup& pickup = aPickUps[indexOyster];
        const CVector pickupPos(UncompressLargeVector(pickup.m_vecPos));
        TransformRealWorldPointToRadarSpace(radarSpace, pickupPos.m_vec2D);
        LimitRadarPoint(radarSpace);
        TransformRadarPointToScreenSpace(screenSpace, radarSpace);

        DrawRadarTrace(screenSpace.x, screenSpace.y, pickupPos.z - center.z, OystersColor);
    }
    if (drawHorse && indexHorse >= 0)
    {
        CPickup& pickup = aPickUps[indexHorse];
        const CVector pickupPos(UncompressLargeVector(pickup.m_vecPos));
        TransformRealWorldPointToRadarSpace(radarSpace, pickupPos.m_vec2D);
        LimitRadarPoint(radarSpace);
        TransformRadarPointToScreenSpace(screenSpace, radarSpace);

        DrawRadarTrace(screenSpace.x, screenSpace.y, pickupPos.z - center.z, HorseshoesColor);
    }
    if (drawSnap && indexSnap >= 0)
    {
        CPickup& pickup = aPickUps[indexSnap];
        const CVector pickupPos(UncompressLargeVector(pickup.m_vecPos));
        TransformRealWorldPointToRadarSpace(radarSpace, pickupPos.m_vec2D);
        LimitRadarPoint(radarSpace);
        TransformRadarPointToScreenSpace(screenSpace, radarSpace);

        DrawRadarTrace(screenSpace.x, screenSpace.y, pickupPos.z - center.z, SnapshotsColor);
    }
}
inline void Map_DrawPickups(const CVector& center, bool showHeight)
{
    static CVector2D radarSpace, screenSpace;

    for (int i = 0; i < 620; ++i)
    {
        CPickup& pickup = aPickUps[i];
        if(pickup.m_nFlags.bDisabled || pickup.m_vecPos.z >= 7680) continue;

        const short model = pickup.m_nModelIndex;
        const unsigned char type = pickup.m_nPickupType;

        if(IsAllowedCollectible(model, type) || IsAllowedPickup(model, type))
        {
            const CVector pickupPos(UncompressLargeVector(pickup.m_vecPos));
            TransformRealWorldPointToRadarSpace(radarSpace, pickupPos.m_vec2D);
            TransformRadarPointToScreenSpace(screenSpace, radarSpace);

            float heightDiff = pickupPos.z - center.z;
            switch(model)
            {
                case 953:
                    if(showHeight) DrawRadarTrace(screenSpace.x, screenSpace.y, heightDiff, OystersColor);
                    else DrawRadarTrace(screenSpace.x, screenSpace.y, OystersColor);
                    break;
                case 954:
                    if(showHeight) DrawRadarTrace(screenSpace.x, screenSpace.y, heightDiff, HorseshoesColor);
                    else DrawRadarTrace(screenSpace.x, screenSpace.y, HorseshoesColor);
                    break;
                case 1253:
                    if(showHeight) DrawRadarTrace(screenSpace.x, screenSpace.y, heightDiff, SnapshotsColor);
                    else DrawRadarTrace(screenSpace.x, screenSpace.y, SnapshotsColor);
                    break;
                case 1247:
                    if(showHeight) DrawRadarTrace(screenSpace.x, screenSpace.y, heightDiff, BribesColor);
                    else DrawRadarTrace(screenSpace.x, screenSpace.y, BribesColor);
                    break;
                case 1242:
                    if(showHeight) DrawRadarTrace(screenSpace.x, screenSpace.y, heightDiff, ArmoursColor);
                    else DrawRadarTrace(screenSpace.x, screenSpace.y, ArmoursColor);
                    break;
                case 1240:
                    if(showHeight) DrawRadarTrace(screenSpace.x, screenSpace.y, heightDiff, HealthsColor);
                    else DrawRadarTrace(screenSpace.x, screenSpace.y, HealthsColor);
                    break;
                default:
                    if(showHeight) DrawRadarTrace(screenSpace.x, screenSpace.y, heightDiff, WeaponsColor);
                    else DrawRadarTrace(screenSpace.x, screenSpace.y, WeaponsColor);
                    break;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
//////      Hooks
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
DECL_HOOKv(DrawRadarBlips, float circleSize)
{
    DrawRadarBlips(circleSize);

    // Dont do any of the code below this line
    if(!EnableMod) return;
    
  #ifdef AML32
    CPlayerPed* player = WorldPlayers[0].m_pPed;
    CVector& playerPos = (WorldPlayers[0].m_pRemoteVehicle) ? WorldPlayers[0].m_pRemoteVehicle->GetPosition() : player->GetPosition();
  #else
    CPlayerPed* player = WorldPlayers[0].pPed;
    CVector& playerPos = (WorldPlayers[0].pRemoteVehicle) ? WorldPlayers[0].pRemoteVehicle->GetPosition() : player->GetPosition();
  #endif
    if(gMobileMenu->m_bDrawMenuMap)
    {
        bool showHeight = player->m_nInterior == 0;
        if(ShowTags) Map_DrawTags(playerPos, showHeight);
        if(ShowStuntJumps) Map_DrawStuntJumps(playerPos, showHeight);
        Map_DrawPickups(playerPos, showHeight);
    }
    else
    {
        if(player->m_nInterior == 0)
        {
            if(ShowTags) Radar_DrawTags(playerPos);
            if(ShowStuntJumps) Radar_DrawStuntJumps(playerPos);
            Radar_DrawPickups(playerPos);
        }
    }
}


////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
//////      Main
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
extern "C" void OnAllModsLoaded()
{
    logger->SetTag("CollectiblesOnRadar");

    pGTASA = aml->GetLib("libGTASA.so");
    hGTASA = aml->GetLibHandle("libGTASA.so");

    // We dont cheat in multiplayer games.
    if(aml->GetLibHandle("libsamp.so") != NULL || aml->GetLibHandle("libvoice.so") != NULL ||
       aml->GetLibHandle("libAlyn_SAMPMOBILE.so") != NULL || aml->HasMod("net.rusjj.resamp")) return;

    sautils = (ISAUtils*)GetInterface("SAUtils");

    SET_TO(gMobileMenu, aml->GetSym(hGTASA, "gMobileMenu"));
    SET_TO(m_radarRange, aml->GetSym(hGTASA, "_ZN6CRadar12m_radarRangeE"));
    SET_TO(ms_numTags, aml->GetSym(hGTASA, "_ZN11CTagManager10ms_numTagsE"));
    SET_TO(ms_tagDesc, aml->GetSym(hGTASA, "_ZN11CTagManager10ms_tagDescE"));
    SET_TO(mp_poolStuntJumps, aml->GetSym(hGTASA, "_ZN17CStuntJumpManager17mp_poolStuntJumpsE"));
    SET_TO(aPickUps, *(uintptr_t*)(pGTASA + BYVER(0x678BF8, 0x84F818)));
    SET_TO(WorldPlayers, *(uintptr_t*)(pGTASA + BYVER(0x6783C8, 0x84E7A8)));

    SET_TO(FindPlayerPed, aml->GetSym(hGTASA, "_Z13FindPlayerPedi"));
    SET_TO(ShowRadarTraceWithHeight, aml->GetSym(hGTASA, "_ZN6CRadar24ShowRadarTraceWithHeightEffjhhhhh"));
    SET_TO(TransformRealWorldPointToRadarSpace, aml->GetSym(hGTASA, "_ZN6CRadar35TransformRealWorldPointToRadarSpaceER9CVector2DRKS0_"));
    SET_TO(LimitRadarPoint, aml->GetSym(hGTASA, "_ZN6CRadar15LimitRadarPointER9CVector2D"));
    SET_TO(TransformRadarPointToScreenSpace, aml->GetSym(hGTASA, "_ZN6CRadar32TransformRadarPointToScreenSpaceER9CVector2DRKS0_"));

    HOOKPLT(DrawRadarBlips, pGTASA + BYVER(0x66E910, 0x83DED0));

    InitializeConfigsPart();
    if(sautils) InitializeSAUtilsPart();
}

static Config cfgLocal("CollectiblesOnRadar.SA");
Config* cfg = &cfgLocal;
