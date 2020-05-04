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
#ifndef VMMLIB__QR_DECOMPOSITION__HPP
#define VMMLIB__QR_DECOMPOSITION__HPP

#include <vmmlib/matrix.hpp>
#include <vmmlib/vector.hpp>
#include <vmmlib/exception.hpp>

#include <cmath>
#include <vector>

/*
* QR decomposition using stabilized gram-schmidt
* A  -> matrix to be factorized
* Q  -> orthonormal
* Rn -> upper triangular
*/

namespace vmml
{

template< size_t M, size_t N, typename T >
void qr_decompose_gram_schmidt(
    const Matrix< M, N, T >& A_,
    Matrix< M, M, T >& Q,
    Matrix< N, N, T >& R
    )
{
    Q   = 0.0;
    R   = 0.0;

    // create a copy of A_ since we will change it in the algorithm
    Matrix< M, N, T > A( A_ );

    Vector< M, T > a_column, q_column;

    #if 0
    // for each column
    for( size_t k = 0; k < N; ++k )
    {
        // compute norm of A's column k
        A.get_column( k, a_column );
        const T a_norm = a_column.length();

        R.at( k, k ) = a_norm;

        if ( a_norm == static_cast< T >( 0.0 ) )
            break;

        Q.set_column( k, a_column / a_norm );

        for( size_t j = k+1; j < N; ++j )
        {
            Q.get_column( k, q_column );
            A.get_column( j, a_column );

            R.at( k, j ) = a_column.dot( q_column );

            for( size_t i = 0; i < M; ++i )
            {
                A( i, j ) = A( i, j ) - R( k, j ) * Q( i, k );
            }
        }
    }
    #else
    Vector< M, T >  v;
    for( unsigned j = 0; j < N; ++j )
    {
        A.get_column( j, v );

        for( unsigned i = 0; i < j; ++i )
        {
            Q.get_column( i, q_column );
            A.get_column( j, a_column );

            R( i, j ) = dot( q_column, a_column );
            v -= q_column * R(i,j);
        }
        R(j,j) = v.length();
        Q.set_column( j, v / R(j,j) );
    }


    #endif

}


} // namespace vmml

#endif

