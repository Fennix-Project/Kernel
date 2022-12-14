#include <gui.hpp>

#include <driver.hpp>
#include <task.hpp>
#include <printf.h>
#include <debug.h>

#include "../kernel.h"
#include "../DAPI.hpp"
#include "../Fex.hpp"

namespace GraphicalUserInterface
{
    void WidgetCollection::OnPaintBackground(Event *e)
    {
    }

    void WidgetCollection::OnPaintForeground(Event *e)
    {
        foreach (auto Panel in this->Panels)
        {
            PutRect(this->Buffer, Panel->rect, Panel->Color);
        }

        foreach (auto Label in this->Labels)
        {
            Label->CharCursorX = Label->rect.Left;
            Label->CharCursorY = Label->rect.Top;
            for (uint64_t i = 0; i < strlen(Label->Text); i++)
                PaintChar(this->CurrentFont, this->Buffer, Label->Text[i], 0xFFFFFF, &Label->CharCursorX, &Label->CharCursorY);
        }

        foreach (auto Button in this->Buttons)
        {
            if (Button->Pressed)
                PutRect(this->Buffer, Button->rect, Button->PressedColor);
            else if (Button->Hover)
                PutRect(this->Buffer, Button->rect, Button->HoverColor);
            else
                PutRect(this->Buffer, Button->rect, Button->Color);
            Button->CharCursorX = Button->rect.Left + (((Button->rect.Width / 2) - (this->CurrentFont->GetInfo().Width * strlen(Button->Text) / 2)));
            Button->CharCursorY = Button->rect.Top + (((Button->rect.Height / 2) - (this->CurrentFont->GetInfo().Height / 2)));
            for (uint64_t i = 0; i < strlen(Button->Text); i++)
                PaintChar(this->CurrentFont, this->Buffer, Button->Text[i], 0xFFFFFF, &Button->CharCursorX, &Button->CharCursorY);
        }
    }

    void WidgetCollection::OnPaint(Event *e)
    {
        static long LastWidth = 0;
        static long LastHeight = 0;

        if (LastWidth != ((Window *)this->ParentWindow)->GetPosition().Width ||
            LastHeight != ((Window *)this->ParentWindow)->GetPosition().Height)
        {
            LastWidth = ((Window *)this->ParentWindow)->GetPosition().Width;
            LastHeight = ((Window *)this->ParentWindow)->GetPosition().Height;

            this->mem->FreePages(this->Buffer->Data, TO_PAGES(this->Buffer->Size));
            delete this->Buffer;

            this->Buffer = new ScreenBitmap;
            this->Buffer->Width = LastWidth;
            this->Buffer->Height = LastHeight;
            this->Buffer->BitsPerPixel = Display->GetBitsPerPixel();
            this->Buffer->Pitch = Display->GetPitch();
            this->Buffer->Size = this->Buffer->Pitch * LastHeight;
            this->Buffer->Data = (uint8_t *)this->mem->RequestPages(TO_PAGES(this->Buffer->Size));
            memset(this->Buffer->Data, 0, this->Buffer->Size);

            NeedRedraw = true;
        }

        if (NeedRedraw)
        {
            memset(this->Buffer->Data, 0, this->Buffer->Size);
            this->OnPaintBackground(e);
            this->OnPaintForeground(e);
            NeedRedraw = false;
        }
        DrawOverBitmap(((Window *)this->ParentWindow)->GetBuffer(), this->Buffer, 0, 0);
    }

    void WidgetCollection::OnMouseDown(Event *e)
    {
        foreach (auto Button in this->Buttons)
        {
            if (Button->rect.Contains(e->MouseDown.X, e->MouseDown.Y))
            {
                if (e->MouseDown.Left)
                {
                    debug("Button Hold");
                    Button->Pressed = true;
                    NeedRedraw = true;
                }
            }
        }
    }

    void WidgetCollection::OnMouseUp(Event *e)
    {
        foreach (auto Button in this->Buttons)
        {
            if (Button->rect.Contains(e->MouseUp.X, e->MouseUp.Y))
            {
                if (e->MouseUp.Left)
                {
                    debug("Button Release");
                    if (Button->Pressed)
                    {
                        debug("Button Clicked");
                        if (Button->OnClick != (uintptr_t) nullptr)
                            ((void (*)(void))Button->OnClick)();

                        Button->Pressed = false;
                        NeedRedraw = true;
                    }
                }
            }
        }
    }

    void WidgetCollection::OnMouseMove(Event *e)
    {
        foreach (auto Button in this->Buttons)
        {
            if (Button->rect.Contains(e->MouseMove.X, e->MouseMove.Y))
            {
                Button->Hover = true;
                NeedRedraw = true;
            }
            else
            {
                Button->Hover = false;
                NeedRedraw = true;
            }
        }
    }
}
