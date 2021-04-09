#pragma once

#include <string>
#include <vector>

#include "beatsaber-hook/shared/config/config-utils.hpp"

#include "GlobalNamespace/PlayerSpecificSettings.hpp"
#include "GlobalNamespace/IBeatmapLevel.hpp"
#include "GlobalNamespace/IDifficultyBeatmap.hpp"
using namespace GlobalNamespace;

namespace FishUtils::AutoSettings {
    void Init();

    class SettingType {
    public:
        std::string name; // Name displayed in UI
        std::string saveName; // Name saved in the config
        std::vector<std::string> options; // All the ways that this setting can be configued
        
        // Applies the given option, which must be in options, to this instance of PlayerSpecificSettings
        virtual void Apply(PlayerSpecificSettings* settings, std::string option) = 0;
    };

    // Describes a map element, like NJS, NPS or offset
    class MapElementType {
    public:
        std::string name; // Name displayed in UI
        std::string saveName; // Name saved in the config

        // Used to calculate default values whenever a setting is created
        float reasonableMinValue;
        float reasonableMaxValue;
        
        // Gets this element from the given map
        virtual float Get(IDifficultyBeatmap* difficulty) = 0;
    };

    struct SettingConfiguration {
    public:
        SettingType* settingType; // Setting that this config configures
        MapElementType* configureBasedOn; // Element that this setting is automatically set based on.

        std::vector<std::string> options; // Stores the (possibly) flipped options to avoid flipping them every time
        std::vector<float> thresholds; // Numbers in-between each of the settings of the SettingType. Always (number of settings in the setting type - 1) in length

        // Playlists overrides take priority over the MapElementType thresholds
        std::vector<std::string> overriddenInPlaylists; // Empty if disabled
        std::string playlistOverrideSetting; // "" if disabled

        SettingConfiguration(rapidjson::Value& value); // Load from a rapidjson value
        SettingConfiguration(SettingType* settingType);
        SettingConfiguration() {}

        void Save(rapidjson::Value& value, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator);
        void SetConfigureBasedOn(MapElementType* elementType);
        void OverrideIfNecessary(PlayerSpecificSettings* playerSettings, IPreviewBeatmapLevel* level, IDifficultyBeatmap* difficulty);

        bool IsOverriddenInPlaylist(std::string playlistName);
        void SetFlipOptions(bool flipOptions);
        bool GetFlipOptions();

    private:
        bool flipOptions;
    };

    void SaveConfigs();

    std::vector<SettingType*>& GetSettingTypes();

    std::vector<MapElementType*>& GetMapElementTypes();

    MapElementType* GetMapElementType(std::string name, bool useSaveName = true);

    SettingType* GetSettingType(std::string name, bool useSaveName = true);

    // Gets the SettingConfiguration for the given SettingType. Returns nullptr if no config exists for this type
    SettingConfiguration* GetSettingConfiguration(SettingType* settingType);

    // Creates a new SettingConfiguration and registers it in the map
    SettingConfiguration* CreateSettingConfiguration(SettingType* settingType);

    // Removes the SettingConfiguration for this SettingType.
    // Does nothing if there is no SettingConfiguration registered for this SettingType.
    // Any pointers to the SettingConfiguration will become invalid
    void RemoveSettingConfiguration(SettingType* settingType);
}