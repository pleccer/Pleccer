#ifndef slic3r_Layer_hpp_
#define slic3r_Layer_hpp_

#include "libslic3r.h"
#include "BoundingBox.hpp"
#include "Flow.hpp"
#include "SurfaceCollection.hpp"
#include "ExtrusionEntityCollection.hpp"

#include <boost/container/small_vector.hpp>

namespace Slic3r {

class ExPolygon;
using ExPolygons = std::vector<ExPolygon>;
class Layer;
using LayerPtrs = std::vector<Layer*>;
class LayerRegion;
using LayerRegionPtrs = std::vector<LayerRegion*>;
class PrintRegion;
class PrintObject;

namespace FillAdaptive {
    struct Octree;
}

namespace FillLightning {
    class Generator;
};

// Range of indices, providing support for range based loops.
template<typename T>
class IndexRange
{
public:
    IndexRange(T ibegin, T iend) : m_begin(ibegin), m_end(iend) {}
    IndexRange() = default;

    // Just a bare minimum functionality iterator required by range-for loop.
    class Iterator {
    public:
        T    operator*() const { return m_idx; }
        bool operator!=(const Iterator &rhs) const { return m_idx != rhs.m_idx; }
        void operator++() { ++ m_idx; }
    private:
        friend class IndexRange<T>;
        Iterator(T idx) : m_idx(idx) {}
        T m_idx;
    };

    Iterator begin()  const { assert(m_begin <= m_end); return Iterator(m_begin); };
    Iterator end()    const { assert(m_begin <= m_end); return Iterator(m_end); };

    bool     empty()  const { assert(m_begin <= m_end); return m_begin >= m_end; }
    T        size()   const { assert(m_begin <= m_end); return m_end - m_begin; }

private:
    // Index of the first extrusion in LayerRegion.
    T    m_begin   { 0 };
    // Index of the last extrusion in LayerRegion.
    T    m_end     { 0 };
};

using ExtrusionRange = IndexRange<uint32_t>;
using ExPolygonRange = IndexRange<uint32_t>;

// Range of extrusions, referencing the source region by an index.
class LayerExtrusionRange : public ExtrusionRange
{
public:
    LayerExtrusionRange(uint32_t iregion, ExtrusionRange extrusion_range) : m_region(iregion), ExtrusionRange(extrusion_range) {}
    LayerExtrusionRange() = default;

    // Index of LayerRegion in Layer.
    uint32_t    region() const { return m_region; };

private:
    // Index of LayerRegion in Layer.
    uint32_t    m_region  { 0 };
};

// One LayerIsland may be filled with solid fill, sparse fill, top / bottom fill.
static constexpr const size_t LayerExtrusionRangesStaticSize = 3;
using LayerExtrusionRanges =
#ifdef NDEBUG
    // To reduce memory allocation in release mode.
    boost::container::small_vector<LayerExtrusionRange, LayerExtrusionRangesStaticSize>;
#else // NDEBUG
    // To ease debugging.
    std::vector<LayerExtrusionRange>;
#endif // NDEBUG

class LayerRegion
{
public:
    [[nodiscard]] Layer*                            layer()         { return m_layer; }
    [[nodiscard]] const Layer*                      layer() const   { return m_layer; }
    [[nodiscard]] const PrintRegion&                region() const  { return *m_region; }

    // collection of surfaces generated by slicing the original geometry
    // divided by type top/bottom/internal
    [[nodiscard]] const SurfaceCollection&          slices() const { return m_slices; }

    // Unspecified fill polygons, used for overhang detection ("ensure vertical wall thickness feature")
    // and for re-starting of infills.
    [[nodiscard]] const ExPolygons&                 fill_expolygons() const { return m_fill_expolygons; }
    // and their bounding boxes
    [[nodiscard]] const BoundingBoxes&              fill_expolygons_bboxes() const { return m_fill_expolygons_bboxes; }
    // Storage for fill regions produced for a single LayerIsland, of which infill splits into multiple islands.
    // Not used for a plain single material print with no infill modifiers.
    [[nodiscard]] const ExPolygons&                 fill_expolygons_composite() const { return m_fill_expolygons_composite; }
    // and their bounding boxes
    [[nodiscard]] const BoundingBoxes&              fill_expolygons_composite_bboxes() const { return m_fill_expolygons_composite_bboxes; }

