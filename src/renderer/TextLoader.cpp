#include "renderer/TextLoader.h"

#define FLOAT_MAX std::numeric_limits<float>::max()

namespace Engine {
namespace Renderer {
    
    TextLoader::TextLoader(const std::string file, const Characters characters, const std::initializer_list<uint32_t> sizes) {
        _file = file;
        _characters = characters;
        _sizes = sizes;
        // Should remove all the duplicates of size above ENGINE_RENDERER_FONTPROGRAM_MAXSIZE
    }

    void TextLoader::Init() {
        ASSERT(Util::FileManager::DoesFileExists(_file), "[Renderer::TextLoader] TextLoader received a file that does not exist '" + _file + "'")
        ASSERT(Util::FileManager::IsFileRegular(_file), "[Renderer::TextLoader] TextLoader received an invalid file '" + _file + "'")
    }
    size_t TextLoader::GetAmountTextures() {
        return _characters._characterCount * _sizes.size();
    }
    void TextLoader::SetTextureSizes(Util::Vec3U32* start) {
        TTFFontParser fontParser = TTFFontParser(Util::FileManager::GetFileLocation(_file), _characters);
        size_t i = 0;
        _fontData.resize(_sizes.size());
        _textureAreas.resize(_sizes.size()*_characters._characterCount);
        for(const uint32_t fontSize : _sizes) {
            if(fontSize < ENGINE_RENDERER_FONTPROGRAM_MAXSIZE)
                _fontData[i] = fontParser.GetScaledData(fontSize);
            else
                _fontData[i] = fontParser.GetNonScaledData();
            
            float scalingFactor = fontSize<ENGINE_RENDERER_FONTPROGRAM_MAXSIZE ? 1.f : (float)fontParser.GetScale(ENGINE_RENDERER_GENERAL_RENDERSIZE);
            for(const char32_t c : _characters) {
                if(_fontData[i]->_glyphs[c]._contours.size() == 0) { start++; continue; }
                Util::Vec2F size = (_fontData[i]->_glyphs[c]._max - _fontData[i]->_glyphs[c]._min) * scalingFactor;
                *start = Util::Vec3U32((uint32_t)std::ceil(size.x+2*ENGINE_RENDERER_SDF_PADDING), (uint32_t)std::ceil(size.y+2*ENGINE_RENDERER_SDF_PADDING), 0);
                start++;
            }
            i++;
        }
    }
    void TextLoader::RenderTexture(Util::AreaU8* texture, const Util::Vec2U32 textureSize, const Util::AreaU32 area, const size_t id) {
        const uint32_t fontID = (uint32_t)std::floor(id / _characters._characterCount);
        const uint32_t fontSize = _sizes[(size_t)fontID];
        TTFFontParser::FontData* fontData = _fontData[(size_t)std::floor(id / _characters._characterCount)].get();
        const char32_t c = _characters[(uint32_t)id - fontID*(uint32_t)_characters._characterCount];
        
        size_t amountCurves = 0;
        for(const auto& contour : fontData->_glyphs[c]._contours) { amountCurves += (size_t)contour._contourLength; }
        _renderingCurves.resize(amountCurves);
        if(amountCurves == 0) return;

        size_t renderCurve = 0;
        for(const auto& contour : fontData->_glyphs[c]._contours) {
            for(size_t i = contour._contourStart; i < contour._contourStart + contour._contourLength; i++) {
                const TTFFontParser::BezierCurve prev = fontData->_curves[i==contour._contourStart? contour._contourStart+contour._contourLength-1 : i-1];
                const TTFFontParser::BezierCurve curve = fontData->_curves[i];
                const TTFFontParser::BezierCurve next = fontData->_curves[i==contour._contourStart+contour._contourLength-1? contour._contourStart : i+1];
                // TODO: Contour offset and scaling
                _renderingCurves[renderCurve] = TriCurve(
                    Curve(
                        prev.degree<4? FLOAT_MAX : (prev.p4 - prev.p3*3 + prev.p2*3 - prev.p1),   
                        prev.degree<3? FLOAT_MAX : (prev.p3 - prev.p2*2 + prev.p1),   
                        prev.p2 - prev.p1,
                        prev.p1
                    ),
                    Curve(
                        curve.degree<4? FLOAT_MAX : (curve.p4 - curve.p3*3 + curve.p2*3 - curve.p1),   
                        curve.degree<3? FLOAT_MAX : (curve.p3 - curve.p2*2 + curve.p1),   
                        curve.p2 - curve.p1,
                        curve.p1
                    ),
                    Curve(
                        next.degree<4? FLOAT_MAX : (next.p4 - next.p3*3 + next.p2*3 - next.p1),   
                        next.degree<3? FLOAT_MAX : (next.p3 - next.p2*2 + next.p1),   
                        next.p2 - next.p1,
                        next.p1
                    )
                );
                renderCurve++;
            }
        }
        /*
        if(id == 'x'-'a') {
            int i = 0;
            for(TriCurve& c : _renderingCurves) {
                Curve& curve = c.curve;
                if(curve.p2.x == FLOAT_MAX && curve.p2.y == FLOAT_MAX) {
                    std::cout << "B" << i << "=KROMME(t*("<<curve.p1.x<<","<<curve.p1.y<<")+("<<curve.P0.x<<","<<curve.P0.y<<"), t, 0, 1)\n";
                } else if(curve.p3.x == FLOAT_MAX && curve.p3.y == FLOAT_MAX) {
                    std::cout << "B" << i << "=KROMME(t^2*("<<curve.p2.x<<","<<curve.p2.y<<")+2*t*("<<curve.p1.x<<","<<curve.p1.y<<")+("<<curve.P0.x<<","<<curve.P0.y<<"), t, 0, 1)\n";                    
                } else {
                    std::cout << "B" << i << "=KROMME(t^3*(" << curve.p3.x << ","<<curve.p3.y<<")+3*t^2*("<<curve.p2.x<<","<<curve.p2.y<<")+3*t*("<<curve.p1.x<<","<<curve.p1.y<<")+("<<curve.P0.x<<","<<curve.P0.y<<"), t, 0, 1)\n";
                }
                i++;
            }
        }*/

        _renderingMaxDistance = ENGINE_RENDERER_PX_RANGE_FACTOR*(float)sqrt(area.w*area.w + area.h*area.h);
        Util::Vec2F min = _fontData[fontID]->_glyphs[c]._min;
        Util::Vec2F max = _fontData[fontID]->_glyphs[c]._max;

        for(uint32_t y = 0; y < area.h; y++) {
            Util::AreaU8* row = texture + (area.y+y)*textureSize.x + area.x;
            for(uint32_t x = 0; x < area.w; x++) {
                *(row+x) = CalculateSignedField(x+0.5f+min.x-ENGINE_RENDERER_SDF_PADDING, max.y-(y+0.5f-ENGINE_RENDERER_SDF_PADDING));
            }
        }
        return;
    }

