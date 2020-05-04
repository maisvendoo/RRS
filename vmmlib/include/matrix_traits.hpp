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
//Copyright (c) 2010 Daniel Pfeifer

#ifndef VMML_MATRIX_TRAITS_HPP__
#define VMML_MATRIX_TRAITS_HPP__

#include <vmmlib/matrix.hpp>
#include <boost/la/matrix_traits.hpp>

namespace boost
{
namespace la
{

template<size_t M, size_t N, typename T>
struct matrix_traits<vmml::matrix<M, N, T> >
{
	typedef vmml::matrix<M, N, T> matrix_type;

	static const int rows = M;
	static const int cols = N;

	typedef T scalar_type;

	template<int Row, int Col>
	static scalar_type r(const matrix_type& m)
	{
		BOOST_STATIC_ASSERT(Row >= 0);
		BOOST_STATIC_ASSERT(Row < rows);
		BOOST_STATIC_ASSERT(Col >= 0);
		BOOST_STATIC_ASSERT(Col < cols);
		return m.at(Col, Row);
	}

	template<int Row, int Col>
	static scalar_type& w(matrix_type& m)
	{
		BOOST_STATIC_ASSERT(Row >= 0);
		BOOST_STATIC_ASSERT(Row < rows);
		BOOST_STATIC_ASSERT(Col >= 0);
		BOOST_STATIC_ASSERT(Col < cols);
		return m.at(Col, Row);
	}

	static scalar_type ir(int row, int col, const matrix_type& m)
	{
		BOOST_ASSERT(row >= 0);
		BOOST_ASSERT(row < rows);
		BOOST_ASSERT(col >= 0);
		BOOST_ASSERT(col < cols);
		return m.at(col, row);
	}

	static scalar_type& iw(int row, int col, matrix_type& m)
	{
		BOOST_ASSERT(row >= 0);
		BOOST_ASSERT(row < rows);
		BOOST_ASSERT(col >= 0);
		BOOST_ASSERT(col < cols);
		return m.at(col, row);
	}
};

} // namespace la
} // namespace boost

#endif /* VMML_MATRIX_TRAITS_HPP */

