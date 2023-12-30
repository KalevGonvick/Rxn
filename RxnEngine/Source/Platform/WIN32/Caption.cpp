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
            newButtonOffset += existingBtn->width;
        }

        int32 newId = static_cast<int32>(m_Buttons.size()) + 1;
        auto newButton = std::make_shared<Caption::Button>(newId, text, command, newButtonOffset, width);
        m_Buttons.push_back(newButton);
    }

    void Caption::RemoveButton(const int32 &removeId)
    {
        for (auto it = m_Buttons.begin(); it != m_Buttons.end();)
        {
            if (it->get()->id == removeId)
            {
                m_Buttons.erase(it);
                break;
            }
            else
            {
                it++;
            }
        }


    }

    std::list<std::shared_ptr<Caption::Button>> &Caption::GetButtons()
    {
        return m_Buttons;
    }

    Command Caption::GetWindowCaptionButtonClicked(POINT &cursor, const RECT &windowRect)
    {
        cursor.x -= windowRect.left;
        cursor.y -= windowRect.top;

        for (auto currentButton = m_Buttons.begin(); currentButton != m_Buttons.end(); currentButton++)
        {
            std::shared_ptr<Button> itButton = *currentButton;

            if (itButton->rect.left < cursor.x && itButton->rect.right > cursor.x && itButton->rect.top < cursor.y && itButton->rect.bottom > cursor.y)
            {
                return itButton->cmd;
            }
        }


        return Command::CB_NOP;
    }






}