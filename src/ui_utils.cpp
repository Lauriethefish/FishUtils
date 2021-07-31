#include "ui_utils.hpp"
#include "main.hpp"

#include "System/Collections/Generic/IReadOnlyList_1.hpp"
using namespace System::Collections::Generic;

namespace FishUtils::UIUtils {
    void CreateTitle(Transform* parent, std::string text, std::string hoverHint) {
        VerticalLayoutGroup* titleLayout = CreateListLikeVerticalLayout(parent);
        Transform* titleLayoutTransform = titleLayout->get_rectTransform();
        ApplyRectPanelBackground(titleLayout->get_gameObject());
        titleLayout->set_padding(RectOffset::New_ctor(8, 8, 2, 2)); // Add some space to the side and top

        TextMeshProUGUI* titleText = BeatSaberUI::CreateText(titleLayoutTransform, text);
        titleText->set_alignment(TextAlignmentOptions::Center);
        titleText->set_fontSize(6.0f);
        if(!hoverHint.empty()) { // If the string is empty then no hover hint was specified
            BeatSaberUI::AddHoverHint(titleText->get_gameObject(), hoverHint);
        }
    }

    HorizontalLayoutGroup* CreateListLikeHorizontalLayout(Transform* parent) {
        HorizontalLayoutGroup* layout = BeatSaberUI::CreateHorizontalLayoutGroup(parent);
        layout->set_childForceExpandWidth(false);
        layout->set_childControlWidth(true);
        layout->set_childForceExpandHeight(false);
        layout->set_childControlHeight(true);

        return layout;
    }

    VerticalLayoutGroup* CreateListLikeVerticalLayout(Transform* parent) {
        VerticalLayoutGroup* layout = BeatSaberUI::CreateVerticalLayoutGroup(parent);
        layout->set_childForceExpandHeight(false);
        layout->set_childControlHeight(true);
        layout->set_childScaleHeight(true);
        layout->set_childAlignment(TextAnchor::UpperCenter);

        return layout;
    }

    void CreateSeparatorLine(Transform* parent, Color color) {
        TextMeshProUGUI* separatorText = BeatSaberUI::CreateText(parent, "__________________________________________________");
        
        separatorText->set_color(color);
        separatorText->set_alignment(TextAlignmentOptions::Center);
    }

    void ApplyRectPanelBackground(GameObject* gameObject) {
        gameObject->AddComponent<Backgroundable*>()->ApplyBackground(il2cpp_utils::createcsstr("round-rect-panel")); // Add the panel background
    }

    void SetToggleText(Toggle* toggle, std::string text) {
        // Takes a bit of effort to find the actual text mesh for the text, not the 0 or 1 mesh
        TextMeshProUGUI* textMesh = toggle->get_gameObject()->get_transform()->GetParent()->get_gameObject()->GetComponentInChildren<TextMeshProUGUI*>();
        textMesh->set_text(il2cpp_utils::createcsstr(text));
    }

    void SetToggleForceNotify(Toggle* toggle, bool newValue) {
        bool oldToggleValue = toggle->m_IsOn;

        toggle->Set(newValue, true);
        // Force invoke if the value was the same
        if(oldToggleValue == newValue) {
            toggle->onValueChanged->Invoke(newValue);
        }
    }

    void RemoveAndChildren(GameObject* gameObject) {
        Transform* transform = gameObject->get_transform();
        Array<Transform*>* children = transform->GetComponentsInChildren<UnityEngine::Transform*>();
        for(int i = 0; i < children->Length(); i++) {
            GameObject::Destroy(children->values[i]->get_gameObject());
        }
    }

    void SetDropdownValue(HMUI::SimpleTextDropdown* dropdown, std::string value) {
        IReadOnlyList_1<Il2CppString*>* texts = dropdown->texts;
        int textsLength = reinterpret_cast<IReadOnlyCollection_1<Il2CppString*>*>(texts)->get_Count();

        for(int i = 0; i < textsLength; i++) {
            Il2CppString* text = texts->get_Item(i);
            std::string cppText = to_utf8(csstrtostr(text));

            if(cppText == value) {
                dropdown->SelectCellWithIdx(i);
                return;
            }
        }

        getLogger().warning("Unable to set dropdown value - no index exists with name %s", value.c_str());
    }
}