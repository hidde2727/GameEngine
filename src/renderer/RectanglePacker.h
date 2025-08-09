#ifndef ENGINE_RENDERER_RECTANGLE_PACKER_H
#define ENGINE_RENDERER_RECTANGLE_PACKER_H

#include "core/PCH.h"
#include "util/Vec2D.h"
#include "util/Vec3D.h"
#include "util/Area.h"

namespace Engine {
namespace Renderer {

    class RectanglePacker {
    public:

        void SetAmountRectangles(const size_t size) { _sizes.resize(size); }
        Util::Vec3U32* GetRectangleInputPtr() { return _sizes.data(); }
        size_t GetAmountInputRectangles() { return _sizes.size(); }

        enum class SortingAlgorithm {
            None,
            SmallWidthFirst,
            BigWidthFirst,
            SmallHeightFirst,
            BigHeightFirst
        };
        void SetSortingAlgorithm(const SortingAlgorithm sortingAlgorithm) { _sortingAlgorithm = sortingAlgorithm; }
        enum class PackingAlgorithm {
            Shelf,
            Skyline,
            MaxRects
        };
        void SetPackingAlgorithm(const PackingAlgorithm packingAlgorithm) { _packingAlgorithm = packingAlgorithm; }
        void SetMaximumBinSize(const Util::Vec2U32 size) { _maxBinSize = size; }

        // Garantuees that the rectangles in the result are in the same order as in the input
        void Pack();

        std::pair<uint32_t, Util::AreaU32>* GetResults() { return _result.data(); }
        size_t GetAmountResults() { return _result.size(); }
        Util::Vec2U32* GetBinSizes() { return _binSizes.data(); }
        size_t GetAmountBins() { return _binSizes.size(); }

    private:

        void PackShelf();
        void PackSkyline();
        void PackMaxRects();
        
        std::vector<Util::Vec3U32> _sizes; // width, height, index at time of insertion by user
        SortingAlgorithm _sortingAlgorithm = SortingAlgorithm::None;
        PackingAlgorithm _packingAlgorithm = PackingAlgorithm::Skyline;
        Util::Vec2U32 _maxBinSize = Util::Vec2U32(1024, 1024);

        // _first=binID, _second=area in that bin
        // x and y of the top-left corner
        std::vector<std::pair<uint32_t, Util::AreaU32>> _result;
        std::vector<Util::Vec2U32> _binSizes;

    };

}
}

#endif