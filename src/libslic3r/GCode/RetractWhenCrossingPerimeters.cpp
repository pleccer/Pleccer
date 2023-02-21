#include "../ClipperUtils.hpp"
#include "../Layer.hpp"
#include "../Polyline.hpp"

#include "RetractWhenCrossingPerimeters.hpp"

namespace Slic3r {

bool RetractWhenCrossingPerimeters::travel_inside_internal_regions(const Layer &layer, const Polyline &travel)
{
    if (m_layer != &layer) {
        // Update cache.
        m_layer = &layer;
        m_internal_islands.clear();
        m_aabbtree_internal_islands.clear();
        // Collect expolygons of internal slices.
        for (const LayerRegion *layerm : layer.regions())
            for (const Surface &surface : layerm->slices().surfaces)
                if (surface.is_internal())
                    m_internal_islands.emplace_back(&surface.expolygon);
        // Calculate bounding boxes of internal slices.
        class BBoxWrapper {
        public:
            BBoxWrapper(const size_t idx, const BoundingBox &bbox) : 
                m_idx(idx),
                // Inflate the bounding box a bit to account for numerical issues.
                m_bbox(bbox.min - Point(SCALED_EPSILON, SCALED_EPSILON), bbox.max + Point(SCALED_EPSILON, SCALED_EPSILON)) {}
            size_t                       idx() const { return m_idx; }
            const AABBTree::BoundingBox& bbox() const { return m_bbox; }
            Point                        centroid() const { return ((m_bbox.min().cast<int64_t>() + m_bbox.max().cast<int64_t>()) / 2).cast<int32_t>(); }
        private:
            size_t                m_idx;
            AABBTree::BoundingBox m_bbox;
        };
        std::vector<BBoxWrapper> bboxes;
        bboxes.reserve(m_internal_islands.size());
        for (size_t i = 0; i < m_internal_islands.size(); ++ i)
            bboxes.emplace_back(i, get_extents(*m_internal_islands[i]));
        // Build AABB tree over bounding boxes of internal slices.
        m_aabbtree_internal_islands.build_modify_input(bboxes);
    }

    BoundingBox           bbox_travel = get_extents(travel);
    AABBTree::BoundingBox bbox_travel_eigen{ bbox_travel.min, bbox_travel.max };
    int result = -1;
    bbox_travel.offset(SCALED_EPSILON);
    AABBTreeIndirect::traverse(m_aabbtree_internal_islands, 
        [&bbox_travel_eigen](const AABBTree::Node &node) {
            return bbox_travel_eigen.intersects(node.bbox);
        },
        [&travel, &bbox_travel, &result, &islands = m_internal_islands](const AABBTree::Node &node) {
            assert(node.is_leaf());
            assert(node.is_valid());
            Polygons clipped = ClipperUtils::clip_clipper_polygons_with_subject_bbox(*islands[node.idx], bbox_travel);
            if (diff_pl(travel, clipped).empty()) {
                // Travel path is completely inside an "internal" island. Don't retract.
                result = int(node.idx);
                // Stop traversal.
                return false;
            }
            // Continue traversal.
            return true;
        });
    return result != -1;
}

} // namespace Slic3r
