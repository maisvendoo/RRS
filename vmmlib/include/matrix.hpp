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
#ifndef VMMLIB__MATRIX__HPP
#define VMMLIB__MATRIX__HPP

#include <vmmlib/vmmlib_config.hpp>

#include <vmmlib/matrix_functors.hpp>
#include <vmmlib/vector.hpp>
#include <vmmlib/math.hpp>
#include <vmmlib/exception.hpp>
#include <vmmlib/enable_if.hpp>

#include <iostream>
#include <iomanip>
#include <vector>
#include <cstddef>
#include <limits>
#include <algorithm>
#include <string>
#include <cstring>
#include <fstream>   // file I/O

namespace vmml
{

// matrix of type T with m rows and n columns
template< size_t M, size_t N, typename T = float >
class Matrix
{
public:
    typedef T                                       value_type;
    typedef T*                                      iterator;
    typedef const T*                                const_iterator;
    typedef std::reverse_iterator< iterator >       reverse_iterator;
    typedef std::reverse_iterator< const_iterator > const_reverse_iterator;

    static const size_t         ROWS = M;
    static const size_t         COLS = N;

    // ctors
    Matrix();

    template< size_t P, size_t Q, typename U >
    Matrix( const Matrix< P, Q, U >& source_ );

    // accessors
    inline T& operator()( size_t row_index, size_t col_index );
    inline const T& operator()( size_t row_index, size_t col_index ) const;

    inline T& at( size_t row_index, size_t col_index );
    inline const T& at( size_t row_index, size_t col_index ) const;

    // element iterators - NOTE: column-major order
    iterator                begin();
    iterator                end();
    const_iterator          begin() const;
    const_iterator          end() const;

    reverse_iterator        rbegin();
    reverse_iterator        rend();
    const_reverse_iterator  rbegin() const;
    const_reverse_iterator  rend() const;

#ifndef VMMLIB_NO_CONVERSION_OPERATORS
    // auto conversion operator
    operator T*();
    operator const T*() const;
#endif

    bool operator==( const Matrix& other ) const;
    bool operator!=( const Matrix& other ) const;

    // due to limited precision, two 'identical' matrices might seem different.
    // this function allows to specify a tolerance when comparing matrices.
    bool equals( const Matrix& other, T tolerance ) const;
    // this version takes a comparison functor to compare the components of
    // the two matrices
    template< typename compare_t >
    bool equals( const Matrix& other, compare_t& cmp ) const;

    void multiply_piecewise( const Matrix& other );

    // (this) matrix = left matrix_mxp * right matrix_pxn
    template< size_t P > void multiply( const Matrix< M, P, T >& left,
                                        const Matrix< P, N, T >& right );

    // convolution operation (extending borders) of (this) matrix and the given kernel
    template< size_t U, size_t V >
    void convolve(const Matrix< U, V, T >& kernel);

    // returned matrix_mxp = (this) matrix * other matrix_nxp;
    // note: using multiply(...) it avoids a copy of the resulting matrix
    template< size_t P >
    Matrix< M, P, T > operator*( const Matrix< N, P, T >& other ) const;

    // operator *= is only enabled for square matrices
    template< size_t O, size_t P, typename TT >
    typename enable_if< M == N && O == P && M == O, TT >::type*
    operator*=( const Matrix< O, P, TT >& right );

    inline Matrix operator+( const Matrix& other ) const;
    inline Matrix operator-( const Matrix& other ) const;

    void operator+=( const Matrix& other );
    void operator-=( const Matrix& other );

    void operator+=( T scalar );
    void operator-=( T scalar );

    template< size_t O, size_t P, size_t Q, size_t R >
    typename enable_if< M == O + Q && N == P + R >::type*
    direct_sum( const Matrix< O, P, T >& m0, const Matrix< Q, R, T >& m1 );

    //
    // matrix-scalar operations / scaling
    //
    Matrix operator*( T scalar );
    void operator*=( T scalar );

    Matrix operator/( T scalar );
    void operator/=( T scalar );

    //
    // matrix-vector operations
    //
    // transform column vector by matrix ( vec = matrix * vec )
    Vector< M, T > operator*( const Vector< N, T >& other ) const;

    // transform column vector by matrix ( vec = matrix * vec )
    // assume homogenous coords, e.g. vec3 = mat4x4 * vec3, with w = 1.0
    template< size_t O >
    Vector< O, T > operator*( const Vector< O, T >& vector_ ) const;

    inline Matrix< M, N, T > operator-() const;
    Matrix< M, N, T > negate() const;

    // compute tensor product: (this) = vector (X) vector
    void tensor( const Vector< M, T >& u, const Vector< N, T >& v );
    // tensor, for api compatibility with old vmmlib version.
    // WARNING: for M = N = 4 only.
    template< size_t uM, size_t vM >
    typename enable_if< uM == 3 && vM == 3 && M == N && M == 4 >::type*
    tensor( const Vector< uM, T >& u, const Vector< vM, T >& v );

    // row_offset and col_offset define the starting indices for the sub_matrix
    // the sub_matrix is extracted according to the size of the target matrix, i.e. ( OXP )
    template< size_t O, size_t P >
    Matrix< O, P, T >
    get_sub_matrix( size_t row_offset, size_t col_offset,
        typename enable_if< O <= M && P <= N >::type* = 0 ) const;

    template< size_t O, size_t P >
    typename enable_if< O <= M && P <= N >::type*
    get_sub_matrix( Matrix< O, P, T >& result,
        size_t row_offset = 0, size_t col_offset = 0 ) const;

    template< size_t O, size_t P >
    typename enable_if< O <= M && P <= N >::type*
    set_sub_matrix( const Matrix< O, P, T >& sub_matrix,
        size_t row_offset = 0, size_t col_offset = 0 );

    // copies a transposed version of *this into transposedMatrix
    void transpose_to( Matrix< N, M, T >& transpose_ ) const;

    //symmetric covariance matrix of a right matrix multiplication: MxN x NxM = MxM
    void	symmetric_covariance( Matrix< M, M, T >& cov_m_ ) const;


    const Matrix& operator=( const Matrix< M, N, T >& source_ );

    // these assignment operators return nothing to avoid silent loss of
    // precision
    template< size_t P, size_t Q, typename U >
    void operator=( const Matrix< P, Q, U >& source_ );
    void operator=( const T old_fashioned_matrix[ M ][ N ] );

    // WARNING: data_array[] must be at least of size M * N - otherwise CRASH!
    // WARNING: assumes row_by_row layout - if this is not the case,
    // use matrix::set( data_array, false )
    void operator=( const T* data_array );
    void operator=( const std::vector< T >& data );

    // sets all elements to fill_value
    void operator=( T fill_value );
    void fill( T fill_value );

    // note: this function copies elements until either the matrix is full or
    // the iterator equals end_.
    template< typename input_iterator_t >
    void set( input_iterator_t begin_, input_iterator_t end_,
        bool row_major_layout = true );

    //sets all matrix values with random values
    //remember to set srand( seed );
    //if seed is set to -1, srand( seed ) was set outside set_random
    //otherwise srand( seed ) will be called with the given seed
    void set_random( int seed = -1 );

    //sets all matrix values with discrete cosine transform coefficients (receive orthonormal coefficients)
    void set_dct();

    void zero();

    double frobenius_norm() const;
    double p_norm( double p ) const;

    template< typename TT >
    void cast_from( const Matrix< M, N, TT >& other );

    void read_csv_file( const std::string& dir_, const std::string& filename_ );
    void write_csv_file( const std::string& dir_, const std::string& filename_ ) const;
    void read_from_raw( const std::string& dir_, const std::string& filename_ ) ;
    void write_to_raw( const std::string& dir_, const std::string& filename_ ) const;
    void read_from_stream( std::istream& is );
    void write_to_stream( std::ostream& os ) const;

    template< typename TT >
        void quantize_to( Matrix< M, N, TT >& quantized_, const T& min_value, const T& max_value ) const;
    template< typename TT >
        void quantize( Matrix< M, N, TT >& quantized_, T& min_value, T& max_value ) const;
    template< typename TT >
        void dequantize( Matrix< M, N, TT >& quantized_, const TT& min_value, const TT& max_value ) const;

    void columnwise_sum( Vector< N, T>& summed_columns_ ) const;
    double sum_elements() const;

    void sum_rows( Matrix< M/2, N, T>& other ) const;
    void sum_columns( Matrix< M, N/2, T>& other ) const;

    template< size_t R >
    typename enable_if< R == M && R == N >::type*
    diag( const Vector< R, T >& diag_values_ );


    //Khatri-Rao Product: columns must be of same size
    template< size_t O >
    void khatri_rao_product( const Matrix< O, N, T >& right_, Matrix< M*O, N, T >& prod_ ) const;
    //Kronecker Product: MxN x_kronecker OxP = M*OxN*P
    template< size_t O, size_t P >
    void kronecker_product( const Matrix< O, P, T >& right_,  Matrix< M*O, N*P, T >& result_) const;

    T get_min() const;
    T get_max() const;
    T get_abs_min() const;
    T get_abs_max() const;

    //returns number of non-zeros
    size_t nnz() const;
    size_t nnz( const T& threshold_ ) const;
    void threshold( const T& threshold_value_ );

    Vector< M, T >  get_column( size_t column_index ) const;
    void get_column( size_t column_index, Vector< M, T>& column ) const;

    void set_column( size_t index, const Vector< M, T >& column );

    void get_column( size_t index, Matrix< M, 1, T >& column ) const;
    void set_column( size_t index, const Matrix< M, 1, T >& column );

    Vector< N, T > get_row( size_t index ) const;
    void get_row( size_t index, Vector< N, T >& row ) const;
    void set_row( size_t index,  const Vector< N, T >& row );

    void get_row( size_t index, Matrix< 1, N, T >& row ) const;
    void set_row( size_t index,  const Matrix< 1, N, T >& row );

    size_t size() const; // return M * N;

    size_t get_number_of_rows() const;
    size_t get_number_of_columns() const;

    T det() const;

    // the return value indicates if the matrix is invertible.
    // we need a tolerance term since the computation of the determinant is
    // subject to precision errors.
    template< size_t O, size_t P, typename TT >
    bool inverse( Matrix< O, P, TT >& inverse_,
                  T tolerance = std::numeric_limits<T>::epsilon(),
        typename enable_if< M == N && O == P && O == M && M >= 2 && M <= 4, TT >::type* = 0 ) const;

    template< size_t O, size_t P >
    typename enable_if< O == P && M == N && O == M && M >= 2 >::type*
    get_adjugate( Matrix< O, P, T >& adjugate ) const;