    // collection of surfaces generated by slicing the original geometry
    // divided by type top/bottom/internal
    [[nodiscard]] const SurfaceCollection&          fill_surfaces() const { return m_fill_surfaces; }

    // collection of extrusion paths/loops filling gaps
    // These fills are generated by the perimeter generator.
    // They are not printed on their own, but they are copied to this->fills during infill generation.
    [[nodiscard]] const ExtrusionEntityCollection&  thin_fills() const { return m_thin_fills; }

    // collection of polylines representing the unsupported bridge edges
    [[nodiscard]] const Polylines&                  unsupported_bridge_edges() const { return m_unsupported_bridge_edges; }

    // ordered collection of extrusion paths/loops to build all perimeters
    // (this collection contains only ExtrusionEntityCollection objects)
    [[nodiscard]] const ExtrusionEntityCollection&  perimeters() const { return m_perimeters; }

    // ordered collection of extrusion paths to fill surfaces
    // (this collection contains only ExtrusionEntityCollection objects)
    [[nodiscard]] const ExtrusionEntityCollection&  fills() const { return m_fills; }
    
    Flow    flow(FlowRole role) const;
    Flow    flow(FlowRole role, double layer_height) const;
    Flow    bridging_flow(FlowRole role) const;

    void    slices_to_fill_surfaces_clipped();
    void    prepare_fill_surfaces();
    // Produce perimeter extrusions, gap fill extrusions and fill polygons for input slices.
    void    make_perimeters(
        // Input slices for which the perimeters, gap fills and fill expolygons are to be generated.
        const SurfaceCollection                                &slices,
        // Ranges of perimeter extrusions and gap fill extrusions per suface, referencing
        // newly created extrusions stored at this LayerRegion.
        std::vector<std::pair<ExtrusionRange, ExtrusionRange>> &perimeter_and_gapfill_ranges,
        // All fill areas produced for all input slices above.
        ExPolygons                                             &fill_expolygons,
        // Ranges of fill areas above per input slice.
        std::vector<ExPolygonRange>                            &fill_expolygons_ranges);
    void    process_external_surfaces(const Layer *lower_layer, const Polygons *lower_layer_covered);
    double  infill_area_threshold() const;
    // Trim surfaces by trimming polygons. Used by the elephant foot compensation at the 1st layer.
    void    trim_surfaces(const Polygons &trimming_polygons);
    // Single elephant foot compensation step, used by the elephant foor compensation at the 1st layer.
    // Trim surfaces by trimming polygons (shrunk by an elephant foot compensation step), but don't shrink narrow parts so much that no perimeter would fit.
    void    elephant_foot_compensation_step(const float elephant_foot_compensation_perimeter_step, const Polygons &trimming_polygons);

    void    export_region_slices_to_svg(const char *path) const;
    void    export_region_fill_surfaces_to_svg(const char *path) const;
    // Export to "out/LayerRegion-name-%d.svg" with an increasing index with every export.
    void    export_region_slices_to_svg_debug(const char *name) const;
    void    export_region_fill_surfaces_to_svg_debug(const char *name) const;

    // Is there any valid extrusion assigned to this LayerRegion?
    bool    has_extrusions() const { return ! this->perimeters().empty() || ! this->fills().empty(); }

protected:
    friend class Layer;
    friend class PrintObject;

    LayerRegion(Layer *layer, const PrintRegion *region) : m_layer(layer), m_region(region) {}
    ~LayerRegion() = default;

private:
    // Modifying m_slices
    friend std::string fix_slicing_errors(LayerPtrs&, const std::function<void()>&);
    template<typename ThrowOnCancel>
    friend void apply_mm_segmentation(PrintObject& print_object, ThrowOnCancel throw_on_cancel);

    Layer                      *m_layer;
    const PrintRegion          *m_region;

