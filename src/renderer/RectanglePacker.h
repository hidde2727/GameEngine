#ifndef ENGINE_RENDERER_RECTANGLE_PACKER_H
#define ENGINE_RENDERER_RECTANGLE_PACKER_H

#include "core/PCH.h"

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

        struct ResultArea {
            ResultArea() {}
            ResultArea(uint32_t bin, Util::AreaU32 area, uint32_t origID) {
                _bin = bin;
                _area = area;
                _origID = origID;
            }
            uint32_t _bin = 0;
            Util::AreaU32 _area;// x and y of the top left corner
            uint32_t _origID = 0;// place in the input vector
        };
        ResultArea* GetResults() { return _result.data(); }
        size_t GetAmountResults() { return _result.size(); }
        Util::Vec2U32* GetBinSizes() { return _binSizes.data(); }
        size_t GetAmountBins() { return _binSizes.size(); }

    private:

        void PackShelf();
        void PackSkyline();
        void PackMaxRects();
        
        std::vector<Util::Vec3U32> _sizes; // x=width, y=height, z=index at time of insertion by user
        SortingAlgorithm _sortingAlgorithm = SortingAlgorithm::None;
        PackingAlgorithm _packingAlgorithm = PackingAlgorithm::Skyline;
        Util::Vec2U32 _maxBinSize = Util::Vec2U32(1024, 1024);

        std::vector<ResultArea> _result;
        std::vector<Util::Vec2U32> _binSizes;

    };

}
}

#endif