    void TextLoader::SetTextureRenderInfo(const Util::AreaF area, const uint32_t boundTexture, const size_t id) {
        _textureAreas[id] = std::pair<Util::AreaF, uint32_t>(area, boundTexture);
    }
    std::shared_ptr<uint8_t> TextLoader::GetRenderInfo() {
        std::shared_ptr<TextRenderInfo> ret = std::make_shared<TextRenderInfo>();

        uint32_t id = 0;
        uint32_t fontDataID = 0;
        for(const uint32_t size : _sizes) {
            TTFFontParser::FontData* fontData = _fontData[fontDataID].get();
            ret->insert(std::pair(size, std::map<char32_t, CharInfo>()));
            for(const char32_t c : _characters) {
                CharInfo* info = &ret->at(size)[c];
                if(_textureAreas[id].first != Util::AreaF(0)) {
                    info->_textureArea = _textureAreas[id].first;
                    info->_boundTexture = _textureAreas[id].second;
                }
                info->_min = fontData->_glyphs[c]._min;
                info->_max = fontData->_glyphs[c]._max;
                info->_advance = fontData->_glyphs[c]._advance;
                info->_leftSideBearing = fontData->_glyphs[c]._leftSideBearing;
                info->_horizontalKerning = fontData->_glyphs[c]._horizontalKerning;

                id++;
            }
            fontDataID++;
        }
                
        return std::reinterpret_pointer_cast<uint8_t>(ret);
    }

    float TextLoader::Orthogonality(const Curve& curve, const Util::Vec2D p, const bool start) {
        if(curve.p2.x == FLOAT_MAX && curve.p2.y == FLOAT_MAX) {
            // First degree
            Util::Vec2D derivitive = curve.p1;
            Util::Vec2D onCurve = curve.P0;
            if(!start) onCurve = curve.p1 + curve.P0;
            return (float)abs(derivitive.normalized().cross((p-onCurve).normalized()));
        }
        else if(curve.p3.x == FLOAT_MAX && curve.p3.y == FLOAT_MAX) {
            // Second degree
            Util::Vec2D derivitive = curve.p1;
            if(!start) derivitive = curve.p2*2 + curve.p1*2;
            Util::Vec2D onCurve = curve.P0;
            if(!start) onCurve = curve.p2 + curve.p1*2 + curve.P0;
            return (float)abs(derivitive.normalized().cross((p-onCurve).normalized()));
        }
        else {
            // Third degree
            Util::Vec2D derivitive = curve.p1;
            if(!start) derivitive = curve.p3*3+curve.p2*6+curve.p1*3;
            Util::Vec2D onCurve = curve.P0;
            if(!start) onCurve = curve.p3 + curve.p2*3 + curve.p1*3 + curve.P0;
            return (float)abs(derivitive.normalized().cross((p-onCurve).normalized()));
        }
    }

