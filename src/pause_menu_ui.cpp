#include "pause_menu_ui.hpp"
#include "pause_tweaks.hpp"
#include "ui_utils.hpp"
#include <cmath>

const float MAX_RESUME_SPEED = 20.0f;
const float MIN_RESUME_SPEED = 0.05f;

namespace FishUtils {
    DEFINE_TYPE(FishUtils, PauseOptionsFlowCoordinator);
    DEFINE_TYPE(FishUtils, PauseButtonRemappingsViewController);
    DEFINE_TYPE(FishUtils, ResumeSpeedViewController);
    DEFINE_TYPE(FishUtils, PauseConfirmationViewController);

    void PauseOptionsFlowCoordinator::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
        if(!firstActivation) {return;}

        getLogger().info("PauseOptionsFlowCoordinator activating");

        PauseButtonRemappingsViewController* remappingsController = BeatSaberUI::CreateViewController<PauseButtonRemappingsViewController*>();
        ResumeSpeedViewController* resumeSpeedController = BeatSaberUI::CreateViewController<ResumeSpeedViewController*>();
        PauseConfirmationViewController* confirmationController = BeatSaberUI::CreateViewController<PauseConfirmationViewController*>();

        this->ProvideInitialViewControllers(
            confirmationController,
            remappingsController,
            resumeSpeedController,
            nullptr,
            nullptr
        );
    }

    void PauseOptionsFlowCoordinator::BackButtonWasPressed(HMUI::ViewController* topViewController) {
        this->parentFlowCoordinator->DismissFlowCoordinator(this, HMUI::ViewController::AnimationDirection::Horizontal, nullptr, false);
    }

    void PauseButtonRemappingsViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
        if(!firstActivation) {return;}

        getLogger().info("PauseButtonRemappingsViewController activating");

        VerticalLayoutGroup* mainLayout = UIUtils::CreateListLikeVerticalLayout(this->get_rectTransform());
        mainLayout->set_spacing(1.0);
        Transform* mainLayoutTransform = mainLayout->get_rectTransform();
        
        
        UIUtils::CreateTitle(mainLayoutTransform, "Remap Pause Buttons", "Specify multiple buttons, all of which must be pressed down to pause the game.\nAvoids accidental pauses");

        VerticalLayoutGroup* overrideToggleLayout = UIUtils::CreateListLikeVerticalLayout(mainLayoutTransform);
        overrideToggleLayout->set_padding(RectOffset::New_ctor(2, 2, 2, 2));
        UIUtils::ApplyRectPanelBackground(overrideToggleLayout->get_gameObject());

        Transform* overrideToggleLayoutTransform = overrideToggleLayout->get_rectTransform();


        Toggle* overrideToggle = BeatSaberUI::CreateToggle(overrideToggleLayoutTransform, "Enable Remapping", PauseTweaks::GetOverrideEnabled(), [this](bool newValue) {
            PauseTweaks::SetOverrideEnabled(newValue);
            this->overridesObject->SetActive(newValue);
        });

        VerticalLayoutGroup* togglesLayout = UIUtils::CreateListLikeVerticalLayout(mainLayoutTransform);
        togglesLayout->set_padding(RectOffset::New_ctor(2, 2, 2, 2));
        UIUtils::ApplyRectPanelBackground(togglesLayout->get_gameObject());
        Transform* togglesLayoutTransform = togglesLayout->get_rectTransform();
        this->overridesObject = togglesLayout->get_gameObject();
        overridesObject->SetActive(PauseTweaks::GetOverrideEnabled());

        Toggle* rightControllerToggle = BeatSaberUI::CreateToggle(togglesLayoutTransform, "Pause with right controller", PauseTweaks::GetRightHanded(), [this](bool enabled) {
            PauseTweaks::SetRightHanded(enabled);
            RefreshToggleText();
        });

        BeatSaberUI::AddHoverHint(rightControllerToggle->get_gameObject(), "Changes to pause with the right controller instead of the left. Does not remap the menu button onto the Oculus button");

        for(PauseTweaks::ButtonMapping mapping : PauseTweaks::GetMappings()) {
            Toggle* toggle = BeatSaberUI::CreateToggle(
                togglesLayoutTransform,
                "", // Set by the RefreshToggleText function
                PauseTweaks::IsRequiredToPause(mapping),
                [mapping](bool enabled) {
                    PauseTweaks::SetRequiredToPause(mapping, enabled);
                }
            );

            this->toggles.push_back(toggle);
        }

        HorizontalLayoutGroup* previewLayout = UIUtils::CreateListLikeHorizontalLayout(togglesLayoutTransform);
        previewLayout->set_childAlignment(TextAnchor::UpperCenter);
        previewLayout->set_spacing(2.5f);
        Transform* previewLayoutTransform = previewLayout->get_rectTransform();

        TextMeshProUGUI* previewInfoText = BeatSaberUI::CreateText(previewLayoutTransform, "Preview:     ");
        this->previewText = BeatSaberUI::CreateText(previewLayoutTransform, ""); // The first preview update happens straight away, and sets this correctly.

        RefreshToggleText();
    }

    void PauseButtonRemappingsViewController::Update() {
        bool isPaused = PauseTweaks::GetIfPauseButtonsPressed();
        getLogger().info("Preview update. Is paused: %s", isPaused ? "true" : "false");
        if(isPaused) {
            previewText->set_text(il2cpp_utils::createcsstr("Your game would be paused"));
            previewText->set_color(Color::get_green());
        }   else    {
            previewText->set_text(il2cpp_utils::createcsstr("Your game would not be paused"));
            previewText->set_color(Color::get_red());
        }
    }

    void PauseButtonRemappingsViewController::RefreshToggleText() {
        getLogger().info("Refreshing right/left handed toggle text");
        getLogger().info("Is right handed: %s", PauseTweaks::GetRightHanded() ? "true" : "false");
        int i = 0;
        for(Toggle* toggle : toggles) {
            PauseTweaks::ButtonMapping mapping = PauseTweaks::GetMappings()[i];

            std::string correctTextType = PauseTweaks::GetRightHanded() ? mapping.rightHandedName : mapping.name;
            getLogger().info("Setting text to %s", correctTextType.c_str());
            UIUtils::SetToggleText(toggle, correctTextType);
            i++;
        }
    }

    void ResumeSpeedViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
        if(!firstActivation) {return;}
        getLogger().info("ResumeSpeedViewController activating");

        VerticalLayoutGroup* mainLayout = UIUtils::CreateListLikeVerticalLayout(this->get_rectTransform());
        mainLayout->GetComponent<LayoutElement*>()->set_flexibleHeight(40.0f); // Otherwise it expands to fill the whole settings menu
        Transform* mainLayoutTransform = mainLayout->get_rectTransform();
        UIUtils::CreateTitle(mainLayoutTransform, "Resume Time", "Does the pause animation take too long, or is it not long enough? Customise it here.");


        std::function<void(float)> onResumeTimeChange = [this](float newValue) {
            getConfig().config["resumeTimeMultiplier"] = newValue;
            getLogger().info("New Multiplier: %f", newValue);
            this->RefreshScoreSubmissionText();
        };
        
        IncrementSetting* resumeTimeSetting = BeatSaberUI::CreateIncrementSetting(mainLayoutTransform, "Resume Time Multiplier", 
            2, // 2 decimal places of precision
            0.05, // 0.05 change with each arrow press
            getConfig().config["resumeTimeMultiplier"].GetFloat(),
            MIN_RESUME_SPEED, // Minimum value
            MAX_RESUME_SPEED, // Maximum value
            onResumeTimeChange
        );

        /*// Convenience buttons for resetting and increasing a large amount (disabled because they like expanding to fit all the space, and setting the preferred and flexible height won't make that stop unfortunately)
        HorizontalLayoutGroup* extraButtonsLayout = UIUtils::CreateListLikeHorizontalLayout(mainLayoutTransform);
        extraButtonsLayout->GetComponent<LayoutElement*>()->set_preferredHeight(10.0f); // Otherwise it expands to fill the whole settings menu

        Transform* extraButtonsLayoutTransform = extraButtonsLayout->get_rectTransform();

        BeatSaberUI::CreateUIButton(extraButtonsLayoutTransform, "Reset", [resumeTimeSetting, onResumeTimeChange]() {
            resumeTimeSetting->CurrentValue = 1.0f;
            resumeTimeSetting->UpdateValue();

            onResumeTimeChange(resumeTimeSetting->CurrentValue);
        });

        BeatSaberUI::CreateUIButton(extraButtonsLayoutTransform, "Increase a lot", [resumeTimeSetting, onResumeTimeChange]() {
            float newValue = resumeTimeSetting->CurrentValue + 5.0f;
            newValue = fmax(newValue, MIN_RESUME_SPEED);
            newValue = fmin(newValue, MAX_RESUME_SPEED);
            resumeTimeSetting->CurrentValue = newValue;
            resumeTimeSetting->UpdateValue();

            onResumeTimeChange(newValue);
        });*/

        UIUtils::CreateSeparatorLine(mainLayoutTransform, Color::get_white());
        this->scoreSubmissionText = BeatSaberUI::CreateText(mainLayoutTransform, "Score submission will not be disabled");
        this->scoreSubmissionText->set_fontSize(3.0f);
        this->scoreSubmissionText->set_alignment(TextAlignmentOptions::Center);
        this->RefreshScoreSubmissionText();
    }

    void ResumeSpeedViewController::RefreshScoreSubmissionText() {
        if(PauseTweaks::ShouldResumeSpeedDisableScoreSubmission()) {
            getLogger().info("Setting score submission text to disabled - current speed < 1");
            scoreSubmissionText->set_text(il2cpp_utils::createcsstr("Score submission will be disabled, as the amount of time you get after resuming has been increased."));
            scoreSubmissionText->set_color(Color::get_red());
        }   else    {
            getLogger().info("Setting score submission text to not disabled - current speed >= 1");
            scoreSubmissionText->set_text(il2cpp_utils::createcsstr("Score submission won't be disabled"));
            scoreSubmissionText->set_color(Color::get_green());
        }
    }

    void PauseConfirmationViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
        if(!firstActivation) {return;}
        getLogger().info("PauseConfirmationViewController activating");

        VerticalLayoutGroup* mainLayout = UIUtils::CreateListLikeVerticalLayout(this->get_rectTransform());
        mainLayout->set_spacing(1.0);
        Transform* mainLayoutTransform = mainLayout->get_rectTransform();

        UIUtils::CreateTitle(mainLayoutTransform, "Pause Menu Confirmation", "Ever accidentally restarted, or gone back to the menu when you've got a really high score on a song?\nHere you can make the buttons require two clicks so that you don't do it again");

        VerticalLayoutGroup* togglesLayout = UIUtils::CreateListLikeVerticalLayout(mainLayoutTransform);
        togglesLayout->set_padding(RectOffset::New_ctor(2, 2, 2, 2));
        UIUtils::ApplyRectPanelBackground(togglesLayout->get_gameObject());
        Transform* togglesLayoutTransform = togglesLayout->get_rectTransform();

        BeatSaberUI::CreateToggle(togglesLayoutTransform, "Confirm on Resume", getConfig().config["confirmOnContinue"].GetBool(), [](bool newValue) {
            getConfig().config["confirmOnContinue"] = newValue;
        });

        BeatSaberUI::CreateToggle(togglesLayoutTransform, "Confirm on Restart", getConfig().config["confirmOnRestart"].GetBool(), [](bool newValue) {
            getConfig().config["confirmOnRestart"] = newValue;
        });

        BeatSaberUI::CreateToggle(togglesLayoutTransform, "Confirm on Menu", getConfig().config["confirmOnMenu"].GetBool(), [](bool newValue) {
            getConfig().config["confirmOnMenu"] = newValue;
        });
    }
}