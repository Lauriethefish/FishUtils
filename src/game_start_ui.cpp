#include "game_start_ui.hpp"
#include "ui_utils.hpp"
#include "main.hpp"
#include "game_start_tweaks.hpp"

namespace FishUtils {
    DEFINE_TYPE(GameStartOptionsViewController);

    void GameStartOptionsViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
        if(!firstActivation) {return;}

        getLogger().info("GameStartOptionsViewController activating");

        VerticalLayoutGroup* mainLayout = UIUtils::CreateListLikeVerticalLayout(this->get_rectTransform());
        Transform* mainLayoutTransform = mainLayout->get_rectTransform();

        UIUtils::CreateTitle(mainLayoutTransform, "Game Launch Options");

        Toggle* healthAndSafetyToggle = BeatSaberUI::CreateToggle(mainLayoutTransform, "Skip health and safety screen", GameStartTweaks::IsHealthAndSafetySkipEnabled(),
            [](bool newValue) {
                GameStartTweaks::SetHealthAndSafetySkipEnabled(newValue);
                // Write the config now rather than waiting until the user exits the settings, since otherwise the user will probably immediately restart their game to test the change, and the change will never get saved
                getConfig().Write();
            }
        );
        BeatSaberUI::AddHoverHint(healthAndSafetyToggle->get_gameObject(), "Automatically skip the health and safety screen when the game starts");

        HMUI::SimpleTextDropdown* autoPressDropDown = BeatSaberUI::CreateDropdown(mainLayoutTransform,
            "Auto-Press Button on Menu Load",
            GameStartTweaks::GetButtonOnGameLoad(),
            {
                "None",
                "Solo",
                "Online",
                "Party",
                "Campaign"
            },
            [](std::string newButton) {
                GameStartTweaks::SetButtonOnGameLoad(newButton);
                // Write the config now rather than waiting until the user exits the settings, since otherwise the user will probably immediately restart their game to test the change, and the change will never get saved
                getConfig().Write();
            }
        );

        BeatSaberUI::AddHoverHint(autoPressDropDown->get_gameObject(), "Automatically press a button once loading into the main menu");
    }
}