#include "pause_tweaks.hpp"

#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

#include "bs-utils/shared/utils.hpp"

#include "GlobalNamespace/OVRInput_Button.hpp"
#include "GlobalNamespace/PauseController.hpp"
#include "GlobalNamespace/GameSongController.hpp"
#include "GlobalNamespace/OVRPlayerController.hpp"
#include "GlobalNamespace/OVRCameraRig.hpp"
#include "GlobalNamespace/PlayerTransforms.hpp"
#include "GlobalNamespace/PauseMenuManager.hpp"
#include "GlobalNamespace/PauseAnimationController.hpp"
#include "GlobalNamespace/ScoreController.hpp"
using namespace GlobalNamespace;

#include "TMPro/TextMeshProUGUI.hpp"
using namespace TMPro;

#include "UnityEngine/Resources.hpp"
#include "UnityEngine/UI/Button.hpp"
#include "UnityEngine/Animator.hpp"

namespace FishUtils::PauseTweaks {
    static std::vector<ButtonMapping> mappings = {
        ButtonMapping(OVRInput::Button::PrimaryIndexTrigger, "Trigger", "trigger"),
        ButtonMapping(OVRInput::Button::PrimaryThumbstick, "Thumbstick Click", "thumbstickClick"),
        ButtonMapping(OVRInput::Button::Start, "Menu Button", "menuButton"),
        ButtonMapping(OVRInput::Button::One, "X Button", "buttonOne", "A Button"),
        ButtonMapping(OVRInput::Button::Two, "Y Button", "buttonTwo", "B Button"),
    };

    void InstallHooks();

    void Init() {
        getLogger().info("Initialising pause tweaks . . .");

        ConfigDocument& config = getConfig().config;
        if(!config.HasMember("overridePauseButtons")) {
            config.AddMember("overridePauseButtons", false, config.GetAllocator());
        }

        if(!config.HasMember("rightHandedPause")) {
            config.AddMember("rightHandedPause", false, config.GetAllocator());
        }

        
        if(!config.HasMember("requiredButtonsToPause")) {
            getLogger().info("No mappings were present, creating config section . . .");
            config.AddMember("requiredButtonsToPause", rapidjson::kObjectType, config.GetAllocator());
        }

        if(!config.HasMember("confirmOnContinue")) {
            config.AddMember("confirmOnContinue", false, config.GetAllocator());
            config.AddMember("confirmOnRestart", false, config.GetAllocator());
            config.AddMember("confirmOnMenu", false, config.GetAllocator());
        }

        if(!config.HasMember("resumeTimeMultiplier")) {
            config.AddMember("resumeTimeMultiplier", 1.0, config.GetAllocator());
        }

        rapidjson::Value& mappingsValue = config["requiredButtonsToPause"];

        // Add any missing mappings
        // This is to allow adding more mappings in the future if necessary
        for(ButtonMapping mapping : mappings) {
            if(!mappingsValue.HasMember(mapping.saveName)) {
                getLogger().info("Adding mapping %s to config", mapping.saveName.c_str());
                bool enabledByDefault = mapping.saveName == "menuButton";
                if(enabledByDefault) {
                    getLogger().info("Mapping enabled by default");
                }

                rapidjson::Value nameValue;
                nameValue.SetString(mapping.saveName, config.GetAllocator());
                mappingsValue.AddMember(nameValue, enabledByDefault, config.GetAllocator());
            }
        }

        getLogger().info("Installing hooks . . .");
        InstallHooks();
        getLogger().info("Pause tweaks installed");
    }

    bool IsRequiredToPause(ButtonMapping mapping) {
        return getConfig().config["requiredButtonsToPause"][mapping.saveName].GetBool();
    }

    void SetRequiredToPause(ButtonMapping mapping, bool required) {
        getConfig().config["requiredButtonsToPause"][mapping.saveName] = required;
    }

    std::vector<ButtonMapping>& GetMappings() {
        return mappings;
    }

    bool GetOverrideEnabled() {
        return getConfig().config["overridePauseButtons"].GetBool();
    }