    // Backed up slices before they are split into top/bottom/internal.
    // Only backed up for multi-region layers or layers with elephant foot compensation.
    //FIXME Review whether not to simplify the code by keeping the raw_slices all the time.
    ExPolygons                  m_raw_slices;

//FIXME make m_slices public for unit tests
public:
    // collection of surfaces generated by slicing the original geometry
    // divided by type top/bottom/internal
    SurfaceCollection           m_slices;

private:
    // Unspecified fill polygons, used for overhang detection ("ensure vertical wall thickness feature")
    // and for re-starting of infills.
    ExPolygons                  m_fill_expolygons;
    // and their bounding boxes
    BoundingBoxes               m_fill_expolygons_bboxes;
    // Storage for fill regions produced for a single LayerIsland, of which infill splits into multiple islands.
    // Not used for a plain single material print with no infill modifiers.
    ExPolygons                  m_fill_expolygons_composite;
    // and their bounding boxes
    BoundingBoxes               m_fill_expolygons_composite_bboxes;

    // Collection of surfaces for infill generation, created by splitting m_slices by m_fill_expolygons.
    SurfaceCollection           m_fill_surfaces;

    // Collection of extrusion paths/loops filling gaps
    // These fills are generated by the perimeter generator.
    // They are not printed on their own, but they are copied to this->fills during infill generation.
    ExtrusionEntityCollection   m_thin_fills;

    // collection of polylines representing the unsupported bridge edges
    Polylines                   m_unsupported_bridge_edges;

    // ordered collection of extrusion paths/loops to build all perimeters
    // (this collection contains only ExtrusionEntityCollection objects)
    ExtrusionEntityCollection   m_perimeters;

    // ordered collection of extrusion paths to fill surfaces
    // (this collection contains only ExtrusionEntityCollection objects)
    ExtrusionEntityCollection   m_fills;

    // collection of expolygons representing the bridged areas (thus not
    // needing support material)
//  Polygons                    bridged;
};

// LayerSlice contains one or more LayerIsland objects,
// each LayerIsland containing a set of perimeter extrusions extruded with one particular PrintRegionConfig parameters
// and one or multiple 
struct LayerIsland
{
private:
    friend class Layer;
    static constexpr const uint32_t fill_region_composite_id = std::numeric_limits<uint32_t>::max();

public:
    // Perimeter extrusions in LayerRegion belonging to this island.
    LayerExtrusionRange   perimeters;
    // Thin fills of the same region as perimeters. Generated by classic perimeter generator, while Arachne puts them into perimeters.
    ExtrusionRange        thin_fills;
    // Infill + gapfill extrusions in LayerRegion belonging to this island.
    LayerExtrusionRanges  fills;
    // Region that is to be filled with the fills above (thin fills, regular fills).
    // Pointing to either LayerRegion::fill_expolygons() or LayerRegion::fill_expolygons_composite()
    // based on this->fill_expolygons_composite() flag.
    ExPolygonRange        fill_expolygons;
    // Index of LayerRegion with LayerRegion::fill_expolygons() if not fill_expolygons_composite().
    uint32_t              fill_region_id;
    bool                  fill_expolygons_composite() const { return this->fill_region_id == fill_region_composite_id; }
    // Centroid of this island used for path planning.
//    Point                 centroid;

    bool                  has_extrusions() const { return ! this->perimeters.empty() || ! this->fills.empty(); }

    void                  add_fill_range(const LayerExtrusionRange &new_fill_range) {
        // Compress ranges.
        if (! this->fills.empty() && this->fills.back().region() == new_fill_range.region() && *this->fills.back().end() == *new_fill_range.begin())
            this->fills.back() = { new_fill_range.region(), { *this->fills.back().begin(), *new_fill_range.end() } };
        else
            this->fills.push_back(new_fill_range);
    }
};

static constexpr const size_t LayerIslandsStaticSize = 1;
using LayerIslands =
#ifdef NDEBUG
    // To reduce memory allocation in release mode.
    boost::container::small_vector<LayerIsland, LayerIslandsStaticSize>;
#else // NDEBUG
    // To ease debugging.
    std::vector<LayerIsland>;
#endif // NDEBUG

// One connected island of a layer. LayerSlice may consist of one or more LayerIslands.
struct LayerSlice
{
    struct Link {
        int32_t     slice_idx;
        float       area;
    };
    static constexpr const size_t LinksStaticSize = 4;
    using Links =
#ifdef NDEBUG
        // To reduce memory allocation in release mode.
        boost::container::small_vector<Link, LinksStaticSize>;
#else // NDEBUG
        // To ease debugging.
        std::vector<Link>;
#endif // NDEBUG

