/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2011 Sandro Santilli <strk@kbt.io>
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: geom/GeometryFactory.java r320 (JTS-1.12)
 *
 **********************************************************************/

#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateArraySequenceFactory.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Point.h>
#include <geos/geom/LineString.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/MultiPoint.h>
#include <geos/geom/MultiLineString.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/util/CoordinateOperation.h>
#include <geos/geom/util/GeometryEditor.h>
#include <geos/util.h>

#include <cassert>
#include <vector>
#include <typeinfo>
#include <cmath>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

#ifdef GEOS_DEBUG
#include <iostream>
#endif

#ifndef GEOS_INLINE
# include <geos/geom/GeometryFactory.inl>
#endif

using namespace std;

namespace geos {
namespace geom { // geos::geom

namespace {

class gfCoordinateOperation: public util::CoordinateOperation {
    using CoordinateOperation::edit;
    const CoordinateSequenceFactory* _gsf;
public:
    gfCoordinateOperation(const CoordinateSequenceFactory* gsf)
        : _gsf(gsf)
    {}
    CoordinateSequence*
    edit(const CoordinateSequence* coordSeq,
         const Geometry*) override
    {
        return _gsf->create(*coordSeq).release();
    }
};

} // anonymous namespace



/*protected*/
GeometryFactory::GeometryFactory()
    :
    precisionModel(new PrecisionModel()),
    SRID(0),
    coordinateListFactory(CoordinateArraySequenceFactory::instance())
    , _refCount(0), _autoDestroy(false)
{
#if GEOS_DEBUG
    std::cerr << "GEOS_DEBUG: GeometryFactory[" << this << "]::GeometryFactory()" << std::endl;
    std::cerr << "\tcreate PrecisionModel[" << precisionModel << "]" << std::endl;
#endif
}

/*public static*/
GeometryFactory::Ptr
GeometryFactory::create()
{
    return GeometryFactory::Ptr(new GeometryFactory());
}

/*protected*/
GeometryFactory::GeometryFactory(const PrecisionModel* pm, int newSRID,
                                 CoordinateSequenceFactory* nCoordinateSequenceFactory)
    :
    SRID(newSRID)
    , _refCount(0), _autoDestroy(false)
{
#if GEOS_DEBUG
    std::cerr << "GEOS_DEBUG: GeometryFactory[" << this << "]::GeometryFactory(PrecisionModel[" << pm << "], SRID)" <<
              std::endl;
#endif
    if(! pm) {
        precisionModel = new PrecisionModel();
    }
    else {
        precisionModel = new PrecisionModel(*pm);
    }

    if(! nCoordinateSequenceFactory) {
        coordinateListFactory = CoordinateArraySequenceFactory::instance();
    }
    else {
        coordinateListFactory = nCoordinateSequenceFactory;
    }
}

/*public static*/
GeometryFactory::Ptr
GeometryFactory::create(const PrecisionModel* pm, int newSRID,
                        CoordinateSequenceFactory* nCoordinateSequenceFactory)
{
    return GeometryFactory::Ptr(
               new GeometryFactory(pm, newSRID, nCoordinateSequenceFactory)
           );
}

/*protected*/
GeometryFactory::GeometryFactory(
    CoordinateSequenceFactory* nCoordinateSequenceFactory)
    :
    precisionModel(new PrecisionModel()),
    SRID(0)
    , _refCount(0), _autoDestroy(false)
{
#if GEOS_DEBUG
    std::cerr << "GEOS_DEBUG: GeometryFactory[" << this << "]::GeometryFactory(CoordinateSequenceFactory[" <<
              nCoordinateSequenceFactory << "])" << std::endl;
#endif
    if(! nCoordinateSequenceFactory) {
        coordinateListFactory = CoordinateArraySequenceFactory::instance();
    }
    else {
        coordinateListFactory = nCoordinateSequenceFactory;
    }
}

/*public static*/
GeometryFactory::Ptr
GeometryFactory::create(
    CoordinateSequenceFactory* nCoordinateSequenceFactory)
{
    return GeometryFactory::Ptr(
               new GeometryFactory(nCoordinateSequenceFactory)
           );
}

/*protected*/
GeometryFactory::GeometryFactory(const PrecisionModel* pm)
    :
    SRID(0),
    coordinateListFactory(CoordinateArraySequenceFactory::instance())
    , _refCount(0), _autoDestroy(false)
{
#if GEOS_DEBUG
    std::cerr << "GEOS_DEBUG: GeometryFactory[" << this << "]::GeometryFactory(PrecisionModel[" << pm << "])" << std::endl;
#endif
    if(! pm) {
        precisionModel = new PrecisionModel();
    }
    else {
        precisionModel = new PrecisionModel(*pm);
    }
}

/*public static*/
GeometryFactory::Ptr
GeometryFactory::create(const PrecisionModel* pm)
{
    return GeometryFactory::Ptr(
               new GeometryFactory(pm)
           );
}

/*protected*/
GeometryFactory::GeometryFactory(const PrecisionModel* pm, int newSRID)
    :
    SRID(newSRID),
    coordinateListFactory(CoordinateArraySequenceFactory::instance())
    , _refCount(0), _autoDestroy(false)
{
#if GEOS_DEBUG
    std::cerr << "GEOS_DEBUG: GeometryFactory[" << this << "]::GeometryFactory(PrecisionModel[" << pm << "], SRID)" <<
              std::endl;
#endif
    if(! pm) {
        precisionModel = new PrecisionModel();
    }
    else {
        precisionModel = new PrecisionModel(*pm);
    }
}

/*public static*/
GeometryFactory::Ptr
GeometryFactory::create(const PrecisionModel* pm, int newSRID)
{
    return GeometryFactory::Ptr(
               new GeometryFactory(pm, newSRID)
           );
}

/*protected*/
GeometryFactory::GeometryFactory(const GeometryFactory& gf)
{
    assert(gf.precisionModel);
    precisionModel = new PrecisionModel(*(gf.precisionModel));
    SRID = gf.SRID;
    coordinateListFactory = gf.coordinateListFactory;
    _autoDestroy = false;
    _refCount = 0;
}

/*public static*/
GeometryFactory::Ptr
GeometryFactory::create(const GeometryFactory& gf)
{
    return GeometryFactory::Ptr(
               new GeometryFactory(gf)
           );
}

/*public virtual*/
GeometryFactory::~GeometryFactory()
{
#if GEOS_DEBUG
    std::cerr << "GEOS_DEBUG: GeometryFactory[" << this << "]::~GeometryFactory()" << std::endl;
#endif
    delete precisionModel;
}

/*public*/
Point*
GeometryFactory::createPointFromInternalCoord(const Coordinate* coord,
        const Geometry* exemplar) const
{
    assert(coord);
    Coordinate newcoord = *coord;
    exemplar->getPrecisionModel()->makePrecise(&newcoord);
    return exemplar->getFactory()->createPoint(newcoord);
}


/*public*/
Geometry*
GeometryFactory::toGeometry(const Envelope* envelope) const
{
    Coordinate coord;

    if(envelope->isNull()) {
        return createPoint();
    }
    if(envelope->getMinX() == envelope->getMaxX() && envelope->getMinY() == envelope->getMaxY()) {
        coord.x = envelope->getMinX();
        coord.y = envelope->getMinY();
        return createPoint(coord);
    }
    auto cl = CoordinateArraySequenceFactory::instance()->
                             create((size_t) 0, 2);
    coord.x = envelope->getMinX();
    coord.y = envelope->getMinY();
    cl->add(coord);
    coord.x = envelope->getMaxX();
    coord.y = envelope->getMinY();
    cl->add(coord);
    coord.x = envelope->getMaxX();
    coord.y = envelope->getMaxY();
    cl->add(coord);
    coord.x = envelope->getMinX();
    coord.y = envelope->getMaxY();
    cl->add(coord);
    coord.x = envelope->getMinX();
    coord.y = envelope->getMinY();
    cl->add(coord);

    Polygon* p = createPolygon(createLinearRing(cl.release()), nullptr);
    return p;
}

/*public*/
const PrecisionModel*
GeometryFactory::getPrecisionModel() const
{
    return precisionModel;
}

/*public*/
Point*
GeometryFactory::createPoint() const
{
    return new Point(nullptr, this);
}

/*public*/
Point*
GeometryFactory::createPoint(const Coordinate& coordinate) const
{
    if(coordinate.isNull()) {
        return createPoint();
    }
    else {
        std::size_t dim = std::isnan(coordinate.z) ? 2 : 3;
        auto cl = coordinateListFactory->create(new vector<Coordinate>(1, coordinate), dim);
        //cl->setAt(coordinate, 0);
        Point* ret = createPoint(cl.release());
        return ret;
    }
}

/*public*/
Point*
GeometryFactory::createPoint(CoordinateSequence* newCoords) const
{
    return new Point(newCoords, this);
}

/*public*/
Point*
GeometryFactory::createPoint(const CoordinateSequence& fromCoords) const
{
    auto newCoords = fromCoords.clone();
    return new Point(newCoords.release(), this);

}

/*public*/
MultiLineString*
GeometryFactory::createMultiLineString() const
{
    return new MultiLineString(nullptr, this);
}

/*public*/
MultiLineString*
GeometryFactory::createMultiLineString(vector<Geometry*>* newLines)
const
{
    return new MultiLineString(newLines, this);
}

/*public*/
MultiLineString*
GeometryFactory::createMultiLineString(const vector<Geometry*>& fromLines)
const
{
    vector<Geometry*>* newGeoms = new vector<Geometry*>(fromLines.size());
    for(size_t i = 0; i < fromLines.size(); i++) {
        const LineString* line = dynamic_cast<const LineString*>(fromLines[i]);
        if(! line) {
            throw geos::util::IllegalArgumentException("createMultiLineString called with a vector containing non-LineStrings");
        }
        (*newGeoms)[i] = new LineString(*line);
    }
    MultiLineString* g = nullptr;
    try {
        g = new MultiLineString(newGeoms, this);
    }
    catch(...) {
        for(size_t i = 0; i < newGeoms->size(); i++) {
            delete(*newGeoms)[i];
        }
        delete newGeoms;
        throw;
    }
    return g;
}

/*public*/
GeometryCollection*
GeometryFactory::createGeometryCollection() const
{
    return new GeometryCollection(nullptr, this);
}

/*public*/
Geometry*
GeometryFactory::createEmptyGeometry() const
{
    return new GeometryCollection(nullptr, this);
}

/*public*/
GeometryCollection*
GeometryFactory::createGeometryCollection(vector<Geometry*>* newGeoms) const
{
    return new GeometryCollection(newGeoms, this);
}

/*public*/
GeometryCollection*
GeometryFactory::createGeometryCollection(const vector<Geometry*>& fromGeoms) const
{
    vector<Geometry*>* newGeoms = new vector<Geometry*>(fromGeoms.size());
    for(size_t i = 0; i < fromGeoms.size(); i++) {
        (*newGeoms)[i] = fromGeoms[i]->clone().release();
    }
    GeometryCollection* g = nullptr;
    try {
        g = new GeometryCollection(newGeoms, this);
    }
    catch(...) {
        for(size_t i = 0; i < newGeoms->size(); i++) {
            delete(*newGeoms)[i];
        }
        delete newGeoms;
        throw;
    }
    return g;
}

/*public*/
MultiPolygon*
GeometryFactory::createMultiPolygon() const
{
    return new MultiPolygon(nullptr, this);
}

/*public*/
MultiPolygon*
GeometryFactory::createMultiPolygon(vector<Geometry*>* newPolys) const
{
    return new MultiPolygon(newPolys, this);
}

/*public*/
MultiPolygon*
GeometryFactory::createMultiPolygon(const vector<Geometry*>& fromPolys) const
{
    vector<Geometry*>* newGeoms = new vector<Geometry*>(fromPolys.size());
    for(size_t i = 0; i < fromPolys.size(); i++) {
        (*newGeoms)[i] = fromPolys[i]->clone().release();
    }
    MultiPolygon* g = nullptr;
    try {
        g = new MultiPolygon(newGeoms, this);
    }
    catch(...) {
        for(size_t i = 0; i < newGeoms->size(); i++) {
            delete(*newGeoms)[i];
        }
        delete newGeoms;
        throw;
    }
    return g;
}

/*public*/
LinearRing*
GeometryFactory::createLinearRing() const
{
    return new LinearRing(nullptr, this);
}

/*public*/
LinearRing*
GeometryFactory::createLinearRing(CoordinateSequence* newCoords) const
{
    return new LinearRing(newCoords, this);
}

/*public*/
Geometry::Ptr
GeometryFactory::createLinearRing(CoordinateSequence::Ptr newCoords) const
{
    return Geometry::Ptr(new LinearRing(std::move(newCoords), this));
}

/*public*/
LinearRing*
GeometryFactory::createLinearRing(const CoordinateSequence& fromCoords) const
{
    auto newCoords = fromCoords.clone();
    LinearRing* g = nullptr;
    // construction failure will delete newCoords
    g = new LinearRing(newCoords.release(), this);
    return g;
}

/*public*/
MultiPoint*
GeometryFactory::createMultiPoint(vector<Geometry*>* newPoints) const
{
    return new MultiPoint(newPoints, this);
}

/*public*/
MultiPoint*
GeometryFactory::createMultiPoint(const vector<Geometry*>& fromPoints) const
{
    vector<Geometry*>* newGeoms = new vector<Geometry*>(fromPoints.size());
    for(size_t i = 0; i < fromPoints.size(); i++) {
        (*newGeoms)[i] = fromPoints[i]->clone().release();
    }

    MultiPoint* g = nullptr;
    try {
        g = new MultiPoint(newGeoms, this);
    }
    catch(...) {
        for(size_t i = 0; i < newGeoms->size(); i++) {
            delete(*newGeoms)[i];
        }
        delete newGeoms;
        throw;
    }
    return g;
}

/*public*/
MultiPoint*
GeometryFactory::createMultiPoint() const
{
    return new MultiPoint(nullptr, this);
}

/*public*/
MultiPoint*
GeometryFactory::createMultiPoint(const CoordinateSequence& fromCoords) const
{
    size_t npts = fromCoords.getSize();
    vector<Geometry*>* pts = new vector<Geometry*>;
    pts->reserve(npts);
    for(size_t i = 0; i < npts; ++i) {
        Point* pt = createPoint(fromCoords.getAt(i));
        pts->push_back(pt);
    }
    MultiPoint* mp = nullptr;
    try {
        mp = createMultiPoint(pts);
    }
    catch(...) {
        for(size_t i = 0; i < npts; ++i) {
            delete(*pts)[i];
        }
        delete pts;
        throw;
    }
    return mp;
}

/*public*/
MultiPoint*
GeometryFactory::createMultiPoint(const std::vector<Coordinate>& fromCoords) const
{
    size_t npts = fromCoords.size();
    vector<Geometry*>* pts = new vector<Geometry*>;
    pts->reserve(npts);
    for(size_t i = 0; i < npts; ++i) {
        Point* pt = createPoint(fromCoords[i]);
        pts->push_back(pt);
    }
    MultiPoint* mp = nullptr;
    try {
        mp = createMultiPoint(pts);
    }
    catch(...) {
        for(size_t i = 0; i < npts; ++i) {
            delete(*pts)[i];
        }
        delete pts;
        throw;
    }
    return mp;
}

/*public*/
Polygon*
GeometryFactory::createPolygon() const
{
    return new Polygon(nullptr, nullptr, this);
}

/*public*/
Polygon*
GeometryFactory::createPolygon(LinearRing* shell, vector<Geometry*>* holes)
const
{
    return new Polygon(shell, holes, this);
}

/*public*/
Polygon*
GeometryFactory::createPolygon(const LinearRing& shell, const vector<Geometry*>& holes)
const
{
    LinearRing* newRing = dynamic_cast<LinearRing*>(shell.clone().release());
    vector<Geometry*>* newHoles = new vector<Geometry*>(holes.size());
    for(size_t i = 0; i < holes.size(); i++) {
        (*newHoles)[i] = holes[i]->clone().release();
    }
    Polygon* g = nullptr;
    try {
        g = new Polygon(newRing, newHoles, this);
    }
    catch(...) {
        delete newRing;
        for(size_t i = 0; i < holes.size(); i++) {
            delete(*newHoles)[i];
        }
        delete newHoles;
        throw;
    }
    return g;
}

/*public*/
LineString*
GeometryFactory::createLineString() const
{
    return new LineString(nullptr, this);
}

/*public*/
std::unique_ptr<LineString>
GeometryFactory::createLineString(const LineString& ls) const
{
    return std::unique_ptr<LineString>(new LineString(ls));
    //return make_unique<LineString>(ls); // TODO why doesn't this work?
}

/*public*/
LineString*
GeometryFactory::createLineString(CoordinateSequence* newCoords)
const
{
    return new LineString(newCoords, this);
}

/*public*/
Geometry::Ptr
GeometryFactory::createLineString(CoordinateSequence::Ptr newCoords)
const
{
    return Geometry::Ptr(new LineString(std::move(newCoords), this));
}

/*public*/
LineString*
GeometryFactory::createLineString(const CoordinateSequence& fromCoords)
const
{
    auto newCoords = fromCoords.clone();
    LineString* g = nullptr;
    // construction failure will delete newCoords
    g = new LineString(newCoords.release(), this);
    return g;
}

/*public*/
Geometry*
GeometryFactory::buildGeometry(vector<Geometry*>* newGeoms) const
{
    if(!newGeoms->size()) {
        // we do not need the vector anymore
        delete newGeoms;
        return createGeometryCollection();
    }

    bool isHeterogeneous = false;
    bool hasGeometryCollection = false;
    GeometryTypeId type = (*newGeoms)[0]->getGeometryTypeId();

    for(Geometry* gp : *newGeoms) {
        GeometryTypeId geometryType = gp->getGeometryTypeId();
        if(type != geometryType) {
            isHeterogeneous = true;
        }
        if(geometryType == GEOS_GEOMETRYCOLLECTION) {
            hasGeometryCollection = true;
        }
    }

    if(isHeterogeneous || hasGeometryCollection) {
        return createGeometryCollection(newGeoms);
    }

    // At this point we know the collection is not hetereogenous.
    bool isCollection = newGeoms->size() > 1;
    if(isCollection) {
        if(type == GEOS_POLYGON) {
            return createMultiPolygon(newGeoms);
        }
        else if(type == GEOS_LINESTRING) {
            return createMultiLineString(newGeoms);
        }
        else if(type == GEOS_LINEARRING) {
            return createMultiLineString(newGeoms);
        }
        else if(type == GEOS_POINT) {
            return createMultiPoint(newGeoms);
        }
        else {
            return createGeometryCollection(newGeoms);
        }
    }

    // since this is not a collection we can delete vector
    Geometry* geom0 = (*newGeoms)[0];
    delete newGeoms;
    return geom0;
}

/*public*/
Geometry*
GeometryFactory::buildGeometry(const vector<Geometry*>& fromGeoms) const
{
    size_t geomsSize = fromGeoms.size();
    if(geomsSize == 0) {
        return createGeometryCollection();
    }

    if(geomsSize == 1) {
        return fromGeoms[0]->clone().release();
    }

    bool isHeterogeneous = false;
    GeometryTypeId type = fromGeoms[0]->getGeometryTypeId();

    for(const Geometry* gp : fromGeoms) {
        GeometryTypeId geometryType = gp->getGeometryTypeId();
        if(type != geometryType) {
            isHeterogeneous = true;
        }
    }

    if(isHeterogeneous) {
        return createGeometryCollection(fromGeoms);
    }

    if(type == GEOS_POLYGON) {
        return createMultiPolygon(fromGeoms);
    }
    else if(type == GEOS_LINESTRING) {
        return createMultiLineString(fromGeoms);
    }
    else if(type == GEOS_LINEARRING) {
        return createMultiLineString(fromGeoms);
    }
    else if(type == GEOS_POINT) {
        return createMultiPoint(fromGeoms);
    }

    throw geos::util::GEOSException("GeometryFactory::buildGeometry encountered an unknown geometry type!");
}

/*public*/
Geometry*
GeometryFactory::createGeometry(const Geometry* g) const
{
    // could this be cached to make this more efficient? Or maybe it isn't enough overhead to bother
    //return g->clone();
    util::GeometryEditor editor(this);
    gfCoordinateOperation coordOp(coordinateListFactory);
    Geometry* ret = editor.edit(g, &coordOp);
    return ret;
}

/*public*/
void
GeometryFactory::destroyGeometry(Geometry* g) const
{
    delete g;
}

/*public static*/
const GeometryFactory*
GeometryFactory::getDefaultInstance()
{
    static GeometryFactory* defInstance = new GeometryFactory();
    return defInstance;
}

/*private*/
void
GeometryFactory::addRef() const
{
    ++_refCount;
}

/*private*/
void
GeometryFactory::dropRef() const
{
    if(! --_refCount) {
        if(_autoDestroy) {
            delete this;
        }
    }
}

void
GeometryFactory::destroy()
{
    assert(!_autoDestroy); // don't call me twice !
    _autoDestroy = true;
    if(! _refCount) {
        delete this;
    }
}

} // namespace geos::geom
} // namespace geos
