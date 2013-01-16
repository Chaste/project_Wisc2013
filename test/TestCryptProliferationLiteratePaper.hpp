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

/* = Connecting models to data in multiscale multicellular tissue simulations =
 *
 * This file runs the main protocols for the above paper to be submitted to WISC2013.
 */

#include <cxxtest/TestSuite.h>

#include <vector>
#include <sstream>
#include <cstdlib> // For system()
#include <boost/assign/list_of.hpp>
#include <boost/make_shared.hpp>
#include <boost/foreach.hpp>

#include "CryptProliferationModel.hpp"
#include "Protocol.hpp"
#include "ProtocolParser.hpp"
#include "ProtocolFileFinder.hpp"
#include "ValueExpression.hpp"

#include "FileFinder.hpp"
#include "OutputFileHandler.hpp"
#include "PetscTools.hpp"

#include "PetscSetupAndFinalize.hpp"

class TestCryptProliferationLiteratePaper : public CxxTest::TestSuite
{
    /*
     * This method runs the given protocol on three different cell-based Chaste models,
     * writing protocol outputs to the given folder.
     *
     * Optionally some of the protocol inputs may be overridden by providing a non-empty map
     * as the third argument.
     *
     * If the copyPlots argument is given as true, then all automatically generated results
     * plots in the sub-folder for each model will be copied to the parent results folder,
     * with names that include the model name, for easy inclusion in the paper.
     */
    void RunProtocol(const std::string& rProtocolName, const std::string& rOutputFolderName,
                     const std::map<std::string, double>& rProtocolInputs,
                     bool copyPlots=false)
    {
        OutputFileHandler handler(rOutputFolderName);
        FileFinder this_test(__FILE__, RelativeTo::ChasteSourceRoot);
        ProtocolFileFinder proto_file("protocols/" + rProtocolName + ".txt", this_test);

        /* Allow the different models to be run simultaneously, if multiple processes are available. */
        PetscTools::IsolateProcesses(true);

        /* Loop over available models. */
        std::vector<CryptProliferationModel::ModelType> model_types = boost::assign::list_of
                (CryptProliferationModel::UNIFORM_WNT)
                (CryptProliferationModel::VARIABLE_WNT)
                (CryptProliferationModel::STOCHASTIC_GEN_BASED);

        for (unsigned i=0; i<model_types.size(); i++)
        {
            if (PetscTools::IsParallel() && i % PetscTools::GetNumProcs() != PetscTools::GetMyRank())
            {
                // Let another process run this model
                continue;
            }

            CryptProliferationModel::ModelType model_type = model_types[i];
            std::string model_name = CryptProliferationModel::GetModelName(model_type);
            // Where to write output for this model
            std::string sub_folder_name = model_name;
            FileFinder::ReplaceSpacesWithUnderscores(sub_folder_name);
            OutputFileHandler sub_handler(handler.FindFile(sub_folder_name));

            // Load the model
            boost::shared_ptr<AbstractSystemWithOutputs> p_model(
                    new CryptProliferationModel(model_type,
                                                sub_handler.FindFile("raw_results")));

            // Load the protocol
            ProtocolParser parser;
            ProtocolPtr p_protocol = parser.ParseFile(proto_file);
            p_protocol->SetOutputFolder(sub_handler);
            p_protocol->SetModel(p_model);

            // Override inputs if requested
            typedef std::pair<std::string, double> StringDoublePair;
            BOOST_FOREACH(StringDoublePair input, rProtocolInputs)
            {
                p_protocol->SetInput(input.first, boost::make_shared<ValueExpression>(boost::make_shared<SimpleValue>(input.second)));
            }

            // Adjust plot titles to be more useful for inclusion in the paper
            // They should look like "a) Model Name"
            std::string letter(1, 'a' + model_type);
            std::string plot_title = letter + ") " + model_name;
            BOOST_FOREACH(PlotSpecificationPtr p_plot_spec, p_protocol->rGetPlotSpecifications())
            {
                p_plot_spec->SetDisplayTitle(plot_title);
                // Also change the plot page size
                p_plot_spec->SetGnuplotTerminal("postscript eps enhanced size 4,3 font 16");
            }

            // Run protocol
            try
            {
                p_protocol->RunAndWrite("outputs");
                if (copyPlots)
                {
                    FileFinder model_output_folder = sub_handler.FindFile("");
                    BOOST_FOREACH(FileFinder graph, model_output_folder.FindMatches("*.eps"))
                    {
                        FileFinder dest = handler.FindFile(sub_folder_name + "-" + graph.GetLeafName());
                        graph.CopyTo(dest);
                    }
                }
            }
            catch (const Exception& r_error)
            {
                std::cerr << r_error.GetMessage() << std::endl;
                TS_FAIL("Error running " + model_name);
            }
        }

        /* Stop isolating processes, to avoid problems running the next protocol. */
        PetscTools::IsolateProcesses(false);
    }

public:
    /*
     * This test runs the inner CryptProliferation protocol on each model, for the default crypt
     * height of 10 nominal cell diameters.  The raw results from the underlying cell-based Chaste
     * simulation can then be used with the Chaste visualisation tools to produce the crypt schematic
     * figure in the paper (Figure N).
     */
    void TestGenerateSteadyStatePlots() throw (Exception)
    {
        std::map<std::string, double> protocol_inputs;
        protocol_inputs["end_time"] = 100.0;
        protocol_inputs["steady_state_time"] = 0.0;
        RunProtocol("CryptProliferation", "CryptProliferationSteadyState", protocol_inputs);
    }

    /*
     * This test runs the main parameter sweep protocol on each of our three variant models.
     */
    void TestParameterSweep() throw (Exception)
    {
        std::map<std::string, double> protocol_inputs;
        RunProtocol("CryptProliferationSweep", "CryptProliferationSweep", protocol_inputs, true);
    }
};

#endif // TESTCRYPTPROLIFERATIONLITERATEPAPER_HPP_
