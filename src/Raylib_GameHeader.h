#pragma once
#include "raylib.h"
#include "Random_Header.h"
#include "Timer_Header.h"
#include <iostream>
#include <vector>

namespace Raylib
{
    inline Timer CLOCK{};
    inline const double TSTART = CLOCK.getTime();
    typedef uint64_t u64;

    //Position with screen-coordinates
    struct screenPos { int x{},y{}; };

    //New Colours

    #define DARKERBLUE  CLITERAL(Color){ 0,82-50,172-50,255 }
    #define MIDDLEBROWN CLITERAL(Color){ 102,85,63,255 }

    inline Color ColorBackground{RAYWHITE};

    //Main Windowclass
    class RayWindow
    {
    public:
        void load();

        bool isLoaded() const
        {
            return loaded;
        }

        void unload();

        ~RayWindow();

        //ScreenSize as int
        int size = 120;                 //Base-unit of the Windowsize in Pixels.
        const int widthStart = 10*size; //10 times the Windowsize: 8 for the board, 2 for the UI.
        int width = widthStart;
        const int heightStart = 8*size; //8 times the Windowsize for the board.
        int height = heightStart;

        //ScreenSize as float
        float sizef = (float)size;
        float widthf = (float)width;
        float heightf = (float)height;

        bool active{false};

    private:
        bool loaded{false};
        Image windowIcon{};
    };
    inline RayWindow Window{};

    //Adjust Window-Size
    void updateWindowSize();

    //Drawing with relative Coordinates: (0,0) is upper left corner, (1,1) is bottom right corner.

    //Takes a Vector2 using relative Coordinates and returnns the corresponding Vector2 in absolute Coordinates.
    Vector2 convertVector2RelativeToAbs(Vector2 xRelative);

    //Takes a Rectangle using relative Coordinates and returns the corresponding Rectangle in absolute Coordinates.
    Rectangle convertRectangleRelativeToAbs(Rectangle rectRelative);

    //Draw a Rectangle using relative Coordinates.
    void drawRectangle(Rectangle rect = {.x=0.375f, .y=0.375f, .width=0.25f, .height=0.25f}, Color color = RED, float rotation = 0.0f, Vector2 rotationOrigin = {0.0f,0.0f});

    //Draw a Line using relative Coordinates.
    void drawLine(Vector2 startpoint, Vector2 endpoint, float thickness = 5.0f, Color color = BLACK);

    //Draw some Text using relative Coordinates.
    void drawText(const std::string &text, Vector2 position = {0.0f,0.0f}, float fontSize = 1.0f, Color tint = WHITE, float spacing = 0.1f, Font font = GetFontDefault() , float rotation = 0.0f, Vector2 rotationOrigin = {0.0f,0.0f} );

    //Get the size of the Text in relative Coordinates
    Vector2 getTextSize(const std::string &text, float fontSize, float spacing, Font font = GetFontDefault());

    //Draw a Circle
    void drawCircle(Vector2 pos = {0.5f,0.5f}, Vector2 radius = {0.25f,0.25f}, Color color = WHITE, Color edgeColor = BLACK);

    //Generic Classes

    class RaylibTexture
    {
    public:
        RaylibTexture(const std::string& name,const Vector2 &size = {Window.widthf/10.0f,Window.heightf/10.0f}): SpriteName{"Sprites\\" + name}
        {
            Size = {0.0f,0.0f,size.x,size.y};
            originPoint = {0.5f*size.x, 0.5f*size.y};
        }

        void load();

        void getTextureSize(Vector2 &textureSize) const;

        void setPosition(const Vector2& position);

        void setSize(const Vector2 &size);

        void setSpritePosition(const Vector2 &position);

        void setSpriteSize(const Vector2 &size);

        void flipHorizontal();

        void flipVertical();

        void draw(float rotation = 0.0f, Color color = WHITE) const;

        Texture2D getTexture() const
        {
            return Texture;
        }

        void unload();

        ~RaylibTexture()
        {
            unload();
        }

    private:
        bool loaded{false},spritesheet{false};
        const std::string SpriteName;
        Texture2D Texture{};
        Rectangle Size{},SpriteSize{};
        Vector2 originPoint{};
    };