    template< size_t O, size_t P >
    typename enable_if< O == P && M == N && O == M && M >= 2 >::type*
    get_cofactors( Matrix< O, P, T >& cofactors ) const;


    // returns the determinant of a square matrix M-1, N-1
    template< size_t O, size_t P >
    T get_minor( Matrix< O, P, T >& minor_, size_t row_to_cut, size_t col_to_cut,
        typename enable_if< O == M-1 && P == N-1 && M == N && M >= 2 >::type* = 0 ) const;

    //
    // 4*4 matrices only
    //
    /** create rotation matrix from parameters.
    * @param angle - angle in radians
    * @param rotation axis - must be normalized!
    */
    template< typename TT >
    Matrix< M, N, T >& rotate( const TT angle, const Vector< M-1, T >& axis,
        typename enable_if< M == N && M == 4, TT >::type* = 0 );

    template< typename TT >
    Matrix< M, N, T >& rotate_x( const TT angle,
        typename enable_if< M == N && M == 4, TT >::type* = 0 );

    template< typename TT >
    Matrix< M, N, T >& rotate_y( const TT angle,
        typename enable_if< M == N && M == 4, TT >::type* = 0 );

    template< typename TT >
    Matrix< M, N, T >& rotate_z( const TT angle,
        typename enable_if< M == N && M == 4, TT >::type* = 0 );

    template< typename TT >
    Matrix< M, N, T >& pre_rotate_x( const TT angle,
        typename enable_if< M == N && M == 4, TT >::type* = 0 );

    template< typename TT >
    Matrix< M, N, T >& pre_rotate_y( const TT angle,
        typename enable_if< M == N && M == 4, TT >::type* = 0 );

    template< typename TT >
    Matrix< M, N, T >& pre_rotate_z( const TT angle,
        typename enable_if< M == N && M == 4, TT >::type* = 0 );

    template< typename TT >
    Matrix< M, N, T >& scale( const TT scale[3],
        typename enable_if< M == N && M == 4, TT >::type* = 0 );

    template< typename TT >
    Matrix< M, N, T >& scale( const TT x_, const T y, const T z,
        typename enable_if< M == N && M == 4, TT >::type* = 0 );

    template< typename TT >
    Matrix< M, N, T >& scale( const Vector< 3, TT >& scale_,
        typename enable_if< M == N && M == 4, TT >::type* = 0 );

    template< typename TT >
    Matrix< M, N, T >& scale_translation( const TT scale_[3],
        typename enable_if< M == N && M == 4, TT >::type* = 0 );

    template< typename TT >
    Matrix< M, N, T >& scale_translation( const Vector< 3, TT >& scale_,
        typename enable_if< M == N && M == 4, TT >::type* = 0 );

    template< typename TT >
    Matrix< M, N, T >& set_translation( const TT x_, const TT y_, const TT z_,
        typename enable_if< M == N && M == 4, TT >::type* = 0 );

    template< typename TT >
    Matrix< M, N, T >& set_translation( const TT trans[3],
        typename enable_if< M == N && M == 4, TT >::type* = 0 );

    template< typename TT >
    Matrix< M, N, T >& set_translation( const Vector< 3, TT >& t,
                        typename enable_if< M == N && M == 4, TT >::type* = 0 );

    template< typename TT >
    void get_translation( Vector< N-1, TT >& translation_ ) const;

    Vector< N-1, T > get_translation() const;

    // hack for static-member-init
    template< typename init_functor_t >
    static const Matrix get_initialized_matrix();

    inline T& x();
    inline T& y();
    inline T& z();


    // tests every component for isnan && isinf
    //   inline bool is_valid() const;  -> moved to class validator

    // legacy/compatibility accessor
    struct row_accessor
    {
        row_accessor( T* array_ ) : array( array_ ) {}
        T&
        operator[]( size_t col_index )
        {
            #ifdef VMMLIB_SAFE_ACCESSORS
            if ( col_index >= N )
                VMMLIB_ERROR( "column index out of bounds", VMMLIB_HERE );
            #endif
            return array[ col_index * M ];
        }

        const T&
        operator[]( size_t col_index ) const
        {
            #ifdef VMMLIB_SAFE_ACCESSORS
            if ( col_index >= N )
                VMMLIB_ERROR( "column index out of bounds", VMMLIB_HERE );
            #endif
            return array[ col_index * M ];
        }

        T* array;
        private: row_accessor() {} // disallow std ctor
    };
    // this is a hack to allow array-style access to matrix elements
    // usage: matrix< 2, 2, float > m; m[ 1 ][ 0 ] = 37.0f;
    inline row_accessor operator[]( size_t row_index )
    {
        #ifdef VMMLIB_SAFE_ACCESSORS
        if ( row_index > M )
            VMMLIB_ERROR( "row index out of bounds", VMMLIB_HERE );
        #endif
        return row_accessor( array + row_index );
    }

    // this is a hack to remove a warning about implicit conversions
    inline row_accessor operator[]( int row_index )
    {
        return ( *this )[ size_t ( row_index ) ];
    }

    friend std::ostream& operator << ( std::ostream& os,
        const Matrix< M, N, T >& matrix )
    {
        const std::ios::fmtflags flags = os.flags();
        const int                prec  = os.precision();

        os.setf( std::ios::right, std::ios::adjustfield );
        os.precision( 5 );

        for( size_t row_index = 0; row_index < M; ++row_index )
        {
            os << "|";
            for( size_t col_index = 0; col_index < N; ++col_index )
            {
                os << std::setw(10) << matrix.at( row_index, col_index ) << " ";
            }
            os << "|" << std::endl;
        }
        os.precision( prec );
        os.setf( flags );
        return os;
    };

    // protected:
    T array[ M * N ]; //!< column by column storage

