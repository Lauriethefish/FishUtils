#include "auto_settings.hpp"
#include "main.hpp"

#include "System/Action.hpp"
#include "System/Action_2.hpp"

#include "GlobalNamespace/BeatmapData.hpp"
#include "GlobalNamespace/IBeatmapLevelData.hpp"
#include "GlobalNamespace/BeatmapLevelPack.hpp"
#include "GlobalNamespace/StandardLevelScenesTransitionSetupDataSO.hpp"
#include "GlobalNamespace/LevelCompletionResults.hpp"
#include "GlobalNamespace/BeatmapLevelsModel.hpp"
#include "GlobalNamespace/MenuTransitionsHelper.hpp"
#include "GlobalNamespace/BeatmapDifficulty.hpp"

#include "UnityEngine/Resources.hpp"
#include "UnityEngine/AudioClip.hpp"

namespace FishUtils::AutoSettings {
    class DebrisSetting : public SettingType {
    public:
        DebrisSetting() {
            this->name = "Reduce Debris";
            this->saveName = "reduceDebris";
            this->options = {
                "No Debris",
                "All Debris"
            };
        }

        virtual void Apply(PlayerSpecificSettings* settings, std::string option) {
            if(option == "No Debris") {
                settings->reduceDebris = true;
            }   else if(option == "All Debris") {
                settings->reduceDebris = false;
            }
        }
    };

    class HUDSetting : public SettingType {
    public:
        HUDSetting() {
            this->name = "HUD Type";
            this->saveName = "hudType";
            this->options = {
                "Advanced HUD",
                "Basic HUD",
                "No HUD"
            };
        }

        virtual void Apply(PlayerSpecificSettings* settings, std::string option) {
            if(option == "Advanced HUD") {
                settings->advancedHud = true;
                settings->noTextsAndHuds = false;
            }   else if(option == "Basic HUD") {
                settings->advancedHud = false;
                settings->noTextsAndHuds = false;
            }   else if(option == "No HUD") {
                settings->advancedHud = false;
                settings->noTextsAndHuds = true;
            }
        }
    };

    class EnvironmentSetting : public SettingType {
        bool isExpertPlus;

    public:
        EnvironmentSetting(bool isExpertPlus) {
            this->isExpertPlus = isExpertPlus;

            if(isExpertPlus) {
                this->name = "Expert+ Environment Effects";
                this->saveName = "expertPlusEffects";
            }   else    {
                this->name = "Environment Effects";
                this->saveName = "effects";
            }
            this->options = {
                "No Effects",
                "No Flickering",
                "All Effects"
            };
        }

        virtual void Apply(PlayerSpecificSettings* settings, std::string option) {
            EnvironmentEffectsFilterPreset effectsFilter;
            if(option == "All Effects") {
                effectsFilter = EnvironmentEffectsFilterPreset::AllEffects;
            }   else if(option == "No Flickering") {
                effectsFilter = EnvironmentEffectsFilterPreset::StrobeFilter;
            }   else if(option == "No Effects") {
                effectsFilter = EnvironmentEffectsFilterPreset::NoEffects;
            }

            if(isExpertPlus) {
                settings->environmentEffectsFilterExpertPlusPreset = effectsFilter;
            }   else    {
                settings->environmentEffectsFilterDefaultPreset = effectsFilter;
            }
        }
    };

    std::vector<SettingType*>& GetSettingTypes() {
        static std::vector<SettingType*> settingTypes {
            new DebrisSetting(),
            new HUDSetting(),
            new EnvironmentSetting(false),
            new EnvironmentSetting(true)
        };
        
        return settingTypes;
    }

    class NJSMapElement : public MapElementType {
    public:
        NJSMapElement() {
            this->name = "Note Jump Speed";
            this->saveName = "njs";
            this->reasonableMinValue = 5.0f;
            this->reasonableMaxValue = 20.0f;
        }

        virtual float Get(IDifficultyBeatmap* difficulty) {
            return difficulty->get_noteJumpMovementSpeed();
        }
    };

