#pragma once
#include "UI3tButton.h"
#include "UIDragDropListEx.h"
#include <functional>

// Native button state handling with texture-only faces and deferred view refresh.
class CUIFieldKitButton final : public CUI3tButton {
    std::function<void()> action;
public:
    explicit CUIFieldKitButton(std::function<void()> callback):action(std::move(callback)){}
    void OnClick() override {
        CUI3tButton::OnClick();
        if(!CUIDragDropListEx::m_drag_item)action();
    }
    void Update() override {
        Enable(!CUIDragDropListEx::m_drag_item);
        CUI3tButton::Update();
    }
};
