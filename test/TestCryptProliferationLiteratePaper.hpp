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

#ifndef TESTCRYPTPROLIFERATIONLITERATEPAPER_HPP_
#define TESTCRYPTPROLIFERATIONLITERATEPAPER_HPP_

#include <cxxtest/TestSuite.h>

#include <vector>
#include <sstream>
#include <cstdlib> // For system()
#include <boost/assign/list_of.hpp>

#include "CryptProliferationModel.hpp"
#include "Protocol.hpp"
#include "ProtocolParser.hpp"
#include "ProtocolFileFinder.hpp"

#include "FileFinder.hpp"
#include "OutputFileHandler.hpp"
#include "PetscTools.hpp"

#include "PetscSetupAndFinalize.hpp"

class TestCryptProliferationLiteratePaper : public CxxTest::TestSuite
{
public:
    void TestParameterSweep() throw (Exception)
    {
        OutputFileHandler handler("CryptProliferationSweep");
        FileFinder this_test(__FILE__, RelativeTo::ChasteSourceRoot);
        ProtocolFileFinder proto_file("protocols/CryptProliferationSweep.txt", this_test);

        // Allow the different models to be run simultaneously, if multiple processes are available
        PetscTools::IsolateProcesses(true);

        // Loop over available models
        std::vector<CryptProliferationModel::ModelType> model_types = boost::assign::list_of
                (CryptProliferationModel::UNIFORM_WNT)
                (CryptProliferationModel::VARIABLE_WNT)
                (CryptProliferationModel::STOCHASTIC_GEN_BASED)
                (CryptProliferationModel::CONTACT_INHIBITION);

        for (unsigned i=0; i<model_types.size(); i++)
        {
            if (PetscTools::IsParallel() && i % PetscTools::GetNumProcs() != PetscTools::GetMyRank())
            {
                // Let another process run this model
                continue;
            }

            CryptProliferationModel::ModelType model_type = model_types[i];
            // Where to write output for this model
            std::stringstream sub_folder_name;
            sub_folder_name << "model" << model_type;
            OutputFileHandler sub_handler(handler.FindFile(sub_folder_name.str()));

            // Load the model
            boost::shared_ptr<AbstractSystemWithOutputs> p_model(
                    new CryptProliferationModel(model_type,
                                                sub_handler.FindFile("raw_results")));

            // Load the protocol
            ProtocolParser parser;
            ProtocolPtr p_protocol = parser.ParseFile(proto_file);
            p_protocol->SetOutputFolder(sub_handler);
            p_protocol->SetModel(p_model);

            // Run protocol
            try
            {
                p_protocol->RunAndWrite("outputs");
            }
            catch (const Exception& r_error)
            {
                std::cerr << r_error.GetMessage() << std::endl;
                TS_FAIL("Error running " + sub_folder_name.str());
            }
        }

        // Now generate versions of the result plots that are labelled for publication
        PetscTools::IsolateProcesses(false);
        PetscTools::Barrier();
        if (PetscTools::AmMaster())
        {
            FileFinder plot_script("CopyPlots.py", this_test);
            EXPECT0(system, (plot_script.GetAbsolutePath() + " " + handler.GetOutputDirectoryFullPath()).c_str());
        }
   }
};

#endif // TESTCRYPTPROLIFERATIONLITERATEPAPER_HPP_
