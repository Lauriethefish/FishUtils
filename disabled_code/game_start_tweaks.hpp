#pragma once

#include "main.hpp"

namespace FishUtils::GameStartTweaks {
    void Init();

    // Returns the button that should be pressed automatically on game load, or None of there is none
    std::string GetButtonOnGameLoad();

    // Options are None, Solo, Online, Campaign or Party.
    void SetButtonOnGameLoad(std::string buttonName);

    bool IsHealthAndSafetySkipEnabled();

    void SetHealthAndSafetySkipEnabled(bool enabled);
}