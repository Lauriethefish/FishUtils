#include "main_settings_ui.hpp"
#include "pause_menu_ui.hpp"
#include "ui_utils.hpp"
#include "auto_settings_ui.hpp"
#include "auto_settings.hpp"

namespace FishUtils {
    static std::string TITLE = "FishUtils Settings";

    DEFINE_TYPE(FishUtils, SettingsFlowCoordinator);
    DEFINE_TYPE(FishUtils, MainSettingsViewController);

    void SettingsFlowCoordinator::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
        if(!firstActivation) {return;}

        getLogger().info("SettingsFlowCoordinator activating");
        this->openSubMenu = nullptr;
        this->SetTitle(il2cpp_utils::createcsstr(TITLE), HMUI::ViewController::AnimationType::Out);

        this->showBackButton = true;
        this->mainViewController = BeatSaberUI::CreateViewController<MainSettingsViewController*>();
        mainViewController->flowCoordinator = this;

        this->ProvideInitialViewControllers(mainViewController, nullptr, nullptr, nullptr, nullptr);
        getLogger().info("Sub-settings options view controller presented");
    }

    void SettingsFlowCoordinator::BackButtonWasPressed(HMUI::ViewController* topViewController) {
        getLogger().info("Back button pressed on settings flow coordinator");
        
        if(openSubMenu) {
            getLogger().info("Currently in a sub-settings menu, replacing with main menu");
            this->SetTitle(il2cpp_utils::createcsstr(TITLE), HMUI::ViewController::AnimationType::In);
            this->ReplaceTopViewController(this->mainViewController, this, this, nullptr, HMUI::ViewController::AnimationType::Out, HMUI::ViewController::AnimationDirection::Horizontal);
            this->openSubMenu = nullptr;
        }   else    {
            getLogger().info("Not in a sub-settings menu - exiting settings");
            getLogger().info("Saving config on settings exit . . .");
            AutoSettings::SaveConfigs();
            getConfig().Write();
            this->parentFlowCoordinator->DismissFlowCoordinator(this, HMUI::ViewController::AnimationDirection::Horizontal, nullptr, false);
        }
    } 

    void MainSettingsViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool systemScreenEnabling) {
        if(!firstActivation) {return;}

        getLogger().info("MainSettingsViewController activating");


        VerticalLayoutGroup* mainLayout = UIUtils::CreateListLikeVerticalLayout(this->get_rectTransform());

        Transform* mainLayoutTransform = mainLayout->get_rectTransform();
    
        UIUtils::CreateTitle(mainLayoutTransform, "Quality of Life Improvements");
        UIUtils::CreateSeparatorLine(mainLayoutTransform);

        AutoSettingsFlowCoordinator* autoSettingsFlowCoordinator = BeatSaberUI::CreateFlowCoordinator<AutoSettingsFlowCoordinator*>();
        PauseOptionsFlowCoordinator* pauseFlowCoordinator = BeatSaberUI::CreateFlowCoordinator<PauseOptionsFlowCoordinator*>();
        // Disabled as it's being moved into its own mod
        //GameStartOptionsViewController* startOptionsViewController = BeatSaberUI::CreateViewController<GameStartOptionsViewController*>();

        CreateSubSettingsUI(mainLayoutTransform, "Automatic Settings", "Auto-change settings depending on NPS, NJS, offset or playlist", autoSettingsFlowCoordinator);
        CreateSubSettingsUI(mainLayoutTransform, "Pause Menu Settings", "Many pause-menu related tweaks", pauseFlowCoordinator);
        // Disabled as it's being moved into its own mod
        //CreateSubSettingsUI(mainLayoutTransform, "Game Start Tweaks", "Bypass the Health and Safety Menu & other options", startOptionsViewController);
    }

    // Sets only the size of the button background and bar, not the text itself
    void SetButtonScaleSafe(Button* button, Vector3 scale) {
        button->get_transform()->set_localScale(scale);

        Vector3 inverseScale = Vector3(
            1.0 / scale.x,
            1.0 / scale.y,
            1.0 / scale.z
        );

        button->GetComponentInChildren<TextMeshProUGUI*>()->get_transform()->set_localScale(inverseScale);
    }

    void MainSettingsViewController::CreateSubSettingsUI(UnityEngine::Transform* transform, std::string menuName, std::string description, HMUI::ViewController* viewController) {
        CreateSubSettingsUI(transform, menuName, description, [this, viewController, menuName]() {
            SettingsFlowCoordinator* flowCoordinator = reinterpret_cast<SettingsFlowCoordinator*>(this->flowCoordinator);

            getLogger().info("Presenting sub-settings ViewController . . .");
            flowCoordinator->SetTitle(il2cpp_utils::createcsstr(menuName), ViewController::AnimationType::In);
            flowCoordinator->ReplaceTopViewController(viewController, flowCoordinator, flowCoordinator, nullptr, ViewController::AnimationType::In, ViewController::AnimationDirection::Horizontal);
            flowCoordinator->openSubMenu = viewController;
            getLogger().info("Done!");
        });
    }

    void MainSettingsViewController::CreateSubSettingsUI(UnityEngine::Transform* transform, std::string menuName, std::string description, HMUI::FlowCoordinator* flowCoordinator) {
        CreateSubSettingsUI(transform, menuName, description, [this, flowCoordinator, menuName]() {
            getLogger().info("Presenting sub-settings FlowCoordinator . . .");

            flowCoordinator->set_showBackButton(true);
            flowCoordinator->SetTitle(il2cpp_utils::createcsstr(menuName), ViewController::AnimationType::In);
            this->flowCoordinator->PresentFlowCoordinator(flowCoordinator, nullptr, HMUI::ViewController::AnimationDirection::Horizontal, false, false);
            getLogger().info("Done!");
        });
    }

    void MainSettingsViewController::CreateSubSettingsUI(Transform* transform, std::string menuName, std::string description, std::function<void()> onEnter) {
        HorizontalLayoutGroup* horizontalLayout = UIUtils::CreateListLikeHorizontalLayout(transform);
        horizontalLayout->get_gameObject()->AddComponent<QuestUI::Backgroundable*>()->ApplyBackground(il2cpp_utils::createcsstr("round-rect-panel"));
        horizontalLayout->set_padding(RectOffset::New_ctor(3, 3, 3, 3));

        Transform* horizontalLayoutTransform = horizontalLayout->get_rectTransform();

        VerticalLayoutGroup* nameAndDescLayout = BeatSaberUI::CreateVerticalLayoutGroup(horizontalLayoutTransform);
        LayoutElement* nameAndDescLayoutElement = nameAndDescLayout->GetComponent<LayoutElement*>();
        nameAndDescLayoutElement->set_preferredWidth(75.0f);
        nameAndDescLayoutElement->set_preferredHeight(12.0f);

        Transform* nameAndDescLayoutTransform = nameAndDescLayout->get_rectTransform();

        BeatSaberUI::CreateText(nameAndDescLayoutTransform, menuName)->set_fontSize(4.0f);
        TextMeshProUGUI* descriptionText = BeatSaberUI::CreateText(nameAndDescLayoutTransform, description);
        descriptionText->set_fontSize(3.0f);
        descriptionText->set_overflowMode(TextOverflowModes::Ellipsis);

        Button* button = BeatSaberUI::CreateUIButton(horizontalLayoutTransform, "Go", "OkButton", onEnter);
    }
}