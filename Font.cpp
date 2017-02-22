//
// Created by PC on 2016/11/28.
//
#include <vector>
#include "Font.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "../../../ThirdParty/stb/stb_truetype.h"
#include "../Resources/Textures/ASingleTexture.h"
#include "../Resources/GPUResources.h"
#include "../Resources/CPUResources.h"


namespace sparrow {

    typedef  struct
    {
        uint32_t size;
        uint32_t textureWidth;
        uint32_t textureHeight;
        uint32_t oversampleX ;
        uint32_t oversampleY ;
        uint32_t firstChar;
        uint32_t charCount;
        stbtt_packedchar* charInfo;
        GLuint texture;
    } FontTexture;


    FontTexture mFontTextur = {40, 1024, 1024, 2, 2, ' ', '~' - ' ', 0, 0};
    stbtt_fontinfo mFontInfo;

    Font::Font():Drawable(){

        mTextBuffer = nullptr;
        mCamera2D = new Camera2D();
        mCamera2D->SetWidth(1000);
        mCamera2D->SetHeight(1000);
        mPixels = nullptr;
        InitFont("Fonts/arial.ttf");
        InitMaterial();
    }

    Font::~Font() {

        if(nullptr != mCamera2D)
            delete mCamera2D;
        if(nullptr != mTextBuffer)
            delete[] mTextBuffer;
        if(nullptr != mPixels)
            free(mPixels);
        if(nullptr != mFontTextur.charInfo)
            delete[] mFontTextur.charInfo;
    }

    Camera2D *Font::GetCamera() {
        return mCamera2D;
    }

    void Font::InitFont(string fontPath){

        void* fontBuffer = 0;
        File* file = new File(fontPath.c_str());
        std::shared_ptr<File> ptrImage(file);
        mPtrFile = ptrImage;
        FILE* fontFile = mPtrFile->Open("r");
        fseek(fontFile, 0, SEEK_END);
        int size = ftell(fontFile); /* how long is the file ? */
        fseek(fontFile, 0, SEEK_SET); /* reset */

        fontBuffer = malloc(size);

        fread(fontBuffer, size, 1, fontFile);
        fclose(fontFile);

        auto atlasData = (char*)malloc(mFontTextur.textureWidth * mFontTextur.textureHeight);

        //Note the bug
        //mFontTextur.charInfo = (stbtt_packedchar*)malloc(mFontTextur.charCount * 1000);
        mFontTextur.charInfo = new stbtt_packedchar[mFontTextur.charCount];


        stbtt_pack_context context;
        if (!stbtt_PackBegin(&context, (unsigned char*)atlasData, mFontTextur.textureWidth, mFontTextur.textureHeight, 0, 1, nullptr))
            return;

        stbtt_PackSetOversampling(&context, mFontTextur.oversampleX, mFontTextur.oversampleY);
        if (!stbtt_PackFontRange(&context, (unsigned char*)fontBuffer, 0, mFontTextur.size, mFontTextur.firstChar, mFontTextur.charCount, mFontTextur.charInfo))
            return;

        stbtt_PackEnd(&context);

        mPixels = malloc(mFontTextur.textureWidth * mFontTextur.textureHeight * 3);

        for(int i = 0; i < mFontTextur.textureWidth * mFontTextur.textureHeight; i++){
            ((char*)mPixels)[i*3] = ((char*)atlasData)[i];
            ((char*)mPixels)[i*3 + 1] = 0.f;
            ((char*)mPixels)[i*3 + 2] = 0.f;
        }

        free(fontBuffer);
        free(atlasData);
    }

    GlyphInfo Font::GetGlyphInfo(uint32_t character, float offsetX, float offsetY){
        stbtt_aligned_quad quad;

        stbtt_GetPackedQuad(mFontTextur.charInfo, mFontTextur.textureWidth, mFontTextur.textureHeight, character - mFontTextur.firstChar, &offsetX, &offsetY, &quad, 1);
        auto xmin = quad.x0;
        auto xmax = quad.x1;
        auto ymin = -quad.y1;
        auto ymax = -quad.y0;

        auto info = GlyphInfo();
        info.offsetX = offsetX;
        info.offsetY = offsetY;
        info.positions[0] = { xmin, ymin, 0 };
        info.positions[1] = { xmin, ymax, 0 };
        info.positions[2] = { xmax, ymax, 0 };
        info.positions[3] = { xmax, ymin, 0 };
        info.uvs[0] = { quad.s0, quad.t1 };
        info.uvs[1] = { quad.s0, quad.t0 };
        info.uvs[2] = { quad.s1, quad.t0 };
        info.uvs[3] = { quad.s1, quad.t1 };

        return info;
    }