    class OffsetMapElement : public MapElementType {
    public:
        OffsetMapElement() {
            this->name = "Note Jump Offset";
            this->saveName = "offset";
            this->reasonableMinValue = -0.5f;
            this->reasonableMaxValue = 1.0f;
        }

        virtual float Get(IDifficultyBeatmap* difficulty) {
            return difficulty->get_noteJumpStartBeatOffset();
        }
    };

    class NPSMapElement : public MapElementType {
    public:
        NPSMapElement() {
            this->name = "Notes per Second";
            this->saveName = "nps";
            this->reasonableMinValue = 5.0f;
            this->reasonableMaxValue = 15.0f;
        }

        virtual float Get(IDifficultyBeatmap* difficulty) {
            BeatmapData* beatmapData = difficulty->get_beatmapData();
            IBeatmapLevelData* beatmapLevelData = difficulty->get_level()->get_beatmapLevelData();
            int notesCount = beatmapData->get_cuttableNotesType();

            // Find the notes per second
            return notesCount / beatmapLevelData->get_audioClip()->get_length();
        }
    };

    std::vector<MapElementType*>& GetMapElementTypes() {
        static std::vector<MapElementType*> mapElementTypes {
            new NPSMapElement(),
            new NJSMapElement(),
            new OffsetMapElement()
        };
        getLogger().info("Map element types length: %lu", mapElementTypes.size());

        return mapElementTypes;
    }

    SettingType* GetSettingType(std::string name, bool useSaveName) {
        for(SettingType* settingType : GetSettingTypes()) {
            if(useSaveName) {
                if(settingType->saveName == name) {
                    return settingType;
                }            
            }   else    {
                if(settingType->name == name) {
                    return settingType;
                }            
            }
        }

        return nullptr;
    }

    MapElementType* GetMapElementType(std::string name, bool useSaveName) {
        for(MapElementType* mapElementType : GetMapElementTypes()) {
            if(useSaveName) {
                if(mapElementType->saveName == name) {
                    return mapElementType;
                }            
            }   else    {
                if(mapElementType->name == name) {
                    return mapElementType;
                }            
            }
        }

        return nullptr;
    }

    SettingConfiguration::SettingConfiguration(rapidjson::Value& value) {
        std::string settingTypeName = value["settingTypeName"].GetString();
        this->settingType = GetSettingType(settingTypeName);

        std::string elementTypeName = value["elementTypeName"].GetString();
        for(MapElementType* elementType : GetMapElementTypes()) {
            if(elementType->saveName == elementTypeName) {
                this->configureBasedOn = elementType;
            }
        }

        rapidjson::GenericArray thresholdsArray = value["thresholds"].GetArray();
        for(rapidjson::Value& value : thresholdsArray) {
            this->thresholds.push_back(value.GetFloat());
        }
    
        this->SetFlipOptions(value["flipOptions"].GetBool());

        rapidjson::GenericArray playlistsArray = value["overriddenInPlaylists"].GetArray();
        for(rapidjson::Value& value : playlistsArray) {
            this->overriddenInPlaylists.push_back(value.GetString());
        }

        this->playlistOverrideSetting = value["playlistOverrideSetting"].GetString();
    }

    SettingConfiguration::SettingConfiguration(SettingType* settingType) {
        this->settingType = settingType;
        this->SetFlipOptions(false);

        std::vector<MapElementType*>& elementTypes = GetMapElementTypes();
        MapElementType* elementType = elementTypes[0];
        this->SetConfigureBasedOn(elementType); // Use the first map element type available
    }

    void SettingConfiguration::SetConfigureBasedOn(MapElementType* elementType) {
        float min = elementType->reasonableMinValue;
        float max = elementType->reasonableMaxValue;
        int thresholdsRequired = settingType->options.size() - 1;
        float increment = (max - min) / thresholdsRequired;

        this->thresholds.clear();
        float currentValue = max - increment;
        for(int i = 0; i < thresholdsRequired; i++) {
            this->thresholds.push_back(currentValue);
            currentValue -= increment;
        }

        this->configureBasedOn = elementType;
    }