    void SetOverrideEnabled(bool enabled) {
        getConfig().config["overridePauseButtons"] = enabled;
    }

    bool GetRightHanded() {
        return getConfig().config["rightHandedPause"].GetBool();
    }

    void SetRightHanded(bool rightHanded) {
        getConfig().config["rightHandedPause"] = rightHanded;
    }

    bool ShouldResumeSpeedDisableScoreSubmission() {
        float currentSpeed = getConfig().config["resumeTimeMultiplier"].GetFloat();

        return currentSpeed + 0.001f < 1; // Workaround for floating point precision issues
    }

    bool GetIfPauseButtonsPressed() {
        // Check that all buttons required to pause are down
        bool allDown = true;
        for(ButtonMapping mapping : GetMappings()) {
            if(!IsRequiredToPause(mapping)) {continue;} // Check that this button is required

            // Make sure to use the correct controller
            bool isDown = OVRInput::Get((OVRInput::Button) mapping.button, GetRightHanded() ? OVRInput::Controller::RTouch : OVRInput::Controller::LTouch);

            if(!isDown) {
                allDown = false;
                break;
            }
        }

        return allDown;
    }


    static bool hasInducedPause = false;
    static bool wasPreviouslyAllDown = false;

    MAKE_HOOK_MATCH(PlayerTransforms_Update, &PlayerTransforms::Update, void, PlayerTransforms* self) {
        PlayerTransforms_Update(self);
        if(!GetOverrideEnabled()) {return;} // If overriding the pause buttons is disabled return
        bool allDown = GetIfPauseButtonsPressed();

        // Pause the game if all buttons required to pause are down
        if(allDown) {
            getLogger().info("Pausing game as all required buttons were all down . . .");
            Array<PauseController*>* controllers = UnityEngine::Resources::FindObjectsOfTypeAll<PauseController*>();
            if(controllers->Length() == 0) {return;} // If we're in the menu, there isn't a PauseController
            hasInducedPause = true;
            reinterpret_cast<PauseController*>(controllers->GetValue(0))->Pause();
        }
    }

    MAKE_HOOK_MATCH(PauseController_Pause, &PauseController::Pause, void, PauseController* self) {
        // If overriding the pause buttons is disabled, don't do anything
        if(!GetOverrideEnabled()) {
            PauseController_Pause(self);
            return;
        } 

        // Only allow the pause if the override method has induced it, since otherwise pausing with the menu button would still work
        if(hasInducedPause) {
            hasInducedPause = false;
            PauseController_Pause(self);
        }
    }


    // Make all three buttons in the pause menu require confirmation (if enabled)
    static std::unordered_map<UnityEngine::UI::Button*, std::string> previousButtonText;
    bool checkConfirmation(UnityEngine::UI::Button* button) {
        getLogger().info("Checking button confirmation status . . .");
        if(previousButtonText.find(button) == previousButtonText.end()) {
            getLogger().info("Setting text to confirm message - button was not confirmed!");
            TextMeshProUGUI* textMesh = button->GetComponentInChildren<TextMeshProUGUI*>(); // Find the text object in the button

            // Store the old text so it can be restored if the pause menu is re-opened
            Il2CppString* oldText = textMesh->get_text();
            previousButtonText[button] = to_utf8(csstrtostr(oldText));

            // Make the user confirm
            textMesh->set_text(il2cpp_utils::createcsstr("Are you sure?"));
            return false;
        }   else    {
            getLogger().info("Confirm message already displayed, performing button action.");
            return true;
        }
    }

    MAKE_HOOK_MATCH(PauseMenuManager_ContinueButtonPressed, &PauseMenuManager::ContinueButtonPressed, void, PauseMenuManager* self) {
        // Perform the confirmation if we need to
        bool isConfirmationEnabled = getConfig().config["confirmOnContinue"].GetBool();
        if(isConfirmationEnabled && !checkConfirmation(self->continueButton)) {return;} // Return if unconfirmed
        
        PauseMenuManager_ContinueButtonPressed(self);
    }