    void Font::TextToInfo(string text){

        std::vector<glm::vec3> vertices;
        std::vector<glm::vec2> uvs;
        std::vector<u_short> indexes;

        uint16_t lastIndex = 0;
        float offsetX = 0, offsetY = 0;
        bool b = 1;
        for (auto c : text)
        {
            auto glyphInfo = GetGlyphInfo(c, offsetX, offsetY);
            offsetX = glyphInfo.offsetX;
            offsetY = glyphInfo.offsetY;

            vertices.emplace_back(glyphInfo.positions[0]);
            vertices.emplace_back(glyphInfo.positions[1]);
            vertices.emplace_back(glyphInfo.positions[2]);
            vertices.emplace_back(glyphInfo.positions[3]);
            uvs.emplace_back(glyphInfo.uvs[0]);
            uvs.emplace_back(glyphInfo.uvs[1]);
            uvs.emplace_back(glyphInfo.uvs[2]);
            uvs.emplace_back(glyphInfo.uvs[3]);
            indexes.push_back(lastIndex);
            indexes.push_back(lastIndex + 1);
            indexes.push_back(lastIndex + 2);
            indexes.push_back(lastIndex);
            indexes.push_back(lastIndex + 2);
            indexes.push_back(lastIndex + 3);

            lastIndex += 4;
        }

        this->SetData((float*)&vertices[0],3 * vertices.size(),GL_STATIC_DRAW,
                      (u_short*)&indexes[0], indexes.size(),GL_STATIC_DRAW,
                      (float*)&uvs[0],2 * uvs.size(),GL_STATIC_DRAW,
                       nullptr,0,GL_STATIC_DRAW);

        vertices.clear();
        uvs.clear();
        indexes.clear();
    }




    void Font::RenderText(string text){

        static int i = 0;
        if(0 == i){
            //TextTexture();   //why cannot work to call it in constructor function
            i++;
        }
        TextToInfo(text);

        glm::mat4 scaleMat = glm::scale(glm::mat4(), glm::vec3(3, 3, 3));
        glm::mat4 translateMat = glm::translate(glm::mat4(), glm::vec3(-60, 0, 0));
        glm::mat4 rotate = glm::scale(glm::mat4(), glm::vec3(1, 1, 1));
        glm::mat4 parentMat = translateMat * rotate * scaleMat;
        //glm::mat4 parentMat = rotate * scaleMat;

        glm::mat4 vMatrix = mCamera2D->GetViewMatrix();
        glm::mat4 pMatrix = mCamera2D->GetProjectionMatrix();
        glm::mat4 vpMatrix = mCamera2D->GetProjectionMatrix() * mCamera2D->GetViewMatrix();
        this->OnComputeModelMatrix(&parentMat);
        glm::mat4 MMatrix = this->GetModelMatrix();
        mMVPMatrix = mCamera2D->GetProjectionMatrix() * mCamera2D->GetViewMatrix()  * MMatrix ;

        glDisable(GL_CULL_FACE);
        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

       // this->Show(*mCamera2D,vpMatrix, pMatrix, vMatrix, &parentMat);


    }

    bool Font::InitMaterial() {

        MaterialResources *materialRes = &MaterialResources::GetInstance();
        CPUResources *resCache = &CPUResources::GetInstance();
        Material *material_font = materialRes->CreateMaterial("FontMaterial");
        VertexShader *vertShader = resCache->CreateResource<VertexShader>("Shaders/font_texture.vert");
        FragmentShader *fragShader = resCache->CreateResource<FragmentShader>("Shaders/font_texture.frag");
        material_font->SetVertexShader(vertShader);
        material_font->SetFragmentShader(fragShader);
        material_font->UseVertexColors(false);
        material_font->AddTexture(new Texture("sparrow", mFontTextur.textureWidth, mFontTextur.textureHeight, GL_RGB, mPixels));
        //Image *image = resCache->CreateResource<Image>("Textures/test.jpg");
        //material_font->AddTexture(new Texture("sparrow", image));

        this->SetMaterial(material_font);

        return true;
    }

    void Font::LoadFontFile(string fontPath){
        InitFont(fontPath);
    }

}