    void SettingConfiguration::OverrideIfNecessary(PlayerSpecificSettings* playerSettings, IPreviewBeatmapLevel* level, IDifficultyBeatmap* difficulty) {
        static BeatmapLevelsModel* beatmapLevelsModel = UnityEngine::Resources::FindObjectsOfTypeAll<BeatmapLevelsModel*>()->values[0];
        getLogger().info("Checking SettingConfiguration of type %s to see if overriding is necessary", this->settingType->saveName.c_str());

        if(!playlistOverrideSetting.empty()) {
            getLogger().info("Playlist overrides enabled, checking first!");
            IBeatmapLevelPack* levelPlaylist = beatmapLevelsModel->GetLevelPackForLevelId(level->get_levelID());
            std::string levelPlaylistName = to_utf8(csstrtostr(levelPlaylist->get_packName()));

            if(IsOverriddenInPlaylist(levelPlaylistName)) {
                getLogger().info("Setting is overridden in this playlist (%s). Setting to %s", levelPlaylistName.c_str(), playlistOverrideSetting.c_str());
                this->settingType->Apply(playerSettings, this->playlistOverrideSetting);
                return;
            }   else    {
                getLogger().info("Setting is not overridden in this playlist (%s). Using thresholds instead", levelPlaylistName.c_str());
            }
        }

        float elementValue = this->configureBasedOn->Get(difficulty);

        std::vector<std::string>& options = this->options;

        std::string chosenOption;
        int i = 0;
        while(true) {
            getLogger().info("Checking for %s", options[i].c_str());
            float threshold = thresholds[i];
            getLogger().info("Value: %f Element value: %f", threshold, elementValue);
            if(elementValue > threshold) {
                std::string option = options[i];
                getLogger().info("Option found that is at threshold: %s", option.c_str());
                chosenOption = option;
                break;
            }

            i++;

            if(i == options.size() - 1) {
                getLogger().info("No thresholds met, using bottom option of %s", options[i].c_str());
                chosenOption = options[i];
                break;
            }
        }

        getLogger().info("Overriding setting to %s", chosenOption.c_str());
        this->settingType->Apply(playerSettings, chosenOption);
    }


    void SettingConfiguration::Save(rapidjson::Value& value, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator) {
        value.AddMember("settingTypeName", rapidjson::Value().SetString(this->settingType->saveName, allocator), allocator);
        value.AddMember("elementTypeName", rapidjson::Value().SetString(this->configureBasedOn->saveName, allocator), allocator);
        
        rapidjson::Value thresholdsArray(rapidjson::kArrayType);
        for(float threshold : thresholds) {
            thresholdsArray.PushBack(threshold, allocator);
        }
        value.AddMember("thresholds", thresholdsArray, allocator);

        value.AddMember("flipOptions", flipOptions, allocator);

        rapidjson::Value playlistsArray(rapidjson::kArrayType);
        for(std::string playlistName : overriddenInPlaylists) {
            playlistsArray.PushBack(rapidjson::Value().SetString(playlistName, allocator), allocator);
        }
        value.AddMember("overriddenInPlaylists", playlistsArray, allocator);

        value.AddMember("playlistOverrideSetting", playlistOverrideSetting, allocator);
    }

    bool SettingConfiguration::IsOverriddenInPlaylist(std::string name) {
        return std::find(overriddenInPlaylists.begin(), overriddenInPlaylists.end(), name) != overriddenInPlaylists.end();
    }

    void SettingConfiguration::SetFlipOptions(bool flipOptions) {
        // Recreate the internal options array, flipping if necessary
        options.clear();
        if(flipOptions) {
            for(int i = settingType->options.size() - 1; i >= 0; i--) {
                options.push_back(settingType->options[i]);
            }
        }   else    {
            options = settingType->options;
        }

        this->flipOptions = flipOptions;
    }

    bool SettingConfiguration::GetFlipOptions() {
        return flipOptions;
    }
    
    static std::unordered_map<std::string, SettingConfiguration> loadedConfigs;
    SettingConfiguration* GetSettingConfiguration(SettingType* settingType) {
        if(!loadedConfigs.contains(settingType->saveName)) {
            return nullptr;
        }
        return &((*loadedConfigs.find(settingType->saveName)).second);
    }

