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
#ifndef VMMLIB__MATRIX_FUNCTORS__HPP
#define VMMLIB__MATRIX_FUNCTORS__HPP

#include <vmmlib/enable_if.hpp>

#include <cstddef>
#include <functional>

namespace vmml
{

template< typename T >
struct set_to_zero_functor
{
    inline void operator()( T& matrix_ ) const
    {
        matrix_ = static_cast< typename T::value_type >( 0.0 );
    }

}; // struct set_to_zero


template< typename T >
struct set_to_identity_functor
{
    inline
    typename enable_if< T::ROWS == T::COLS >::type*
    operator()( T& matrix_ )
    {
        set_to_zero_functor< T >()( matrix_ );
        for( size_t index = 0; index < T::ROWS; ++index )
        {
            matrix_( index, index ) = static_cast< typename T::value_type >( 1.0 );
        }
        return 0; // for sfinae
    }
}; // struct set_to_identity


// this functor compares to matrices, and also returns true/equal if
// the matrices have the same values but some rows/columns are inverted
template< typename T >
struct matrix_equals_allow_inverted_rows : std::binary_function< const T&, const T&, bool >
{

    bool operator()( const T& matrix0, const T& matrix1 )
    {
        const size_t r = matrix0.get_number_of_rows();
        bool ok = true;
        for( size_t index = 0; ok && index < r; ++index )
        {
            if ( matrix0.get_row( index ) != matrix1.get_row( index )
                && matrix0.get_row( index ) != - matrix1.get_row( index ) )
            {
                ok = false;
            }

        }
        return ok;
    }

    bool operator()( const T& matrix0, const T& matrix1, typename T::value_type tolerance )
    {
        const size_t r = matrix0.get_number_of_rows();
        bool ok = true;
        for( size_t index = 0; ok && index < r; ++index )
        {
            if (
                ! matrix0.get_row( index ).equals( matrix1.get_row( index ), tolerance )
                && ! matrix0.get_row( index ).equals( - matrix1.get_row( index ), tolerance )
                )
            {
                ok = false;
            }

        }
        return ok;
    }


}; // struct matrix_equals_allow_inverted_rows

template< typename T >
struct matrix_equals_allow_inverted_columns : std::binary_function< const T&, const T&, bool >
{

    bool operator()( const T& matrix0, const T& matrix1 )
    {
        const size_t r = matrix0.get_number_of_columns();
        bool ok = true;
        for( size_t index = 0; ok && index < r; ++index )
        {
            if ( matrix0.get_column( index ) != matrix1.get_column( index )
                && matrix0.get_column( index ) != - matrix1.get_column( index ) )
            {
                ok = false;
            }

        }
        return ok;
    }

    bool operator()( const T& matrix0, const T& matrix1, typename T::value_type tolerance )
    {
        const size_t r = matrix0.get_number_of_columns();
        bool ok = true;
        for( size_t index = 0; ok && index < r; ++index )
        {
            if (
                ! matrix0.get_column( index ).equals( matrix1.get_column( index ), tolerance )
                && ! matrix0.get_column( index ).equals( - matrix1.get_column( index ), tolerance )
                )
            {
                ok = false;
            }

        }
        return ok;
    }


}; // struct matrix_equals_allow_inverted_columns


} // namespace vmml

#endif
