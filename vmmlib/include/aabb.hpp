/*
 * Copyright (c) 2006-2015, Visualization and Multimedia Lab,
 *                          University of Zurich <http://vmml.ifi.uzh.ch>,
 *                          Eyescale Software GmbH,
 *                          Blue Brain Project, EPFL
 *
 * This file is part of VMMLib <https://github.com/VMML/vmmlib/>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.  Redistributions in binary
 * form must reproduce the above copyright notice, this list of conditions and
 * the following disclaimer in the documentation and/or other materials provided
 * with the distribution.  Neither the name of the Visualization and Multimedia
 * Lab, University of Zurich nor the names of its contributors may be used to
 * endorse or promote products derived from this software without specific prior
 * written permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef VMMLIB__AXIS_ALIGNED_BOUNDING_BOX__HPP
#define VMMLIB__AXIS_ALIGNED_BOUNDING_BOX__HPP

#include <vmmlib/vector.hpp>
#include <limits>

namespace vmml
{
/**
 * An axis-aligned bounding box.
 *
 * An empty bounding box has undefined, implementation-specific values. Read
 * operations (getMin(), getMax(), getDimension(), isIn(), etc.) have undefined
 * semantics on an empty bounding box. set() and merge() operations will define
 * the bounding box correctly.
 */
template< typename T > class AABB
{
public:
    /** Create an empty bounding box. */
    AABB();
    AABB( const Vector< 3, T >& pMin,
                            const Vector< 3, T >& pMax );
    AABB( const Vector< 4, T >& sphere );
    AABB( T cx, T cy, T cz, T size );

    bool isIn( const Vector< 3, T >& pos );
    bool isIn2d( const Vector< 3, T >& pos ); // only x and y components are checked
    bool isIn( const Vector< 4, T >& sphere );

    void set( const Vector< 3, T >& pMin, const Vector< 3, T >& pMax );
    void set( T cx, T cy, T cz, T size );
    void setMin( const Vector< 3, T >& pMin );
    void setMax( const Vector< 3, T >& pMax );

    const Vector< 3, T >& getMin() const;
    const Vector< 3, T >& getMax() const;
    Vector< 3, T >& getMin();
    Vector< 3, T >& getMax();

    void merge( const AABB< T >& aabb );
    void merge( const Vector< 3, T >& point );

    void setEmpty();
    bool isEmpty() const;

    AABB operator*( const T value ) const;
    AABB operator/( const T value ) const;
    AABB operator+( const T value ) const;
    AABB operator-( const T value ) const;

    void operator*=( const T value );
    void operator/=( const T value );
    void operator+=( const T value );
    void operator-=( const T value );

    template< class U >
    bool operator==( const AABB< U >& other ) const;
    template< class U >
    bool operator!=( const AABB< U >& other ) const;

    Vector< 3, T > getCenter() const;
    Vector< 3, T > getDimension() const;

    static AABB< T > makeUnitBox();

protected:
    Vector< 3, T > _min;
    Vector< 3, T > _max;
};

#ifndef VMMLIB_NO_TYPEDEFS
typedef AABB< float >  AABBf;
typedef AABB< double > AABBd;
#endif

template< typename T >
inline std::ostream& operator << ( std::ostream& os,
                                   const AABB< T >& aabb )
{
    return os << aabb.getMin() << " - " << aabb.getMax();
}

template< typename T > AABB< T >::AABB()
    : _min( std::numeric_limits< T >::max( ))
    , _max( std::numeric_limits< T >::min( ))
{}

template<> inline AABB< float >::AABB()
    : _min( std::numeric_limits< float >::max( ))
    , _max( -std::numeric_limits< float >::max( ))
{}

template<> inline AABB< double >::AABB()
    : _min( std::numeric_limits< double >::max( ))
    , _max( -std::numeric_limits< double >::max( ))
{}

template< typename T >
AABB< T >::AABB( const Vector< 3, T >& pMin,
                 const Vector< 3, T >& pMax)
    : _min( pMin )
    , _max( pMax )
{}

template< typename T >
AABB< T >::AABB( const Vector< 4, T >& sphere )
{
    _max = _min = sphere.getCenter();
    _max += sphere.getRadius();
    _min -= sphere.getRadius();
}

template< typename T >
AABB< T >::AABB( T cx, T cy, T cz, T size )
{
    _max = _min = Vector< 3, T >( cx, cy, cz );
    _max += size;
    _min -= size;
}

template< typename T >
inline bool AABB< T >::isIn( const Vector< 4, T >& sphere )
{
    Vector< 3, T > sv ( sphere.getCenter() );
    sv += sphere.getRadius();
    if ( sv.x() > _max.x() || sv.y() > _max.y() || sv.z() > _max.z() )
        return false;
    sv -= sphere.getRadius() * 2.0f;
    if ( sv.x() < _min.x() || sv.y() < _min.y() || sv.z() < _min.z() )
        return false;
    return true;
}

