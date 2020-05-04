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
#ifndef VMMLIB__VMMLIB_CONFIG__HPP
#define VMMLIB__VMMLIB_CONFIG__HPP

// #define VMMLIB_NO_SFINAE

#ifdef _MSC_VER
#  ifndef HAVE_SSIZE_T
#    include <basetsd.h>
typedef SSIZE_T ssize_t;
#    define HAVE_SSIZE_T
#  endif
#endif

// enabling this switch will have the following effect:
// operator T* will not be compiled, but for vectors,
// operator[] will instead be used. This means you can
// use vec[2] as before, but things like glVertex3fv( vec )
// will not work anymore.
//#define VMMLIB_NO_CONVERSION_OPERATORS

#ifndef VMMLIB_CUSTOM_CONFIG
#  ifndef NDEBUG
#    define VMMLIB_SAFE_ACCESSORS
#  endif
#  define VMMLIB_THROW_EXCEPTIONS
#endif

// Define VMMLIB_NO_TYPEDEFS to prevent creating typedefs for common types (e.g. Vector2i)
//#define VMMLIB_NO_TYPEDEFS

// Define VMMLIB_OLD_TYPEDEFS to keep using the old lowercase names (backwards compatibility)
#define VMMLIB_OLD_TYPEDEFS

#endif
