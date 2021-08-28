#include "main.hpp"
#include "main_settings_ui.hpp"
#include "pause_menu_ui.hpp"
#include "pause_tweaks.hpp"
#include "auto_settings_ui.hpp"
#include "auto_settings.hpp"

#include "questui/shared/QuestUI.hpp"
#include "questui/shared/BeatSaberUI.hpp"

#include "custom-types/shared/register.hpp"

#include "bs-utils/shared/utils.hpp"

#include "GlobalNamespace/MenuTransitionsHelper.hpp"
#include "System/Action_1.hpp"
#include "Zenject/DiContainer.hpp"

static ModInfo modInfo; // Stores the ID and version of our mod, and is sent to the modloader upon startup

// Loads the config from disk using our modInfo, then returns it for use
Configuration& getConfig() {
    static Configuration config(modInfo);
    return config;
}

// Returns a logger, useful for printing debug messages
Logger& getLogger() {
    static Logger* logger = new Logger(modInfo);
    return *logger;
}

ModInfo& getModInfo() {
    return modInfo;
}

MAKE_HOOK_MATCH(MenuTransitionsHelper_RestartGame, &MenuTransitionsHelper::RestartGame, void, MenuTransitionsHelper* self, System::Action_1<Zenject::DiContainer*>* finishCallback)
{
    FishUtils::AutoSettings::ClearCachedPointers();
    MenuTransitionsHelper_RestartGame(self, finishCallback);
}

// Called at the early stages of game loading
extern "C" void setup(ModInfo& info) {
    info.id = ID;
    info.version = VERSION;
    modInfo = info;
	
    getConfig().Load(); // Load the config file
    getLogger().info("Completed setup!");
}

// Called later on in the game loading - a good time to install function hooks
extern "C" void load() {
    bs_utils::Submission::enable(modInfo);

    getLogger().info("Calling il2cpp functions init");
    il2cpp_functions::Init();
    getLogger().info("Done");

    getLogger().info("Registering custom types . . .");
    custom_types::Register::AutoRegister();
    getLogger().info("Done");

    getConfig().Load();
    FishUtils::PauseTweaks::Init();
    FishUtils::AutoSettings::Init();
    INSTALL_HOOK(getLogger(), MenuTransitionsHelper_RestartGame);
    getConfig().Write();

    getLogger().info("Registering main flow coordinator . . .");
    QuestUI::Init();
    QuestUI::Register::RegisterModSettingsFlowCoordinator<FishUtils::SettingsFlowCoordinator*>(modInfo);
    getLogger().info("Done");
}