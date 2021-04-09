#include "game_start_tweaks.hpp"

#include <optional>

#include "GlobalNamespace/MainMenuViewController.hpp"
#include "GlobalNamespace/HealthWarningFlowCoordinator.hpp"
#include "GlobalNamespace/HealthWarningFlowCoordinator_InitData.hpp"
#include "GlobalNamespace/GameScenesManager.hpp"
using namespace GlobalNamespace;

namespace FishUtils::GameStartTweaks {
    void InstallHooks();

    void Init() {
        ConfigDocument& config = getConfig().config;
        if(!config.HasMember("skipHealthAndSafety")) {
            config.AddMember("skipHealthAndSafety", false, config.GetAllocator());
        }

        if(!config.HasMember("buttonOnGameLoad")) {
            config.AddMember("buttonOnGameLoad", "None", config.GetAllocator());
        }

        InstallHooks();
    }

    std::string GetButtonOnGameLoad() {
        return getConfig().config["buttonOnGameLoad"].GetString();
    }

    void SetButtonOnGameLoad(std::string buttonName) {
        getConfig().config["buttonOnGameLoad"] = rapidjson::Value(buttonName, getConfig().config.GetAllocator());
    }

    bool IsHealthAndSafetySkipEnabled() {
        return getConfig().config["skipHealthAndSafety"].GetBool();
    }

    void SetHealthAndSafetySkipEnabled(bool enabled) {
        getConfig().config["skipHealthAndSafety"] = enabled;
    }

    MAKE_HOOK_OFFSETLESS(MainMenuViewController_DidActivate, void, MainMenuViewController* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
        MainMenuViewController_DidActivate(self, firstActivation, addedToHierarchy, screenSystemEnabling);
        if(!firstActivation) {return;}

        std::string buttonToPress = GetButtonOnGameLoad();
        getLogger().info("Pressing button: " + buttonToPress);
        if(buttonToPress == "None") {
            getLogger().info("Not pressing (button is None)");
            return;
        }   else if(buttonToPress == "Solo") {
            self->HandleMenuButton(MainMenuViewController::MenuButton::SoloFreePlay);
        }   else if(buttonToPress == "Campaign") {
            self->HandleMenuButton(MainMenuViewController::MenuButton::SoloCampaign);
        }   else if(buttonToPress == "Online") {
            self->HandleMenuButton(MainMenuViewController::MenuButton::Multiplayer);
        }   else if(buttonToPress == "Party") {
            self->HandleMenuButton(MainMenuViewController::MenuButton::Party);
        }
    }

    MAKE_HOOK_OFFSETLESS(HealthWarningFlowCoordinator_DidActivate, void, HealthWarningFlowCoordinator* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
        if(IsHealthAndSafetySkipEnabled()) {
            getLogger().info("Moving to next scene to bypass health and safety");
            self->gameScenesManager->ReplaceScenes(self->initData->nextScenesTransitionSetupData, 0.0f, nullptr, nullptr);
        }   else    {
            getLogger().info("Health and safety skip is disabled, calling method as normal");
            HealthWarningFlowCoordinator_DidActivate(self, firstActivation, addedToHierarchy, screenSystemEnabling);
        }
    }

    void InstallHooks() {
        INSTALL_HOOK_OFFSETLESS(getLogger(), MainMenuViewController_DidActivate,
            il2cpp_utils::FindMethodUnsafe("", "MainMenuViewController", "DidActivate", 3)
        );

        INSTALL_HOOK_OFFSETLESS(getLogger(), HealthWarningFlowCoordinator_DidActivate,
            il2cpp_utils::FindMethodUnsafe("", "HealthWarningFlowCoordinator", "DidActivate", 3)
        );
    }
}