/*

Copyright (c) 2005-2012, University of Oxford.
All rights reserved.

University of Oxford means the Chancellor, Masters and Scholars of the
University of Oxford, having an administrative office at Wellington
Square, Oxford OX1 2JD, UK.

This file is part of Chaste.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.
 * Neither the name of the University of Oxford nor the names of its
   contributors may be used to endorse or promote products derived from this
   software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "RestrictedEnvironment.hpp"

#include <typeinfo>
#include <boost/foreach.hpp>
#include "BacktraceException.hpp"

typedef std::pair<std::string, AbstractValuePtr> StringValuePair;

RestrictedEnvironment::RestrictedEnvironment(const std::map<std::string,
                                             AbstractValuePtr>& rInitialDefinitions)
    : Environment(true /* allow overwrite */)
{
    BOOST_FOREACH(StringValuePair definition, rInitialDefinitions)
    {
        Environment::DefineName(definition.first, definition.second, "RestrictedEnvironment constructor");
    }
}

/**
 * Modify a name-value mapping in the environment.  This checks that the type of the supplied
 * value matches the current definition, then calls the base class method.
 */
void RestrictedEnvironment::OverwriteDefinition(const std::string& rName, const AbstractValuePtr pValue,
                                                const std::string& rCallerLocation)
{
    std::map<std::string, AbstractValuePtr>::iterator it = mBindings.find(rName);
    if (it != mBindings.end())
    {
        if (!((pValue->IsDouble() == it->second->IsDouble())
              || (pValue->IsArray() == it->second->IsArray())
              || (typeid(*pValue) == typeid(*it->second))))
        {
            PROTO_EXCEPTION2("New definition for '" << rName << "' has a different type.",
                             rCallerLocation);
        }
    }
    Environment::OverwriteDefinition(rName, pValue, rCallerLocation);
}

/**
 * Adding new definitions to a restricted environment is not allowed: this method always throws.
 */
void RestrictedEnvironment::DefineName(const std::string& rName, const AbstractValuePtr pValue,
                                       const std::string& rCallerLocation)
{
    PROTO_EXCEPTION2("New names may not be defined in a restricted environment.",
                     rCallerLocation);
}

/**
 * Removing definitions from a restricted environment is not allowed: this method always throws.
 */
void RestrictedEnvironment::RemoveDefinition(const std::string& rName, const std::string& rCallerLocation)
{
    PROTO_EXCEPTION2("Definitions may not be removed from a restricted environment.",
                     rCallerLocation);
}
