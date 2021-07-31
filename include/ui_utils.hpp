#pragma once

// Contains useful functions for avoiding repeating code in the FishUtils UI
// Also contains a bunch of common UI related includes
// I am aware that this is using namespace in a header, but it's only included within UI CPP files that need these namespaces for convenience

#include "questui/shared/BeatSaberUI.hpp"
#include "questui/shared/CustomTypes/Components/Backgroundable.hpp"
#include "questui/shared/CustomTypes/Components/Settings/IncrementSetting.hpp"
using namespace QuestUI;

#include "HMUI/ViewController_AnimationType.hpp"
#include "HMUI/ViewController_AnimationDirection.hpp"
#include "HMUI/Touchable.hpp"

#include "UnityEngine/UI/VerticalLayoutGroup.hpp"
#include "UnityEngine/UI/HorizontalLayoutGroup.hpp"
#include "UnityEngine/UI/GridLayoutGroup.hpp"
#include "UnityEngine/UI/Toggle.hpp"
using namespace UnityEngine::UI;

#include "UnityEngine/Transform.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/RectOffset.hpp"
#include "UnityEngine/Resources.hpp"
using namespace UnityEngine;

#include "TMPro/TextMeshProUGUI.hpp"
#include "TMPro/TextAlignmentOptions.hpp"
using namespace TMPro;

namespace FishUtils::UIUtils {
    // Creates a decent looking title with a round-rect background on the specified Transform
    void CreateTitle(Transform* parent, std::string text, std::string hoverHint = "");

    // Creates a HorizontalLayoutGroup on transform with childForceExpandWidth set to false and childControlWidth set to true.
    // Also sets the child alignment to middleCenter so that elements can stack left-to-right without being force-expanded to fit the available space
    HorizontalLayoutGroup* CreateListLikeHorizontalLayout(Transform* parent);

    // Creates a VerticalLayoutGroup on transform with childForceExpandHeight set to false and childControlHeight set to true
    // Also sets the child alignment to upperCenter so that elements can stack up-to-down without being force-expanded to fit the available space
    VerticalLayoutGroup* CreateListLikeVerticalLayout(Transform* parent);

    // Creates a horizontal separator line thing to show a divide between two areas of a UI
    // Blue by default because this is FishCore
    void CreateSeparatorLine(Transform* parent, Color color = {0, 0.5, 0.5, 1.0});

    // Applies a round-rect-panel background using QuestUI's backgroundable
    void ApplyRectPanelBackground(GameObject* gameObject);

    // Sets the text of a toggle, by finding the right text mesh
    void SetToggleText(Toggle* toggle, std::string text);

    // Sets the value of a toggle, and also calls the notify function, even if the value didn't change
    void SetToggleForceNotify(Toggle* toggle, bool newValue);

    // Removes this GameObject then all of its children
    void RemoveAndChildren(GameObject* gameObject);

    // Finds the index of value in this dropdown and sets the selected index to that index
    void SetDropdownValue(HMUI::SimpleTextDropdown* dropdown, std::string value);
}