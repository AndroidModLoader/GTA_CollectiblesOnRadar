/* Boom! This one is included in the main file! */

const char* pYesNo[] = 
{
    "FEM_OFF",
    "FEM_ON",
};
enum eSettingToChange : unsigned char
{
    STC_EnableMod,
    STC_ShowTags,
    STC_ShowSnapshots,
    STC_ShowHorseshoes,
    STC_ShowOysters,
    STC_ShowStuntJumps,
    STC_ShowNearest,
    STC_ShowBribes,
    STC_ShowArmours,
    STC_ShowWeapons,
};

ConfigEntry *cfgEnableMod; bool EnableMod;
ConfigEntry *cfgShowTags; bool ShowTags;
ConfigEntry *cfgShowSnapshots; bool ShowSnapshots;
ConfigEntry *cfgShowHorseshoes; bool ShowHorseshoes;
ConfigEntry *cfgShowOysters; bool ShowOysters;
ConfigEntry *cfgShowStuntJumps; bool ShowStuntJumps;
ConfigEntry *cfgShowNearest; bool ShowNearest;
ConfigEntry *cfgShowBribes; bool ShowBribes;
ConfigEntry *cfgShowArmours; bool ShowArmours;
ConfigEntry *cfgShowWeapons; bool ShowWeapons;

rgba_t TagsColor {0x55, 0xFF, 0x55, 0xFF};
rgba_t SnapshotsColor {0xFF, 0xA1, 0xD0, 0xFF};
rgba_t HorseshoesColor {0x93, 0xC0, 0xEE, 0xFF};
rgba_t OystersColor {0xFF, 0xFF, 0x99, 0xFF};
rgba_t StuntJumpsColor {0xCC, 0x99, 0xFF, 0xFF};
rgba_t BribesColor {0xFF, 0xFF, 0x00, 0xA0};
rgba_t ArmoursColor {0xFF, 0xFF, 0xFF, 0xA0};
rgba_t WeaponsColor {0xFF, 0x00, 0xFF, 0xA0};

void OnSettingChange(int oldVal, int newVal, void* data)
{
    eSettingToChange set = (eSettingToChange)(intptr_t)data; // why do i need to cast it to int first? bruh compiler moment
    switch(set)
    {
        case STC_EnableMod:
        {
            EnableMod = newVal != 0;
            cfgEnableMod->SetBool(newVal != 0);
            break;
        }
        case STC_ShowTags:
        {
            ShowTags = newVal != 0;
            cfgShowTags->SetBool(newVal != 0);
            break;
        }
        case STC_ShowSnapshots:
        {
            ShowSnapshots = newVal != 0;
            cfgShowSnapshots->SetBool(newVal != 0);
            break;
        }
        case STC_ShowHorseshoes:
        {
            ShowHorseshoes = newVal != 0;
            cfgShowHorseshoes->SetBool(newVal != 0);
            break;
        }
        case STC_ShowOysters:
        {
            ShowOysters = newVal != 0;
            cfgShowOysters->SetBool(newVal != 0);
            break;
        }
        case STC_ShowStuntJumps:
        {
            ShowStuntJumps = newVal != 0;
            cfgShowStuntJumps->SetBool(newVal != 0);
            break;
        }
        case STC_ShowNearest:
        {
            ShowNearest = newVal != 0;
            cfgShowNearest->SetBool(newVal != 0);
            break;
        }
        case STC_ShowBribes:
        {
            ShowBribes = newVal != 0;
            cfgShowBribes->SetBool(newVal != 0);
            break;
        }
        case STC_ShowArmours:
        {
            ShowArmours = newVal != 0;
            cfgShowArmours->SetBool(newVal != 0);
            break;
        }
        case STC_ShowWeapons:
        {
            ShowWeapons = newVal != 0;
            cfgShowWeapons->SetBool(newVal != 0);
            break;
        }
    }
    cfg->Save();
}

