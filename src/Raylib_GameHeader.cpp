#include "Raylib_GameHeader.h"

//RayWindow

void Raylib::RayWindow::load()
{
    if(!loaded)
    {
        SetConfigFlags(FLAG_WINDOW_RESIZABLE);
        InitWindow(width,height,"Chess");
        windowIcon = LoadImage("Sprites\\WindowIcon.png");
        ImageFormat(&windowIcon, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
        SetWindowIcon(windowIcon);
        SetTargetFPS(60);
        SetExitKey(NULL);
        active = true;
        loaded = true;
    }
}

void Raylib::RayWindow::unload()
{
    if(loaded)
    {
        UnloadImage(windowIcon);
        CloseWindow();
        loaded = false;
    }
}

Raylib::RayWindow::~RayWindow()
{
    unload();
}

void Raylib::updateWindowSize()
{
    Window.width = GetScreenWidth();
    Window.height = GetScreenHeight();
    Window.size = Window.height / 8;

    Window.sizef =   (float)Window.size;
    Window.widthf =  (float)Window.width;
    Window.heightf = (float)Window.height;
}

//Drawing with relative Coordinates

Vector2 Raylib::convertVector2RelativeToAbs(Vector2 xRelative)
{
    Vector2 xAbs{};

    xAbs.x = xRelative.x * Window.widthf;
    xAbs.y = xRelative.y * Window.heightf;

    return xAbs;
}

Rectangle Raylib::convertRectangleRelativeToAbs(Rectangle rectRelative)
{
    Rectangle rectAbs{};

    rectAbs.width  = rectRelative.width  * Window.widthf;
    rectAbs.height = rectRelative.height * Window.heightf;

    rectAbs.x = rectRelative.x * Window.widthf;
    rectAbs.y = rectRelative.y * Window.heightf;

    return rectAbs;
}

void Raylib::drawRectangle(Rectangle rect, Color color, float rotation, Vector2 rotationOrigin)
{
    DrawRectanglePro( convertRectangleRelativeToAbs(rect), convertVector2RelativeToAbs(rotationOrigin), rotation, color);
}

void Raylib::drawRectangleLines(Rectangle rect, float lineThick, Color color)
{
    DrawRectangleLinesEx(convertRectangleRelativeToAbs(rect), lineThick, color);
}

void Raylib::drawLine(Vector2 startpoint, Vector2 endpoint, float thickness, Color color)
{
    const Vector2 windowRatio = {Window.widthf / (float)Window.widthStart, Window.heightf / (float)Window.heightStart};
    thickness *= windowRatio.x * windowRatio.y;

    DrawLineEx(convertVector2RelativeToAbs(startpoint), convertVector2RelativeToAbs(endpoint), thickness, color);
}

void Raylib::drawText(const std::string &text, Vector2 position, float fontSize, Color tint, float spacing, Font font, float rotation, Vector2 rotationOrigin)
{
    Vector2 windowRatio = {Window.widthf / (float)Window.widthStart, Window.heightf / (float)Window.heightStart};

    spacing *= windowRatio.x * windowRatio.y * fontSize; //Adjust spacing both based on the new window size and on the fontSize.
    fontSize *= windowRatio.x * windowRatio.y;

    bool textFitsWindow = false;
    while(!textFitsWindow)
    {
        Vector2 textSize = MeasureTextEx(font, text.c_str(), fontSize, spacing);

        if(position.x + (textSize.x/Window.widthf) >= 1.0 || position.y + (textSize.y/Window.heightf) >= 1.0)
        {
            fontSize -= 1.0f;
        }
        else
            textFitsWindow = true;
    }

    DrawTextPro(font, text.c_str(), convertVector2RelativeToAbs(position), convertVector2RelativeToAbs(rotationOrigin), rotation, fontSize, spacing, tint);
}

Vector2 Raylib::getTextSize(const std::string &text, float fontSize, float spacing, Font font)
{
    const Vector2 windowRatio = {Window.widthf / (float)Window.widthStart, Window.heightf / (float)Window.heightStart};

    spacing *= windowRatio.x * windowRatio.y * fontSize; //Adjust spacing both based on the new window size and on the fontSize.
    fontSize *= windowRatio.x * windowRatio.y;
    
    Vector2 textSize = MeasureTextEx(font, text.c_str(), fontSize, spacing);
    return { textSize.x/Window.widthf, textSize.y/Window.heightf };
}

void Raylib::drawCircle(Vector2 pos, Vector2 radius, Color color, Color edgeColor)
{
    const Vector2 posAbs = convertVector2RelativeToAbs(pos);
    const Vector2 radiusAbs = convertVector2RelativeToAbs(radius);

    DrawEllipse((int)posAbs.x, (int)posAbs.y, radiusAbs.x, radiusAbs.y, color);
    DrawEllipseLines((int)posAbs.x, (int)posAbs.y, radiusAbs.x, radiusAbs.y, edgeColor);
}

//RaylibTexture

void Raylib::RaylibTexture::load()
{
    if(!loaded && Window.isLoaded())
    {
        Texture = LoadTexture(SpriteName.c_str());
        if(!spritesheet)
        {
            SpriteSize.width = (float)Texture.width;
            SpriteSize.height = (float)Texture.height;
        }
        loaded = true;
    }
}

void Raylib::RaylibTexture::getTextureSize(Vector2 &textureSize) const
{
    if(loaded)
    {
        textureSize.x = (float)Texture.width;
        textureSize.y = (float)Texture.height;
    }
    else
    {
        std::cerr << "ERROR: getTextureSize for " << SpriteName << "; Texture not yet loaded!\n";
        textureSize.x = 0.0f;
        textureSize.y = 0.0f;
    }
}

void Raylib::RaylibTexture::setPosition(const Vector2& position)
{
    Size.x = position.x;
    Size.y = position.y;
}

void Raylib::RaylibTexture::setSize(const Vector2 &size)
{
    Size.width = size.x;
    Size.height = size.y;
    originPoint = {0.5f*size.x, 0.5f*size.y};
}

void Raylib::RaylibTexture::setSpritePosition(const Vector2 &position)
{
    SpriteSize.x = position.x;
    SpriteSize.y = position.y;
    spritesheet = true;
}

void Raylib::RaylibTexture::setSpriteSize(const Vector2 &size)
{
    SpriteSize.width = size.x;
    SpriteSize.height = size.y;
    spritesheet = true;
}

void Raylib::RaylibTexture::flipHorizontal()
{
    SpriteSize.width = -SpriteSize.width;
}

void Raylib::RaylibTexture::flipVertical()
{
    SpriteSize.height = -SpriteSize.height;
}

void Raylib::RaylibTexture::draw(float rotation, Color color) const
{
    if(loaded)
    {
        Rectangle rectangle = { .x = Size.x + originPoint.x, .y = Size.y + originPoint.y, .width = Size.width, .height = Size.height };
        DrawTexturePro(Texture, SpriteSize, convertRectangleRelativeToAbs(rectangle),
                       convertVector2RelativeToAbs(originPoint), rotation, color);
    }
}

void Raylib::RaylibTexture::unload()
{
    if(loaded)
    {
        UnloadTexture(Texture);
        loaded = false;
    }
}

//Button

void Raylib::Button::initialize(const Vector2 &pos,const Vector2 &s,const char* txt)
{
    x_draw = pos;
    size = s;
    pressed = false;
    selected = false;
    if(txt != nullptr)
        text = txt;
}

void Raylib::Button::move(const Vector2 &newpos)
{
    x_draw = newpos;
}

void Raylib::Button::drawButtonText(Color colorText) const
{
    if(text.length() > 0)
    {
        constexpr float spacing = 0.1f;
        Vector2 txtLength = getTextSize(text,fontSize,spacing);

        Vector2 position{};
        position.x = x_draw.x + 0.5f*size.x - 0.5f*txtLength.x;

        if(textInsideButton)
            position.y = x_draw.y + 0.5f*size.y - 0.5f*txtLength.y;
        else
            position.y = x_draw.y + size.y + 0.005f;
        
        Raylib::drawText(text,position,fontSize, colorText, spacing);
    }
}

void Raylib::Button::draw(Color colorOn,Color colorOff,Color colorText,float rotation) const
{
    if(Window.isLoaded())
    {
        if(drawCircle)
        {
            const Vector2 xCircle = {x_draw.x + 0.5f*size.x, x_draw.y + 0.5f*size.y};
            const Vector2 radius = {0.5f*size.x, 0.5f*size.y};

            if(!pressed)
                Raylib::drawCircle(xCircle, radius, colorOff);
            else
                Raylib::drawCircle(xCircle, radius, colorOn);
        }
        else
        {
            Rectangle buttonRect = {x_draw.x+size.x/2.0f,x_draw.y+size.y/2.0f,size.x,size.y};

            if(!pressed)
                drawRectangle(buttonRect,colorOff,rotation,{size.x/2.0f,size.y/2.0f});
            else
                drawRectangle(buttonRect, colorOn, rotation, {size.x/2.0f,size.y/2.0f});

            //Draw borders
            constexpr float lineThick = 3.0f;
            Rectangle borderRect = {x_draw.x,x_draw.y,size.x,size.y};
            DrawRectangleLinesEx(convertRectangleRelativeToAbs(borderRect), lineThick, BLACK);
        }

        drawButtonText(colorText);
    }
}

void Raylib::Button::drawTextured(RaylibTexture &texture,Color colorOn,Color colorOff,Color colorText,float rotation) const
{
    texture.setPosition({x_draw.x, x_draw.y});
    texture.setSize(size);

    if(!pressed)
        texture.draw(rotation, colorOff);
    else
        texture.draw(rotation, colorOn);

    drawButtonText(colorText);
}

void Raylib::Button::checkInput()
{
    if(checkMouseCollision())
        selected = true;
    else
        selected = false;

    pressedOld = pressed;

    if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && selected)
        pressed = true;

    if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        pressed = false;
}

