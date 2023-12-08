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

        RXN_LOGGER::PrintLnSeperator();
        RXN_LOGGER::Trace(L"Cursor at x: %d y: %d", cursor.x, cursor.y);
        RXN_LOGGER::Trace(L"Window at top: %d, left: %d, right: %d, bottom: %d", windowRect.top, windowRect.left, windowRect.right, windowRect.bottom);

        for (auto currentButton = m_Buttons.begin(); currentButton != m_Buttons.end(); currentButton++)
        {
            std::shared_ptr<Button> itButton = *currentButton;
            RXN_LOGGER::Trace(L"Button rect at top: %d, left: %d, right: %d, bottom: %d", itButton->rect.top, itButton->rect.left, itButton->rect.right, itButton->rect.bottom);


            if (itButton->rect.left < cursor.x && itButton->rect.right > cursor.x && itButton->rect.top < cursor.y && itButton->rect.bottom > cursor.y)
            {
                RXN_LOGGER::Trace(L"Click within bounds!!!");
                RXN_LOGGER::PrintLnSeperator();
                return itButton->cmd;
            }
        }
        RXN_LOGGER::PrintLnSeperator();


        return Command::CB_NOP;
    }






}