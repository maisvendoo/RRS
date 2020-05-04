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

#ifndef VMMLIB__QUATERNION__HPP
#define VMMLIB__QUATERNION__HPP

#include <vmmlib/vector.hpp>
#include <vmmlib/matrix.hpp>
#include <vmmlib/math.hpp>
#include <vmmlib/enable_if.hpp>

#include <vmmlib/exception.hpp>
#include <vmmlib/vmmlib_config.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <limits>


// - declaration - //

#define QUATERNION_TRACE_EPSILON 1e-5

namespace vmml
{

template < typename T >
class Quaternion : private Vector< 4, T >
{
public:
    typedef Vector< 4, T >  super;

    using super::operator();
    //using super::operator=;
    using super::at;
    using super::x;
    using super::y;
    using super::z;
    using super::w;
    using super::array;
    using super::find_min;
    using super::find_max;
    using super::find_min_index;
    using super::find_max_index;
    using super::iter_set;

    //constructors
    Quaternion() { w() = 1.; }
    Quaternion( T x, T y, T z, T w );

    Quaternion( const Vector< 3, T >& xyz , T w );
    // initializes the quaternion with xyz, sets w to zero
    Quaternion( const Vector< 3, T >& xyz );

    // uses the top-left 3x3 part of the supplied matrix as rotation matrix
    template< size_t M >
    Quaternion( const Matrix< M, M, T >& rotation_matrix_,
        typename enable_if< M >= 3 >::type* = 0 );

    void zero();
    void identity();

    template< size_t D > void set( const Matrix< D, D, T >& rotation_matrix_ );

    void set( T ww, T xx, T yy, T zz);
    void set( Vector< 3, T >& xyz, T w );

    template< typename input_iterator_t >
    void set( input_iterator_t begin_, input_iterator_t end_ );

    bool operator==( const T& a ) const;
    bool operator!=( const T& a ) const;

    bool operator==( const Quaternion& rhs ) const;
    bool operator!=( const Quaternion& rhs ) const;

    bool operator==( const Vector< 4, T >& a ) const;
    bool operator!=( const Vector< 4, T >& a ) const;

    bool is_akin( const Quaternion& a,
                 const T& delta = std::numeric_limits< T >::epsilon() );

    void conjugate();
    Quaternion get_conjugate() const;

    T abs() const;
    T squared_abs() const;

    T normalize();
    Quaternion get_normalized() const;

    Quaternion negate() const;
    Quaternion operator-() const;

    Quaternion& operator=(const Quaternion& other);
    const Vector< 4, T >& operator=( const Vector< 4, T >& other );

    //
    // quaternion/quaternion operations
    //
    Quaternion operator+( const Quaternion< T >& rhs ) const;
    Quaternion operator-( const Quaternion< T >& rhs ) const;
    // caution: a * q != q * a in general
    Quaternion operator*( const Quaternion< T >& rhs ) const;
    void operator+=( const Quaternion< T >& rhs );
    void operator-=( const Quaternion< T >& rhs );
    // caution: a *= q != q *= a in general
    void operator*=( const Quaternion< T >& rhs );

    //
    // quaternion/scalar operations
    //
    Quaternion operator*( T a ) const;
    Quaternion operator/( T a ) const;

    void operator*=( T a );
    void operator/=( T a );

    //
    //quaternion/vector operations
    //
    Quaternion operator+( const Vector< 3, T >& a ) const;
    Quaternion operator-( const Vector< 3, T >& a ) const;
    Quaternion operator*( const Vector< 3, T >& a ) const;

    void operator+=( const Vector< 3, T >& a );
    void operator-=( const Vector< 3, T >& a );
    void operator*=( const Vector< 3, T >& a );

    // vec3 = this x b
    Vector< 3, T > cross( const Quaternion< T >& b ) const;

    T dot( const Quaternion< T >& a ) const;
    static T dot( const Quaternion< T >& a, const Quaternion< T >& b );

    // returns multiplicative inverse
    Quaternion inverse();

    void normal( const Quaternion& aa, const Quaternion& bb, const Quaternion& cc,  const Quaternion& dd );
    Quaternion normal( const Quaternion& aa, const Quaternion& bb, const Quaternion& cc );

