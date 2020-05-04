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

#ifndef VMMLIB__VECTOR__HPP
#define VMMLIB__VECTOR__HPP

#include <vmmlib_config.hpp>
#include <math.hpp>
#include <enable_if.hpp>
#include <exception.hpp>

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <cstring>
#include <limits>
#include <algorithm>

namespace vmml
{

template< size_t M, typename T = float >
class Vector
{
public:
    typedef T                                       value_type;
    typedef T*                                      iterator;
    typedef const T*                                const_iterator;
    typedef std::reverse_iterator< iterator >       reverse_iterator;
    typedef std::reverse_iterator< const_iterator > const_reverse_iterator;

    static const size_t DIMENSION = M;

    // constructors
    Vector() : array() {} // http://stackoverflow.com/questions/5602030
    explicit Vector( const T& a ); // sets all components to a;
    Vector( const T& x, const T& y );
    Vector( const T& x, const T& y, const T& z );
    Vector( const T& x, const T& y, const T& z, const T& w );

#ifndef SWIG
    // initializes the first M-1 values from vector_, the last from last_
    Vector( const Vector< M-1, T >& vector_, T last_ );
#endif

    Vector( const T* values );

#ifdef __OSG_MATH
    template< typename OSGVEC3 >
    explicit Vector( const OSGVEC3& from,
                     typename enable_if< M == 3, OSGVEC3 >::type* = 0 );
#endif

    // vec< M > with homogeneous coordinates <-> vec< M-1 > conversion ctor
    // to-homogenous-coordinates ctor
    template< size_t N >
    Vector( const Vector< N, T >& source_,
            typename enable_if< N == M - 1 >::type* = 0 );

    // from-homogenous-coordinates vector
    template< size_t N >
    Vector( const Vector< N, T >& source_,
            typename enable_if< N == M + 1 >::type* = 0  );

    template< typename U > Vector( const Vector< M, U >& source_ );

    // iterators
    inline iterator begin();
    inline iterator end();
    inline const_iterator begin() const;
    inline const_iterator end() const;
    inline reverse_iterator rbegin();
    inline reverse_iterator rend();
    inline const_reverse_iterator rbegin() const;
    inline const_reverse_iterator rend() const;

#  ifndef VMMLIB_NO_CONVERSION_OPERATORS
    // conversion operators
    inline operator T*();
    inline operator const T*() const;
#  else
    inline T& operator[]( size_t index );
    inline const T& operator[]( size_t index ) const;
#  endif

    // accessors
    inline T& operator()( size_t index );
    inline const T& operator()( size_t index ) const;

    inline T& at( size_t index );
    inline const T& at( size_t index ) const;

    // element accessors for M <= 4;
    inline T& x();
    inline T& y();
    inline T& z();
    inline T& w();
    inline const T& x() const;
    inline const T& y() const;
    inline const T& z() const;
    inline const T& w() const;

    // pixel color element accessors for M<= 4
    inline T& r();
    inline T& g();
    inline T& b();
    inline T& a();
    inline const T& r() const;
    inline const T& g() const;
    inline const T& b() const;
    inline const T& a() const;

    bool operator==( const Vector& other ) const;
    bool operator!=( const Vector& other ) const;
    bool equals( const Vector& other,
                 T tolerance = std::numeric_limits< T >::epsilon() ) const;
    bool operator<( const Vector& other ) const;

    // remember kids: c_arrays are dangerous and evil!
    Vector& operator=( const T* c_array );
    T operator=( T filler );

    Vector& operator=( const Vector& other );

    // returns void to avoid 'silent' loss of precision when chaining
    template< typename U > void operator=( const Vector< M, U >& other );

    // to-homogenous-coordinates assignment operator
    // non-chainable because of sfinae
    template< size_t N >
    typename enable_if< N == M - 1 >::type*
        operator=( const Vector< N, T >& source_ );

    // from-homogenous-coordinates assignment operator
    // non-chainable because of sfinae
    template< size_t N >
    typename enable_if< N == M + 1 >::type*
        operator=( const Vector< N, T >& source_ );

    Vector operator*( const Vector& other ) const;
    Vector operator/( const Vector& other ) const;
    Vector operator+( const Vector& other ) const;
    Vector operator-( const Vector& other ) const;

    void operator*=( const Vector& other );
    void operator/=( const Vector& other );
    void operator+=( const Vector& other );
    void operator-=( const Vector& other );

    Vector operator*( const T other ) const;
    Vector operator/( const T other ) const;
    Vector operator+( const T other ) const;
    Vector operator-( const T other ) const;

    void operator*=( const T other );
    void operator/=( const T other );
    void operator+=( const T other );
    void operator-=( const T other );

    Vector operator-() const;

    const Vector& negate();

    void set( T a ); // sets all components to a;
#ifndef SWIG
    void set( const Vector< M-1, T >& v, T a );
#endif
    template< size_t N >
    void set( const Vector< N, T >& v );

    // sets the first few components to a certain value
    void set( T x, T y );
    void set( T x, T y, T z );
    void set( T x, T y, T z, T w );

    template< typename input_iterator_t >
    void iter_set( input_iterator_t begin_, input_iterator_t end_ );

    // compute the cross product of two vectors
    // note: there's also a free function:
    // vector<> cross( const vector<>, const vector<> )

