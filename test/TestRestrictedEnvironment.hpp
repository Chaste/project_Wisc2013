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

#ifndef TESTRESTRICTEDENVIRONMENT_HPP_
#define TESTRESTRICTEDENVIRONMENT_HPP_

#include <cxxtest/TestSuite.h>

#include "RestrictedEnvironment.hpp"

#include <boost/assign/list_of.hpp>
#include "NdArray.hpp"
#include "ValueTypes.hpp"
#include "ProtoHelperMacros.hpp"

class TestRestrictedEnvironment : public CxxTest::TestSuite
{
public:
    void TestMainOperations() throw (Exception)
    {
        NdArray<double> array_0d(1.0);
        AbstractValuePtr p_zero = CV(0.0);
        AbstractValuePtr p_one = boost::make_shared<ArrayValue>(array_0d);
        AbstractValuePtr p_null = boost::make_shared<NullValue>();
        std::map<std::string, AbstractValuePtr> initial_values
            = boost::assign::map_list_of("zero", p_zero)
                                        ("one", p_one);
        EnvironmentPtr p_env(new RestrictedEnvironment(initial_values));
        Environment& env = *p_env;

        TS_ASSERT_THROWS_CONTAINS(env.DefineName("anything", CV(0), "TestRestrictedEnvironment"),
                                  "New names may not be defined in a restricted environment.");
        TS_ASSERT_THROWS_CONTAINS(env.RemoveDefinition("one", "TestRestrictedEnvironment"),
                                  "Definitions may not be removed from a restricted environment.");
        TS_ASSERT_THROWS_CONTAINS(env.OverwriteDefinition("missing", CV(0), "TestRestrictedEnvironment"),
                                  "Name missing is not defined and may not be overwritten.");
        TS_ASSERT_THROWS_CONTAINS(env.OverwriteDefinition("zero", p_null, "TestRestrictedEnvironment"),
                                  "New definition for 'zero' has a different type.");
        TS_ASSERT_EQUALS(env.GetNumberOfDefinitions(), 2u);
        TS_ASSERT_EQUALS(env.GetDefinedNames(), boost::assign::list_of("one")("zero"));
        TS_ASSERT_EQUALS(env.Lookup("zero", "TestRestrictedEnvironment"), p_zero);
        TS_ASSERT_EQUALS(env.Lookup("one", "TestRestrictedEnvironment"), p_one);
    }
};

#endif // TESTRESTRICTEDENVIRONMENT_HPP_
