#pragma once

#include "main.hpp"

#include "custom-types/shared/macros.hpp"

#include "HMUI/FlowCoordinator.hpp"
#include "HMUI/ViewController.hpp"
#include "UnityEngine/UI/Toggle.hpp"
#include "UnityEngine/GameObject.hpp"
#include "TMPro/TextMeshProUGUI.hpp"

DECLARE_CLASS_CODEGEN(FishUtils, PauseOptionsFlowCoordinator, HMUI::FlowCoordinator,
    DECLARE_OVERRIDE_METHOD(void, DidActivate, il2cpp_utils::FindMethodUnsafe("HMUI", "FlowCoordinator", "DidActivate", 3), bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling);
    DECLARE_OVERRIDE_METHOD(void, BackButtonWasPressed, il2cpp_utils::FindMethodUnsafe("HMUI", "FlowCoordinator", "BackButtonWasPressed", 1), HMUI::ViewController* topViewController);
)

DECLARE_CLASS_CODEGEN(FishUtils, PauseButtonRemappingsViewController, HMUI::ViewController,
    DECLARE_INSTANCE_FIELD(UnityEngine::GameObject*, overridesObject);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, previewText);

    DECLARE_OVERRIDE_METHOD(void, DidActivate, il2cpp_utils::FindMethodUnsafe("HMUI", "ViewController", "DidActivate", 3), bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling);

    DECLARE_INSTANCE_METHOD(void, Update);
public:
    std::vector<UnityEngine::UI::Toggle*> toggles;
    // Swaps the buttons from left -> right handed or vice-versa, setting their text to the correct option
    void RefreshToggleText();
)

DECLARE_CLASS_CODEGEN(FishUtils, ResumeSpeedViewController, HMUI::ViewController,
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, scoreSubmissionText);

    DECLARE_OVERRIDE_METHOD(void, DidActivate, il2cpp_utils::FindMethodUnsafe("HMUI", "ViewController", "DidActivate", 3), bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling);
    DECLARE_INSTANCE_METHOD(void, RefreshScoreSubmissionText);
)

DECLARE_CLASS_CODEGEN(FishUtils, PauseConfirmationViewController, HMUI::ViewController,
    DECLARE_OVERRIDE_METHOD(void, DidActivate, il2cpp_utils::FindMethodUnsafe("HMUI", "ViewController", "DidActivate", 3), bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling);
)