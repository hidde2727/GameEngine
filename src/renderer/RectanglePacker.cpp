#include "renderer/RectanglePacker.h"

namespace Engine {
namespace Renderer {

    void RectanglePacker::Pack() {
        ASSERT(_sizes.size() > 0, "[Renderer::RectanglePacker] Cannot pack rectangles without an input")
        // Give every rectangle his index
        for(uint32_t i = 0; i < _sizes.size(); i++) {
            _sizes[i].z = i;
            ASSERT(_sizes[i].x <= _maxBinSize.x && _sizes[i].y <= _maxBinSize.y, "[Renderer::RectanglePacker] Cannot pack rectangles that are bigger then the maximum bin size");
        }
        // Sort the rectangles
        switch(_sortingAlgorithm) {
            case SortingAlgorithm::SmallWidthFirst: std::sort(_sizes.begin(), _sizes.end(), [](const auto& first, const auto& second) { return first.x < second.x; });
            case SortingAlgorithm::BigWidthFirst: std::sort(_sizes.begin(), _sizes.end(), [](const auto& first, const auto& second) { return first.x > second.x; });
            case SortingAlgorithm::SmallHeightFirst: std::sort(_sizes.begin(), _sizes.end(), [](const auto& first, const auto& second) { return first.y < second.y; });
            case SortingAlgorithm::BigHeightFirst: std::sort(_sizes.begin(), _sizes.end(), [](const auto& first, const auto& second) { return first.y > second.y; });
            default: break;
        }
        // Pack them
        _result.resize(_sizes.size());
        switch(_packingAlgorithm) {
            case PackingAlgorithm::Shelf: PackShelf(); break;
            case PackingAlgorithm::Skyline: PackSkyline(); break;
            case PackingAlgorithm::MaxRects: PackMaxRects(); break;
            default: THROW("[Renderer::RectanglePacker] No packing algorithm specified for RectanglePacker");
        }
    }

    void RectanglePacker::PackShelf() {
        uint32_t rowHeight = _sizes[0].y;
        uint32_t x = 0;
        uint32_t y = 0;
        uint32_t binWidth = 0;
        uint32_t bin = 0;
        for(const auto rect : _sizes) {
            if(x + rect.x > _maxBinSize.x) {
                if(y + rowHeight + rect.y > _maxBinSize.y) {
                    _binSizes.push_back(Util::Vec2U32(binWidth, y+rowHeight));
                    // Start a new bin
                    y = 0;
                    x = 0;
                    rowHeight = rect.y;
                    bin++;
                } else {
                    // Start a new row
                    y += rowHeight;
                    rowHeight = rect.y;
                    x = 0;
                }
            }
            if(rect.y > rowHeight) rowHeight = rect.y;
            _result[rect.z] = ResultArea(bin, Util::AreaU32(x, y, rect.x, rect.y), rect.z);
            x += rect.x;
            if(x > binWidth) binWidth = x;
        }
        _binSizes.push_back(Util::Vec2U32(binWidth, y+rowHeight));
    }
    void RectanglePacker::PackSkyline() {
        THROW("[Renderer::RectanglePacker] Sorry, skyline packing is not implemented yet");
    }
    void RectanglePacker::PackMaxRects() {
        THROW("[Renderer::RectanglePacker] Sorry, maxrects packing is not implemented yet");
    }

}
}