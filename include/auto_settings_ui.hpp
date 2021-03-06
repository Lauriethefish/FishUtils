#pragma once
#include "auto_settings.hpp"

#include "custom-types/shared/macros.hpp"

#include "HMUI/FlowCoordinator.hpp"
#include "HMUI/ViewController.hpp"
#include "HMUI/SimpleTextDropdown.hpp"

#include "UnityEngine/UI/Toggle.hpp"
#include "UnityEngine/GameObject.hpp"

#include <unordered_map>
#include <string>

DECLARE_CLASS_CODEGEN(FishUtils, AutoSettingsFlowCoordinator, HMUI::FlowCoordinator,
    DECLARE_INSTANCE_FIELD(HMUI::ViewController*, playlistOverridesView);
    DECLARE_INSTANCE_FIELD(HMUI::ViewController*, thresholdsView);

    DECLARE_OVERRIDE_METHOD(void, DidActivate, il2cpp_utils::FindMethodUnsafe("HMUI", "FlowCoordinator", "DidActivate", 3), bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling);
    DECLARE_OVERRIDE_METHOD(void, BackButtonWasPressed, il2cpp_utils::FindMethodUnsafe("HMUI", "FlowCoordinator", "BackButtonWasPressed", 1), HMUI::ViewController* topViewController);
)

DECLARE_CLASS_CODEGEN(FishUtils, AutoSettingSelectionViewController, HMUI::ViewController,
    DECLARE_INSTANCE_FIELD(AutoSettingsFlowCoordinator*, flowCoordinator);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Toggle*, settingEnabledToggle);
    DECLARE_INSTANCE_FIELD(bool, wasSettingConfigurationViewOpen);

    DECLARE_OVERRIDE_METHOD(void, DidActivate, il2cpp_utils::FindMethodUnsafe("HMUI", "ViewController", "DidActivate", 3), bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling);

    DECLARE_INSTANCE_METHOD(void, OpenSettingConfigurationView);
    DECLARE_INSTANCE_METHOD(void, CloseSettingConfigurationView);

public:
    FishUtils::AutoSettings::SettingType* selectedSetting;
    FishUtils::AutoSettings::SettingConfiguration* selectedSettingConfig;
)

// Workaround for comma in type definition being interpreted as another argument in DECLARE_CLASS_CODEGEN
// This isn't a map because whenever I tried to insert a toggle in a map it SIGABRT'ed, and the full error won't print so I'm unsure as to why
// I never actually have to access by key here anyway, so a vector of pairs works fine
typedef std::vector<std::pair<std::string, UnityEngine::UI::Toggle*>> PlaylistToggles;

DECLARE_CLASS_CODEGEN(FishUtils, PlaylistOverridesViewController, HMUI::ViewController,
    DECLARE_INSTANCE_FIELD(HMUI::SimpleTextDropdown*, setSettingToDropdown);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Toggle*, enableToggle);
    DECLARE_INSTANCE_FIELD(UnityEngine::GameObject*, enabledLayoutGameObject);

    DECLARE_OVERRIDE_METHOD(void, DidActivate, il2cpp_utils::FindMethodUnsafe("HMUI", "ViewController", "DidActivate", 3), bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling);

public:
    FishUtils::AutoSettings::SettingConfiguration* setting;
    PlaylistToggles playlistToggles;
)

DECLARE_CLASS_CODEGEN(FishUtils, ThresholdsViewController, HMUI::ViewController,
    DECLARE_INSTANCE_FIELD(UnityEngine::Transform*, mainLayoutTransform);
    DECLARE_INSTANCE_FIELD(UnityEngine::GameObject*, thresholdsObject);
    DECLARE_INSTANCE_FIELD(HMUI::SimpleTextDropdown*, parameterDropdown);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Toggle*, flipOptionsToggle);

    DECLARE_OVERRIDE_METHOD(void, DidActivate, il2cpp_utils::FindMethodUnsafe("HMUI", "ViewController", "DidActivate", 3), bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling);

    DECLARE_INSTANCE_METHOD(void, RefreshThresholdSettings);

public:
    FishUtils::AutoSettings::SettingConfiguration* setting;
)