    BoundingBox         bbox;
    Links               overlaps_above;
    Links               overlaps_below;
    // One island for each region or region set that generates its own perimeters.
    // For multi-material prints or prints with regions of different perimeter parameters,
    // a LayerSlice may be split into multiple LayerIslands.
    // For most prints there will be just one island.
    LayerIslands        islands;

    bool    has_extrusions() const { for (const LayerIsland &island : islands) if (island.has_extrusions()) return true; return false; }
};

using LayerSlices = std::vector<LayerSlice>;

class Layer 
{
public:
    // Sequential index of this layer in PrintObject::m_layers, offsetted by the number of raft layers.
    size_t              id() const          { return m_id; }
    void                set_id(size_t id)   { m_id = id; }
    PrintObject*        object()            { return m_object; }
    const PrintObject*  object() const      { return m_object; }

    Layer              *upper_layer;
    Layer              *lower_layer;
//    bool                slicing_errors;
    coordf_t            slice_z;       // Z used for slicing in unscaled coordinates
    coordf_t            print_z;       // Z used for printing in unscaled coordinates
    coordf_t            height;        // layer height in unscaled coordinates
    coordf_t            bottom_z() const { return this->print_z - this->height; }

    //Extrusions estimated to be seriously malformed, estimated during "Estimating curled extrusions" step. These lines should be avoided during fast travels.
    Lines               malformed_lines;

    // Collection of expolygons generated by slicing the possibly multiple meshes of the source geometry 
    // (with possibly differing extruder ID and slicing parameters) and merged.
    // For the first layer, if the Elephant foot compensation is applied, this lslice is uncompensated, therefore
    // it includes the Elephant foot effect, thus it corresponds to the shape of the printed 1st layer.
    // These lslices aka islands are chained by the shortest traverse distance and this traversal
    // order will be applied by the G-code generator to the extrusions fitting into these lslices.
    // These lslices are also used to detect overhangs and overlaps between successive layers, therefore it is important
    // that the 1st lslice is not compensated by the Elephant foot compensation algorithm.
    ExPolygons 				lslices;
    std::vector<size_t>     lslice_indices_sorted_by_print_order;
    LayerSlices             lslices_ex;

    size_t                  region_count() const { return m_regions.size(); }
    const LayerRegion*      get_region(int idx) const { return m_regions[idx]; }
    LayerRegion*            get_region(int idx) { return m_regions[idx]; }
    LayerRegion*            add_region(const PrintRegion *print_region);
    const LayerRegionPtrs&  regions() const { return m_regions; }
    // Test whether whether there are any slices assigned to this layer.
    bool                    empty() const;    
    void                    make_slices();
    // After creating the slices on all layers, chain the islands overlapping in Z.
    static void             build_up_down_graph(Layer &below, Layer &above);
    // Backup and restore raw sliced regions if needed.
    //FIXME Review whether not to simplify the code by keeping the raw_slices all the time.
    void                    backup_untyped_slices();
    void                    restore_untyped_slices();
    // To improve robustness of detect_surfaces_type() when reslicing (working with typed slices), see GH issue #7442.
    void                    restore_untyped_slices_no_extra_perimeters();
    // Slices merged into islands, to be used by the elephant foot compensation to trim the individual surfaces with the shrunk merged slices.
    ExPolygons              merged(float offset) const;
    template <class T> bool any_internal_region_slice_contains(const T &item) const {
        for (const LayerRegion *layerm : m_regions) if (layerm->slices().any_internal_contains(item)) return true;
        return false;
    }
    template <class T> bool any_bottom_region_slice_contains(const T &item) const {
        for (const LayerRegion *layerm : m_regions) if (layerm->slices().any_bottom_contains(item)) return true;
        return false;
    }
    void                    make_perimeters();
    // Phony version of make_fills() without parameters for Perl integration only.
    void                    make_fills() { this->make_fills(nullptr, nullptr, nullptr); }
    void                    make_fills(FillAdaptive::Octree* adaptive_fill_octree, FillAdaptive::Octree* support_fill_octree, FillLightning::Generator* lightning_generator);
    void 					make_ironing();

