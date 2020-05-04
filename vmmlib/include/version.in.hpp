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

#ifndef VMMLIB_VERSION_H
#define VMMLIB_VERSION_H

namespace vmml
{
    // vmmlib version macros and functions
    /** The current major version. @version 1.5 */
#   define VMMLIB_VERSION_MAJOR @VERSION_MAJOR@

    /** The current minor version. @version 1.5 */
#   define VMMLIB_VERSION_MINOR @VERSION_MINOR@

    /** The current patch level. @version 1.5 */
#   define VMMLIB_VERSION_PATCH @VERSION_PATCH@

/** True if the current version is newer than the given one. @version 1.5 */
#   define VMMLIB_VERSION_GT( MAJOR, MINOR, PATCH )                   \
    ( (VMMLIB_VERSION_MAJOR>MAJOR) ||                                 \
      (VMMLIB_VERSION_MAJOR==MAJOR &&                                 \
       (VMMLIB_VERSION_MINOR>MINOR ||                                 \
        (VMMLIB_VERSION_MINOR==MINOR && VMMLIB_VERSION_PATCH>PATCH))))

/** True if the current version is equal or newer to the given. @version 1.5 */
#   define VMMLIB_VERSION_GE( MAJOR, MINOR, PATCH )                       \
    ( (VMMLIB_VERSION_MAJOR>MAJOR) ||                                     \
      (VMMLIB_VERSION_MAJOR==MAJOR &&                                     \
       (VMMLIB_VERSION_MINOR>MINOR ||                                     \
        (VMMLIB_VERSION_MINOR==MINOR && VMMLIB_VERSION_PATCH>=PATCH))))

/** True if the current version is older than the given one. @version 1.5 */
#   define VMMLIB_VERSION_LT( MAJOR, MINOR, PATCH )                       \
    ( (VMMLIB_VERSION_MAJOR<MAJOR) ||                                     \
      (VMMLIB_VERSION_MAJOR==MAJOR &&                                     \
       (VMMLIB_VERSION_MINOR<MINOR ||                                     \
        (VMMLIB_VERSION_MINOR==MINOR && VMMLIB_VERSION_PATCH<PATCH))))

/** True if the current version is older or equal to the given. @version 1.5 */
#   define VMMLIB_VERSION_LE( MAJOR, MINOR, PATCH )                       \
    ( (VMMLIB_VERSION_MAJOR<MAJOR) ||                                     \
      (VMMLIB_VERSION_MAJOR==MAJOR &&                                     \
       (VMMLIB_VERSION_MINOR<MINOR ||                                     \
        (VMMLIB_VERSION_MINOR==MINOR && VMMLIB_VERSION_PATCH<=PATCH))))
}

#endif //VMMLIB_VERSION_H
