#ifndef UITEXT_H
#define UITEXT_H

#include "PouEngine/ui/UiElement.h"
#include "PouEngine/ui/UiPicture.h"

namespace pou
{

class UiText : public UiElement
{
    public:
        UiText(UserInterface *interface);
        virtual ~UiText();

        virtual void render(UiRenderer *renderer);

        virtual void update(const Time &elapsedTime = Time(0), uint32_t localTime = -1) override;

        using UiElement::setSize;
        virtual void setSize(glm::vec2 s);

        void setFont(FontAsset *font);
        void setText(const std::string &text);
        void setFontSize(int pt);
        void setColor(const glm::vec4 &color);
        void setTextAlign(TextAlignType textAlign);
        void setVerticalAlign(bool verticalAlign);

        const std::string &getText();

    protected:
        void generatedGlyphes();
        int computeWordSize(size_t charPos);
        //void jumpLine();

    private:
        FontAsset *m_font;
        std::string m_text;
        int m_fontSize;
        glm::vec4 m_color;
        TextAlignType m_textAlign;
        bool m_verticalAlign;

        bool m_needToUpdateGlyphes;
        std::vector< std::shared_ptr<UiPicture> > m_glyphes;
};

}

#endif // UITEXT_H
