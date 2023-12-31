#include "Rxn.h"
#include "Caption.h"

namespace Rxn::Platform::Win32
{
    Caption::Caption() = default;
    Caption::~Caption() = default;

    void Caption::AddButton(const WString &text, const Command &command, int32 width)
    {
        int32 newButtonOffset = 0;

        for (const auto &existingBtn : m_Buttons)
        {
            newButtonOffset += existingBtn.width;
        }

        int32 newId = static_cast<int32>(m_Buttons.size()) + 1;
        m_Buttons.emplace_back(newId, text, command, newButtonOffset, width);
    }

    std::vector<Caption::Button> &Caption::GetButtons()
    {
        return m_Buttons;
    }

    Command Caption::GetWindowCaptionButtonClicked(POINT &cursor, const RECT &windowRect) const
    {
        cursor.x -= windowRect.left;
        cursor.y -= windowRect.top;

        for (const auto & currentButton : m_Buttons)
        {
            if (currentButton.rect.left < cursor.x && currentButton.rect.right > cursor.x && currentButton.rect.top < cursor.y && currentButton.rect.bottom > cursor.y)
            {
                return currentButton.cmd;
            }
        }


        return Command::CB_NOP;
    }






}