bool Raylib::Button::checkMouseCollision() const
{
    bool xCol{false},yCol{false};
    Vector2 mousePos = {(float)GetMouseX()/Window.widthf, (float)GetMouseY()/Window.heightf,};

    if(mousePos.x >= x_draw.x && mousePos.x <= x_draw.x + size.x)
        xCol = true;

    if(mousePos.y >= x_draw.y && mousePos.y <= x_draw.y + size.y)
        yCol = true;

    return xCol && yCol;
}

void Raylib::SwitchButton::checkInput()
{
    if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && checkMouseCollision())
        selected = true;

    pressedOld = pressed;
    if(selected && IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && checkMouseCollision())
    {
        selected = false;
        if(pressed)
            pressed = false;
        else
            pressed = true;
    }
}

//Speechbubble

void Raylib::SpeechBubble::load()
{
    if(!loaded)
    {
        texture.load();
        adjustBubbleSize();
        loaded = true;
    }
}

void Raylib::SpeechBubble::changeText(const std::string &t)
{
    text = t;
    adjustBubbleSize();
}

void Raylib::SpeechBubble::changeFontSize(float s)
{
    fontsize = s;
    adjustBubbleSize();
}

void Raylib::SpeechBubble::move(const Vector2 &p)
{
    pos = {p.x,p.y};
    texture.setPosition(pos);
    textpos = {p.x + border/2.0f,p.y + bubbleheight/2.0f - textsize.y/2.0f};

    if(!fliped)
        speechpos = {pos.x + 0.295f*size.x,pos.y+size.y};
    else
        speechpos = {pos.x + (1.0f-0.295f)*size.x,pos.y+size.y};

}

