#ifndef ENGINE_RENDERER_TEXTLOADER_H
#define ENGINE_RENDERER_TEXTLOADER_H

// MSDF generation base on https://github.com/Chlumsky/msdfgen/files/3050967/thesis.pdf

#include "core/PCH.h"
#include "renderer/TextureMap.h"
#include "renderer/ttf/FontParser.h"

#include "util/EquationSolvers.h"

namespace Engine {
namespace Renderer {

    #define ENGINE_RENDERER_FONTPROGRAM_MAXSIZE 64
    #define ENGINE_RENDERER_GENERAL_RENDERSIZE 32
    #define ENGINE_RENDERER_SDF_PADDING 2
    #define ENGINE_RENDERER_PX_RANGE_FACTOR 0.5f

    struct CharInfo {
        Util::AreaF _textureArea;
        uint32_t _boundTexture = UINT32_MAX;

        Util::Vec2F _min;
        Util::Vec2F _max;

        float _leftSideBearing;
        float _advance;
        std::map<char32_t, int16_t> _horizontalKerning;
    };
    typedef std::map<uint32_t, std::map<char32_t, CharInfo>> TextRenderInfo;

    class TextLoader : public AssetLoader {
    public:

        TextLoader(const std::string file, const Characters characters, const std::initializer_list<uint32_t> sizes);

        void Init() override;
        size_t GetAmountTextures() override;
        void SetTextureSizes(Util::Vec3U32* start) override;
        void RenderTexture(Util::AreaU8* texture, const Util::Vec2U32 textureSize, const Util::AreaU32 area, const size_t id) override;

        void SetTextureRenderInfo(const Util::AreaF area, const uint32_t boundTexture, const size_t id) override;
        std::shared_ptr<uint8_t> GetRenderInfo() override;

    private:
        struct Curve {
            Curve(const Util::Vec2D p3, const Util::Vec2D p2, const Util::Vec2D p1, const Util::Vec2D P0) : p3(p3), p2(p2), p1(p1), P0(P0) {}
            Curve() {}
            Util::Vec2D p3;
            Util::Vec2D p2;
            Util::Vec2D p1;
            Util::Vec2D P0;
        };
        struct TriCurve {
            TriCurve(const Curve prev, const Curve curve, const Curve next) : prev(prev), curve(curve), next(next) {}
            TriCurve() {}
            Curve prev;
            Curve curve;
            Curve next;
        };
            
        inline float Orthogonality(const Curve& curve, const Util::Vec2D p, const bool start);
        inline float Distance(const Curve prev, const Curve curve, const Curve next, const Util::Vec2D p, const bool pseudo);
        Util::AreaU8 CalculateSignedField(const float x, const float y);

        std::string _file;
        Characters _characters;
        std::vector<uint32_t> _sizes;
            
        std::vector<std::unique_ptr<TTFFontParser::FontData>> _fontData;
        std::vector<std::pair<Util::AreaF, uint32_t>> _textureAreas;
            
        std::vector<TriCurve> _renderingCurves;
        float _renderingMaxDistance;

    };

}
}

#endif