    // result = vec1.cross( vec2 ) => retval result = vec1 x vec2
    template< typename TT >
    Vector cross( const Vector< M, TT >& rhs,
                  typename enable_if< M == 3, TT >::type* = 0 ) const;

    // result.cross( vec1, vec2 ) => (this) = vec1 x vec2
    template< typename TT >
    void cross( const Vector< M, TT >& a, const Vector< M, TT >& b,
                typename enable_if< M == 3, TT >::type* = 0 );


    // compute the dot product of two vectors
    // note: there's also a free function:
    // T dot( const vector<>, const vector<> );
    inline T dot( const Vector& other ) const;


    // normalize the vector
    // note: there's also a free function:
    // vector<> normalize( const vector<> );
    inline T normalize();

    //sets all vector components to random values
    //remember to set srand( seed );
    //if seed is set to -1, srand( seed ) was set outside set_random
    //otherwise srand( seed ) will be called with the given seed
    void set_random( int seed = -1 );

    inline T length() const;
    inline T squared_length() const;

    inline T distance( const Vector& other ) const;
    inline T squared_distance( const Vector& other ) const;

    /** @return the product of all elements of this vector */
    T product() const;

    template< typename TT >
    Vector< 3, T > rotate( const T theta, Vector< M, TT > axis,
                           typename enable_if< M == 3, TT >::type* = 0 ) const;

    // right hand system, CCW triangle
    // (*this) = normal of v0,v1,v2
    void compute_normal( const Vector& v0, const Vector& v1, const Vector& v2 );
    // retval = normal of (this), v1, v2
    Vector compute_normal( const Vector& v1, const Vector& v2 ) const;

    /** @return the sub vector at the given position and length. */
    template< size_t N >
    Vector< N, T >& get_sub_vector( size_t offset = 0,
        typename enable_if< M >= N >::type* = 0 );

    /** @return the sub vector at the given position and length. */
    template< size_t N >
    const Vector< N, T >& get_sub_vector( size_t offset = 0,
        typename enable_if< M >= N >::type* = 0 ) const;

    // sphere functions - sphere layout: center xyz, radius w
    template< typename TT >
    inline Vector< 3, T > project_point_onto_sphere(
        const Vector< 3, TT >& point,
        typename enable_if< M == 4, TT >::type* = 0 ) const;

    // returns a negative distance if the point lies in the sphere
    template< typename TT >
    inline T distance_to_sphere( const Vector< 3, TT >& point,
        typename enable_if< M == 4, TT >::type* = 0 ) const;

    // plane functions - plane layout; normal xyz, distance w
    template< typename TT >
    inline T distance_to_plane( const Vector< 3, TT >& point,
        typename enable_if< M == 4, TT >::type* = 0 ) const;

    template< typename TT >
    inline Vector< 3, T > project_point_onto_plane(
        const Vector< 3, TT >& point,
        typename enable_if< M == 4, TT >::type* = 0 ) const;

    // returns the index of the minimal resp. maximal value in the vector
    size_t      find_min_index() const;
    size_t      find_max_index() const;

    // returns the index of the minimal resp. maximal value in the vector
    size_t      find_abs_min_index() const;
    size_t      find_abs_max_index() const;

    // returns minimal resp. maximal value in the vector
    T&          find_min();
    T&          find_max();
    const T&    find_min() const;
    const T&    find_max() const;

    void clamp( const T& min = 0.0, const T& max = 1.0 );

    template< typename TT >
    void scale_to( Vector< M, TT >& scaled_vector,
        T min_value = -1.0, T max_value = 1.0 ) const;

    inline static size_t size(); // returns M

    bool is_unit_vector() const;

    // perturbs each component by randomly + or - the perturbation parameter
    void perturb( T perturbation = 0.0001 );

    void sqrt_elementwise();
    double norm() const; //l2 norm

    // computes the reciprocal value for each component, x = 1/x;
    // WARNING: might result in nans if division by 0!
    void reciprocal();
    // computes the reciprocal value for each component, x = 1/x;
    // checks every component for 0, sets to max value if zero.
    void reciprocal_safe();

    template< typename TT >
    void cast_from( const Vector< M, TT >& other );

    size_t nnz() const;

    // test each component of the vector for isnan and isinf
    //  inline bool is_valid() const; -> moved to class validator

    void read_from_stream( std::istream& is );
    void write_to_stream( std::ostream& os ) const;

    friend std::ostream& operator<< ( std::ostream& os, const Vector& vector_ )
    {
        const std::ios::fmtflags flags = os.flags();
        const int                prec  = os.precision();

        os.setf( std::ios::right, std::ios::adjustfield );
        os.precision( 5 );
        os << "[ ";
        for( size_t index = 0; index < M; ++index )
            os << std::setw(10) << vector_.at( index ) << " ";
        os << "]";
        os.precision( prec );
        os.setf( flags );
        return os;
    }

    T array[ M ];    //!< storage

#ifndef SWIG
    // Vector3 defaults
    static const Vector FORWARD;
    static const Vector BACKWARD;
    static const Vector UP;
    static const Vector DOWN;
    static const Vector LEFT;
    static const Vector RIGHT;

    static const Vector ONE;
    static const Vector ZERO;

