//
// Created by PC on 2016/11/28 by YanTingchao.
// Use stb_truetype.h & stb_easy_font.h to complete the work
// Usage: Render text to terminal
//
// Todo:
//      add Chinese font ttf
//      crashproof on bad data
//      optimize:efficiency,storage,more operating function
//      optimize:storage
//      optimize:more operating function
//
// Version History
//      1.00(2016-12-03) complete the basical functions
//
// Function Details
//      Create text box to render
//          InitFont();                 ---initialize ttf for box
//          TextToInfo();               ---generate the vertices & uv & indices of text
//          RenderText();               ---render text


#ifndef VRAPP_AFONTBASE_H
#define VRAPP_AFONTBASE_H

#include <GLES2/gl2.h>
#include <string>
#include "Camera2D.h"
#include "../IO/File.h"
#include "../Resources/Textures/Image.h"
#include "Drawable.h"



using namespace std;

namespace sparrow{

    typedef  struct {
        glm::vec3 positions[4];
        glm::vec2 uvs[4];
        float offsetX, offsetY;
    }GlyphInfo;

    class Font : public Drawable{
    public:
        Font();
        ~Font();

        Camera2D* GetCamera();

        bool InitMaterial();
        void LoadFontFile(string fontPath);

        void InitFont(string fontPath);
        void TextToInfo(string text);
        void RenderText(string text);

    private:
        GlyphInfo GetGlyphInfo(uint32_t character, float offsetX, float offsetY);

    private:
        std::shared_ptr<File> mPtrFile;
        void* mPixels;
        char* mTextBuffer;

        Camera2D* mCamera2D;
    };
}

#endif //VRAPP_AFONTBASE_H