void Raylib::SpeechBubble::moveTail(Vector2 p)
{
    speechpos = p;

    if(!fliped)
        pos = { p.x - 0.295f*size.x, p.y - size.y};
    else
        pos = {p.x - (1.0f-0.295f)*size.x, p.y - size.y};

    texture.setPosition(pos);
    textpos = {pos.x + border/2.0f,pos.y + bubbleheight/2.0f - textsize.y/2.0f};
}

void Raylib::SpeechBubble::draw()
{
    if(loaded)
    {
        texture.draw();
        drawText(text, textpos, fontsize, BLACK, spacing);
    }
}

void Raylib::SpeechBubble::flip()
{
    if(loaded)
    {
        texture.flipHorizontal();

        if(fliped)
        {
            fliped = false;
            speechpos = {pos.x + 0.295f*size.x,pos.y+size.y};
        }
        else
        {
            fliped = true;
            speechpos = {pos.x + (1.0f-0.295f)*size.x,pos.y+size.y};
        }
    }
}

void Raylib::SpeechBubble::unload()
{
    if(loaded)
    {
        texture.unload();
        loaded = false;
    }
}

void Raylib::SpeechBubble::adjustBubbleSize()
{
    if(Window.isLoaded())
    {
        textsize = getTextSize( text,fontsize,spacing );
        size = {textsize.x+border,2.0f*textsize.y};
        texture.setSize(size);

        bubbleheight = 0.7463f * size.y;
        speechpos = {pos.x + 0.295f*size.x,pos.y+size.y}; //Where the speechbubble starts.

        textpos.x = pos.x + border/2.0f;
        textpos.y = pos.y + bubbleheight/2.0f - textsize.y/2.0f;
    }
}