    SettingConfiguration* CreateSettingConfiguration(SettingType* settingType) {
        getLogger().info("Creating new config for setting type %s", settingType->saveName.c_str());
        SettingConfiguration newConfig(settingType);
        loadedConfigs[settingType->saveName] = newConfig;

        return GetSettingConfiguration(settingType);
    }

    void RemoveSettingConfiguration(SettingType* settingType) {
        getLogger().info("Removing config for setting type %s", settingType->saveName.c_str());
        loadedConfigs.erase(settingType->saveName);
    }

    void LoadConfigs() {
        ConfigDocument& config = getConfig().config;
        auto& allocator = config.GetAllocator();
        if(!config.HasMember("autoSettings")) {
            config.AddMember("autoSettings", rapidjson::kArrayType, allocator);
        }

        rapidjson::Value& autoSettingsArray = config["autoSettings"];
        for(rapidjson::Value& value : autoSettingsArray.GetArray()) {
            SettingConfiguration loadedConfig(value);

            loadedConfigs[loadedConfig.settingType->saveName] = loadedConfig;
        }
    }

    void SaveConfigs() {
        getLogger().info("Saving AutoSettings configs . . .");
        ConfigDocument& config = getConfig().config;
        auto& allocator = config.GetAllocator();

        rapidjson::Value autoSettingsArray;
        autoSettingsArray.SetArray();
        for(auto& config : loadedConfigs) {
            rapidjson::Value configValue;
            configValue.SetObject();
            config.second.Save(configValue, allocator);

            autoSettingsArray.PushBack(configValue, allocator);
        }

        config["autoSettings"] = autoSettingsArray;
    }

    void InstallHooks();

    void Init() {
        LoadConfigs();
        InstallHooks();
    }

    // Copies every property of settings to a new instance of PlayerSpecificSettings
    // Used to avoid changing the actual settings when temporarily overriding them for a map
    PlayerSpecificSettings* CloneSettings(PlayerSpecificSettings* settings) {
        PlayerSpecificSettings* clone = PlayerSpecificSettings::New_ctor();
        clone->adaptiveSfx = settings->adaptiveSfx;
        clone->advancedHud = settings->advancedHud;
        clone->automaticPlayerHeight = settings->automaticPlayerHeight;
        clone->autoRestart = settings->autoRestart;
        clone->hideNoteSpawnEffect = settings->hideNoteSpawnEffect;
        clone->leftHanded = settings->leftHanded;
        clone->noFailEffects = settings->noFailEffects;
        clone->noteJumpStartBeatOffset = settings->noteJumpStartBeatOffset;
        clone->noTextsAndHuds = settings->noTextsAndHuds;
        clone->playerHeight = settings->playerHeight;
        clone->reduceDebris = settings->reduceDebris;
        clone->saberTrailIntensity = settings->saberTrailIntensity;
        clone->sfxVolume = settings->sfxVolume;
        clone->environmentEffectsFilterDefaultPreset = settings->environmentEffectsFilterDefaultPreset;
        clone->environmentEffectsFilterExpertPlusPreset = settings->environmentEffectsFilterExpertPlusPreset;
        return clone;
    }