    // static members
    static const Matrix< M, N, T > IDENTITY;
    static const Matrix< M, N, T > ZERO;

}; // class matrix


#ifndef VMMLIB_NO_TYPEDEFS
typedef vmml::Matrix< 3, 3, double > Matrix3d; //!< A 3x3 double matrix
typedef vmml::Matrix< 4, 4, double > Matrix4d; //!< A 4x4 double matrix
typedef vmml::Matrix< 3, 3, float >  Matrix3f; //!< A 3x3 float matrix
typedef vmml::Matrix< 4, 4, float >  Matrix4f; //!< A 4x4 float matrix

#ifdef VMMLIB_OLD_TYPEDEFS
typedef Matrix< 3, 3, float > mat3f;
typedef Matrix< 3, 3, double > mat3d;
typedef Matrix< 4, 4, float > mat4f;
typedef Matrix< 4, 4, double > mat4d;
template< size_t M, size_t N, typename T = float >
using matrix = Matrix<M, N, T>;
#endif

#endif

/*
*   free functions
*/

template< size_t M, size_t N, typename T >
bool equals( const Matrix< M, N, T >& m0, const Matrix< M, N, T >& m1,
             const T tolerance = std::numeric_limits< T >::epsilon())
{
    return m0.equals( m1, tolerance );
}


template< size_t M, size_t N, typename T >
inline void multiply( const Matrix< M, N, T >& left,
                      const Matrix< M, N, T >& right,
                      Matrix< M, N, T >& result )
{
    result.multiply( left, right );
}



template< size_t M, size_t N, typename T >
template< size_t U, size_t V >
void Matrix< M, N, T>::convolve(const Matrix< U, V, T >& kernel)
{
    Matrix< M, N, T> temp;  // do not override original values instantly as old values are needed for calculation

    for(size_t y_ = 0; y_ < N; ++y_)
    {
        for(size_t x_ = 0; x_ < M; ++x_)
        {
            double sum = 0.0;

            for(size_t j = 0; j < V; ++j)
            {
                int srcy = y_ - V/2 + j;

                // Extending border values
                if(srcy < 0)       srcy = 0;
                if(srcy >= int(N)) srcy = N-1;

                for(size_t i = 0; i < U; ++i)
                {
                    int srcx = x_ - U/2 + i;

                    // Extending border values
                    if(srcx < 0)       srcx = 0;
                    if(srcx >= int(M)) srcx = M-1;

                    sum += kernel.at(j,i) * at(srcy,srcx);
                }
            }
            temp.at(y_,x_) = sum;
        }
    }

    *this = temp;
}



template< size_t M, size_t N, size_t P, typename T >
inline void
multiply( const Matrix< M, P, T >& left, const Matrix< P, N, T >& right,
    Matrix< M, N, T >& result )
{
    result.multiply( left, right );
}


template< size_t M, size_t N, typename T >
inline typename enable_if< M == N >::type* identity( Matrix< M, N, T >& m )
{
    m = static_cast< T >( 0.0 );
    for( size_t index = 0; index < M; ++index )
        m( index, index ) = static_cast< T >( 1.0 );
    return 0; // for sfinae
}



template< typename T >
inline T compute_determinant( const Matrix< 1, 1, T >& matrix_ )
{
    return matrix_.array[ 0 ];
}



template< typename T >
inline T compute_determinant( const Matrix< 2, 2, T >& matrix_ )
{
    const T& a = matrix_( 0, 0 );
    const T& b = matrix_( 0, 1 );
    const T& c = matrix_( 1, 0 );
    const T& d = matrix_( 1, 1 );
    return a * d - b * c;
}



template< typename T >
inline T compute_determinant( const Matrix< 3, 3, T >& m_ )
{
    return
          m_( 0,0 ) * ( m_( 1,1 ) * m_( 2,2 ) - m_( 1,2 ) * m_( 2,1 ) )
        + m_( 0,1 ) * ( m_( 1,2 ) * m_( 2,0 ) - m_( 1,0 ) * m_( 2,2 ) )
        + m_( 0,2 ) * ( m_( 1,0 ) * m_( 2,1 ) - m_( 1,1 ) * m_( 2,0 ) );
}


template< typename T >
inline T compute_determinant( const Matrix< 4, 4, T >& m )
{
    T m00   = m( 0, 0 );
    T m10   = m( 1, 0 );
    T m20   = m( 2, 0 );
    T m30   = m( 3, 0 );
    T m01   = m( 0, 1 );
    T m11   = m( 1, 1 );
    T m21   = m( 2, 1 );
    T m31   = m( 3, 1 );
    T m02   = m( 0, 2 );
    T m12   = m( 1, 2 );
    T m22   = m( 2, 2 );
    T m32   = m( 3, 2 );
    T m03   = m( 0, 3 );
    T m13   = m( 1, 3 );
    T m23   = m( 2, 3 );
    T m33   = m( 3, 3 );

    return
        m03 * m12 * m21 * m30
            - m02 * m13 * m21 * m30
            - m03 * m11 * m22 * m30
            + m01 * m13 * m22 * m30
            + m02 * m11 * m23 * m30
            - m01 * m12 * m23 * m30
            - m03 * m12 * m20 * m31
            + m02 * m13 * m20 * m31
            + m03 * m10 * m22 * m31
            - m00 * m13 * m22 * m31
            - m02 * m10 * m23 * m31
            + m00 * m12 * m23 * m31
            + m03 * m11 * m20 * m32
            - m01 * m13 * m20 * m32
            - m03 * m10 * m21 * m32
            + m00 * m13 * m21 * m32
            + m01 * m10 * m23 * m32
            - m00 * m11 * m23 * m32
            - m02 * m11 * m20 * m33
            + m01 * m12 * m20 * m33
            + m02 * m10 * m21 * m33
            - m00 * m12 * m21 * m33
            - m01 * m10 * m22 * m33
            + m00 * m11 * m22 * m33;
}



template< typename T >
bool compute_inverse( const Matrix< 2, 2, T >& m_, Matrix< 2, 2, T >& inverse_,
                      T tolerance_ = std::numeric_limits<T>::epsilon())
{
    const T det_ = compute_determinant( m_ );
    if( fabs( det_ ) < tolerance_ )
        return false;

    const T detinv = static_cast< T >( 1.0 ) / det_;

    m_.get_adjugate( inverse_ ); // set inverse_ to the adjugate of M
    inverse_ *= detinv;
    return true;
}



template< typename T >
bool compute_inverse( const Matrix< 3, 3, T >& m_, Matrix< 3, 3, T >& inverse_,
                      T tolerance_ = std::numeric_limits<T>::epsilon() )
{
    // Invert a 3x3 using cofactors.  This is about 8 times faster than
    // the Numerical Recipes code which uses Gaussian elimination.
    inverse_.at( 0, 0 ) = m_.at( 1, 1 ) * m_.at( 2, 2 ) - m_.at( 1, 2 ) * m_.at( 2, 1 );
    inverse_.at( 0, 1 ) = m_.at( 0, 2 ) * m_.at( 2, 1 ) - m_.at( 0, 1 ) * m_.at( 2, 2 );
    inverse_.at( 0, 2 ) = m_.at( 0, 1 ) * m_.at( 1, 2 ) - m_.at( 0, 2 ) * m_.at( 1, 1 );
    inverse_.at( 1, 0 ) = m_.at( 1, 2 ) * m_.at( 2, 0 ) - m_.at( 1, 0 ) * m_.at( 2, 2 );
    inverse_.at( 1, 1 ) = m_.at( 0, 0 ) * m_.at( 2, 2 ) - m_.at( 0, 2 ) * m_.at( 2, 0 );
    inverse_.at( 1, 2 ) = m_.at( 0, 2 ) * m_.at( 1, 0 ) - m_.at( 0, 0 ) * m_.at( 1, 2 );
    inverse_.at( 2, 0 ) = m_.at( 1, 0 ) * m_.at( 2, 1 ) - m_.at( 1, 1 ) * m_.at( 2, 0 );
    inverse_.at( 2, 1 ) = m_.at( 0, 1 ) * m_.at( 2, 0 ) - m_.at( 0, 0 ) * m_.at( 2, 1 );
    inverse_.at( 2, 2 ) = m_.at( 0, 0 ) * m_.at( 1, 1 ) - m_.at( 0, 1 ) * m_.at( 1, 0 );

    const T determinant =
          m_.at( 0, 0 ) * inverse_.at( 0, 0 )
        + m_.at( 0, 1 ) * inverse_.at( 1, 0 )
        + m_.at( 0, 2 ) * inverse_.at( 2, 0 );

    if ( fabs( determinant ) <= tolerance_ )
        return false; // matrix is not invertible

    const T detinv = static_cast< T >( 1.0 ) / determinant;

    inverse_.at( 0, 0 ) *= detinv;
    inverse_.at( 0, 1 ) *= detinv;
    inverse_.at( 0, 2 ) *= detinv;
    inverse_.at( 1, 0 ) *= detinv;
    inverse_.at( 1, 1 ) *= detinv;
    inverse_.at( 1, 2 ) *= detinv;
    inverse_.at( 2, 0 ) *= detinv;
    inverse_.at( 2, 1 ) *= detinv;
    inverse_.at( 2, 2 ) *= detinv;

    return true;
}


template< typename T >
bool compute_inverse( const Matrix< 4, 4, T >& m_,
    Matrix< 4, 4, T >& inv_,
    T tolerance_ = std::numeric_limits<T>::epsilon() )
{
    const T* array = m_.array;

    // tuned version from Claude Knaus
    /* first set of 2x2 determinants: 12 multiplications, 6 additions */
    const T t1[6] = { array[ 2] * array[ 7] - array[ 6] * array[ 3],
                      array[ 2] * array[11] - array[10] * array[ 3],
                      array[ 2] * array[15] - array[14] * array[ 3],
                      array[ 6] * array[11] - array[10] * array[ 7],
                      array[ 6] * array[15] - array[14] * array[ 7],
                      array[10] * array[15] - array[14] * array[11] };

    /* first half of comatrix: 24 multiplications, 16 additions */
    inv_.array[0] = array[ 5] * t1[5] - array[ 9] * t1[4] + array[13] * t1[3];
    inv_.array[1] = array[ 9] * t1[2] - array[13] * t1[1] - array[ 1] * t1[5];
    inv_.array[2] = array[13] * t1[0] - array[ 5] * t1[2] + array[ 1] * t1[4];
    inv_.array[3] = array[ 5] * t1[1] - array[ 1] * t1[3] - array[ 9] * t1[0];
    inv_.array[4] = array[ 8] * t1[4] - array[ 4] * t1[5] - array[12] * t1[3];
    inv_.array[5] = array[ 0] * t1[5] - array[ 8] * t1[2] + array[12] * t1[1];
    inv_.array[6] = array[ 4] * t1[2] - array[12] * t1[0] - array[ 0] * t1[4];
    inv_.array[7] = array[ 0] * t1[3] - array[ 4] * t1[1] + array[ 8] * t1[0];

   /* second set of 2x2 determinants: 12 multiplications, 6 additions */
    const T t2[6] = { array[ 0] * array[ 5] - array[ 4] * array[ 1],
                      array[ 0] * array[ 9] - array[ 8] * array[ 1],
                      array[ 0] * array[13] - array[12] * array[ 1],
                      array[ 4] * array[ 9] - array[ 8] * array[ 5],
                      array[ 4] * array[13] - array[12] * array[ 5],
                      array[ 8] * array[13] - array[12] * array[ 9] };

    /* second half of comatrix: 24 multiplications, 16 additions */
    inv_.array[8]  = array[ 7] * t2[5] - array[11] * t2[4] + array[15] * t2[3];
    inv_.array[9]  = array[11] * t2[2] - array[15] * t2[1] - array[ 3] * t2[5];
    inv_.array[10] = array[15] * t2[0] - array[ 7] * t2[2] + array[ 3] * t2[4];
    inv_.array[11] = array[ 7] * t2[1] - array[ 3] * t2[3] - array[11] * t2[0];
    inv_.array[12] = array[10] * t2[4] - array[ 6] * t2[5] - array[14] * t2[3];
    inv_.array[13] = array[ 2] * t2[5] - array[10] * t2[2] + array[14] * t2[1];
    inv_.array[14] = array[ 6] * t2[2] - array[14] * t2[0] - array[ 2] * t2[4];
    inv_.array[15] = array[ 2] * t2[3] - array[ 6] * t2[1] + array[10] * t2[0];

   /* determinant: 4 multiplications, 3 additions */
   const T determinant = array[0] * inv_.array[0] + array[4] * inv_.array[1] +
                         array[8] * inv_.array[2] + array[12] * inv_.array[3];

   if( fabs( determinant ) <= tolerance_ )
        return false; // matrix is not invertible

   /* division: 16 multiplications, 1 division */
   const T detinv = static_cast< T >( 1.0 ) / determinant;

   for( size_t i = 0; i != 16; ++i )
       inv_.array[i] *= detinv;

    return true;
}


// this function returns the transpose of a matrix
// however, using matrix::transpose_to( .. ) avoids the copy.
template< size_t M, size_t N, typename T >
inline Matrix< N, M, T >
transpose( const Matrix< M, N, T >& Matrix_ )
{
    Matrix< N, M, T > transpose_;
    Matrix_.transpose_to( transpose_ );
    return transpose_;
}


template< size_t M, size_t N, typename T >
bool is_positive_definite( const Matrix< M, N, T >& matrix_,
    const T limit = -1e12,
    typename enable_if< M == N && M <= 3 >::type* = 0 )
{
    if ( matrix_.at( 0, 0 ) < limit )
        return false;

    // sylvester criterion
    if ( M > 1 )
    {
        Matrix< 2, 2, T > m;
        matrix_.get_sub_matrix( m, 0, 0 );
        if ( compute_determinant( m ) < limit )
            return false;
    }

    if ( M > 2 )
    {
        Matrix< 3, 3, T > m;
        matrix_.get_sub_matrix( m, 0, 0 );
        if ( compute_determinant( m ) < limit )
            return false;
    }

    return true;
}


template< size_t M, size_t N, typename T >
Matrix< M, N, T >::Matrix()
    : array() // http://stackoverflow.com/questions/5602030
{
    if( M == N )
        for( size_t i = 0; i < M; ++i )
            at( i, i ) = static_cast< T >( 1.0 );
}

template< size_t M, size_t N, typename T >
template< size_t P, size_t Q, typename U >
Matrix< M, N, T >::Matrix( const Matrix< P, Q, U >& source_ )
{
    (*this) = source_;
}


template< size_t M, size_t N, typename T >
inline T& Matrix< M, N, T >::at( size_t row_index, size_t col_index )
{
#ifdef VMMLIB_SAFE_ACCESSORS
    if ( row_index >= M || col_index >= N )
        VMMLIB_ERROR( "at( row, col ) - index out of bounds", VMMLIB_HERE );
#endif
    return array[ col_index * M + row_index ];
}



template< size_t M, size_t N, typename T >
const inline T&
Matrix< M, N, T >::at( size_t row_index, size_t col_index ) const
{
#ifdef VMMLIB_SAFE_ACCESSORS
    if ( row_index >= M || col_index >= N )
        VMMLIB_ERROR( "at( row, col ) - index out of bounds", VMMLIB_HERE );
#endif
    return array[ col_index * M + row_index ];
}


template< size_t M, size_t N, typename T >
inline T&
Matrix< M, N, T >::operator()( size_t row_index, size_t col_index )
{
    return at( row_index, col_index );
}



template< size_t M, size_t N, typename T >
const inline T&
Matrix< M, N, T >::operator()( size_t row_index, size_t col_index ) const
{
    return at( row_index, col_index );
}

#ifndef VMMLIB_NO_CONVERSION_OPERATORS

template< size_t M, size_t N, typename T >
Matrix< M, N, T >::
operator T*()
{
    return array;
}



template< size_t M, size_t N, typename T >
Matrix< M, N, T >::
operator const T*() const
{
    return array;
}

#endif


template< size_t M, size_t N, typename T >
bool
Matrix< M, N, T >::
operator==( const Matrix< M, N, T >& other ) const
{
    bool is_ok = true;
    for( size_t i = 0; is_ok && i < M * N; ++i )
    {
        is_ok = array[ i ] == other.array[ i ];
    }
    return is_ok;
}




template< size_t M, size_t N, typename T >
bool
Matrix< M, N, T >::
operator!=( const Matrix< M, N, T >& other ) const
{
    return ! operator==( other );
}



template< size_t M, size_t N, typename T >
bool
Matrix< M, N, T >::
equals( const Matrix< M, N, T >& other, T tolerance ) const
{
    bool is_ok = true;
    for( size_t row_index = 0; is_ok && row_index < M; row_index++)
    {
        for( size_t col_index = 0; is_ok && col_index < N; col_index++)
        {
            is_ok = fabs( at( row_index, col_index ) - other( row_index, col_index ) ) < tolerance;
        }
    }
    return is_ok;
}



template< size_t M, size_t N, typename T >
template< typename compare_t >
bool Matrix< M, N, T >::equals( const Matrix< M, N, T >& other_matrix,
                                compare_t& cmp ) const
{
    bool is_ok = true;
    for( size_t row = 0; is_ok && row < M; ++row )
    {
        for( size_t col = 0; is_ok && col < N; ++col)
        {
            is_ok = cmp( at( row, col ), other_matrix.at( row, col ) );
        }
    }
    return is_ok;
}

#if (( __GNUC__ > 4 ) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 3)) )
#  pragma GCC diagnostic ignored "-Warray-bounds" // gcc 4.4.7 bug WAR
#endif
template< size_t M, size_t N, typename T >
const Matrix< M, N, T >&
Matrix< M, N, T >::operator=( const Matrix< M, N, T >& source_ )
{
    memcpy( array, source_.array, M * N * sizeof( T ));
    return *this;
}
#if (( __GNUC__ > 4 ) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 3)) )
#  pragma GCC diagnostic warning "-Warray-bounds"
#endif

template< size_t M, size_t N, typename T >
template< size_t P, size_t Q, typename U >
void Matrix< M, N, T >::operator=( const Matrix< P, Q, U >& source_ )
{
    const size_t minL =  P < M ? P : M;
    const size_t minC =  Q < N ? Q : N;

    if( minL < M || minC < N )
        zero();

    for ( size_t i = 0 ; i < minL ; i++ )
        for ( size_t j = 0 ; j < minC ; j++ )
            at( i,j ) = static_cast< T >( source_( i, j ));
}


template< size_t M, size_t N, typename T >
void Matrix< M, N, T >::operator=( const T old_fashioned_matrix[ M ][ N ] )
{
    for( size_t row = 0; row < M; row++)
    {
        for( size_t col = 0; col < N; col++)
        {
            at( row, col ) = old_fashioned_matrix[ row ][ col ];
        }
    }

}


// WARNING: data_array[] must be at least of size M * N - otherwise CRASH!
// WARNING: assumes row_by_row layout - if this is not the case,
// use matrix::set( data_array, false )
template< size_t M, size_t N, typename T >
void
Matrix< M, N, T >::operator=( const T* data_array )
{
    set( data_array, data_array + M * N, true );
}



template< size_t M, size_t N, typename T >
void
Matrix< M, N, T >::operator=( const std::vector< T >& data )
{
    if ( data.size() < M * N )
        VMMLIB_ERROR( "index out of bounds.", VMMLIB_HERE );

    set( data.begin(), data.end(), true );
}


template< size_t M, size_t N, typename T >
void
Matrix< M, N, T >::multiply_piecewise( const Matrix& other )
{
    for( size_t row_index = 0; row_index < M; row_index++)
    {
        for( size_t col_index = 0; col_index < N; col_index++ )
        {
            T& value = at( row_index, col_index );
            value *= other.at( row_index, col_index );
        }
    }
}


template< size_t M, size_t N, typename T >
template< size_t P >
void
Matrix< M, N, T >::multiply(
    const Matrix< M, P, T >& left,
    const Matrix< P, N, T >& right
    )
{
    for( size_t row_index = 0; row_index < M; row_index++)
    {
        for( size_t col_index = 0; col_index < N; col_index++)
        {
            T& component = at( row_index, col_index );
            component = static_cast< T >( 0.0 );
            for( size_t p = 0; p < P; p++)
            {
                component += left.at( row_index, p ) * right.at( p, col_index );
            }
        }
    }
}



template< size_t M, size_t N, typename T >
template< size_t P >
Matrix< M, P, T >
Matrix< M, N, T >::operator*( const Matrix< N, P, T >& other ) const
{
    Matrix< M, P, T > result;
    result.multiply( *this, other );
    return result;
}



template< size_t M, size_t N, typename T >
template< size_t O, size_t P, typename TT >
typename enable_if< M == N && O == P && M == O, TT >::type*
Matrix< M, N, T >::operator*=( const Matrix< O, P, TT >& right )
{
    Matrix< M, N, T > copy( *this );
    multiply( copy, right );
    return 0;
}

template< size_t M, size_t N, typename T >
Matrix< M, N, T >
Matrix< M, N, T >::operator/( T scalar )
{
    Matrix< M, N, T > result;

    for( size_t row_index = 0; row_index < M; ++row_index )
    {
        for( size_t col_index = 0; col_index < N; ++col_index )
        {
            result.at( row_index, col_index ) = at( row_index, col_index ) / scalar;
        }
    }
    return result;
}



template< size_t M, size_t N, typename T >
void
Matrix< M, N, T >::operator/=( T scalar )
{
    for( size_t row_index = 0; row_index < M; ++row_index )
    {
        for( size_t col_index = 0; col_index < N; ++col_index )
        {
            at( row_index, col_index ) /= scalar;
        }
    }
}





template< size_t M, size_t N, typename T >
Matrix< M, N, T >
Matrix< M, N, T >::operator*( T scalar )
{
    Matrix< M, N, T > result;

    for( size_t row_index = 0; row_index < M; ++row_index )
    {
        for( size_t col_index = 0; col_index < N; ++col_index )
        {
            result.at( row_index, col_index ) = at( row_index, col_index ) * scalar;
        }
    }
    return result;
}



template< size_t M, size_t N, typename T >
void
Matrix< M, N, T >::operator*=( T scalar )
{
    for( size_t row_index = 0; row_index < M; ++row_index )
    {
        for( size_t col_index = 0; col_index < N; ++col_index )
        {
            at( row_index, col_index ) *= scalar;
        }
    }
}



template< size_t M, size_t N, typename T >
Vector< M, T > Matrix< M, N, T >::operator*( const Vector< N, T >& vec ) const
{
    Vector< M, T > result;

    // this < M, 1 > = < M, P > * < P, 1 >
    for( size_t i = 0; i < M; ++i )
    {
        T tmp( 0 );
        for( size_t j = 0; j < N; ++j )
            tmp += at( i, j ) * vec.at( j );
        result.at( i ) = tmp;
    }
    return result;
}



// transform vector by matrix ( vec = matrix * vec )
// assume homogenous coords( for vec3 = mat4 * vec3 ), e.g. vec[3] = 1.0
template< size_t M, size_t N, typename T >
template< size_t O > Vector< O, T >
Matrix< M, N, T >::operator*( const Vector< O, T >& vector_ ) const
{
    Vector< O, T > result;
    T tmp;
    for( size_t row_index = 0; row_index < M; ++row_index )
    {
        tmp = 0.0;
        for( size_t col_index = 0; col_index < N-1; ++col_index )
        {
            tmp += vector_( col_index ) * at( row_index, col_index );
        }
        if ( row_index < N - 1 )
            result( row_index ) = tmp + at( row_index, N-1 ); // * 1.0 -> homogeneous vec4
        else
        {
            tmp += at( row_index, N - 1  );
            for( size_t col_index = 0; col_index < N - 1; ++col_index )
            {
                result( col_index ) /= tmp;
            }
        }
    }
    return result;
}



template< size_t M, size_t N, typename T >
inline Matrix< M, N, T >
Matrix< M, N, T >::operator-() const
{
    return negate();
}



template< size_t M, size_t N, typename T >
Matrix< M, N, T >
Matrix< M, N, T >::negate() const
{
    Matrix< M, N, T > result;
    result *= -1.0;
    return result;
}



template< size_t M, size_t N, typename T >
void
Matrix< M, N, T >::tensor( const Vector< M, T >& u, const Vector< N, T >& v )
{
    for ( size_t col_index = 0; col_index < N; ++col_index )
        for ( size_t row_index = 0; row_index < M; ++row_index )
            at( row_index, col_index ) = u.array[ col_index ] *
                                         v.array[ row_index ];
}



template< size_t M, size_t N, typename T >
template< size_t uM, size_t vM >
typename enable_if< uM == 3 && vM == 3 && M == N && M == 4 >::type*
Matrix< M, N, T >::tensor( const Vector< uM, T >& u, const Vector< vM, T >& v )
{
    for ( size_t col_index = 0; col_index < 3; ++col_index )
    {
        for ( size_t row_index = 0; row_index < 3; ++row_index )
            at( row_index, col_index ) = u.array[ col_index ] *
                                         v.array[ row_index ];

        at( 3, col_index ) = u.array[ col_index ];
    }

    for ( size_t row_index = 0; row_index < 3; ++row_index )
        at( row_index, 3 ) = v.array[ row_index ];

    at( 3, 3 ) = 1.0;

    return 0;
}



template< size_t M, size_t N, typename T >
void
Matrix< M, N, T >::
transpose_to( Matrix< N, M, T >& tM ) const
{
    for( size_t row = 0; row < M; ++row )
    {
        for( size_t col = 0; col < N; ++col )
        {
            tM.at( col, row ) = at( row, col );
        }
    }
}

template< size_t M, size_t N, typename T >
void
Matrix< M, N, T >::
symmetric_covariance( Matrix< M, M, T >& cov_m_ ) const
{
    T tmp = 0;
    for( size_t row = 0; row < M; ++row )
    {
        for( size_t col = row; col < M; ++col )
        {
            for ( size_t k = 0; k < N; ++k )
            {
                tmp += (at( row, k ) * at( col, k ));
            }

            cov_m_.at( row, col ) = tmp;
            cov_m_.at( col, row ) = tmp;
            tmp = 0;
        }
    }
}



template< size_t M, size_t N, typename T >
Vector< M, T > Matrix< M, N, T >::get_column( size_t index ) const
{
    Vector< M, T > column;
    get_column( index, column );
    return column;
}



template< size_t M, size_t N, typename T >
void Matrix< M, N, T >::get_column( size_t index, Vector< M, T >& column ) const
{
#ifdef VMMLIB_SAFE_ACCESSORS
    if ( index >= N )
        VMMLIB_ERROR( "get_column() - index out of bounds.", VMMLIB_HERE );
#endif

    memcpy( &column.array[0], &array[ M * index ], M * sizeof( T ) );
}



template< size_t M, size_t N, typename T >
void Matrix< M, N, T >::set_column( size_t index, const Vector< M, T >& column )
{
    #ifdef VMMLIB_SAFE_ACCESSORS

    if ( index >= N )
        VMMLIB_ERROR( "set_column() - index out of bounds.", VMMLIB_HERE );

    #endif

    memcpy( array + M * index, column.array, M * sizeof( T ) );
}

template< size_t M, size_t N, typename T >
void Matrix< M, N, T >::get_column( size_t index, Matrix< M, 1, T >& column )
    const
{
#ifdef VMMLIB_SAFE_ACCESSORS
    if ( index >= N )
        VMMLIB_ERROR( "get_column() - index out of bounds.", VMMLIB_HERE );
#endif

    memcpy( column.array, array + M * index, M * sizeof( T ) );
}

template< size_t M, size_t N, typename T >
void Matrix< M, N, T >::set_column( size_t index,
                                    const Matrix< M, 1, T >& column )
{
#ifdef VMMLIB_SAFE_ACCESSORS
    if ( index >= N )
        VMMLIB_ERROR( "set_column() - index out of bounds.", VMMLIB_HERE );
#endif

    memcpy( &array[ M * index ], column.array, M * sizeof( T ) );
}

template< size_t M, size_t N, typename T >
Vector< N, T > Matrix< M, N, T >::get_row( size_t index ) const
{
    Vector< N, T > row;
    get_row( index, row );
    return row;
}

template< size_t M, size_t N, typename T >
void Matrix< M, N, T >::get_row( size_t row_index, Vector< N, T >& row ) const
{
#ifdef VMMLIB_SAFE_ACCESSORS
    if ( row_index >= M )
        VMMLIB_ERROR( "get_row() - index out of bounds.", VMMLIB_HERE );
#endif

    for( size_t col_index = 0; col_index < N; ++col_index )
    {
        row.at( col_index ) = at( row_index, col_index );
    }
}

template< size_t M, size_t N, typename T >
void Matrix< M, N, T >::set_row( size_t row_index, const Vector< N, T >& row )
{
#ifdef VMMLIB_SAFE_ACCESSORS
    if ( row_index >= M )
        VMMLIB_ERROR( "set_row() - index out of bounds.", VMMLIB_HERE );
#endif

    for( size_t col_index = 0; col_index < N; ++col_index )
    {
        at( row_index, col_index ) = row.at( col_index );
    }
}

template< size_t M, size_t N, typename T >
void Matrix< M, N, T >::get_row( size_t row_index, Matrix< 1, N, T >& row )
    const
{
#ifdef VMMLIB_SAFE_ACCESSORS
    if ( row_index >= M )
        VMMLIB_ERROR( "get_row() - index out of bounds.", VMMLIB_HERE );
#endif

    for( size_t col_index = 0; col_index < N; ++col_index )
    {
        row.at( 0, col_index ) = at( row_index, col_index );
    }
}

template< size_t M, size_t N, typename T >
void Matrix< M, N, T >::set_row( size_t row_index,
                                 const Matrix< 1, N, T >& row )
{
#ifdef VMMLIB_SAFE_ACCESSORS
    if ( row_index >= M )
        VMMLIB_ERROR( "set_row() - index out of bounds.", VMMLIB_HERE );
#endif

    for( size_t col_index = 0; col_index < N; ++col_index )
    {
        at( row_index, col_index ) = row.at( 0, col_index );
    }
}

template< size_t M, size_t N, typename T >
size_t
Matrix< M, N, T >::
get_number_of_rows() const
{
    return M;
}



template< size_t M, size_t N, typename T >
size_t
Matrix< M, N, T >::
get_number_of_columns() const
{
    return N;
}



template< size_t M, size_t N, typename T >
void
Matrix< M, N, T >::
fill( T fillValue )
{
    for( size_t row_index = 0; row_index < M; ++row_index )
    {
        for( size_t col_index = 0; col_index < N; ++col_index )
        {
            at( row_index, col_index ) = fillValue;
        }
    }
}


template< size_t M, size_t N, typename T >
inline T&
Matrix< M, N, T >::x()
{
    return array[ 12 ];
}



template< size_t M, size_t N, typename T >
inline T&
Matrix< M, N, T >::y()
{
    return array[ 13 ];
}



template< size_t M, size_t N, typename T >
inline T&
Matrix< M, N, T >::z()
{
    return array[ 14 ];
}

template< size_t M, size_t N, typename T >
template< typename input_iterator_t >
void Matrix< M, N, T >::set( input_iterator_t begin_,
                             input_iterator_t end_, bool row_major_layout )
{
    input_iterator_t it( begin_ );
    if( row_major_layout )
    {
        for( size_t row = 0; row < M; ++row )
        {
            for( size_t col = 0; col < N; ++col, ++it )
            {
                if ( it == end_ )
                    return;
                at( row, col ) = static_cast< T >( *it );
            }
        }
    }
    else
    {
        std::copy( it, it + ( M * N ), begin() );
    }
}



template< size_t M, size_t N, typename T >
void Matrix< M, N, T >::zero()
{
    fill( static_cast< T >( 0.0 ) );
}

template< size_t M, size_t N, typename T >
void Matrix< M, N, T >::operator=( T value_ )
{
    std::fill( begin(), end(), value_ );
}



template< size_t M, size_t N, typename T >
inline Matrix< M, N, T >
Matrix< M, N, T >::operator+( const Matrix< M, N, T >& other ) const
{
    Matrix< M, N, T > result( *this );
    result += other;
    return result;
}



template< size_t M, size_t N, typename T >
void Matrix< M, N, T >::operator+=( const Matrix< M, N, T >& other )
{
    iterator it = begin(), it_end = end();
    const_iterator other_it = other.begin();
    for( ; it != it_end; ++it, ++other_it )
    {
        *it += *other_it;
    }
}


template< size_t M, size_t N, typename T >
void Matrix< M, N, T >::operator+=( T scalar )
{
    iterator it = begin(), it_end = end();
    for( ; it != it_end; ++it )
    {
        *it += scalar;
    }
}

template< size_t M, size_t N, typename T >
inline Matrix< M, N, T >
Matrix< M, N, T >::
operator-( const Matrix< M, N, T >& other ) const
{
    Matrix< M, N, T > result( *this );
    result -= other;
    return result;
}



template< size_t M, size_t N, typename T >
void
Matrix< M, N, T >::
operator-=( const Matrix< M, N, T >& other )
{
    iterator it = begin(), it_end = end();
    const_iterator other_it = other.begin();
    for( ; it != it_end; ++it, ++other_it )
    {
        *it -= *other_it;
    }
}

template< size_t M, size_t N, typename T >
void
Matrix< M, N, T >::operator-=( T scalar )
{
    iterator it = begin(), it_end = end();
    for( ; it != it_end; ++it )
    {
        *it -= scalar;
    }
}


template< size_t M, size_t N, typename T >
template< size_t O, size_t P, size_t Q, size_t R >
typename enable_if< M == O + Q && N == P + R >::type*
Matrix< M, N, T >::direct_sum( const Matrix< O, P, T >& upper_left,
    const Matrix< Q, R, T >& lower_right )
{
    (*this) = static_cast< T >( 0.0 );

    for( size_t row = 0; row < O; ++row )
    {
        for( size_t col = 0; col < P; ++col )
        {
            at( row, col ) = upper_left( row, col );
        }
    }

    for( size_t row = 0; row < Q; ++row )
    {
        for( size_t col = 0; col < R; ++col )
        {
            at( O + row, P + col ) = lower_right( row, col );
        }
    }

    return 0;
}



template< size_t M, size_t N, typename T >
template< size_t O, size_t P >
Matrix< O, P, T >
Matrix< M, N, T >::get_sub_matrix( size_t row_offset, size_t col_offset,
typename enable_if< O <= M && P <= N >::type* ) const
{
    Matrix< O, P, T > result;
    get_sub_matrix( result, row_offset, col_offset );
    return result;
}



template< size_t M, size_t N, typename T >
template< size_t O, size_t P >
typename enable_if< O <= M && P <= N >::type*
Matrix< M, N, T >::
get_sub_matrix( Matrix< O, P, T >& result,
    size_t row_offset, size_t col_offset ) const
{
    #ifdef VMMLIB_SAFE_ACCESSORS
    if ( O + row_offset > M || P + col_offset > N )
        VMMLIB_ERROR( "index out of bounds.", VMMLIB_HERE );
    #endif

    for( size_t row = 0; row < O; ++row )
    {
        for( size_t col = 0; col < P; ++col )
        {
            result.at( row, col )
                = at( row_offset + row, col_offset + col );
        }
    }
    return 0;
}



template< size_t M, size_t N, typename T >
template< size_t O, size_t P >
typename enable_if< O <= M && P <= N >::type*
Matrix< M, N, T >::
set_sub_matrix( const Matrix< O, P, T >& sub_matrix,
    size_t row_offset, size_t col_offset )
{
    for( size_t row = 0; row < O; ++row )
    {
        for( size_t col = 0; col < P; ++col )
        {
            at( row_offset + row, col_offset + col )
                = sub_matrix.at( row, col );
        }
    }
    return 0; // for sfinae
}



template< size_t M, size_t N, typename T >
inline T
Matrix< M, N, T >::det() const
{
    return compute_determinant( *this );
}



template< size_t M, size_t N, typename T >
template< size_t O, size_t P, typename TT >
inline bool Matrix< M, N, T >::inverse( Matrix< O, P, TT >& inverse_,
                                        T tolerance, typename
    enable_if< M == N && O == P && O == M && M >= 2 && M <= 4, TT >::type* )
    const
{
    return compute_inverse( *this, inverse_, tolerance );
}



template< size_t M, size_t N, typename T >
template< size_t O, size_t P >
typename enable_if< O == P && M == N && O == M && M >= 2 >::type*
Matrix< M, N, T >::
get_adjugate( Matrix< O, P, T >& adjugate ) const
{
    get_cofactors( adjugate );
    adjugate = transpose( adjugate );
    return 0;
}



template< size_t M, size_t N, typename T >
template< size_t O, size_t P >
typename enable_if< O == P && M == N && O == M && M >= 2 >::type*
Matrix< M, N, T >::
get_cofactors( Matrix< O, P, T >& cofactors ) const
{
    Matrix< M-1, N-1, T >   minor_;

    const size_t _negate = 1u;
    for( size_t row_index = 0; row_index < M; ++row_index )
    {
        for( size_t col_index = 0; col_index < N; ++col_index )
        {
            if ( ( row_index + col_index ) & _negate )
                cofactors( row_index, col_index ) = -get_minor( minor_, row_index, col_index );
            else
                cofactors( row_index, col_index ) = get_minor( minor_, row_index, col_index );
        }
    }
    return 0;
}



template< size_t M, size_t N, typename T >
template< size_t O, size_t P >
T
Matrix< M, N, T >::
get_minor( Matrix< O, P, T >& minor_, size_t row_to_cut, size_t col_to_cut,
typename enable_if< O == M-1 && P == N-1 && M == N && M >= 2 >::type* ) const
{
    size_t row_offset = 0;
    size_t col_offset = 0;
    for( size_t row_index = 0; row_index < M; ++row_index )
    {
        if ( row_index == row_to_cut )
            row_offset = -1;
        else
        {
            for( size_t col_index = 0; col_index < M; ++col_index )
            {
                if ( col_index == col_to_cut )
                    col_offset = -1;
                else
                    minor_.at( row_index + row_offset, col_index + col_offset )
                        = at( row_index, col_index );
            }
            col_offset = 0;
        }
    }
    return compute_determinant( minor_ );
}

template< size_t M, size_t N, typename T >
template< typename TT >
Matrix< M, N, T >& Matrix< M, N, T >::rotate(
    const TT angle_, const Vector< M-1, T >& axis,
    typename enable_if< M == N && M == 4, TT >::type* )
{
    const T angle = static_cast< T >( angle_ );

    const T sine      = std::sin( angle );
    const T cosine    = std::cos( angle );

    // this is necessary since Visual Studio cannot resolve the
    // std::pow()-call correctly if we just use 2.0 directly.
    // this way, the '2.0' is converted to the same format
    // as the axis components

    const T _zero = 0.0;
    const T one = 1.0;
    const T two = 2.0;

    array[0]  = cosine + ( one - cosine ) * std::pow( axis.array[0], two );
    array[1]  = ( one - cosine ) * axis.array[0] * axis.array[1] + sine * axis.array[2];
    array[2]  = ( one - cosine ) * axis.array[0] * axis.array[2] - sine * axis.array[1];
    array[3]  = _zero;

    array[4]  = ( one - cosine ) * axis.array[0] * axis.array[1] - sine * axis.array[2];
    array[5]  = cosine + ( one - cosine ) * std::pow( axis.array[1], two );
    array[6]  = ( one - cosine ) * axis.array[1] *  axis.array[2] + sine * axis.array[0];
    array[7]  = _zero;

    array[8]  = ( one - cosine ) * axis.array[0] * axis.array[2] + sine * axis.array[1];
    array[9]  = ( one - cosine ) * axis.array[1] * axis.array[2] - sine * axis.array[0];
    array[10] = cosine + ( one - cosine ) * std::pow( axis.array[2], two );
    array[11] = _zero;

    array[12] = _zero;
    array[13] = _zero;
    array[14] = _zero;
    array[15] = one;

    return *this;
}

template< size_t M, size_t N, typename T >
template< typename TT >
Matrix< M, N, T >& Matrix< M, N, T >::rotate_x( const TT angle_,
                              typename enable_if< M == N && M == 4, TT >::type* )
{
    const T angle       = static_cast< T >( angle_ );

    const T sine        = std::sin( angle );
    const T cosine      = std::cos( angle );

    T tmp;

    tmp         = array[ 4 ] * cosine + array[ 8 ] * sine;
    array[ 8 ]  = - array[ 4 ] * sine + array[ 8 ] * cosine;
    array[ 4 ]  = tmp;

    tmp         = array[ 5 ] * cosine + array[ 9 ] * sine;
    array[ 9 ]  = - array[ 5 ] * sine + array[ 9 ] * cosine;
    array[ 5 ]  = tmp;

    tmp         = array[ 6 ] * cosine + array[ 10 ] * sine;
    array[ 10 ] = - array[ 6 ] * sine + array[ 10 ] * cosine;
    array[ 6 ]  = tmp;

    tmp         = array[ 7 ] * cosine + array[ 11 ] * sine;
    array[ 11 ] = - array[ 7 ] * sine + array[ 11 ] * cosine;
    array[ 7 ]  = tmp;

    return *this;
}

template< size_t M, size_t N, typename T >
template< typename TT >
Matrix< M, N, T >& Matrix< M, N, T >::rotate_y( const TT angle_,
                              typename enable_if< M == N && M == 4, TT >::type* )
{
    const T angle = static_cast< T >( angle_ );

    const T sine      = std::sin( angle );
    const T cosine    = std::cos( angle );

    T tmp;

    tmp         = array[ 0 ] * cosine   - array[ 8 ] * sine;
    array[ 8 ]  = array[ 0 ] * sine     + array[ 8 ] * cosine;
    array[ 0 ]  = tmp;

    tmp         = array[ 1 ] * cosine   - array[ 9 ] * sine;
    array[ 9 ]  = array[ 1 ] * sine     + array[ 9 ] * cosine;
    array[ 1 ]  = tmp;

    tmp         = array[ 2 ] * cosine   - array[ 10 ] * sine;
    array[ 10 ] = array[ 2 ] * sine     + array[ 10 ] * cosine;
    array[ 2 ]  = tmp;

    tmp         = array[ 3 ] * cosine   - array[ 11 ] * sine;
    array[ 11 ] = array[ 3 ] * sine     + array[ 11 ] * cosine;
    array[ 3 ]  = tmp;

    return *this;
}

template< size_t M, size_t N, typename T >
template< typename TT >
Matrix< M, N, T >& Matrix< M, N, T >::rotate_z( const TT angle_,
                              typename enable_if< M == N && M == 4, TT >::type* )
{
    const T angle = static_cast< T >( angle_ );

    const T sine      = std::sin( angle );
    const T cosine    = std::cos( angle );

    T tmp;

    tmp         = array[ 0 ] * cosine + array[ 4 ] * sine;
    array[ 4 ]  = - array[ 0 ] * sine + array[ 4 ] * cosine;
    array[ 0 ]  = tmp;

    tmp         = array[ 1 ] * cosine + array[ 5 ] * sine;
    array[ 5 ]  = - array[ 1 ] * sine + array[ 5 ] * cosine;
    array[ 1 ]  = tmp;

    tmp         = array[ 2 ] * cosine + array[ 6 ] * sine;
    array[ 6 ]  = - array[ 2 ] * sine + array[ 6 ] * cosine;
    array[ 2 ]  = tmp;

    tmp         = array[ 3 ] * cosine + array[ 7 ] * sine;
    array[ 7 ]  = - array[ 3 ] * sine + array[ 7 ] * cosine;
    array[ 3 ]  = tmp;

    return *this;
}

template< size_t M, size_t N, typename T >
template< typename TT >
Matrix< M, N, T >& Matrix< M, N, T >::pre_rotate_x( const TT angle_,
                              typename enable_if< M == N && M == 4, TT >::type* )
{
    const T angle = static_cast< T >( angle_ );

    const T sine      = std::sin( angle );
    const T cosine    = std::cos( angle );

    T tmp;

    tmp         = array[ 1 ];
    array[ 1 ]  = array[ 1 ] * cosine + array[ 2 ] * sine;
    array[ 2 ]  = tmp * -sine + array[ 2 ] * cosine;

    tmp         = array[ 5 ];
    array[ 5 ]  = array[ 5 ] * cosine + array[ 6 ] * sine;
    array[ 6 ]  = tmp * -sine + array[ 6 ] * cosine;

    tmp         = array[ 9 ];
    array[ 9 ]  = array[ 9 ] * cosine + array[ 10 ] * sine;
    array[ 10 ] = tmp * -sine + array[ 10 ] * cosine;

    tmp         = array[ 13 ];
    array[ 13 ] = array[ 13 ] * cosine + array[ 14 ] * sine;
    array[ 14 ] = tmp * -sine + array[ 14 ] * cosine;

    return *this;
}

template< size_t M, size_t N, typename T >
template< typename TT >
Matrix< M, N, T >& Matrix< M, N, T >::pre_rotate_y( const TT angle_,
                              typename enable_if< M == N && M == 4, TT >::type* )
{
    const T angle = static_cast< T >( angle_ );

    const T sine      = std::sin( angle );
    const T cosine    = std::cos( angle );

    T tmp;

    tmp         = array[ 0 ];
    array[ 0 ]  = array[ 0 ] * cosine - array[ 2 ] * sine;
    array[ 2 ]  = tmp * sine + array[ 2 ] * cosine;

    tmp         = array[ 4 ];
    array[ 4 ]  = array[ 4 ] * cosine - array[ 6 ] * sine;
    array[ 6 ]  = tmp * sine + array[ 6 ] * cosine;

    tmp         = array[ 8 ];
    array[ 8 ]  = array[ 8 ] * cosine - array[ 10 ] * sine;
    array[ 10 ] = tmp * sine + array[ 10 ] * cosine;

    tmp         = array[ 12 ];
    array[ 12 ] = array[ 12 ] * cosine - array[ 14 ] * sine;
    array[ 14 ] = tmp * sine + array[ 14 ] * cosine;

    return *this;
}

template< size_t M, size_t N, typename T >
template< typename TT >
Matrix< M, N, T >& Matrix< M, N, T >::pre_rotate_z( const TT angle_,
                              typename enable_if< M == N && M == 4, TT >::type* )
{
    const T angle = static_cast< T >( angle_ );

    const T sine      = std::sin( angle );
    const T cosine    = std::cos( angle );

    T tmp;

    tmp         = array[ 0 ];
    array[ 0 ]  = array[ 0 ] * cosine + array[ 1 ] * sine;
    array[ 1 ]  = tmp * -sine + array[ 1 ] * cosine;

    tmp         = array[ 4 ];
    array[ 4 ]  = array[ 4 ] * cosine + array[ 5 ] * sine;
    array[ 5 ]  = tmp * -sine + array[ 5 ] * cosine;

    tmp         = array[ 8 ];
    array[ 8 ]  = array[ 8 ] * cosine + array[ 9 ] * sine;
    array[ 9 ]  = tmp * -sine + array[ 9 ] * cosine;

    tmp         = array[ 12 ];
    array[ 12 ] = array[ 12 ] * cosine + array[ 13 ] * sine;
    array[ 13 ] = tmp * -sine + array[ 13 ] * cosine;

    return *this;
}

template< size_t M, size_t N, typename T >
template< typename TT >
Matrix< M, N, T >& Matrix< M, N, T >::scale( const TT _scale[3],
                              typename enable_if< M == N && M == 4, TT >::type* )
{
    const T scale0 = static_cast< T >( _scale[ 0 ] );
    const T scale1 = static_cast< T >( _scale[ 1 ] );
    const T scale2 = static_cast< T >( _scale[ 2 ] );

    array[0]  *= scale0;
    array[1]  *= scale0;
    array[2]  *= scale0;
    array[3]  *= scale0;
    array[4]  *= scale1;
    array[5]  *= scale1;
    array[6]  *= scale1;
    array[7]  *= scale1;
    array[8]  *= scale2;
    array[9]  *= scale2;
    array[10] *= scale2;
    array[11] *= scale2;

    return *this;
}

template< size_t M, size_t N, typename T >
template< typename TT >
Matrix< M, N, T >& Matrix< M, N, T >::scale( const TT x_, const T y_, const T z_,
                              typename enable_if< M == N && M == 4, TT >::type* )
{
    const T _x = static_cast< T >( x_ );

    array[0]  *= _x;
    array[1]  *= _x;
    array[2]  *= _x;
    array[3]  *= _x;
    array[4]  *= y_;
    array[5]  *= y_;
    array[6]  *= y_;
    array[7]  *= y_;
    array[8]  *= z_;
    array[9]  *= z_;
    array[10] *= z_;
    array[11] *= z_;

    return *this;
}

template< size_t M, size_t N, typename T >
template< typename TT >
inline Matrix< M, N, T >& Matrix< M, N, T >::scale(
    const Vector< 3, TT >& scale_,
    typename enable_if< M == N && M == 4, TT >::type* )
{
    return scale( scale_.array );
}



template< size_t M, size_t N, typename T >
template< typename TT >
Matrix< M, N, T >& Matrix< M, N, T >::scale_translation( const TT scale_[3],
                              typename enable_if< M == N && M == 4, TT >::type* )
{
    array[12] *= static_cast< T >( scale_[0] );
    array[13] *= static_cast< T >( scale_[1] );
    array[14] *= static_cast< T >( scale_[2] );
    return *this;
}

template< size_t M, size_t N, typename T >
template< typename TT >
inline Matrix< M, N, T >& Matrix< M, N, T >::scale_translation(
    const Vector< 3, TT >& scale_,
    typename enable_if< M == N && M == 4, TT >::type* )
{
    return scale_translation( scale_.array );
}

template< size_t M, size_t N, typename T >
template< typename TT >
inline Matrix< M, N, T >& Matrix< M, N, T >::set_translation(
    const TT x_, const TT y_, const TT z_,
    typename enable_if< M == N && M == 4, TT >::type* )
{
    array[12] = static_cast< T >( x_ );
    array[13] = static_cast< T >( y_ );
    array[14] = static_cast< T >( z_ );
    return *this;
}

template< size_t M, size_t N, typename T >
template< typename TT >
inline Matrix< M, N, T >& Matrix< M, N, T >::set_translation( const TT trans[3],
                              typename enable_if< M == N && M == 4, TT >::type* )
{
    array[12] = static_cast< T >( trans[ 0 ] );
    array[13] = static_cast< T >( trans[ 1 ] );
    array[14] = static_cast< T >( trans[ 2 ] );
    return *this;
}

template< size_t M, size_t N, typename T >
template< typename TT >
inline Matrix< M, N, T >& Matrix< M, N, T >::set_translation(
    const Vector< 3, TT >& translation_,
    typename enable_if< M == N && M == 4, TT >::type* )
{
    return set_translation( translation_.array );
}

template< size_t M, size_t N, typename T > template< typename TT > inline
void Matrix< M, N, T >::get_translation( Vector< N-1, TT >& translation_ )
    const
{
    for( size_t i = 0; i < N-1; ++i )
        translation_.array[ i ] = array[ i + M * (N - 1) ];
}


template< size_t M, size_t N, typename T >
inline Vector< N-1, T > Matrix< M, N, T >::get_translation() const
{
    Vector< N-1, T > result;
    get_translation( result );
    return result;
}



template< size_t M, size_t N, typename T >
size_t
Matrix< M, N, T >::
size() const
{
    return M * N;
}



template< size_t M, size_t N, typename T >
typename Matrix< M, N, T >::iterator
Matrix< M, N, T >::
begin()
{
    return array;
}



template< size_t M, size_t N, typename T >
typename Matrix< M, N, T >::iterator
Matrix< M, N, T >::
end()
{
    return array + size();
}



template< size_t M, size_t N, typename T >
typename Matrix< M, N, T >::const_iterator
Matrix< M, N, T >::
begin() const
{
    return array;
}



template< size_t M, size_t N, typename T >
typename Matrix< M, N, T >::const_iterator
Matrix< M, N, T >::
end() const
{
    return array + size();
}



template< size_t M, size_t N, typename T >
typename Matrix< M, N, T >::reverse_iterator
Matrix< M, N, T >::
rbegin()
{
    return array + size() - 1;
}



template< size_t M, size_t N, typename T >
typename Matrix< M, N, T >::reverse_iterator
Matrix< M, N, T >::
rend()
{
    return array - 1;
}



template< size_t M, size_t N, typename T >
typename Matrix< M, N, T >::const_reverse_iterator
Matrix< M, N, T >::
rbegin() const
{
    return array + size() - 1;
}



template< size_t M, size_t N, typename T >
typename Matrix< M, N, T >::const_reverse_iterator
Matrix< M, N, T >::
rend() const
{
    return array - 1;
}



template< size_t M, size_t N, typename T > template< typename init_functor_t >
const Matrix< M, N, T > Matrix< M, N, T >::get_initialized_matrix()
{
    Matrix< M, N, T > matrix_;
    init_functor_t()( matrix_ );
    return matrix_;
}


// it's ugly, but it allows having properly initialized static members
// without having to worry about static initialization order.
template< size_t M, size_t N, typename T >
const Matrix< M, N, T >
Matrix< M, N, T >::IDENTITY(
    Matrix< M, N, T >::get_initialized_matrix<
                           set_to_identity_functor< Matrix< M, N, T > > >( ));


template< size_t M, size_t N, typename T >
const Matrix< M, N, T >
Matrix< M, N, T >::ZERO(
    Matrix< M, N, T >::
        get_initialized_matrix< set_to_zero_functor< Matrix< M, N, T > > >()
    );



template< size_t M, size_t N, typename T >
double
Matrix< M, N, T >::frobenius_norm( ) const
{
    double norm = 0.0;

    const_iterator it = begin(), it_end = end();
    for( ; it != it_end; ++it )
    {
        norm += *it * *it;
    }

    return std::sqrt(norm);
}

template< size_t M, size_t N, typename T >
double
Matrix< M, N, T >::p_norm( double p ) const
{
    double norm = 0.0;

    const_iterator it = begin(), it_end = end();
    for( ; it != it_end; ++it )
    {
        norm += std::pow(*it, p);
    }

    return std::pow(norm,1./p);
}


template< size_t M, size_t N, typename T  >
template< size_t O >
void
Matrix< M, N, T >::khatri_rao_product( const Matrix< O, N, T >& right_, Matrix< M*O, N, T >& prod_ ) const
{
    //build product for every column
    for (size_t col = 0; col < N; ++col )
    {
        for ( size_t m = 0; m < M; ++m )
        {
            for (size_t o = 0; o < O; ++o )
            {
                prod_.at(O*m + o, col) = at( m, col ) * right_.at( o, col );
            }
        }
    }
}

template< size_t M, size_t N, typename T  >
template< size_t O, size_t P >
void
Matrix< M, N, T >::kronecker_product( const Matrix< O, P, T >& right_, Matrix< M*O, N*P, T >& result_ ) const
{
    //build product for every column
    for (size_t m = 0; m < M; ++m )
    {
        for ( size_t n = 0; n < N; ++n )
        {
            for (size_t o = 0; o < O; ++o )
            {
                for (size_t p = 0; p < P; ++p )
                {
                    result_.at(O*m + o, P*n + p) = at( m, n ) * right_.at( o, p );
                }
            }
        }
    }
}


template< size_t M, size_t N, typename T  >
template< typename TT >
void
Matrix< M, N, T >::cast_from( const Matrix< M, N, TT >& other )
{
    typedef vmml::Matrix< M, N, TT > matrix_tt_type ;
    typedef typename matrix_tt_type::const_iterator tt_const_iterator;

    iterator it = begin(), it_end = end();
    tt_const_iterator other_it = other.begin();
    for( ; it != it_end; ++it, ++other_it )
    {
        *it = static_cast< T >( *other_it );
    }
}


template< size_t M, size_t N, typename T  >
T
Matrix< M, N, T >::get_min() const
{
    T min_value = static_cast<T>(std::numeric_limits<T>::max());

    const_iterator  it = begin(),
    it_end = end();
    for( ; it != it_end; ++it)
    {
        if ( *it < min_value ) {
            min_value = *it;
        }
    }
    return min_value;
}

template< size_t M, size_t N, typename T  >
T
Matrix< M, N, T >::get_max() const
{
    T max_value = static_cast<T>(0);

    const_iterator  it = begin(),
    it_end = end();
    for( ; it != it_end; ++it)
    {
        if ( *it > max_value ) {
            max_value = *it;
        }
    }
    return max_value;
}


template< size_t M, size_t N, typename T  >
T
Matrix< M, N, T >::get_abs_min() const
{
    T min_value = static_cast<T>(std::numeric_limits<T>::max());

    const_iterator  it = begin(),
    it_end = end();
    for( ; it != it_end; ++it)
    {
        if ( fabs(*it) < fabs(min_value) ) {
            min_value = fabs(*it);
        }
    }
    return min_value;
}

template< size_t M, size_t N, typename T  >
T
Matrix< M, N, T >::get_abs_max() const
{
    T max_value = static_cast<T>(0);

    const_iterator  it = begin(),
    it_end = end();
    for( ; it != it_end; ++it)
    {
        if ( fabs(*it) > fabs(max_value) ) {
            max_value = fabs(*it);
        }
    }
    return max_value;
}



template< size_t M, size_t N, typename T >
size_t
Matrix< M, N, T >::nnz() const
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

template< size_t M, size_t N, typename T >
size_t
Matrix< M, N, T >::nnz( const T& threshold_ ) const
{
    size_t counter = 0;

    const_iterator  it = begin(),
    it_end = end();
    for( ; it != it_end; ++it)
    {
        if ( fabs(*it) > threshold_ ) {
            ++counter;
        }
    }

    return counter;
}

template< size_t M, size_t N, typename T >
void
Matrix< M, N, T >::threshold( const T& threshold_value_ )
{
    iterator  it = begin(),
    it_end = end();
    for( ; it != it_end; ++it)
    {
        if ( fabs(*it) <= threshold_value_ ) {
            *it = static_cast<T> (0);
        }
    }
}


template< size_t M, size_t N, typename T >
template< typename TT  >
void
Matrix< M, N, T >::quantize_to( Matrix< M, N, TT >& quantized_, const T& min_value, const T& max_value ) const
{
    long max_tt_range = long(std::numeric_limits< TT >::max());
    long min_tt_range = long(std::numeric_limits< TT >::min());
    long tt_range = (max_tt_range - min_tt_range);

    T t_range = max_value - min_value;

    typedef Matrix< M, N, TT > m_tt_type ;
    typedef typename m_tt_type::iterator tt_iterator;
    tt_iterator it_quant = quantized_.begin();
    const_iterator it = begin(), it_end = end();

    for( ; it != it_end; ++it, ++it_quant )
    {
        if (std::numeric_limits<TT>::is_signed ) {
            *it_quant = TT( std::min( std::max( min_tt_range, long(( *it * tt_range / t_range ) + 0.5)), max_tt_range ));
        } else {
            *it_quant = TT( std::min( std::max( min_tt_range, long(((*it - min_value) * tt_range / t_range) + 0.5)), max_tt_range ));
        }
    }
}


template< size_t M, size_t N, typename T >
template< typename TT  >
void
Matrix< M, N, T >::quantize( Matrix< M, N, TT >& quantized_, T& min_value, T& max_value ) const
{
    min_value = get_min();
    max_value = get_max();
    quantize_to( quantized_, min_value, max_value );
}



template< size_t M, size_t N, typename T >
template< typename TT  >
void
Matrix< M, N, T >::dequantize( Matrix< M, N, TT >& dequantized_, const TT& min_value, const TT& max_value ) const
{
    long max_t_range = long(std::numeric_limits< T >::max());
    long min_t_range = long(std::numeric_limits< T >::min());
    long t_range = (max_t_range - min_t_range);

    TT tt_range = max_value - min_value;

    typedef Matrix< M, N, TT > m_tt_type ;
    typedef typename m_tt_type::iterator tt_iterator;
    tt_iterator it_dequant = dequantized_.begin();
    const_iterator it = begin(), it_end = end();
    for( ; it != it_end; ++it, ++it_dequant )
    {
        if (std::numeric_limits<T>::is_signed ) {
            *it_dequant = std::min( std::max( min_value, TT((TT(*it) / t_range) * tt_range)), max_value );
        } else {
            *it_dequant = std::min( std::max( min_value, TT((((TT(*it) / t_range)) * tt_range ) + min_value)), max_value );
        }
    }
}

template< size_t M, size_t N, typename T >
void
Matrix< M, N, T >::columnwise_sum( Vector< N, T>& summed_columns_ ) const
{

    for ( size_t n = 0; n < N; ++n )
    {
        T value = 0;
        for ( size_t m = 0; m < M; ++m )
        {
            value += at( m, n );
        }
        summed_columns_.at( n ) = value;
    }
}

template< size_t M, size_t N, typename T >
double
Matrix< M, N, T >::sum_elements( ) const
{
    double sum = 0.0;

    const_iterator it = begin(), it_end = end();
    for( ; it != it_end; ++it )
    {
        sum += *it;
    }

    return sum;
}

template< size_t M, size_t N, typename T >
template< size_t R>
typename enable_if< R == M && R == N>::type*
Matrix< M, N, T >::diag( const Vector< R, T >& diag_values_ )
{
    zero();
    for( size_t r = 0; r < R; ++r )
    {
        at(r, r) = static_cast< T >( diag_values_.at(r) );
    }
    return 0;
}


template< size_t M, size_t N, typename T >
void
Matrix< M, N, T >::set_dct()
{
    const double num_rows = M;
    double fill_value = 0.0f;
    for( size_t row = 0; row < M; ++row )
    {
        // to receive orthonormality
        const double weight = ( row == 0.0 ) ? std::sqrt(1/num_rows) :
                                               std::sqrt(2/num_rows);
        for( size_t col = 0; col < N; ++col )
        {
            fill_value = (2 * col + 1) * row * M_PI / (2*M);
            fill_value = std::cos( fill_value );
            fill_value *= weight;
            at( row, col ) = static_cast< T >( fill_value )  ;
        }
    }
}


template< size_t M, size_t N, typename T >
void
Matrix< M, N, T >::set_random( int seed )
{
    if ( seed >= 0 )
        srand( seed );

    double fillValue = 0.0f;
    for( size_t row = 0; row < M; ++row )
    {
        for( size_t col = 0; col < N; ++col )
        {
            fillValue = rand();
            fillValue /= RAND_MAX;
            at( row, col ) = -1.0 + 2.0 * static_cast< double >( fillValue )  ;
        }
    }
}

template< size_t M, size_t N, typename T >
void
Matrix< M, N, T >::write_to_raw( const std::string& dir_, const std::string& filename_ ) const
{
    int dir_length = dir_.size() -1;
    int last_separator = dir_.find_last_of( "/");
    std::string path = dir_;
    if (last_separator < dir_length ) {
        path.append( "/" );
    }
    path.append( filename_ );
    //check for format
    if( filename_.find( "raw", filename_.size() -3) == std::string::npos )
    {
        path.append( ".");
        path.append( "raw" );
    }
    std::string path_raw = path;

    std::ofstream outfile;
    outfile.open( path_raw.c_str() );
    if( outfile.is_open() ) {
        size_t len_slice = sizeof(T) * M*N;
        outfile.write( (char*)&(*this), len_slice );
        outfile.close();
    } else {
        std::cout << "no file open" << std::endl;
    }
}

template< size_t M, size_t N, typename T >
void
Matrix< M, N, T >::read_from_raw( const std::string& dir_, const std::string& filename_ )
{
    int dir_length = dir_.size() -1;
    int last_separator = dir_.find_last_of( "/");
    std::string path = dir_;
    if (last_separator < dir_length ) {
        path.append( "/" );
    }
    path.append( filename_ );

    size_t max_file_len = 2147483648u - sizeof(T) ;
    size_t len_data = sizeof(T) * size();
    char* data = new char[ len_data ];
    std::ifstream infile;
    infile.open( path.c_str(), std::ios::in);

    if( infile.is_open())
    {
        iterator  it = begin(),
        it_end = end();

        while ( len_data > 0 )
        {
            size_t len_read = (len_data % max_file_len ) > 0 ?
                                        len_data % max_file_len : len_data;
            len_data -= len_read;
            infile.read( data, len_read );

            T* T_ptr = (T*)&(data[0]);
            for( ; (it != it_end) && (len_read > 0); ++it, len_read -= sizeof(T) )
            {
                *it = *T_ptr; ++T_ptr;
            }
        }

        delete[] data;
        infile.close();
    } else {
        std::cout << "no file open" << std::endl;
    }

}




template< size_t M, size_t N, typename T >
void
Matrix< M, N, T >::write_csv_file( const std::string& dir_, const std::string& filename_ ) const
{
    int dir_length = dir_.size() -1;
    int last_separator = dir_.find_last_of( "/");
    std::string path = dir_;
    if (last_separator < dir_length ) {
        path.append( "/" );
    }
    path.append( filename_ );
    //check for format
    int suffix_pos = filename_.find( "csv", filename_.size() -3);
    if( suffix_pos == (-1)) {
        path.append( ".");
        path.append( "csv" );
    }

    std::ofstream outfile;
    outfile.open( path.c_str() );
    if( outfile.is_open() ) {
        outfile << *this  << std::endl;
        outfile.close();
    } else {
        std::cout << "no file open" << std::endl;
    }

}

template< size_t M, size_t N, typename T >
void
Matrix< M, N, T >::read_csv_file( const std::string& dir_, const std::string& filename_ )
{
    int dir_length = dir_.size() -1;
    int last_separator = dir_.find_last_of( "/");
    std::string path = dir_;
    if (last_separator < dir_length ) {
        path.append( "/" );
    }
    path.append( filename_ );
    //check for format
    int suffix_pos = filename_.find( "csv", filename_.size() -3);
    if( suffix_pos == (-1)) {
        path.append( ".");
        path.append( "csv" );
    }

    std::ifstream infile;
    infile.open( path.c_str(), std::ios::in);
    if( infile.is_open() ) {
        //TODO: not yet implemented
        //infile >> *this  >> std::endl;
        infile.close();
    } else {
        std::cout << "no file open" << std::endl;
    }

}

template< size_t M, size_t N, typename T >
void
Matrix< M, N, T >::read_from_stream( std::istream& is )
{
    for( size_t row_index = 0; row_index < M; ++row_index )
    {
        for( size_t col_index = 0; col_index < N; ++col_index )
        {
            is >> at( row_index, col_index );
        }
    }
}

template< size_t M, size_t N, typename T >
void
Matrix< M, N, T >::write_to_stream( std::ostream& os ) const
{
     for( size_t row_index = 0; row_index < M; ++row_index )
     {
         for( size_t col_index = 0; col_index < N; ++col_index )
         {
             os << at( row_index, col_index ) << " ";
         }
     }
}


template< size_t M, size_t N, typename T >
void
Matrix< M, N, T >::sum_rows( Matrix< M/2, N, T>& other ) const
{
    typedef Vector< N, T > row_type;

    row_type* row0 = new row_type;
    row_type* row1 = new row_type;

    other.zero();

    for ( size_t row = 0; row < M; ++row )
    {
        get_row( row++, *row0 );
        if ( row < M )
        {
            get_row( row, *row1 );
            *row0 += *row1;
            other.set_row( row/2 , *row0 );
        }
    }

    delete row0;
    delete row1;
}

template< size_t M, size_t N, typename T >
void
Matrix< M, N, T >::sum_columns( Matrix< M, N/2, T>& other ) const
{
    typedef Vector< M, T > col_type;

    col_type* col0 = new col_type;
    col_type* col1 = new col_type;

    other.zero();

    for ( size_t col = 0; col< N; ++col )
    {
        get_column( col++, *col0 );
        if ( col < N )
        {
            get_column( col, *col1 );
            *col0 += *col1;
            other.set_column( col/2, *col0 );
        }
    }

    delete col0;
    delete col1;
}



} // namespace vmml

#endif
