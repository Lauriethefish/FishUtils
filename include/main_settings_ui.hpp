#pragma once

#include "main.hpp"
#include "custom-types/shared/macros.hpp"

#include "HMUI/ViewController.hpp"
#include "HMUI/FlowCoordinator.hpp"

#include <functional>

DECLARE_CLASS_CODEGEN(FishUtils, MainSettingsViewController, HMUI::ViewController,
    DECLARE_INSTANCE_FIELD(HMUI::FlowCoordinator*, flowCoordinator);

    DECLARE_OVERRIDE_METHOD(void, DidActivate, il2cpp_utils::FindMethodUnsafe("HMUI", "ViewController", "DidActivate", 3), bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling);

    REGISTER_FUNCTION(MainSettingsViewController,
        REGISTER_FIELD(flowCoordinator);
        
        REGISTER_METHOD(DidActivate);
    )

public:
    void CreateSubSettingsUI(UnityEngine::Transform* transform, std::string menuName, std::string description, HMUI::ViewController* viewController);
    void CreateSubSettingsUI(UnityEngine::Transform* transform, std::string menuName, std::string description, HMUI::FlowCoordinator* flowCoordinator);
    void CreateSubSettingsUI(UnityEngine::Transform* transform, std::string menuName, std::string description, std::function<void()> onEnter);
)

DECLARE_CLASS_CODEGEN(FishUtils, SettingsFlowCoordinator, HMUI::FlowCoordinator,
    DECLARE_INSTANCE_FIELD(MainSettingsViewController*, mainViewController);
    DECLARE_INSTANCE_FIELD(HMUI::ViewController*, openSubMenu);

    DECLARE_OVERRIDE_METHOD(void, DidActivate, il2cpp_utils::FindMethodUnsafe("HMUI", "FlowCoordinator", "DidActivate", 3), bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling);
    DECLARE_OVERRIDE_METHOD(void, BackButtonWasPressed, il2cpp_utils::FindMethodUnsafe("HMUI", "FlowCoordinator", "BackButtonWasPressed", 1), HMUI::ViewController* topViewController);

    REGISTER_FUNCTION(SettingsFlowCoordinator,
        REGISTER_FIELD(mainViewController);
        REGISTER_FIELD(openSubMenu);

        REGISTER_METHOD(DidActivate);
        REGISTER_METHOD(BackButtonWasPressed);
    )
)