    // to combine two rotations, multiply the respective quaternions before using rotate
    // instead of rotating twice for increased performance, but be aware of non-commutativity!
    void rotate( T theta, const Vector< 3, T >& a );
    Quaternion rotate( T theta, Vector< 3, T >& axis, const Vector< 3, T >& a );
    Quaternion rotate_x( T theta, const Vector< 3, T >& a );
    Quaternion rotate_y( T theta, const Vector< 3, T >& a );
    Quaternion rotate_z( T theta, const Vector< 3, T >& a );

    static Quaternion slerp( T a, const Quaternion& p,
        const Quaternion& q, const T epsilon = 1e-13 );

    Matrix< 3, 3, T > get_rotation_matrix() const;

    template< size_t D > void get_rotation_matrix( Matrix< D, D, T >& result ) const;

    friend std::ostream& operator<< ( std::ostream& os, const Quaternion& q )
    {
        os << "(";
        size_t index = 0;
        for( ; index < 3; ++index )
        {
            os << q.at( index ) << ", ";
        }
        os << q.at( index ) << ") ";
        return os;
    };

    static const Quaternion IDENTITY;
    static const Quaternion QUATERI;
    static const Quaternion QUATERJ;
    static const Quaternion QUATERK;

}; // class quaternion

#ifndef VMMLIB_NO_TYPEDEFS
typedef Quaternion< float >  Quaternionf;
typedef Quaternion< double > Quaterniond;

#ifdef VMMLIB_OLD_TYPEDEFS
typedef Quaternion< float >  quaternionf;
typedef Quaternion< double > quaterniond;
template< typename T >
using quaternion = Quaternion<T>;
#endif

#endif

// - implementation - //

template < typename T >
const Quaternion< T > Quaternion< T >::IDENTITY( 0, 0, 0, 1 );

template < typename T >
const Quaternion< T > Quaternion< T >::QUATERI( 1, 0, 0, 0 );

template < typename T >
const Quaternion< T > Quaternion< T >::QUATERJ( 0, 1, 0, 0 );

template < typename T >
const Quaternion< T > Quaternion< T >::QUATERK( 0, 0, 1, 0 );

template < typename T >
Quaternion< T >::Quaternion( T x_, T y_, T z_, T w_ )
{
    x() = x_;
    y() = y_;
    z() = z_;
    w() = w_;
}



template < typename T >
Quaternion< T >::Quaternion( const Vector< 3, T >& xyz, T w_ )
{
    super::set( xyz, w_ );
}




template < typename T >
Quaternion< T >::Quaternion( const Vector< 3, T >& xyz )
{
    super::set( xyz, static_cast< T >( 0.0 ) );
}



template< typename T > template< size_t M >
Quaternion< T >::Quaternion( const Matrix< M, M, T >& rotation_matrix_,
                             typename enable_if< M >= 3 >::type* )
{
    this->template set< M >( rotation_matrix_ );
}