template< typename T >
inline bool AABB< T >::isIn( const Vector< 3, T >& pos )
{
    if ( pos.x() > _max.x() || pos.y() > _max.y() || pos.z() > _max.z() ||
         pos.x() < _min.x() || pos.y() < _min.y() || pos.z() < _min.z( ))
    {
        return false;
    }
    return true;
}

template< typename T >
inline bool AABB< T >::isIn2d( const Vector< 3, T >& pos )
{
    if ( pos.x() > _max.x() || pos.y() > _max.y() || pos.x() < _min.x() ||
         pos.y() < _min.y( ))
    {
        return false;
    }
    return true;
}

template< typename T >
inline void AABB< T >::set( const Vector< 3, T >& pMin,
                            const Vector< 3, T >& pMax )
{
    _min = pMin;
    _max = pMax;
}

template< typename T >
inline void AABB< T >::set( T cx, T cy, T cz, T size )
{
    Vector< 3, T > center( cx, cy, cz );
    _min = center - size;
    _max = center + size;
}

template< typename T >
inline void AABB< T >::setMin( const Vector< 3, T >& pMin )
{
    _min = pMin;
}

template< typename T >
inline void AABB< T >::setMax( const Vector< 3, T >& pMax )
{
    _max = pMax;
}

template< typename T >
inline const Vector< 3, T >& AABB< T >::getMin() const
{
    return _min;
}

template< typename T >
inline const Vector< 3, T >& AABB< T >::getMax() const
{
    return _max;
}

template< typename T > inline Vector< 3, T >& AABB< T >::getMin()
{
    return _min;
}

template< typename T > inline Vector< 3, T >& AABB< T >::getMax()
{
    return _max;
}

template< typename T > AABB< T >
AABB< T >::operator*( const T value ) const
{
    AABB result = *this;
    result *= value;
    return result;
}

template< typename T > AABB< T >
AABB< T >::operator/( const T value ) const
{
    AABB result = *this;
    result /= value;
    return result;
}

template< typename T > AABB< T >
AABB< T >::operator+( const T value ) const
{
    AABB result = *this;
    result += value;
    return result;
}

template< typename T > AABB< T >
AABB< T >::operator-( const T value ) const
{
    AABB result = *this;
    result -= value;
    return result;
}

template< typename T >
void AABB< T >::operator*=( const T value )
{
    _min *= value;
    _max *= value;
}

template< typename T >
void AABB< T >::operator/=( const T value )
{
    _min /= value;
    _max /= value;
}

template< typename T >
void AABB< T >::operator+=( const T value )
{
    _min += value;
    _max += value;
}

template< typename T >
void AABB< T >::operator-=( const T value )
{
    _min -= value;
    _max -= value;
}

template< typename T > template< class U > bool
AABB< T >::operator==( const AABB< U >& other )
    const
{
    return _min == other._min && _max == other._max;
}

template< typename T > template< class U > bool
AABB< T >::operator!=( const AABB< U >& other )
    const
{
    return _min != other._min || _max != other._max;
}

template< typename T >
Vector< 3, T > AABB< T >::getCenter() const
{
    return _min + ( ( _max - _min ) * 0.5f );
}

template< typename T >
Vector< 3, T > AABB< T >::getDimension() const
{
    return _max - _min;
}

template< typename T >
void AABB< T >::merge( const AABB<T>& aabb )
{
    const Vector< 3, T >& min = aabb.getMin();
    const Vector< 3, T >& max = aabb.getMax();

    if ( min.x() < _min.x() )
        _min.x() = min.x();
    if ( min.y() < _min.y() )
        _min.y() = min.y();
    if ( min.z() < _min.z() )
        _min.z() = min.z();

    if ( max.x() > _max.x() )
        _max.x() = max.x();
    if ( max.y() > _max.y() )
        _max.y() = max.y();
    if ( max.z() > _max.z() )
        _max.z() = max.z();
}

template< typename T >
void AABB< T >::merge( const Vector< 3, T >& point )
{
    if ( point.x() < _min.x() )
        _min.x() = point.x();
    if ( point.y() < _min.y() )
        _min.y() = point.y();
    if ( point.z() < _min.z() )
        _min.z() = point.z();

    if ( point.x() > _max.x() )
        _max.x() = point.x();
    if ( point.y() > _max.y() )
        _max.y() = point.y();
    if ( point.z() > _max.z() )
        _max.z() = point.z();
}

template< typename T >inline
void AABB< T >::setEmpty()
{
    _min = std::numeric_limits< T >::max();
    _max = -std::numeric_limits< T >::max();
}


template< typename T > inline bool AABB< T >::isEmpty() const
{
    return ( _min.x() >=  _max.x() || _min.y() >=  _max.y() ||
             _min.z() >=  _max.x( ));
}

template< typename T >
AABB< T > AABB< T >::makeUnitBox()
{
    return AABB( Vector< 3, T >::ZERO, Vector< 3, T >::ONE );
}

} //namespace vmml

#endif
