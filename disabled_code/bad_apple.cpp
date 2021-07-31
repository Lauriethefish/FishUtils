#include "bad_apple.hpp"
#include "ui_utils.hpp"
#include "main.hpp"
#include "UnityEngine/Time.hpp"

namespace FishUtils::BadApple {
    const int SCREEN_WIDTH = 20;
    const int SCREEN_HEIGHT = 10;

    DEFINE_TYPE(BadAppleViewController);

    void BadAppleViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
        if(!firstActivation) {
            return;
        }

        VerticalLayoutGroup* rowsLayout = BeatSaberUI::CreateVerticalLayoutGroup(this->get_rectTransform());
        HMUI::Touchable* touchable = this->GetComponent<HMUI::Touchable*>();
        GameObject::Destroy(touchable);

        Transform* rowsLayoutTransform = rowsLayout->get_rectTransform();
        for(int y = 0; y < SCREEN_HEIGHT; y++) {
            HorizontalLayoutGroup* rowLayout = BeatSaberUI::CreateHorizontalLayoutGroup(rowsLayoutTransform);
            Transform* rowLayoutTransform = rowLayout->get_rectTransform();
            
            screenSegments.push_back(std::vector<UnityEngine::UI::Button*>());
            for(int x = 0; x < SCREEN_WIDTH; x++) {
                HorizontalLayoutGroup* buttonLayout = BeatSaberUI::CreateHorizontalLayoutGroup(rowLayoutTransform);
                LayoutElement* buttonLayoutElement = buttonLayout->GetComponent<LayoutElement*>();
                buttonLayoutElement->set_minWidth(6.5);
                buttonLayoutElement->set_minHeight(6.5);

                Button* button = BeatSaberUI::CreateUIButton(buttonLayout->get_rectTransform(), "", "OkButton", Vector2(), Vector2(6.5, 6.5), nullptr); 
                button->get_gameObject()->get_transform()->set_localScale(Vector3(1.0, 1.5, 1.0));

                screenSegments[y].push_back(button);
            }
        }

        getLogger().info("Loading bad apple file . . .");
        std::ifstream fileStream;
        fileStream.open("/sdcard/result.badapple", std::ios::in | std::ios::binary);

        this->file = FishUtils::BadApple::BadAppleFile(fileStream);

        fileStream.close();

        getLogger().info("Loaded all " + std::to_string(file.frames.size()) + " frames");
    }

    void BadAppleViewController::Update() {
        getLogger().info("BadAppleViewController updating");

        currentTime += Time::get_deltaTime();

        FrameInfo chosenFrame;
        while(currentFrameIndex < file.frames.size()) {
            FrameInfo& currentFrame = file.frames[currentFrameIndex];

            if(currentFrame.time > currentTime) {
                chosenFrame = currentFrame;
                break;
            }
            currentFrameIndex++;
        }

        for(ChangeInfo change : chosenFrame.changes) {
            this->screenSegments[change.y][change.x]->get_gameObject()->SetActive(!change.newValue);
        }
        
    }

    ChangeInfo::ChangeInfo(std::ifstream& stream) {
        stream.read((char*)&x, 4);
        stream.read((char*)&y, 4);
        stream.read((char*)&newValue, 1);
    }

    FrameInfo::FrameInfo(std::ifstream& stream) {
        stream.read((char*)&this->time, 4);
        int32_t changesLength;
        stream.read((char*)&changesLength, 4);

        for (int i = 0; i < changesLength; i++) {
            this->changes.push_back(ChangeInfo(stream));
        }

        getLogger().info("Loaded frame with " + std::to_string(changes.size()) + " changes!");
    }

    BadAppleFile::BadAppleFile(std::ifstream& stream) {
        int32_t framesLength;
        stream.read((char*)&framesLength, 4);

        getLogger().info("Frames length: " + std::to_string(framesLength));

        for (int i = 0; i < framesLength; i++) {
            this->frames.push_back(FrameInfo(stream));
        }
    }
}