    class Button
    {
    public:
        Button(const Vector2 &pos = {0.0f,0.0f},const Vector2 &s = {(float)(Window.width)/10.0f,(float)(Window.height)/10.0f},const char* txt = nullptr,const char* texture = nullptr):
            x_draw{pos},size{s}
        {
            if(txt != nullptr)
                text = txt;
        }

        void initialize(const Vector2 &pos,const Vector2 &s = {(float)(Window.width)/10.0f,(float)(Window.height)/10.0f},const char* txt = nullptr);

        void move(const Vector2 &newpos);

        void drawButtonText(Color colorText) const;

        void draw(Color colorOn = GRAY,Color colorOff = LIGHTGRAY,Color colorText = WHITE,float rotation = 0.0f) const;

        void drawTextured(RaylibTexture &texture,Color colorOn = GRAY,Color colorOff = LIGHTGRAY,Color colorText = WHITE,float rotation = 0.0f) const;

        Vector2 getPos() const { return x_draw; }
        Vector2 getSize() const { return size; }

        virtual void checkInput();

        bool pressed{false},pressedOld{false},selected{false},textInsideButton = false, drawCircle = false;
        float fontSize{15.0f};

    protected:
        Vector2 x_draw,size;
        std::string text{};

        bool checkMouseCollision() const;
    };

    class SwitchButton: public Button
    {
    public:
        SwitchButton(const Vector2 &pos = {0.0f,0.0f},const Vector2 &s = {(float)(Window.width)/10.0f,(float)(Window.height)/10.0f},const char* txt = nullptr):
            Button(pos,s,txt)
        {}

        void checkInput() override;
    };

    class SpeechBubble
    {
    public:
        SpeechBubble(const std::string &t = "Hello!",float s = 10.0f,const Vector2 &p = {0.0f,0.0f}):
            text{t},fontsize{s},pos{p},texture{"SpeechBubble.png"}
        {}

        void load();

        void changeText(const std::string &t);

        void changeFontSize(float s);

        void move(const Vector2 &p);

        void moveTail(Vector2 p);

        void draw();

        void flip();

        Vector2 getSpeechPos() const
        {
            return speechpos;
        }

        Vector2 getBubbleSize() const
        {
            return size;
        }

        Vector2 getBubblePos() const
        {
            return pos;
        }

        void unload();

        ~SpeechBubble()
        {
            unload();
        }

        bool fliped = false;

    private:
        bool loaded{false};
        float fontsize,bubbleheight{};
        const float border{10.0f/Window.widthf},spacing{0.1f};
        std::string text;
        Vector2 pos,textpos{},speechpos{},size{},textsize{};
        RaylibTexture texture;

        void adjustBubbleSize();
    };

    class Slider
    {
    public:
        Slider(int min, int max, Rectangle rectBox = {0.375f,0.25f,0.25f,0.05f}): 
            min{min}, max{max}, value{(max-min)/2+1}, rectBox{rectBox}
        {            
            const float dx = (float)(max-min+1);

            rectSlider.width = rectBox.width / dx;
            rectSlider.height = rectBox.height;

            rectSlider.x = rectBox.x + (float)(value-1)*rectSlider.width;
            rectSlider.y = rectBox.y;
        }

        void input();

        void draw() const;

        Vector2 getPos() const
        {
            return {rectBox.x,rectBox.y};
        }

        Vector2 getSize() const
        {
            return {rectBox.width,rectBox.height};
        }

        void setPos(Vector2 pos);

        void setSize(Vector2 size);

        Color colorBox = DARKGRAY, colorSlider = GRAY, colorBorder = BLACK;

        const int min, max;
        int value;

    private:

        bool testMouseSliderCollision() const;

        void convertMouseXtoSliderX();

        bool selected = false;
        Rectangle rectBox, rectSlider;
    };

    //"Level" that can be loaded and drawn and accepts input.
    class Level;
    inline std::vector<Level*> LevelList{};

    inline std::size_t LevelID{};

    class Level
    {
    public:
        Level()
        {
            LevelList.push_back(this);
            id = LevelList.size() - 1;
        }

        virtual void load(){}

        std::size_t ID() const
        {
            return id;
        }

        bool isLoaded() const
        {
            return loaded;
        }

        virtual void input(){}

        virtual void draw(){}

        virtual void unload(){}

        virtual ~Level()
        {
            unload();
        }

    protected:
        bool loaded{false};
        std::size_t id{};
    };
}