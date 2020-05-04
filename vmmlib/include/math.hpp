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
#ifndef VMMLIB__MATH__HPP
#define VMMLIB__MATH__HPP

#include <cmath>

#ifndef M_PI
#	define M_PI 3.14159265358979323846
#endif

namespace vmml
{

namespace math
{
// helpers for certain cmath functions


template< class T >
inline T squared( const T a )
{
    return ( a == 0.0 ) ? 0.0 : a * a;
}



/*
 * Computes (a2 + b2)1/2 without destructive underflow or overflow.
 */
template< class T >
inline T pythag( T a, T b )
{
    a = std::abs(a);
    b = std::abs(b);
    if ( a > b )
        return a * std::sqrt( 1.0 + squared( b / a ) );
    else
        return ( b == 0.0 ) ? 0.0 : b * sqrt( 1.0 + squared( a / b ) );
}



template< class T >
inline T sign( T a, T b )
{
    return ( b >= 0.0 ) ? std::abs( a ) : -std::abs( a );
}



template< typename T >
struct abs_less
{
    T operator()( const T& a, const T& b )
    {
        return std::abs(a) < std::abs( b );
    }
};


template< typename T >
struct abs_greater
{
    T operator()( const T& a, const T& b )
    {
        return std::abs(a) > std::abs( b );
    }
};


} // namespace math

} // namespace vmml

#endif
