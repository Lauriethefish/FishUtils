#include "auto_settings_ui.hpp"
#include "main.hpp"
#include "ui_utils.hpp"

#include "System/Collections/Generic/List_1.hpp"
using namespace System::Collections::Generic;

#include "GlobalNamespace/BeatmapLevelsModel.hpp"
#include "GlobalNamespace/BeatmapLevelPackCollection.hpp"
#include "GlobalNamespace/IBeatmapLevelPack.hpp"

namespace FishUtils {
    DEFINE_TYPE(FishUtils, AutoSettingsFlowCoordinator);
    DEFINE_TYPE(FishUtils, AutoSettingSelectionViewController);
    DEFINE_TYPE(FishUtils, PlaylistOverridesViewController);
    DEFINE_TYPE(FishUtils, ThresholdsViewController);

    void AutoSettingsFlowCoordinator::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
        if(!firstActivation) {return;}

        getLogger().info("AutoSettingsFlowCoordinator activating");

        this->set_showBackButton(true);
        AutoSettingSelectionViewController* selectionViewController = BeatSaberUI::CreateViewController<AutoSettingSelectionViewController*>();
        selectionViewController->flowCoordinator = this;
        this->playlistOverridesView = BeatSaberUI::CreateViewController<PlaylistOverridesViewController*>();
        this->thresholdsView = BeatSaberUI::CreateViewController<ThresholdsViewController*>();