void InitializeConfigsPart()
{
    cfgEnableMod = cfg->Bind("enable_mod", true, "MAIN"); EnableMod = cfgEnableMod->GetBool();
    cfgShowTags = cfg->Bind("show_tags", true, "MAIN"); ShowTags = cfgShowTags->GetBool();
    cfgShowSnapshots = cfg->Bind("show_snapshots", true, "MAIN"); ShowSnapshots = cfgShowSnapshots->GetBool();
    cfgShowHorseshoes = cfg->Bind("show_horseshoes", true, "MAIN"); ShowHorseshoes = cfgShowHorseshoes->GetBool();
    cfgShowOysters = cfg->Bind("show_oysters", true, "MAIN"); ShowOysters = cfgShowOysters->GetBool();
    cfgShowStuntJumps = cfg->Bind("show_usjs", true, "MAIN"); ShowStuntJumps = cfgShowStuntJumps->GetBool();
    cfgShowNearest = cfg->Bind("show_nearest", true, "MAIN"); ShowNearest = cfgShowNearest->GetBool();
    cfgShowBribes = cfg->Bind("show_bribes", false, "EXTRA"); ShowBribes = cfgShowBribes->GetBool();
    cfgShowArmours = cfg->Bind("show_armours", false, "EXTRA"); ShowArmours = cfgShowArmours->GetBool();
    cfgShowWeapons = cfg->Bind("show_weapons", false, "EXTRA"); ShowWeapons = cfgShowWeapons->GetBool();

    TagsColor = cfg->GetColor("color_tag", TagsColor, "COLORS");
    SnapshotsColor = cfg->GetColor("color_snapshot", SnapshotsColor, "COLORS");
    HorseshoesColor = cfg->GetColor("color_horseshoe", HorseshoesColor, "COLORS");
    OystersColor = cfg->GetColor("color_oyster", OystersColor, "COLORS");
    StuntJumpsColor = cfg->GetColor("color_usj", StuntJumpsColor, "COLORS");
    BribesColor = cfg->GetColor("color_bribe", BribesColor, "EXTRA");
    ArmoursColor = cfg->GetColor("color_armour", ArmoursColor, "EXTRA");
    WeaponsColor = cfg->GetColor("color_weapon", WeaponsColor, "EXTRA");
}

void InitializeSAUtilsPart()
{
    eTypeOfSettings settingTab = sautils->AddSettingsTab("Collectibles On Radar");

    sautils->AddClickableItem(eTypeOfSettings::SetType_Mods, "Collectibles On Radar", cfgEnableMod->GetInt(), 0, sizeofA(pYesNo)-1, pYesNo, OnSettingChange, (void*)STC_EnableMod);
    sautils->AddClickableItem(settingTab, "Show Spray Tags", cfgShowTags->GetInt(), 0, sizeofA(pYesNo)-1, pYesNo, OnSettingChange, (void*)STC_ShowTags);
    sautils->AddClickableItem(settingTab, "Show Camera Snapshots", cfgShowSnapshots->GetInt(), 0, sizeofA(pYesNo)-1, pYesNo, OnSettingChange, (void*)STC_ShowSnapshots);
    sautils->AddClickableItem(settingTab, "Show Horseshoes", cfgShowHorseshoes->GetInt(), 0, sizeofA(pYesNo)-1, pYesNo, OnSettingChange, (void*)STC_ShowHorseshoes);
    sautils->AddClickableItem(settingTab, "Show Oysters", cfgShowOysters->GetInt(), 0, sizeofA(pYesNo)-1, pYesNo, OnSettingChange, (void*)STC_ShowOysters);
    sautils->AddClickableItem(settingTab, "Show Stunt Jumps", cfgShowStuntJumps->GetInt(), 0, sizeofA(pYesNo)-1, pYesNo, OnSettingChange, (void*)STC_ShowStuntJumps);
    sautils->AddClickableItem(settingTab, "Show the Nearest", cfgShowNearest->GetInt(), 0, sizeofA(pYesNo)-1, pYesNo, OnSettingChange, (void*)STC_ShowNearest);
    sautils->AddClickableItem(settingTab, "Show Bribes", cfgShowBribes->GetInt(), 0, sizeofA(pYesNo)-1, pYesNo, OnSettingChange, (void*)STC_ShowBribes);
    sautils->AddClickableItem(settingTab, "Show Armours", cfgShowArmours->GetInt(), 0, sizeofA(pYesNo)-1, pYesNo, OnSettingChange, (void*)STC_ShowArmours);
    sautils->AddClickableItem(settingTab, "Show Weapons", cfgShowWeapons->GetInt(), 0, sizeofA(pYesNo)-1, pYesNo, OnSettingChange, (void*)STC_ShowWeapons);
}