    // Unit vectors
    static const Vector UNIT_X;
    static const Vector UNIT_Y;
    static const Vector UNIT_Z;
#endif

}; // class vector

//
// typedefs and statics
//
#ifndef SWIG
template< size_t M, typename T >
const Vector< M, T > Vector< M, T >::FORWARD( 0, 0, -1 );
template< size_t M, typename T >
const Vector< M, T > Vector< M, T >::BACKWARD( 0, 0, 1 );
template< size_t M, typename T >
const Vector< M, T > Vector< M, T >::UP( 0, 1, 0 );
template< size_t M, typename T >
const Vector< M, T > Vector< M, T >::DOWN( 0, -1, 0 );
template< size_t M, typename T >
const Vector< M, T > Vector< M, T >::LEFT( -1, 0, 0 );
template< size_t M, typename T >
const Vector< M, T > Vector< M, T >::RIGHT( 1, 0, 0 );
template< size_t M, typename T >

const Vector< M, T > Vector< M, T >::ONE( static_cast< T >( 1 ));
template< size_t M, typename T >
const Vector< M, T > Vector< M, T >::ZERO( static_cast< T >( 0 ));
template< size_t M, typename T >

const Vector< M, T > Vector< M, T >::UNIT_X( 1, 0, 0 );
template< size_t M, typename T >
const Vector< M, T > Vector< M, T >::UNIT_Y( 0, 1, 0 );
template< size_t M, typename T >
const Vector< M, T > Vector< M, T >::UNIT_Z( 0, 0, 1 );
#endif

#ifndef VMMLIB_NO_TYPEDEFS
#  ifdef _MSC_VER
     typedef UINT8 uint8_t;
#  endif
typedef vmml::Vector< 2, int > Vector2i;
typedef vmml::Vector< 3, int > Vector3i;
typedef vmml::Vector< 4, int > Vector4i;
typedef vmml::Vector< 2, unsigned > Vector2ui;
typedef vmml::Vector< 3, unsigned > Vector3ui;
typedef vmml::Vector< 4, unsigned > Vector4ui;
typedef vmml::Vector< 3, double > Vector3d;
typedef vmml::Vector< 4, double > Vector4d;
typedef vmml::Vector< 2, float > Vector2f;
typedef vmml::Vector< 3, float > Vector3f;
typedef vmml::Vector< 4, float > Vector4f;
typedef vmml::Vector< 3, uint8_t > Vector3ub;
typedef vmml::Vector< 4, uint8_t > Vector4ub;

#ifdef VMMLIB_OLD_TYPEDEFS
typedef Vector< 2, float > vec2f;
typedef Vector< 2, double > vec2d;
typedef Vector< 3, float > vec3f;
typedef Vector< 3, double > vec3d;
typedef Vector< 4, float > vec4f;
typedef Vector< 4, double > vec4d;
template< size_t M, typename T >
using vector = Vector<M, T>;
#endif

#endif

//
//  some free functions for convenience
//

template< size_t M, typename T >
bool equals( const Vector< M, T >& a, const Vector< M, T >& b )
{
    return a.equals( b );
}


// allows float * vector, not only vector * float
template< size_t M, typename T >
static Vector< M, T > operator* ( T factor, const Vector< M, T >& vector_ )
{
    return vector_ * factor;
}


template< size_t M, typename T >
inline T dot( const Vector< M, T >& first, const Vector< M, T >& second )
{
    return first.dot( second );
}


template< size_t M, typename T >
inline Vector< M, T > cross( const Vector< 3, T >& a, const Vector< 3, T >& b )
{
    return a.cross( b );
}


template< size_t M, typename T >
inline Vector< M, T > normalize( const Vector< M, T >& vector_ )
{
    Vector< M, T > v( vector_ );
    v.normalize();
    return v;
}

template< typename T >
inline Vector< 4, T > compute_plane( const Vector< 3, T >& a,
                                     const Vector< 3, T >& b,
                                     const Vector< 3, T >& c )
{
    const Vector< 3, T > cb = b - c;
    const Vector< 3, T > ba = a - b;

    Vector< 4, T > plane = cb.cross( ba );
    plane.normalize();
    plane.w() = -plane.x() * a.x() - plane.y() * a.y() - plane.z() * a.z();
    return plane;
}

template< size_t M, typename T >
Vector< M, T >::Vector( const T& _a )
{
    for( iterator it = begin(), it_end = end(); it != it_end; ++it )
    {
        *it = _a;
    }
}

template< size_t M, typename T >
Vector< M, T >::Vector( const T& _x, const T& _y )
{
    array[ 0 ] = _x;
    array[ 1 ] = _y;
}


template< size_t M, typename T >
Vector< M, T >::Vector( const T& _x, const T& _y, const T& _z )
{
    array[ 0 ] = _x;
    array[ 1 ] = _y;
    array[ 2 ] = _z;
}



template< size_t M, typename T >
Vector< M, T >::Vector( const T& _x, const T& _y, const T& _z, const T& _w )
{
    array[ 0 ] = _x;
    array[ 1 ] = _y;
    array[ 2 ] = _z;
    array[ 3 ] = _w;
}


template< size_t M, typename T >
Vector< M, T >::Vector( const T* values )
{
    memcpy( array, values, M * sizeof( T ));
}

#ifdef __OSG_MATH
template< size_t M, typename T >
template< typename OSGVEC3 >
Vector< M, T >::Vector( const OSGVEC3& from,
                        typename enable_if< M == 3, OSGVEC3 >::type* )
{
    array[ 0 ] = from.x();
    array[ 1 ] = from.y();
    array[ 2 ] = from.z();
}
#endif

#ifndef SWIG
template< size_t M, typename T >
// initializes the first M-1 values from vector_, the last from last_
Vector< M, T >::Vector( const Vector< M-1, T >& vector_, T last_ )
{
    typename Vector< M-1, T >::const_iterator
        it = vector_.begin(), it_end = vector_.end();

    iterator my_it = begin();

    for( ; it != it_end; ++it, ++my_it )
    {
        (*my_it) = *it;
    }
    (*my_it) = last_;
}
#endif



// to-homogenous-coordinates ctor
template< size_t M, typename T >
template< size_t N >
Vector< M, T >::Vector( const Vector< N, T >& source_,
                        typename enable_if< N == M - 1 >::type* )
{
    (*this) = source_;
}




// from-homogenous-coordinates ctor
template< size_t M, typename T >
template< size_t N >
Vector< M, T >::Vector( const Vector< N, T >& source_,
                        typename enable_if< N == M + 1 >::type* )
{
    (*this) = source_;
}


template< size_t M, typename T >
template< typename U >
Vector< M, T >::Vector( const Vector< M, U >& source_ )
{
    (*this) = source_;
}



template< size_t M, typename T > void Vector< M, T >::set( T _a )
{
    for( iterator it = begin(), it_end = end(); it != it_end; ++it )
        *it = _a;
}


#ifndef SWIG
template< size_t M, typename T >
void Vector< M, T >::set( const Vector< M-1, T >& v, T _a )
{
    memcpy( array, v.array, sizeof( T ) * (M-1) );
    at( M-1 ) = _a;
}
#endif

template< size_t M, typename T > template< size_t N >
void Vector< M, T >::set( const Vector< N, T >& v )
{
    size_t minimum = M;
    if (N < M) minimum = N;
    memcpy( array, v.array, sizeof( T ) * minimum );
}

template< size_t M, typename T >
void Vector< M, T >::set( T _x, T _y )
{
    array[ 0 ] = _x;
    array[ 1 ] = _y;
}


template< size_t M, typename T >
void Vector< M, T >::set( T _x, T _y, T _z )
{
    array[ 0 ] = _x;
    array[ 1 ] = _y;
    array[ 2 ] = _z;
}



template< size_t M, typename T >
void Vector< M, T >::set( T _x, T _y, T _z, T _w )
{
    array[ 0 ] = _x;
    array[ 1 ] = _y;
    array[ 2 ] = _z;
    array[ 3 ] = _w;
}


template< size_t M, typename T >
inline T&
Vector< M, T >::operator()( size_t index )
{
    return at( index );
}



template< size_t M, typename T >
inline const T&
Vector< M, T >::operator()( size_t index ) const
{
    return at( index );
}



template< size_t M, typename T >
inline T&
Vector< M, T >::at( size_t index )
{
    #ifdef VMMLIB_SAFE_ACCESSORS
    if ( index >= M )
    {
        VMMLIB_ERROR( "at() - index out of bounds", VMMLIB_HERE );
    }
    #endif
    return array[ index ];
}



template< size_t M, typename T >
inline const T&
Vector< M, T >::at( size_t index ) const
{
    #ifdef VMMLIB_SAFE_ACCESSORS
    if ( index >= M )
    {
        VMMLIB_ERROR( "at() - index out of bounds", VMMLIB_HERE );
    }
    #endif
    return array[ index ];
}


#ifndef VMMLIB_NO_CONVERSION_OPERATORS

template< size_t M, typename T >
Vector< M, T >::operator T*()
{
    return array;
}



template< size_t M, typename T >
Vector< M, T >::operator const T*() const
{
    return array;
}
#else

template< size_t M, typename T >
T&
Vector< M, T >::operator[]( size_t index )
{
    return at( index );
}

template< size_t M, typename T >
const T&
Vector< M, T >::operator[]( size_t index ) const
{
    return at( index );
}


#endif


#if 0
template< size_t M, typename T >
inline T&
Vector< M, T >::operator[]( size_t index )
{
    return at( index );
}



template< size_t M, typename T >
inline const T&
Vector< M, T >::operator[]( size_t index ) const
{
    return at( index );
}
#endif


template< size_t M, typename T >
Vector< M, T >
Vector< M, T >::operator*( const Vector< M, T >& other ) const
{
    Vector< M, T > result;
    for( size_t index = 0; index < M; ++index )
        result.at( index ) = at( index ) * other.at( index );
    return result;
}



template< size_t M, typename T >
Vector< M, T >
Vector< M, T >::operator/( const Vector< M, T >& other ) const
{
    Vector< M, T > result;
    for( size_t index = 0; index < M; ++index )
        result.at( index ) = at( index ) / other.at( index );
    return result;
}



template< size_t M, typename T >
Vector< M, T >
Vector< M, T >::operator+( const Vector< M, T >& other ) const
{
    Vector< M, T > result;
    for( size_t index = 0; index < M; ++index )
        result.at( index ) = at( index ) + other.at( index );
    return result;
}



template< size_t M, typename T >
Vector< M, T >
Vector< M, T >::operator-( const Vector< M, T >& other ) const
{
    Vector< M, T > result;
    for( size_t index = 0; index < M; ++index )
        result.at( index ) = at( index ) - other.at( index );
    return result;
}




template< size_t M, typename T >
void
Vector< M, T >::operator*=( const Vector< M, T >& other )
{
    for( size_t index = 0; index < M; ++index )
        at( index ) *= other.at( index );
}



template< size_t M, typename T >
void
Vector< M, T >::operator/=( const Vector< M, T >& other )
{
    for( size_t index = 0; index < M; ++index )
        at( index ) /= other.at( index );
}



template< size_t M, typename T >
void
Vector< M, T >::operator+=( const Vector< M, T >& other )
{
    for( size_t index = 0; index < M; ++index )
        at( index ) += other.at( index );
}



template< size_t M, typename T >
void
Vector< M, T >::operator-=( const Vector< M, T >& other )
{
    for( size_t index = 0; index < M; ++index )
        at( index ) -= other.at( index );
}



template< size_t M, typename T >
Vector< M, T >
Vector< M, T >::operator*( const T other ) const
{
    Vector< M, T > result;
    for( size_t index = 0; index < M; ++index )
        result.at( index ) = at( index ) * other;
    return result;
}



template< size_t M, typename T >
Vector< M, T >
Vector< M, T >::operator/( const T other ) const
{
    Vector< M, T > result;
    for( size_t index = 0; index < M; ++index )
        result.at( index ) = at( index ) / other;
    return result;
}



template< size_t M, typename T >
Vector< M, T >
Vector< M, T >::operator+( const T other ) const
{
    Vector< M, T > result;
    for( size_t index = 0; index < M; ++index )
        result.at( index ) = at( index ) + other;
    return result;
}



template< size_t M, typename T >
Vector< M, T >
Vector< M, T >::operator-( const T other ) const
{
    Vector< M, T > result;
    for( size_t index = 0; index < M; ++index )
        result.at( index ) = at( index ) - other;
    return result;
}




template< size_t M, typename T >
void
Vector< M, T >::operator*=( const T other )
{
    for( size_t index = 0; index < M; ++index )
        at( index ) *= other;
}



template< size_t M, typename T >
void
Vector< M, T >::operator/=( const T other )
{
    for( size_t index = 0; index < M; ++index )
        at( index ) /= other;
}



template< size_t M, typename T >
void
Vector< M, T >::operator+=( const T other )
{
    for( size_t index = 0; index < M; ++index )
        at( index ) += other;
}



template< size_t M, typename T >
void
Vector< M, T >::operator-=( const T other )
{
    for( size_t index = 0; index < M; ++index )
        at( index ) -= other;
}



template< size_t M, typename T >
Vector< M, T >
Vector< M, T >::operator-() const
{
    Vector< M, T > v( *this );
    return v.negate();
}



template< size_t M, typename T >
const Vector< M, T >&
Vector< M, T >::negate()
{
    for( size_t index = 0; index < M; ++index )
        array[ index ] = -array[ index ];
    return *this;
}



template< size_t M, typename T >
inline T&
Vector< M, T >::x()
{
    return array[ 0 ];
}



template< size_t M, typename T >
inline T&
Vector< M, T >::y()
{
    return array[ 1 ];
}



template< size_t M, typename T >
inline T&
Vector< M, T >::z()
{
    return array[ 2 ];
}



template< size_t M, typename T >
inline T&
Vector< M, T >::w()
{
    return array[ 3 ];
}



template< size_t M, typename T >
inline const T&
Vector< M, T >::x() const
{
    return array[ 0 ];
}



template< size_t M, typename T >
inline const T&
Vector< M, T >::y() const
{
    return array[ 1 ];
}



template< size_t M, typename T >
inline const T&
Vector< M, T >::z() const
{
    return array[ 2 ];
}



template< size_t M, typename T >
inline const T&
Vector< M, T >::w() const
{
    return array[ 3 ];
}


template< size_t M, typename T >
inline T&
Vector< M, T >::r()
{
    return array[ 0 ];
}



template< size_t M, typename T >
inline T&
Vector< M, T >::g()
{
    return array[ 1 ];
}



template< size_t M, typename T >
inline T&
Vector< M, T >::b()
{
    return array[ 2 ];
}



template< size_t M, typename T >
inline T&
Vector< M, T >::a()
{
    return array[ 3 ];
}



template< size_t M, typename T >
inline const T&
Vector< M, T >::r() const
{
    return array[ 0 ];
}



template< size_t M, typename T >
inline const T&
Vector< M, T >::g() const
{
    return array[ 1 ];
}



template< size_t M, typename T >
inline const T&
Vector< M, T >::b() const
{
    return array[ 2 ];
}



template< size_t M, typename T >
inline const T&
Vector< M, T >::a() const
{
    return array[ 3 ];
}

// result = vec1.cross( vec2 ) => result = vec1 x vec2
template< size_t M, typename T >
template< typename TT >
inline Vector< M, T > Vector< M, T >::cross( const Vector< M, TT >& rhs,
                                 typename enable_if< M == 3, TT >::type* ) const
{
    Vector< M, T > result;
    result.cross( *this, rhs );
    return result;
}



// result.cross( vec1, vec2 ) => (this) = vec1 x vec2
template< size_t M, typename T >
template< typename TT >
void Vector< M, T >::cross( const Vector< M, TT >& aa,
                            const Vector< M, TT >& bb,
                            typename enable_if< M == 3, TT >::type* )
{
    array[ 0 ] = aa.y() * bb.z() - aa.z() * bb.y();
    array[ 1 ] = aa.z() * bb.x() - aa.x() * bb.z();
    array[ 2 ] = aa.x() * bb.y() - aa.y() * bb.x();
}



template< size_t M, typename T >
inline T Vector< M, T >::dot( const Vector< M, T >& other ) const
{
    T tmp = 0.0;
    for( size_t index = 0; index < M; ++index )
        tmp += at( index ) * other.at( index );

    return tmp;
}


template< size_t M, typename T >
inline T Vector< M, T >::normalize()
{
    const T len = length();

    if ( len == 0 )
        return 0;

    const T tmp = 1.0 / len;
    (*this) *= tmp;
    return len;
}

template< size_t M, typename T >
inline T Vector< M, T >::length() const
{
    return std::sqrt( squared_length() );
}

template< size_t M, typename T >
inline T Vector< M, T >::squared_length() const
{
    T _squared_length = 0.0;
    for( const_iterator it = begin(), it_end = end(); it != it_end; ++it )
        _squared_length += (*it) * (*it);

    return _squared_length;
}



template< size_t M, typename T >
inline T
Vector< M, T >::distance( const Vector< M, T >& other ) const
{
    return std::sqrt( squared_distance( other ) );
}



template< size_t M, typename T >
inline T Vector< M, T >::squared_distance( const Vector< M, T >& other ) const
{
    Vector< M, T > tmp( *this );
    tmp -= other;
    return tmp.squared_length();
}

template< size_t M, typename T > inline T Vector< M, T >::product() const
{
    T result = at( 0 );
    for( size_t i = 1; i < M; ++i )
        result *= at( i );
    return result;
}

template< size_t M, typename T >
void Vector< M, T >::compute_normal( const Vector< M, T >& aa,
                                     const Vector< M, T >& bb,
                                     const Vector< M, T >& cc )
{
    Vector< M, T > u,v;
    // right hand system, CCW triangle
    u = bb - aa;
    v = cc - aa;
    cross( u, v );
    normalize();
}



template< size_t M, typename T >
Vector< M, T >
Vector< M, T >::compute_normal( const Vector< M, T >& bb,
                                const Vector< M, T >& cc ) const
{
    Vector< M, T > tmp;
    tmp.compute_normal( *this, bb, cc);
    return tmp;
}

template< size_t M, typename T >
template< typename TT >
Vector< 3, T > Vector< M, T >::rotate( const T theta, Vector< M, TT > axis,
            typename enable_if< M == 3, TT >::type* ) const
{
    axis.normalize();

    const T costheta = std::cos( theta );
    const T sintheta = std::sin( theta );

    return Vector< 3, T >(
        (costheta + ( 1.0f - costheta ) * axis.x() * axis.x() ) * x()    +
        (( 1 - costheta ) * axis.x() * axis.y() - axis.z() * sintheta ) * y() +
        (( 1 - costheta ) * axis.x() * axis.z() + axis.y() * sintheta ) * z(),

        (( 1 - costheta ) * axis.x() * axis.y() + axis.z() * sintheta ) * x() +
        ( costheta + ( 1 - costheta ) * axis.y() * axis.y() ) * y() +
        (( 1 - costheta ) * axis.y() * axis.z() - axis.x() * sintheta ) * z(),

        (( 1 - costheta ) * axis.x() * axis.z() - axis.y() * sintheta ) * x() +
        (( 1 - costheta ) * axis.y() * axis.z() + axis.x() * sintheta ) * y() +
        ( costheta + ( 1 - costheta ) * axis.z() * axis.z() ) * z() );
}


// sphere layout: center xyz, radius w
template< size_t M, typename T >
template< typename TT >
inline Vector< 3, T >
Vector< M, T >::
project_point_onto_sphere( const Vector< 3, TT >& point,
    typename enable_if< M == 4, TT >::type* ) const
{
    const Vector< 3, T >& _center = get_sub_vector< 3 >( 0 );

    Vector< 3, T > projected_point( point );
    projected_point -= _center;
    projected_point.normalize();
    projected_point *= w();
    return _center + projected_point;
}



// sphere layout: center xyz, radius w
template< size_t M, typename T >
template< typename TT >
inline T
Vector< M, T >::
distance_to_sphere( const Vector< 3, TT >& point,
    typename enable_if< M == 4, TT >::type* ) const
{
    const Vector< 3, T >& center_ = get_sub_vector< 3 >( 0 );
    return ( point - center_ ).length() - w();
}

template< size_t M, typename T > template< size_t N > inline
Vector< N, T >& Vector< M, T >::get_sub_vector( size_t offset,
                                           typename enable_if< M >= N >::type* )
{
    assert( offset <= M - N );
    return reinterpret_cast< Vector< N, T >& >( *( begin() + offset ) );
}

template< size_t M, typename T > template< size_t N > inline
const Vector< N, T >& Vector< M, T >::get_sub_vector( size_t offset,
                                     typename enable_if< M >= N >::type* ) const
{
    assert( offset <= M - N );
    return reinterpret_cast< const Vector< N, T >& >( *( begin() + offset ) );
}


// plane: normal xyz, distance w
template< size_t M, typename T > template< typename TT >
inline T Vector< M, T >::distance_to_plane( const Vector< 3, TT >& point,
    typename enable_if< M == 4, TT >::type* ) const
{
    const Vector< 3, T >& normal = get_sub_vector< 3 >( 0 );
    return normal.dot( point ) + w();
}



// plane: normal xyz, distance w
template< size_t M, typename T >
template< typename TT >
Vector< 3, T >
Vector< M, T >::project_point_onto_plane( const Vector< 3, TT >& point,
    typename enable_if< M == 4, TT >::type* ) const
{
    const Vector< 3, T >& normal = get_sub_vector< 3 >( 0 );
    return point - ( normal * distance_to_plane( point ) );
}



template< size_t M, typename T >
bool Vector< M, T >::operator==( const Vector< M, T >& other ) const
{
    return memcmp( array, other.array, sizeof( array )) == 0;
}


template< size_t M, typename T >
bool Vector< M, T >::operator!=( const Vector< M, T >& other ) const
{
    return ! this->operator==( other );
}


template< size_t M, typename T >
bool
Vector< M, T >::equals( const Vector< M, T >& other, T tolerance ) const
{
    for( size_t index = 0; index < M; ++index )
        if( fabs( at( index ) - other( index ) ) >= tolerance )
            return false;
    return true;

}


template< size_t M, typename T >
bool
Vector< M, T >::operator<( const Vector< M, T >& other ) const
{
    for(size_t index = 0; index < M; ++index )
    {
        if (at( index ) < other.at( index )) return true;
        if (other.at( index ) < at( index )) return false;
    }
    return false;
}


// to-homogenous-coordinates assignment operator
// non-chainable because of sfinae
template< size_t M, typename T > template< size_t N >
typename enable_if< N == M - 1 >::type*
Vector< M, T >::operator=( const Vector< N, T >& source_ )
{
    std::copy( source_.begin(), source_.end(), begin() );
    at( M - 1 ) = static_cast< T >( 1.0 );
    return 0;
}


// from-homogenous-coordinates assignment operator
// non-chainable because of sfinae
template< size_t M, typename T > template< size_t N >
typename enable_if< N == M + 1 >::type*
Vector< M, T >::operator=( const Vector< N, T >& source_ )
{
    const T w_reci = static_cast< T >( 1.0 ) / source_( M );
    iterator it = begin(), it_end = end();
    for( size_t index = 0; it != it_end; ++it, ++index )
        *it = source_( index ) * w_reci;
    return 0;
}


template< size_t M, typename T >
Vector< M, T >& Vector< M, T >::operator=( const T* c_array )
{
    iter_set( c_array, c_array + M );
    return *this;
}



template< size_t M, typename T >
T Vector< M, T >::operator=( T filler_value )
{
    for( size_t index = 0; index < M; ++index )
        at( index ) = filler_value;
    return filler_value;
}




template< size_t M, typename T >
Vector< M, T >& Vector< M, T >::operator=( const Vector< M, T >& other )
{
    if( this != &other )
        memcpy( array, other.array, M * sizeof( T ) );
    return *this;
}



// returns void to avoid 'silent' loss of precision when chaining
template< size_t M, typename T > template< typename U >
void Vector< M, T >::operator=( const Vector< M, U >& source_ )
{
    typedef typename Vector< M, U >::const_iterator u_c_iter;
    u_c_iter it = source_.begin(), it_end = source_.end();
    for( iterator my_it = begin(); it != it_end; ++it, ++my_it )
        *my_it = static_cast< T >( *it );
}



template< size_t M, typename T >
template< typename input_iterator_t >
void
Vector< M, T >::iter_set( input_iterator_t begin_, input_iterator_t end_ )
{
    input_iterator_t in_it = begin_;
    iterator it = begin(), it_end = end();
    for( ; it != it_end && in_it != end_; ++it, ++in_it )
        (*it) = static_cast< T >( *in_it );
}

template< size_t M, typename T >
void Vector< M, T >::clamp( const T& min, const T& max )
{
    for( size_t i = 0; i < M; ++i )
    {
        if( array[i] < min )
            array[i] = min;
        if( array[i] > max )
            array[i] = max;
    }
}



template< size_t M, typename T >
template< typename TT >
void
Vector< M, T >::scale_to( Vector< M, TT >& result_,
    T min_value, T max_value ) const
{
    T range       = max_value-min_value;
    T half_range  = range * 0.5;
    T scale       = ( 1.0 / range ) * static_cast< T >( std::numeric_limits< TT >::max() );

    for( size_t index = 0; index < M; ++index )
    {
        result_.at( index )
            = static_cast< TT >( ( at( index ) + half_range ) * scale );
    }

}



template< size_t M, typename T >
inline size_t
Vector< M, T >::size()
{
    return M;
}



template< size_t M, typename T >
size_t
Vector< M, T >::find_min_index() const
{
    return std::min_element( begin(), end() ) - begin();
}



template< size_t M, typename T >
size_t
Vector< M, T >::find_max_index() const
{
    return std::max_element( begin(), end() ) - begin();
}



template< size_t M, typename T >
size_t
Vector< M, T >::find_abs_min_index() const
{
    return std::min_element( begin(), end(), vmml::math::abs_less< T >() ) - begin();
}



template< size_t M, typename T >
size_t
Vector< M, T >::find_abs_max_index() const
{
    return std::max_element( begin(), end(), vmml::math::abs_greater< T >() ) - begin();
}



template< size_t M, typename T >
T&
Vector< M, T >::find_min()
{
    return *std::min_element( begin(), end() );
}



template< size_t M, typename T >
const T&
Vector< M, T >::find_min() const
{
    return *std::min_element( begin(), end() );
}



template< size_t M, typename T >
T&
Vector< M, T >::find_max()
{
    return *std::max_element( begin(), end() );
}



template< size_t M, typename T >
const T&
Vector< M, T >::find_max() const
{
    return *std::max_element( begin(), end() );
}


template< size_t M, typename T >
inline typename Vector< M, T >::iterator
Vector< M, T >::begin()
{
    return array;
}


template< size_t M, typename T >
inline typename Vector< M, T >::iterator
Vector< M, T >::end()
{
    return array + M; ;
}


template< size_t M, typename T >
inline typename Vector< M, T >::const_iterator
Vector< M, T >::begin() const
{
    return array;
}


template< size_t M, typename T >
inline typename Vector< M, T >::const_iterator
Vector< M, T >::end() const
{
    return array + M; ;
}



template< size_t M, typename T >
inline typename Vector< M, T >::reverse_iterator
Vector< M, T >::rbegin()
{
    return array + M - 1;
}


template< size_t M, typename T >
inline typename Vector< M, T >::reverse_iterator
Vector< M, T >::rend()
{
    return array - 1;
}


template< size_t M, typename T >
inline typename Vector< M, T >::const_reverse_iterator
Vector< M, T >::rbegin() const
{
    return array + M - 1;
}


template< size_t M, typename T >
inline typename Vector< M, T >::const_reverse_iterator
Vector< M, T >::rend() const
{
    return array - 1;
}



template< size_t M, typename T >
bool
Vector< M, T >::is_unit_vector() const
{
    const_iterator it = begin(), it_end = end();
    bool one = false;
    for( ; it != it_end; ++it )
    {
        if ( *it == 1.0 )
        {
            if ( one )
                return false;
            one = true;
        }
        else if ( *it != 0.0 )
        {
            return false;
        }
    }
    return one;
}




template< size_t M, typename T >
void
Vector< M, T >::perturb( T perturbation )
{
    for( iterator it = begin(), it_end = end(); it != it_end; ++it )
    {
        (*it) += ( rand() & 1u ) ? perturbation : -perturbation;
    }

}

template< size_t M, typename T >
void
Vector< M, T >::sqrt_elementwise()
{
    for( iterator it = begin(), it_end = end(); it != it_end; ++it )
    {
        (*it) = std::sqrt(*it);
    }
}



template< size_t M, typename T >
void
Vector< M, T >::reciprocal()
{
    for( iterator it = begin(), it_end = end(); it != it_end; ++it )
    {
        (*it) = static_cast< T >( 1.0 ) / (*it);
    }
}



template< size_t M, typename T >
void
Vector< M, T >::reciprocal_safe()
{
    for( iterator it = begin(), it_end = end(); it != it_end; ++it )
    {
        T& v = *it;

        if ( v == static_cast< T >( 0 ) )
            v = std::numeric_limits< T >::max();
        else
            v = static_cast< T >( 1.0 ) / v;
    }
}



template< size_t M, typename T >
template< typename TT >
void
Vector< M, T >::cast_from( const Vector< M, TT >& other )
{
    typedef vmml::Vector< M, TT > vector_tt_type ;
    typedef typename vector_tt_type::const_iterator tt_const_iterator;

    iterator it = begin(), it_end = end();
    tt_const_iterator other_it = other.begin();
    for( ; it != it_end; ++it, ++other_it )
    {
        *it = static_cast< T >( *other_it );
    }
}

template< size_t M, typename T >
size_t
Vector< M, T >::nnz() const
{
    size_t counter = 0;

    const_iterator  it = begin(),
    it_end = end();
    for( ; it != it_end; ++it)
    {
        if ( *it != 0 ) {
            ++counter;
        }
    }

    return counter;
}

template< size_t M, typename T >
void
Vector< M, T >::read_from_stream( std::istream& is )
{
    for( size_t i = 0; i < M; ++i )
    {
        is >> at( i );
    }
}

template< size_t M, typename T >
void
Vector< M, T >::write_to_stream( std::ostream& os ) const
{
    for( size_t i = 0; i < M; ++i )
    {
         os << at( i ) << " ";
    }
}

template< size_t M, typename T >
double
Vector< M, T >::norm( ) const
{
    double norm_v = 0.0;

    const_iterator it = begin(), it_end = end();
    for( ; it != it_end; ++it )
    {
        norm_v += *it * *it;
    }

    return std::sqrt(norm_v);
}

template< size_t M, typename T >
void
Vector< M, T >::set_random( int seed )
{
    if ( seed >= 0 )
        srand( seed );

    for( size_t i = 0; i < M; ++i )
    {
        const double fillValue = double( rand( )) / double( RAND_MAX );
        at( i ) = -1.0 + 2.0 * fillValue;
    }
}


} // namespace vmml

#endif