    void                    export_region_slices_to_svg(const char *path) const;
    void                    export_region_fill_surfaces_to_svg(const char *path) const;
    // Export to "out/LayerRegion-name-%d.svg" with an increasing index with every export.
    void                    export_region_slices_to_svg_debug(const char *name) const;
    void                    export_region_fill_surfaces_to_svg_debug(const char *name) const;

    // Is there any valid extrusion assigned to this LayerRegion?
    virtual bool            has_extrusions() const { for (auto layerm : m_regions) if (layerm->has_extrusions()) return true; return false; }
//    virtual bool            has_extrusions() const { for (const LayerSlice &lslice : lslices_ex) if (lslice.has_extrusions()) return true; return false; }

protected:
    friend class PrintObject;
    friend std::vector<Layer*> new_layers(PrintObject*, const std::vector<coordf_t>&);
    friend std::string fix_slicing_errors(LayerPtrs&, const std::function<void()>&);

    Layer(size_t id, PrintObject *object, coordf_t height, coordf_t print_z, coordf_t slice_z) :
        upper_layer(nullptr), lower_layer(nullptr), 
        //slicing_errors(false),
        slice_z(slice_z), print_z(print_z), height(height),
        m_id(id), m_object(object) {}
    virtual ~Layer();
    // Clear fill extrusions, remove them from layer islands.
    void clear_fills();

private:
    void sort_perimeters_into_islands(
        // Slices for which perimeters and fill_expolygons were just created.
        // The slices may have been created by merging multiple source slices with the same perimeter parameters.
        const SurfaceCollection                                         &slices,
        // Region where the perimeters, gap fills and fill expolygons are stored.
        const uint32_t                                                   region_id,
        // Perimeters and gap fills produced by the perimeter generator for the slices,
        // sorted by the source slices.
        const std::vector<std::pair<ExtrusionRange, ExtrusionRange>>    &perimeter_and_gapfill_ranges,
        // Fill expolygons produced for all source slices above.
        ExPolygons                                                      &&fill_expolygons,
        // Fill expolygon ranges sorted by the source slices.
        const std::vector<ExPolygonRange>                               &fill_expolygons_ranges,
        // If the current layer consists of multiple regions, then the fill_expolygons above are split by the source LayerRegion surfaces.
        const std::vector<uint32_t>                                     &layer_region_ids);

    // Sequential index of layer, 0-based, offsetted by number of raft layers.
    size_t              m_id;
    PrintObject        *m_object;
    LayerRegionPtrs     m_regions;
};

class SupportLayer : public Layer 
{
public:
    // Polygons covered by the supports: base, interface and contact areas.
    // Used to suppress retraction if moving for a support extrusion over these support_islands.
    ExPolygons                  support_islands;
    // Slightly inflated bounding boxes of the above, for faster intersection query.
    BoundingBoxes               support_islands_bboxes;
    // Extrusion paths for the support base and for the support interface and contacts.
    ExtrusionEntityCollection   support_fills;


    // Is there any valid extrusion assigned to this LayerRegion?
    virtual bool                has_extrusions() const { return ! support_fills.empty(); }

    // Zero based index of an interface layer, used for alternating direction of interface / contact layers.
    size_t                      interface_id() const { return m_interface_id; }

protected:
    friend class PrintObject;

    // The constructor has been made public to be able to insert additional support layers for the skirt or a wipe tower
    // between the raft and the object first layer.
    SupportLayer(size_t id, size_t interface_id, PrintObject *object, coordf_t height, coordf_t print_z, coordf_t slice_z) :
        Layer(id, object, height, print_z, slice_z), m_interface_id(interface_id) {}
    virtual ~SupportLayer() = default;

    size_t m_interface_id;
};

template<typename LayerContainer>
inline std::vector<float> zs_from_layers(const LayerContainer &layers)
{
    std::vector<float> zs;
    zs.reserve(layers.size());
    for (const Layer *l : layers)
        zs.emplace_back((float)l->slice_z);
    return zs;
}

extern BoundingBox get_extents(const LayerRegion &layer_region);
extern BoundingBox get_extents(const LayerRegionPtrs &layer_regions);

}

#endif
