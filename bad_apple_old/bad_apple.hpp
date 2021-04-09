#pragma once

#include <vector>

#include "custom-types/shared/macros.hpp"
#include "HMUI/ViewController.hpp"
#include "UnityEngine/UI/Button.hpp"
#include <fstream>

namespace FishUtils::BadApple {
    void Init();

    struct ChangeInfo {
        int32_t x;
        int32_t y;
        bool newValue;

        ChangeInfo(std::ifstream& stream);
    };

    struct FrameInfo {
        float time;
        std::vector<ChangeInfo> changes;

        FrameInfo(std::ifstream& stream);
        FrameInfo(){}
    };

    struct BadAppleFile {
        std::vector<FrameInfo> frames;

        BadAppleFile(std::ifstream& stream);
    };
}

DECLARE_CLASS_CODEGEN(FishUtils::BadApple, BadAppleViewController, HMUI::ViewController,
    DECLARE_OVERRIDE_METHOD(void, DidActivate, il2cpp_utils::FindMethodUnsafe("HMUI", "ViewController", "DidActivate", 3), bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling);

    DECLARE_METHOD(void, Update);

    REGISTER_FUNCTION(BadAppleViewController,
        REGISTER_METHOD(DidActivate);

        REGISTER_METHOD(Update);
    )

public:
    std::vector<std::vector<UnityEngine::UI::Button*>> screenSegments;
    FishUtils::BadApple::BadAppleFile file;
    float currentTime = 0;
    int currentFrameIndex = 0;
)