//Slider

bool Raylib::Slider::testMouseSliderCollision() const
{
    const Vector2 mousePos = {(float)GetMouseX()/Window.widthf, (float)GetMouseY()/Window.heightf,};
    const bool xCol = ( mousePos.x >= rectSlider.x && mousePos.x <= rectSlider.x + rectSlider.width );
    const bool yCol = (mousePos.y >= rectSlider.y && mousePos.y <= rectSlider.y + rectSlider.height);

    return xCol && yCol;
}

void Raylib::Slider::convertMouseXtoSliderX()
{
    const float mouseX = (float)GetMouseX()/Window.widthf;
    if(mouseX <= rectBox.x)
    {
        rectSlider.x = rectBox.x;
        value = min;
    }
    else if(mouseX >= rectBox.x + rectBox.width)
    {
        rectSlider.x = rectBox.x + rectBox.width - rectSlider.width;
        value = max;
    }
    else
    {
        const float posRel = (mouseX - rectBox.x) / rectBox.width;
        const float maxf = (float)(max), minf = (float)(min);
        value = (int)((maxf-minf+1) *posRel + minf);
        if(value >= min && value <= max)
            rectSlider.x = rectBox.x + (float)(value-1)*rectSlider.width;
    }
}

void Raylib::Slider::input()
{
    if(testMouseSliderCollision() && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        selected = true;
    }

    if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        selected = false;

    if(selected)
        convertMouseXtoSliderX();
}

void Raylib::Slider::draw() const
{
    constexpr float lineThick = 3.0f;

    drawRectangle(rectBox,colorBox);
    DrawRectangleLinesEx(convertRectangleRelativeToAbs(rectBox), lineThick, colorBorder);

    drawRectangle(rectSlider, colorSlider);
    DrawRectangleLinesEx(convertRectangleRelativeToAbs(rectSlider), lineThick, colorBorder);
}

void Raylib::Slider::setPos(Vector2 pos)
{
    const float dx = rectSlider.x - rectBox.x;

    rectBox.x = pos.x;
    rectBox.y = pos.y;
    rectSlider.x = pos.x + dx;
    rectSlider.y = pos.y;
}

void Raylib::Slider::setSize(Vector2 size)
{
    rectBox.width = size.x;
    rectBox.height = size.y;
    rectSlider.width = rectBox.width / (float)(max-min+1);
    rectSlider.height = size.y;
}