        this->ProvideInitialViewControllers(selectionViewController, nullptr, nullptr, nullptr, nullptr);
    }

    void AutoSettingsFlowCoordinator::BackButtonWasPressed(HMUI::ViewController* topViewController) {
        this->parentFlowCoordinator->DismissFlowCoordinator(this, HMUI::ViewController::AnimationDirection::Horizontal, nullptr, false);
    }

    void AutoSettingSelectionViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
        if(!firstActivation) {
            // Reopen the setting configuration ViewControllers if they were closed due to the back button being pressed
            if(wasSettingConfigurationViewOpen) {
                this->OpenSettingConfigurationView();
            }
            return;
        }

        getLogger().info("AutoSettingSelectionViewController activating");

        VerticalLayoutGroup* mainLayout = UIUtils::CreateListLikeVerticalLayout(this->get_rectTransform());
        mainLayout->set_spacing(1.0);
        mainLayout->set_childAlignment(TextAnchor::UpperCenter);
        Transform* mainLayoutTransform = mainLayout->get_rectTransform();

        UIUtils::CreateTitle(mainLayoutTransform, "Settings To Configure", "Automatically change various settings based on NPS, NJS, or playlist");

        HorizontalLayoutGroup* addSettingLayout = UIUtils::CreateListLikeHorizontalLayout(mainLayoutTransform);
        addSettingLayout->set_childAlignment(TextAnchor::UpperCenter);
        Transform* addSettingLayoutTransform = addSettingLayout->get_rectTransform();

        // Allow the user to switch between multiple settings, and enable/disable them
        std::vector<std::string> availableSettings;
        for(AutoSettings::SettingType* settingType : AutoSettings::GetSettingTypes()) {
            availableSettings.push_back(settingType->name);
        }

        std::function<void(std::string)> onSelectedSettingChange = [this] (std::string newSetting) {
            this->selectedSetting = AutoSettings::GetSettingType(newSetting, false);
            this->selectedSettingConfig = AutoSettings::GetSettingConfiguration(selectedSetting);

            // We need to force this to be called initially, even if it was previously enabled on the last setting
            // Otherwise, the layout will not update when moving between settings if the previous and current setting were both enabled
            UIUtils::SetToggleForceNotify(this->settingEnabledToggle, selectedSettingConfig);
        };

        HMUI::SimpleTextDropdown* selectSettingDropdown = BeatSaberUI::CreateDropdown(mainLayoutTransform, "Available Settings", availableSettings[0], availableSettings, onSelectedSettingChange);

        this->settingEnabledToggle = BeatSaberUI::CreateToggle(mainLayoutTransform, "Automatically Set Setting", [this](bool newValue){
            getLogger().info("Processing setting enabled change. New value: %s", newValue ? "true" : "false");
            // Even if the new setting is enabled as well, we need to close the setting view so that DidActivate is called again, thus refreshing it for the new setting
            this->CloseSettingConfigurationView();

            if(newValue) {
                selectedSettingConfig = AutoSettings::GetSettingConfiguration(this->selectedSetting);
                if(!selectedSettingConfig) {
                    selectedSettingConfig = AutoSettings::CreateSettingConfiguration(this->selectedSetting);
                }
                this->OpenSettingConfigurationView();
            }   else    {
                AutoSettings::RemoveSettingConfiguration(this->selectedSetting);
            }
        });
        onSelectedSettingChange(availableSettings[0]);
    }

    void AutoSettingSelectionViewController::OpenSettingConfigurationView() {
        getLogger().info("Option enabled/selected - opening configuration UI");

        PlaylistOverridesViewController* playlistOverrides = reinterpret_cast<PlaylistOverridesViewController*>(this->flowCoordinator->playlistOverridesView);
        ThresholdsViewController* thresholds = reinterpret_cast<ThresholdsViewController*>(this->flowCoordinator->thresholdsView);

        playlistOverrides->setting = this->selectedSettingConfig;
        thresholds->setting = this->selectedSettingConfig;

        flowCoordinator->SetLeftScreenViewController(playlistOverrides, AnimationType::In);
        flowCoordinator->SetRightScreenViewController(thresholds, AnimationType::In);

        this->wasSettingConfigurationViewOpen = true;
    }

    void AutoSettingSelectionViewController::CloseSettingConfigurationView() {
        getLogger().info("Option deselected - closing configuration UI");
        flowCoordinator->SetLeftScreenViewController(nullptr, AnimationType::Out);
        flowCoordinator->SetRightScreenViewController(nullptr, AnimationType::Out);

        this->wasSettingConfigurationViewOpen = false;
    }

    void PlaylistOverridesViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
        static BeatmapLevelsModel* beatmapLevelsModel = Resources::FindObjectsOfTypeAll<BeatmapLevelsModel*>()->values[0]; // Avoid multiple FindObjectsOfTypeAll calls

        if(firstActivation) { // On first activation, we create all of the widgets. Then we can update them when moving to a different setting
            getLogger().info("Performing initial setup for PlaylistOverridesViewController");
            VerticalLayoutGroup* mainLayout = UIUtils::CreateListLikeVerticalLayout(this->get_rectTransform());
            Transform* mainLayoutTransform = mainLayout->get_rectTransform();

            UIUtils::CreateTitle(mainLayoutTransform, "Playlist Overrides", "Force the setting to be a particular value when in a certain set of playlists. Always takes priority over map thresholds if in a playlist where this is enabled");

            this->enableToggle = BeatSaberUI::CreateToggle(mainLayoutTransform, "Enable", [this](bool newValue) {
                enabledLayoutGameObject->SetActive(newValue);

                // When playlist overrides are enabled, we need to use the default setting type (the first) for this SettingTYpe
                if(newValue) {
                    getLogger().info("Setting to default setting type . . .");
                    std::string defaultSetting = this->setting->settingType->options[0];
                    getLogger().info("%s", defaultSetting.c_str());
                    this->setting->playlistOverrideSetting = defaultSetting;
                }   else    {
                    this->setting->playlistOverrideSetting = "";
                }

                // Workaround for setting the text without re-writing to the config
                getLogger().info("Setting current dropdown text");
                this->setSettingToDropdown->text->set_text(il2cpp_utils::createcsstr(this->setting->playlistOverrideSetting));
            });

            VerticalLayoutGroup* enabledLayout = UIUtils::CreateListLikeVerticalLayout(mainLayoutTransform);
            this->enabledLayoutGameObject = enabledLayout->get_gameObject();
            Transform* enabledLayoutTransform = enabledLayout->get_rectTransform();

            this->setSettingToDropdown = BeatSaberUI::CreateDropdown(enabledLayoutTransform, "Set the setting to", "None", {"None"}, [this](std::string newValue) {
                this->setting->playlistOverrideSetting = newValue;
            });

            BeatSaberUI::CreateText(enabledLayoutTransform, "When in these playlists");

            GridLayoutGroup* playlistsLayout = BeatSaberUI::CreateGridLayoutGroup(enabledLayoutTransform);
            playlistsLayout->set_padding(RectOffset::New_ctor(2, 2, 2, 2));
            playlistsLayout->set_constraint(GridLayoutGroup::Constraint::FixedColumnCount);
            playlistsLayout->set_constraintCount(3);
            playlistsLayout->set_cellSize(UnityEngine::Vector2(32.0f, 6.0f));
            playlistsLayout->set_spacing(UnityEngine::Vector2(1.0f, 0.8f));
            UIUtils::ApplyRectPanelBackground(playlistsLayout->get_gameObject());
            Transform* playlistsLayoutTransform = playlistsLayout->get_rectTransform();

            getLogger().info("Adding all playlists to the UI . . .");
            Array<IBeatmapLevelPack*>* playlists = beatmapLevelsModel->get_allLoadedBeatmapLevelPackCollection()->get_beatmapLevelPacks();
            for(int i = 0; i < playlists->get_Length(); i++) {
                IBeatmapLevelPack* playlist = playlists->values[i];
                std::string playlistName = to_utf8(csstrtostr(playlist->get_packName()));

                Toggle* toggle = BeatSaberUI::CreateToggle(playlistsLayoutTransform, playlistName, [this, playlistName](bool newValue) {
                    AutoSettings::SettingConfiguration* setting = this->setting;
                    if(newValue) {
                        setting->overriddenInPlaylists.push_back(playlistName);
                    }   else    {
                        auto location = std::find(setting->overriddenInPlaylists.begin(), setting->overriddenInPlaylists.end(), playlistName);
                        if(location != setting->overriddenInPlaylists.end()) {
                            setting->overriddenInPlaylists.erase(location);
                        }
                    }
                });

                UnityEngine::Transform* toggleParentTransform = toggle->get_transform()->GetParent();
                TextMeshProUGUI* toggleText = toggleParentTransform->get_gameObject()->GetComponentInChildren<TextMeshProUGUI*>();
                // Help all of the playlists to actually fit in the layout
                toggleText->set_overflowMode(TextOverflowModes::Ellipsis);
                toggleText->set_fontSize(2.5);
                getLogger().info("Inserting with name %s", playlistName.c_str());
                this->playlistToggles.push_back({playlistName, toggle}); // Store the playlist toggles for later so that we can change them when moving between multiple settings
            }
        }

        bool isEnabled = !this->setting->playlistOverrideSetting.empty(); // playlistOverrideSetting is empty if this is disabled
        getLogger().info("Is setting enabled: %s", isEnabled ? "true" : "false");
        UIUtils::SetToggleForceNotify(enableToggle, isEnabled);
        
        getLogger().info("Setting options for setting type %s to playlists UI", setting->settingType->saveName.c_str());
        List_1<Il2CppString*>* optionsList = List_1<Il2CppString*>::New_ctor();
        for(std::string optionName : setting->settingType->options) {
            optionsList->Add(il2cpp_utils::createcsstr(optionName));
        }
        this->setSettingToDropdown->SetTexts(reinterpret_cast<IReadOnlyList_1<Il2CppString*>*>(optionsList));

        // Since we may have just swapped from a different setting, we need to update all of the toggles to be in the correct configuration
        getLogger().info("Updating playlist toggles . . . Length: %lu", this->playlistToggles.size());
        for(auto pair : playlistToggles) {
            std::string playlistName = pair.first;
            Toggle* toggle = pair.second;

            bool isEnabled = this->setting->IsOverriddenInPlaylist(playlistName);
            toggle->Set(isEnabled, true);
        }
    }

    void ThresholdsViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
        if(firstActivation) {
            getLogger().info("Performing initial setup for ThresholdsViewController");
            VerticalLayoutGroup* mainLayout = UIUtils::CreateListLikeVerticalLayout(this->get_rectTransform());
            this->mainLayoutTransform = mainLayout->get_rectTransform();
            UIUtils::CreateTitle(mainLayoutTransform, "Configure Thresholds", "Automatically set the setting depending on various map parameters. NOTE: If a playlist override is found, it will always take priority over this");

            std::vector<std::string> mapElements;
            for(AutoSettings::MapElementType* elementType : AutoSettings::GetMapElementTypes()) {
                mapElements.push_back(elementType->name);
            }

            this->parameterDropdown = BeatSaberUI::CreateDropdown(mainLayoutTransform, "Map Parameter", mapElements[0], mapElements, [this](std::string newValue){
                getLogger().info("Changing map element type to %s", newValue.c_str());
                this->setting->SetConfigureBasedOn(AutoSettings::GetMapElementType(newValue, false));
                this->RefreshThresholdSettings();
            });

            this->flipOptionsToggle = BeatSaberUI::CreateToggle(mainLayoutTransform, "Flip Options", [this](bool newValue){
                getLogger().info("Changing flip options to %s", newValue ? "true" : "false");
                this->setting->SetFlipOptions(newValue);
                this->RefreshThresholdSettings();
            });

            BeatSaberUI::AddHoverHint(flipOptionsToggle->get_gameObject(), "Change the order of the options, to allow high NPS enabling debris for example. Not sure why you'd want this, but it's here in case you do.");
        }

        UIUtils::SetDropdownValue(this->parameterDropdown, this->setting->configureBasedOn->name);
        UIUtils::SetToggleForceNotify(this->flipOptionsToggle, this->setting->GetFlipOptions());
    }

    void ThresholdsViewController::RefreshThresholdSettings() {
        if(thresholdsObject) {
            getLogger().info("Removing previous thresholds from layout");
            UIUtils::RemoveAndChildren(thresholdsObject); // Remove the old thresholds
        }

        VerticalLayoutGroup* thresholdsLayout = UIUtils::CreateListLikeVerticalLayout(mainLayoutTransform);
        thresholdsLayout->set_childForceExpandWidth(false);
        thresholdsLayout->set_childControlWidth(true);
        thresholdsLayout->set_padding(RectOffset::New_ctor(2, 2, 2, 2));
        UIUtils::ApplyRectPanelBackground(thresholdsLayout->get_gameObject());

        this->thresholdsObject = thresholdsLayout->get_gameObject();
        Transform* thresholdsLayoutTransform = thresholdsLayout->get_rectTransform();

        int i = 0;
        getLogger().info("Setting has option count %lu", this->setting->settingType->options.size());
        getLogger().info("Setting has theshold count %lu", this->setting->thresholds.size());
        for(std::string settingOption : this->setting->options) {
            BeatSaberUI::CreateText(thresholdsLayoutTransform, settingOption);
            if(i < setting->thresholds.size()) {
                UIUtils::CreateSeparatorLine(thresholdsLayoutTransform);
                BeatSaberUI::CreateIncrementSetting(thresholdsLayoutTransform,
                    "^ Greater than ^",
                    2,
                    0.25f,
                    setting->thresholds[i],
                    [this, i](float newValue) {
                        this->setting->thresholds[i] = newValue;
                    }
                );
                UIUtils::CreateSeparatorLine(thresholdsLayoutTransform);
            }

            i++;
        }
    }
}