 // top-left 3x3 is interpreted as rot matrix.
template < typename T > template< size_t D >
void Quaternion< T >::set( const Matrix< D, D, T >& M )
{
    T trace = M( 0, 0 ) + M( 1, 1 ) + M( 2,2 ) + 1.0;

    // very small traces may introduce a big numerical error
    if( trace > QUATERNION_TRACE_EPSILON )
    {
        T s = 0.5 / sqrt( trace );
        x() = M( 2, 1 ) - M( 1, 2 );
        x() *= s;

        y() = M( 0, 2 ) - M( 2, 0 );
        y() *= s;

        z() = M( 1, 0 ) - M( 0, 1 );
        z() *= s;

        w() = 0.25 / s;
    }
    else
    {
        Vector< 3, T > diag( M( 0, 0 ), M( 1, 1 ), M( 2, 2 ) );
        size_t largest = diag.find_max_index();

        // 0, 0 is largest
        if ( largest == 0 )
        {
            T s = 0.5 / sqrt( 1.0 + M( 0, 0 ) - M( 1, 1 ) - M( 2, 2 ) );
            x() = 0.25 / s;

            y() = M( 0,1 ) + M( 1,0 );
            y() *= s;

            z() = M( 0,2 ) + M( 2,0 );
            z() *= s;

            w() = M( 1,2 ) - M( 2,1 );
            w() *= s;
        }
        else if ( largest == 1 )
        {
            T s = 0.5 / sqrt( 1.0 + M( 1,1 ) - M( 0,0 ) - M( 2,2 ) );
            x() = M( 0,1 ) + M( 1,0 );
            x() *= s;

            y() = 0.25 / s;

            z() = M( 1,2 ) + M( 2,1 );
            z() *= s;

            w() = M( 0,2 ) - M( 2,0 );
            w() *= s;
        }
        // 2, 2 is largest
        else if ( largest == 2 )
        {
            T s = 0.5 / sqrt( 1.0 + M( 2,2 ) - M( 0,0 ) - M( 1,1 ) );
            x() = M( 0,2 ) + M( 2,0 );
            x() *= s;

            y() = M( 1,2 ) + M( 2,1 );
            y() *= s;

            z() = 0.25 / s;

            w() = M( 0,1 ) - M( 1,0 );
            w() *= s;
        }
        else
        {
            (*this) = super::ZERO;
            assert( 0 );
        }
    }
}



template < typename T >
void Quaternion< T >::zero()
{
    (*this) = super::ZERO;
}



template < typename T >
void Quaternion< T >::identity()
{
    (*this) = IDENTITY;
}



template < typename T >
void Quaternion< T >::set( T xx, T yy, T zz, T ww )
{
    x() = xx;
    y() = yy;
    z() = zz;
    w() = ww;
}



template< typename T >
void Quaternion< T >::set( Vector< 3, T >& xyz, T _w )
{
    x() = xyz.x();
    y() = xyz.y();
    z() = xyz.z();
    w() = _w;
}



template < typename T >
template< typename input_iterator_t >
void Quaternion< T >::set( input_iterator_t begin_, input_iterator_t end_ )
{
    super::template set< input_iterator_t >( begin_, end_ );
}



template < typename T >
bool Quaternion< T >::operator==( const T& a ) const
{
    return ( w() == a && x() == 0 && y() == 0 && z() == 0 );
}



template < typename T >
bool Quaternion< T >::operator!=( const T& a ) const
{
    return ( w() != a || x() != 0 || y() != 0 || z() != 0 );
}


template < typename T >
bool Quaternion< T >::operator==( const Vector< 4, T >& a ) const
{
    return this->operator==(
        reinterpret_cast< const Quaternion< T >& >( a )
            );
}


template < typename T >
bool
Quaternion< T >::operator!=( const Vector< 4, T >& a ) const
{
    return ! this->operator==( a );
}



template < typename T >
bool
Quaternion< T >::operator==( const Quaternion& rhs ) const
{
    return (
        w() == rhs.w() &&
        x() == rhs.x() &&
        y() == rhs.y() &&
        z() == rhs.z()
        );
}



template < typename T >
bool
Quaternion< T >::operator!=( const Quaternion& rhs ) const
{
    return ! this->operator==( rhs );
}



template < typename T >
bool
Quaternion< T >::is_akin( const Quaternion& a, const T& delta )
{
    if( fabsf( w() - a.w() ) > delta ||
        fabsf( x() - a.x() ) > delta ||
        fabsf( y() - a.y() ) > delta ||
        fabsf( z() - a.z() ) > delta
        )
        return false;
    return true;
}



template < typename T >
void Quaternion< T >::conjugate()
{
    x() = -x();
    y() = -y();
    z() = -z();
}



template < typename T >
Quaternion< T > Quaternion< T >::get_conjugate() const
{
    return Quaternion< T > ( -x(), -y(), -z(), w() );
}



template < typename T >
T
Quaternion< T >::abs() const
{
    return sqrt( squared_abs() );
}



template < typename T >
T Quaternion< T >::squared_abs() const
{
    return x() * x() + y() * y() + z() * z() + w() * w();
}



template < typename T >
Quaternion< T > Quaternion< T >::inverse()
{
    Quaternion< T > q( *this );
    q.conjugate();

    T tmp = squared_abs();
    tmp = static_cast< T >( 1.0 ) / tmp;
    return q * tmp;
}



template < typename T >
T Quaternion< T >::normalize()
{
    T len = abs();
    if( len == 0.0 )
        return 0.0;
    len = 1.0f / len;
    this->operator*=( len );
    return len;
}



template < typename T >
Quaternion< T >
Quaternion< T >::get_normalized() const
{
    Quaternion< T > q( *this );
    q.normalize();
    return q;
}



//
// quaternion/quaternion operations
//

template < typename T >
Quaternion< T >
Quaternion< T >::operator+( const Quaternion< T >& rhs ) const
{
    return Quaternion( x() + rhs.x(), y() + rhs.y(), z() + rhs.z(), w() + rhs.w() );
}



template < typename T >
Quaternion< T >
Quaternion< T >::operator-( const Quaternion< T >& rhs ) const
{
    return Quaternion( x() - rhs.x(), y() - rhs.y(), z() - rhs.z(), w() - rhs.w() );
}



// returns Grasssmann product
template < typename T >
Quaternion< T >
Quaternion< T >::operator*( const Quaternion< T >& rhs ) const
{
    Quaternion< T > ret( *this );
    ret *= rhs;
    return ret;
}



// Grassmann product
template < typename T >
void Quaternion< T >::operator*=( const Quaternion< T >& q )
{
    #if 0
    Quaternion< T > orig( *this );
    x() = orig.w() * a.x() + orig.x() * a.w() + orig.y() * a.z() - orig.z() * a.y();
    y() = orig.w() * a.y() + orig.y() * a.w() + orig.z() * a.x() - orig.x() * a.z();
    z() = orig.w() * a.z() + orig.z() * a.w() + orig.x() * a.y() - orig.y() * a.x();
    w() = orig.w() * a.w() - orig.x() * a.x() - orig.y() * a.y() - orig.z() * a.z();
    #else

    // optimized version, 7 less mul, but 15 more add/subs
    // after Henrik Engstrom, from a gamedev.net article.

    T* _array = super::array;

    const T& a_ = _array[ 3 ];
    const T& b_ = _array[ 0 ];
    const T& c = _array[ 1 ];
    const T& d = _array[ 2 ];
    const T& _x = q.array[ 3 ];
    const T& _y = q.array[ 0 ];
    const T& _z = q.array[ 1 ];
    const T& _w = q.array[ 2 ];

    const T tmp_00 = (d - c) * (_z - _w);
    const T tmp_01 = (a_ + b_) * (_x + _y);
    const T tmp_02 = (a_ - b_) * (_z + _w);
    const T tmp_03 = (c + d) * (_x - _y);
    const T tmp_04 = (d - b_) * (_y - _z);
    const T tmp_05 = (d + b_) * (_y + _z);
    const T tmp_06 = (a_ + c) * (_x - _w);
    const T tmp_07 = (a_ - c) * (_x + _w);
    const T tmp_08 = tmp_05 + tmp_06 + tmp_07;
    const T tmp_09 = 0.5 * (tmp_04 + tmp_08);

    _array[ 3 ] = tmp_00 + tmp_09 - tmp_05;
    _array[ 0 ] = tmp_01 + tmp_09 - tmp_08;
    _array[ 1 ] = tmp_02 + tmp_09 - tmp_07;
    _array[ 2 ] = tmp_03 + tmp_09 - tmp_06;

    #endif
}





template < typename T >
Quaternion< T >
Quaternion< T >::operator-() const
{
    return Quaternion( -x(), -y(), -z(), -w() );
}



template < typename T >
void Quaternion< T >::operator+=( const Quaternion< T >& q )
{
    array[ 0 ] += q.array[ 0 ];
    array[ 1 ] += q.array[ 1 ];
    array[ 2 ] += q.array[ 2 ];
    array[ 3 ] += q.array[ 3 ];
}



template < typename T >
void Quaternion< T >::operator-=( const Quaternion< T >& q )
{
    array[ 0 ] -= q.array[ 0 ];
    array[ 1 ] -= q.array[ 1 ];
    array[ 2 ] -= q.array[ 2 ];
    array[ 3 ] -= q.array[ 3 ];
}



//
// quaternion/scalar operations
//

template < typename T >
Quaternion< T >
Quaternion< T >::operator*( const T a_ ) const
{
    return Quaternion( x() * a_, y() * a_, z() * a_, w() * a_ );
}



template < typename T >
Quaternion< T >
Quaternion< T >::operator/( T a_ ) const
{
    if ( a_ == 0.0 )
    {
        VMMLIB_ERROR( "Division by zero.", VMMLIB_HERE );
    }
    a_ = 1.0 / a_;
    return Quaternion( x() * a_, y() * a_, z() * a_, w() * a_ );
}



template < typename T >
void Quaternion< T >::operator*=( T q )
{
    array[ 0 ] *= q;
    array[ 1 ] *= q;
    array[ 2 ] *= q;
    array[ 3 ] *= q;
}



template < typename T >
void Quaternion< T >::operator/=( T q )
{
    if ( q == 0.0 )
    {
        VMMLIB_ERROR( "Division by zero", VMMLIB_HERE );
    }
    q = 1.0f / q;
    this->operator*=( q );
}


//quaternion/vector operations

template < typename T >
Quaternion< T >
Quaternion< T >::operator+( const Vector< 3, T >& a ) const
{
    return Quaternion( x() + a.x(), y() + a.y(), z() + a.z(), w() );
}



template < typename T >
Quaternion< T >
Quaternion< T >::operator-( const Vector< 3, T >& a ) const
{
    return Quaternion( w(), x() - a.x(), y() - a.y(), z() - a.z() );
}



template < typename T >
Quaternion< T >
Quaternion< T >::operator*( const Vector< 3, T >& a ) const
{
    return Quaternion( -x() * a.x() - y() * a.y() - z() * a.z(),
                        w() * a.x() + y() * a.z() - z() * a.y(),
                        w() * a.y() + z() * a.x() - x() * a.z(),
                        w() * a.z() + x() * a.y() - y() * a.x()  );
}



template < typename T >
void Quaternion< T >::operator+=( const Vector< 3, T >& xyz )
{
    x() += xyz.x();
    y() += xyz.y();
    y() += xyz.z();
}



template < typename T >
void Quaternion< T >::operator-=( const Vector< 3, T >& xyz )
{
    x() -= xyz.x();
    y() -= xyz.y();
    z() -= xyz.z();
    return;
}



template < typename T >
void Quaternion< T >::operator*=(const Vector< 3, T >& a )
{
    T _x = x();
    T _y = y();
    T _z = z();
    T _w = w();

    x() = _w * a.x() + _y * a.z() - _z * a.y();
    y() = _w * a.y() + _z * a.x() - _x * a.z();
    z() = _w * a.z() + _x * a.y() - _y * a.x();
    w() = -_x * a.x() - _y * a.y() - _z * a.z();
}




template < typename T >
Vector< 3, T > Quaternion< T >::cross( const Quaternion< T >& bb ) const
{
    Vector< 3, T > result;

    result.array[ 0 ] = y() * bb.z() - z() * bb.y();
    result.array[ 1 ] = z() * bb.x() - x() * bb.z();
    result.array[ 2 ] = x() * bb.y() - y() * bb.x();

    return result;
}



template < typename T >
T Quaternion< T >::dot( const Quaternion< T >& q ) const
{
    return w() * q.w() + x() * q.x() + y() * q.y() + z() * q.z();
}



template < typename T >
T Quaternion< T >::
dot( const Quaternion< T >& p, const Quaternion< T >& q )
{
    return p.w() * q.w() + p.x() * q.x() + p.y() * q.y() + p.z() * q.z();
}



template < typename T >
void Quaternion< T >::normal( const Quaternion< T >& aa,
                              const Quaternion< T >& bb,
                              const Quaternion< T >& cc,
                              const Quaternion< T >& dd )
{
    //right hand system, CCW triangle
    const Quaternion< T > quat_t = bb - aa;
    const Quaternion< T > quat_u = cc - aa;
    const Quaternion< T > quat_v = dd - aa;
    cross( quat_t );
    cross( quat_u );
    cross( quat_v );
    normalize();
}



template < typename T >
Quaternion< T > Quaternion< T >::normal( const Quaternion< T >& aa,
                                         const Quaternion< T >& bb,
                                         const Quaternion< T >& cc )
{
    Quaternion< T > tmp;
    tmp.normal( *this, aa, bb, cc );
    return tmp;
}


// to combine two rotations, multiply the respective quaternions before using rotate
// instead of rotating twice for increased performance, but be aware of non-commutativity!
// (the first rotation quaternion has to be the first factor)
template< typename T >
Quaternion< T > Quaternion< T >::rotate( T theta, Vector< 3, T >& axis,
                                         const Vector< 3, T >& a )
{
    Quaternion< T > p = a;
    T alpha = theta / 2;
    Quaternion< T > q = std::cos( alpha ) + ( std::sin( alpha ) * axis.normalize() );
    return q * p * q.invert();
}



template< typename T >
Quaternion< T > Quaternion< T >::rotate_x( T theta, const Vector< 3, T >& a )
{
    Quaternion< T > p = a;
    T alpha = theta / 2;
    Quaternion< T > q = std::cos( alpha ) + ( std::sin( alpha ) *  QUATERI );
    return q * p * q.invert();
}



template< typename T >
Quaternion< T > Quaternion< T >::rotate_y( T theta, const Vector< 3, T >& a )
{
    Quaternion< T > p = a;
    T alpha = theta / 2;
    Quaternion< T > q = std::cos( alpha ) + ( std::sin( alpha ) *  QUATERJ );
    return q * p * q.invert();
}



template< typename T >
Quaternion< T > Quaternion< T >::rotate_z( T theta, const Vector< 3, T >& a )
{
    Quaternion< T > p = a;
    T alpha = theta / 2;
    Quaternion< T > q = std::cos( alpha ) + ( std::sin( alpha ) *  QUATERK );
    return q * p * q.invert();
}



template < typename T >
Matrix< 3, 3, T >
Quaternion< T >::get_rotation_matrix() const
{
    Matrix< 3, 3, T > result;
    get_rotation_matrix< 3 >( result );
    return result;
}



template < typename T > template< size_t D >
void Quaternion< T >::get_rotation_matrix( Matrix< D, D, T >& M ) const
{
    T w2 = w() * w();
    T x2 = x() * x();
    T y2 = y() * y();
    T z2 = z() * z();
    T wx = w() * x();
    T wy = w() * y();
    T wz = w() * z();
    T xy = x() * y();
    T xz = x() * z();
    T yz = y() * z();

    M( 0, 0 ) = w2 + x2 - y2 - z2;
    M( 0, 1 ) = 2. * (xy - wz);
    M( 0, 2 ) = 2. * (xz + wy);
    M( 1, 0 ) = 2. * (xy + wz);
    M( 1, 1 ) = w2 - x2 + y2 - z2;
    M( 1, 2 ) = 2. * (yz - wx);
    M( 2, 0 ) = 2. * (xz - wy);
    M( 2, 1 ) = 2. * (yz + wx);
    M( 2, 2 ) = w2 - x2 - y2 + z2;

}

template< typename T >
Quaternion< T > Quaternion< T >::
slerp( T a, const Quaternion< T >& p, const Quaternion< T >& q, const T epsilon )
{
    Quaternion< T > px = p.get_normalized();
    Quaternion< T > qx = q.get_normalized();

    T cosine = px.dot( qx );

    // check if inverted rotation is needed
    if ( cosine < 0.0 )
    {
        cosine = -cosine;
        qx = -qx;
    }

    const T abs_cos = static_cast< T >( fabs(cosine) );
    const T one_x   = static_cast< T >( 1. - epsilon );
    if( abs_cos < one_x )
    {
        // standard slerp
        T sine = sqrt( 1. - ( cosine * cosine ) );
        T angle = atan2( sine, cosine );
        T coeff1 = std::sin( ( 1.0 - a ) * angle) / sine;
        T coeff2 = std::sin( a * angle ) / sine;

        qx *= coeff2;
        px *= coeff1;

        px += qx;
    }
    else
    {
        // linear interpolation for very small angles
        px *= 1. - a;
        qx *= a;
        px += qx;
        px.normalize();
    }

    return px;
}


template < typename T >
Quaternion< T >& Quaternion< T >::operator=(const Quaternion& other)
{
    memcpy( array, other.array, 4 * sizeof( T ) );
    return *this;
}


template < typename T >
const Vector< 4, T >&
Quaternion< T >::operator=( const Vector< 4, T >& other )
{
    memcpy( array, other.array, 4 * sizeof( T ) );
    return other;
}


}
#endif