    #define SIGN(val) ((val)>0? 1.f : -1.f)
    float TextLoader::Distance(const Curve prev, const Curve curve, const Curve next, const Util::Vec2D p, const bool pseudo) {
        double distance = FLOAT_MAX;
        double t = FLOAT_MAX;
        double sign;
        if(curve.p2.x == FLOAT_MAX && curve.p2.y == FLOAT_MAX) {
            // First degree
            t = ((p - curve.P0)*curve.p1)/(curve.p1*curve.p1);
            if(!pseudo) t = std::clamp(t, 0., 1.);

            Util::Vec2D BMinusP =  curve.p1*t + curve.P0 - p;// p1*t + P0 - p
            distance = BMinusP.length();
            sign = SIGN(curve.p1.cross(BMinusP));// (p1)*BMinusP
        }
        else if(curve.p3.x == FLOAT_MAX && curve.p3.y == FLOAT_MAX) {
            // Second degree
            double solutions[3];
            const int amountSolutions = Util::SolveCubic(solutions, 
                (curve.p2*curve.p2)                             / 133296128.,// t^3
                (curve.p1*curve.p2*3)                           / 133296128.,// t^2
                (curve.p1*curve.p1*2-curve.p2*(p - curve.P0))   / 133296128.,// t
                ((p - curve.P0)*curve.p1*-1)                    / 133296128.
            );
            for(int i = 0; i < amountSolutions; i++) {
                if(!pseudo) solutions[i] = std::clamp(solutions[i], 0., 1.);
                Util::Vec2D BMinusP = (curve.p2*solutions[i]+curve.p1*2)*solutions[i]+curve.P0 - p;// p2*t^2 + 2*p1*t + P0 - p
                double distanceLocal = BMinusP.length();
                if(distanceLocal < distance) {
                    distance = distanceLocal;
                    sign = SIGN((curve.p2*solutions[i]*2+curve.p1*2).cross(BMinusP));// (2*p2*t + 2*p1)*BMinusP
                    t = solutions[i];
                }
            }
        }
        else {
            // Third degree
            double solutions[5];
            const double a = curve.p3*curve.p3;
            const double b = curve.p2*curve.p3*5;
            const double c = curve.p1*curve.p3*4+curve.p2*curve.p2*6;
            const double d = curve.p1*curve.p2*9+curve.p3*(curve.P0-p);
            const double e = curve.p1*curve.p1*3+curve.p2*(curve.P0-p)*2;
            const double f = curve.p1*(curve.P0-p);
            const int amountSolutions = Util::SolveQuintic(solutions,
                (curve.p3*curve.p3) 	                        / 133296128.,// t^5
                (curve.p2*curve.p3*5)                           / 133296128.,// t^4
                (curve.p1*curve.p3*4+curve.p2*curve.p2*6)       / 133296128.,// t^3
                (curve.p1*curve.p2*9+curve.p3*(curve.P0-p))     / 133296128.,// t^2
                (curve.p1*curve.p1*3+curve.p2*(curve.P0-p)*2)   / 133296128.,// t
                (curve.p1*(curve.P0-p))                         / 133296128.
            );
            for(int i = 0; i < amountSolutions; i++) {
                if(!pseudo) solutions[i] = std::clamp(solutions[i], 0., 1.);
                Util::Vec2D BMinusP = ((curve.p3*solutions[i]+curve.p2*3)*solutions[i]+curve.p1*3)*solutions[i]+curve.P0 - p;// p3*t^3 + 3*p2*t^2 + 3*p1*t + P0 - p
                double distanceLocal = BMinusP.length();
                if(distanceLocal < distance) {
                    distance = distanceLocal;
                    sign = SIGN(((curve.p3*3*solutions[i]+curve.p2*6)*solutions[i]+curve.p1*3).cross(BMinusP));// (3*p3*t^2 + 6*p2*t + 3*p1)*BMinusP
                    t = solutions[i];
                }
            }
        }

        if(t >= 0.999) {
            if(Orthogonality(curve, p, false) < Orthogonality(next, p, true)) return FLOAT_MAX;
        } else if(t <= 0.001) {
            if(Orthogonality(curve, p, true) < Orthogonality(prev, p, false)) return FLOAT_MAX;
        }
        return (float)distance*(float)sign;
    }
    Util::AreaU8 TextLoader::CalculateSignedField(const float x, const float y) {
        float lowestDistance = FLOAT_MAX;
        for(const TriCurve& curve : _renderingCurves) {
            const float distance = Distance(curve.prev, curve.curve, curve.next, Util::Vec2F(x, y), false);
            if(std::abs(distance) < std::abs(lowestDistance)) lowestDistance = distance;
        }
        float color = std::clamp(((lowestDistance/_renderingMaxDistance)+0.5f), 0.f, 1.f);
        color = pow(color, 1.0f / 2.2f)*255.f;// UNORM -> SRGB
        return Util::AreaU8((uint8_t)color, (uint8_t)color, (uint8_t)color, 255);
    }

}
}