    PlayerSpecificSettings* PerformSettingOverrides(PlayerSpecificSettings* originalSettings, IPreviewBeatmapLevel* previewBeatmapLevel, IDifficultyBeatmap* difficultyBeatmap) {
        PlayerSpecificSettings* clonedSettings = CloneSettings(originalSettings);
        for(auto& pair : loadedConfigs) {
            getLogger().info("Checking setting %s", pair.second.settingType->saveName.c_str());
            pair.second.OverrideIfNecessary(clonedSettings, previewBeatmapLevel, difficultyBeatmap);
        }
        
        getLogger().info("Checked all settings");
        return clonedSettings;
    }
        

    
    // Called when a non-multiplayer/solo/party level is started
    MAKE_HOOK_MATCH(MenuTransitionsHelper_StartStandardLevel, static_cast<void (MenuTransitionsHelper::*)(
        Il2CppString*,
        IDifficultyBeatmap*,
        IPreviewBeatmapLevel*,
        OverrideEnvironmentSettings*,
        ColorScheme*,
        GameplayModifiers*,
        PlayerSpecificSettings*,
        PracticeSettings*,
        Il2CppString*,
        bool,
        System::Action*,
        System::Action_2<StandardLevelScenesTransitionSetupDataSO*, LevelCompletionResults*>*)>(&MenuTransitionsHelper::StartStandardLevel),
        void,
        MenuTransitionsHelper* self,
        Il2CppString* gameMode,
        IDifficultyBeatmap* difficultyBeatmap,
        IPreviewBeatmapLevel* previewBeatmapLevel,
        OverrideEnvironmentSettings* overrideEnvironmentSettings,
        ColorScheme* overrideColorScheme,
        GameplayModifiers* gameplayModifiers,
        PlayerSpecificSettings* playerSpecificSettings,
        PracticeSettings* practiceSettings,
        Il2CppString* backButtonText,
        bool useTestNoteCutCountEffects,
        System::Action* beforeSceneSwitchCallback,
        System::Action_2<StandardLevelScenesTransitionSetupDataSO*, LevelCompletionResults*>* levelFinishedCallback) {

        MenuTransitionsHelper_StartStandardLevel(
            self,
            gameMode,
            difficultyBeatmap,
            previewBeatmapLevel,
            overrideEnvironmentSettings,
            overrideColorScheme,
            gameplayModifiers,
            PerformSettingOverrides(playerSpecificSettings, previewBeatmapLevel, difficultyBeatmap),
            practiceSettings,
            backButtonText,
            useTestNoteCutCountEffects,
            beforeSceneSwitchCallback,
            levelFinishedCallback
        );
    }


    // Called when starting a multiplayer level
    MAKE_HOOK_MATCH(MenuTransitionsHelper_StartMultiplayerLevel,  static_cast<void (MenuTransitionsHelper::*)(
        Il2CppString*,
        IPreviewBeatmapLevel*,
        BeatmapDifficulty,
        BeatmapCharacteristicSO*,
        IDifficultyBeatmap*,
        ColorScheme*,
        GameplayModifiers*,
        PlayerSpecificSettings*,
        PracticeSettings*,
        Il2CppString*,
        bool,
        System::Action*,
        System::Action_2<MultiplayerLevelScenesTransitionSetupDataSO*, MultiplayerResultsData*>*,
        System::Action_1<DisconnectedReason>*)>(&MenuTransitionsHelper::StartMultiplayerLevel), void,
        MenuTransitionsHelper* self,
        Il2CppString* gameMode,
        IPreviewBeatmapLevel* previewBeatmapLevel,
        BeatmapDifficulty beatmapDifficulty,
        BeatmapCharacteristicSO* beatmapCharacteristic,
        IDifficultyBeatmap* difficultyBeatmap,
        ColorScheme* overrideColorScheme,
        GameplayModifiers* gameplayModifiers,
        PlayerSpecificSettings* playerSpecificSettings,
        PracticeSettings* practiceSettings,
        Il2CppString* backButtonText,
        bool useTestNoteCutSoundEffects,
        System::Action* beforeSceneSwitchCallback,
        System::Action_2<MultiplayerLevelScenesTransitionSetupDataSO*, MultiplayerResultsData*>* levelFinishedCallback,
        System::Action_1<DisconnectedReason>* didDisconnectCallback) {
        
        MenuTransitionsHelper_StartMultiplayerLevel(
            self,
            gameMode,
            previewBeatmapLevel,
            beatmapDifficulty,
            beatmapCharacteristic,
            difficultyBeatmap,
            overrideColorScheme,
            gameplayModifiers,
            PerformSettingOverrides(playerSpecificSettings, previewBeatmapLevel, difficultyBeatmap),
            practiceSettings,
            backButtonText,
            useTestNoteCutSoundEffects,
            beforeSceneSwitchCallback,
            levelFinishedCallback,
            didDisconnectCallback
        );
    }

    void InstallHooks() {
        INSTALL_HOOK(getLogger(), MenuTransitionsHelper_StartStandardLevel);
        INSTALL_HOOK(getLogger(), MenuTransitionsHelper_StartMultiplayerLevel);
    }
}