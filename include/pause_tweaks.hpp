#pragma once

#include "main.hpp"

#include <memory>
#include <functional>

#include "GlobalNamespace/OVRInput_Button.hpp"
using namespace GlobalNamespace;

namespace FishUtils::PauseTweaks {
    struct ButtonMapping {
        OVRInput::Button button;
        std::string name;
        std::string rightHandedName;
        std::string saveName;

        ButtonMapping(OVRInput::Button button, std::string name, std::string saveName) : button(button), name(name), saveName(saveName), rightHandedName(name) {}; // Used for non-changing buttons, such as the joystick click
        ButtonMapping(OVRInput::Button button, std::string name, std::string saveName, std::string rightHandedName) : button(button), name(name), saveName(saveName), rightHandedName(rightHandedName) {}; // Used for changing buttons, like X or Y
    };

    void Init();

    // Returns true if the specified mapping is in the list of required buttons to be paused
    bool IsRequiredToPause(ButtonMapping mapping);

    // Sets if the specified mapping must be held down to pause the game
    void SetRequiredToPause(ButtonMapping mapping, bool required);

    // Gets all registered ButtonMappings
    std::vector<ButtonMapping>& GetMappings();


    // Gets if overriding the required buttons is currently on
    // If this isn't on, the mappings have no effect
    bool GetOverrideEnabled();

    // Sets if override the required buttons is on
    void SetOverrideEnabled(bool enabled);


    // Gets if the pause buttons are currently flipped
    bool GetRightHanded();

    // Sets if button mappings will be flipped to the right controller
    void SetRightHanded(bool rightHanded);

    // Gets if all of the required buttons to pause the game are currently pressed
    // Will not work if pause button overriding is not enabled
    bool GetIfPauseButtonsPressed();

    // Checks if the (overridden) resume animation speed should disable score submission
    // i.e. it is less than 1
    bool ShouldResumeSpeedDisableScoreSubmission();
}