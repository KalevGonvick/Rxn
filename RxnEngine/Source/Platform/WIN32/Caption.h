/*****************************************************************//**
 * \file   Caption.h
 * \brief
 *
 * \author kalev
 * \date   July 2023
 *********************************************************************/
#pragma once

namespace Rxn::Platform::Win32
{
    /**
     * .
     */
    enum class Command : int
    {
        CB_NOP = -1,
        CB_CLOSE = 0,
        CB_MINIMIZE = 1,
        CB_MAXIMIZE = 2
    };

    class Caption
    {
    public:

        Caption();
        ~Caption();

    public:

        struct Button
        {
            int         id;
            WString     txt{};
            Command     cmd;
            int         offset;
            int         width;
            RECT        rect{};

            Button(const int &btnId, const WString &btnTxt, const Command &btnCmd, const int &btnOffset, int btnWidth)
                : txt(btnTxt)
                , id(btnId)
                , cmd(btnCmd)
                , offset(btnOffset)
                , width(btnWidth)
            {}
        };

        int32 m_ShowTitle = 1;

        /**
         * .
         *
         * \param text
         * \param command
         * \param width
         */
        void AddButton(const WString &text, const Command &command, int32 width = 50);

        /**
         * .
         *
         * \return
         */
        std::vector<Caption::Button> &GetButtons();

        /**
         * .
         *
         * \param cursor
         * \param windowRect
         * \return
         */
        Command GetWindowCaptionButtonClicked(POINT &cursor, const RECT &windowRect) const;

    private:


        std::vector<Caption::Button> m_Buttons;

    };


}