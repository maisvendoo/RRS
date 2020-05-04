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
#ifndef VMML_VECTOR_TRAITS_HPP
#define VMML_VECTOR_TRAITS_HPP

#include <vmmlib/vector.hpp>
#include <boost/la/vector_traits.hpp>

namespace boost
{
namespace la
{

template<size_t M, typename T>
struct vector_traits<vmml::vector<M, T> >
{
	typedef vmml::vector<M, T> vector_type;

	static const int dim = M;

	typedef T scalar_type;

	template<int I>
	static scalar_type r(const vector_type& v)
	{
		BOOST_STATIC_ASSERT(I >= 0);
		BOOST_STATIC_ASSERT(I < dim);
		return v.array[I];
	}

	template<int I>
	static scalar_type& w(vector_type& v)
	{
		BOOST_STATIC_ASSERT(I >= 0);
		BOOST_STATIC_ASSERT(I < dim);
		return v.array[I];
	}

	static scalar_type ir(int i, const vector_type& v)
	{
		BOOST_ASSERT(i >= 0);
		BOOST_ASSERT(i < dim);
		return v.array[i];
	}

	static scalar_type& iw(int i, vector_type& v)
	{
		BOOST_ASSERT(i >= 0);
		BOOST_ASSERT(i < dim);
		return v.array[i];
	}
};

} // namespace la
} // namespace boost

#endif /* VMML_VECTOR_TRAITS_HPP */