    MAKE_HOOK_MATCH(PauseMenuManager_RestartButtonPressed, &PauseMenuManager::RestartButtonPressed, void, PauseMenuManager* self) {
        // Perform the confirmation if we need to
        bool isConfirmationEnabled = getConfig().config["confirmOnRestart"].GetBool();
        if(isConfirmationEnabled && !checkConfirmation(self->restartButton)) {return;} // Return if unconfirmed
        
        previousButtonText.clear(); // If we restart the song, we don't need to worry about changed button text
        PauseMenuManager_RestartButtonPressed(self);
    }

    MAKE_HOOK_MATCH(PauseMenuManager_MenuButtonPressed, &PauseMenuManager::MenuButtonPressed, void, PauseMenuManager* self) {
        // Perform the confirmation if we need to
        bool isConfirmationEnabled = getConfig().config["confirmOnMenu"].GetBool();
        if(isConfirmationEnabled && !checkConfirmation(self->backButton)) {return;} // Return if unconfirmed
        
        previousButtonText.clear(); // If we go back to the menu, we don't need to worry about changed button text
        PauseMenuManager_MenuButtonPressed(self);
    }

    // Changes all the buttons back to their default text if it was changed
    void RestoreButtonText() {
        getLogger().info("Restoring changed button text . . .");
        for(auto& entry : previousButtonText) {
            // Change the text back to what it was
            TextMeshProUGUI* textMesh = entry.first->GetComponentInChildren<TextMeshProUGUI*>();
            textMesh->set_text(il2cpp_utils::createcsstr(entry.second));
        }
        previousButtonText.clear(); // Empty the map - no buttons are overridden
    }

    // Change the confirmation text back to default for the next time the pause menu is opened
    // This is done here, instead of in PauseMenuManager_ContinueButtonPressed, since otherwise you see the default text for a split second
    MAKE_HOOK_MATCH(PauseAnimationController_ResumeFromPauseAnimationDidFinish, &PauseAnimationController::ResumeFromPauseAnimationDidFinish, void, PauseAnimationController* self) {
        RestoreButtonText();
        PauseAnimationController_ResumeFromPauseAnimationDidFinish(self);
    }


    // Called whenever the resume button is pressed while the game is paused
    MAKE_HOOK_MATCH(PauseAnimationController_StartResumeFromPauseAnimation, &PauseAnimationController::StartResumeFromPauseAnimation, void, PauseAnimationController* self) {
        UnityEngine::Animator* animator = self->animator;
        float speed = getConfig().config["resumeTimeMultiplier"].GetFloat();

        if(speed != 1.0) { // Only set the speed if it was not the default, just to be sure that we don't mess something up if disabled
            // Disable score submission if the amount of time was decreased
            if(speed < 1.0) {
                getLogger().info("Disabling score submission . . .");
                bs_utils::Submission::disable(getModInfo());
            }

            // Set the speed of the resume animation
            getLogger().info("Setting pause animation speed to: %f", speed);
            animator->set_speed(speed);
        }

        PauseAnimationController_StartResumeFromPauseAnimation(self);
    }

    // Manually re-enable score submission when scoring starts, because bs-utils isn't doing it automatically . . .
    MAKE_HOOK_MATCH(ScoreController_Start, &ScoreController::Start, void, ScoreController* self) {
        getLogger().info("Re-enabling score submission . . .");
        bs_utils::Submission::enable(getModInfo());
        ScoreController_Start(self);
    }


    void InstallHooks() {
        INSTALL_HOOK(getLogger(), PlayerTransforms_Update);
        INSTALL_HOOK(getLogger(), PauseController_Pause);

        INSTALL_HOOK(getLogger(), PauseMenuManager_ContinueButtonPressed);
        INSTALL_HOOK(getLogger(), PauseMenuManager_RestartButtonPressed);
        INSTALL_HOOK(getLogger(), PauseMenuManager_MenuButtonPressed);
        INSTALL_HOOK(getLogger(), PauseAnimationController_ResumeFromPauseAnimationDidFinish);

        INSTALL_HOOK(getLogger(), PauseAnimationController_StartResumeFromPauseAnimation);
        INSTALL_HOOK(getLogger(), ScoreController_Start);
    }
}