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

/* Additional files to include on the auto-generated wiki page:
 *
 * Requires: protocols/CryptProliferation.txt
 * Requires: protocols/CryptProliferationSweep.txt
 */

#ifndef TESTCRYPTPROLIFERATIONLITERATEPAPER_HPP_
#define TESTCRYPTPROLIFERATIONLITERATEPAPER_HPP_

/* = Connecting models to data in multiscale multicellular tissue simulations =
 *
 * This Chaste test file runs the main protocols for the above
 * [http://dx.doi.org/10.1016/j.procs.2013.05.235 paper published in ICCS2013].
 *
 * == How to run this code ==
 *
 * For performance, it is recommended to build Chaste using the `GccOptNative` build type when using
 * the Functional Curation extension project, on which this code is built.  You can run the code shown
 * below using the commands:
 * {{{
 * cd path_to_Chaste
 * scons chaste_libs=1 build=GccOptNative projects/Wisc2013/test/TestCryptProliferationLiteratePaper.hpp
 * }}}
 * A clean build of Chaste takes a considerable amount of time.  If you have multiple cores available
 * then the process can be sped up greatly using the '-j' flag to scons, e.g.
 * {{{
 * scons -j4 chaste_libs=1 build=GccOptNative projects/Wisc2013/test/TestCryptProliferationLiteratePaper.hpp
 * }}}
 * to build on 4 cores.  You can additionally run the code itself in parallel, in order to run each value
 * in the main parameter sweep simultaneously, using 5 cores with the `GccOptNative_5` build type, e.g.
 * {{{
 * scons -j4 chaste_libs=1 build=GccOptNative_5 projects/Wisc2013/test/TestCryptProliferationLiteratePaper.hpp
 * }}}
 * With these settings on our test machine, reproducing the paper results takes about 19 hours.
 */

/* == The code itself ==
 *
 * The first step is to include the header files we need.  This code is written as a Chaste test suite, for
 * easy execution using the Chaste build framework.  We thus need to include the `TestSuite.h` header, along
 * with various system libraries, Functional Curation headers, and functionality from core Chaste.
 */
#include <cxxtest/TestSuite.h>

#include <vector>
#include <boost/assign/list_of.hpp>
#include <boost/make_shared.hpp>
#include <boost/foreach.hpp>

// Code in this project, defining the crypt model
#include "CryptProliferationModel.hpp"

// Functional Curation headers
#include "Protocol.hpp"
#include "ProtocolParser.hpp"
#include "ProtocolFileFinder.hpp"
#include "ValueExpression.hpp"

// Core Chaste headers
#include "FileFinder.hpp"
#include "OutputFileHandler.hpp"
#include "PetscTools.hpp"
#include "NumericFileComparison.hpp"

/* This final header, from core Chaste, is needed to enable running in parallel. */
#include "PetscSetupAndFinalize.hpp"

class TestCryptProliferationLiteratePaper : public CxxTest::TestSuite
{
    /*
     * The helper method below runs the given protocol on three different cell-based Chaste models,
     * writing protocol outputs to the given folder.
     *
     * Optionally some of the protocol inputs may be overridden by providing a non-empty map
     * as the third argument.
     *
     * The fourth argument modelsInParallel specifies whether, if multiple processes are available,
     * to run the protocol on multiple models simultaneously, or whether the protocol should try to
     * parallelise internally.
     *
     * If the copyPlots argument is given as true, then all automatically generated results
     * plots in the sub-folder for each model will be copied to the parent results folder,
     * with names that include the model name, for easy inclusion in the paper.
     *
     * If the rCheckResults vector is non-empty, then this list of results data files will be
     * checked against recorded values, in order to ensure that the simulation results have not changed.
     */
    void RunProtocol(const std::string& rProtocolName, const std::string& rOutputFolderName,
                     const std::map<std::string, double>& rProtocolInputs,
                     bool modelsInParallel,
                     bool copyPlots=false,
                     const std::vector<std::string>& rCheckResults=std::vector<std::string>())
    {
        /* Create the folder to which outputs should be written. */
        OutputFileHandler handler(rOutputFolderName);
        /* Locate the protocol definition on the file system. */
        FileFinder this_test(__FILE__, RelativeTo::ChasteSourceRoot);
        ProtocolFileFinder proto_file("protocols/" + rProtocolName + ".txt", this_test);

        if (modelsInParallel)
        {
            /* Allow the different models to be run simultaneously, if multiple processes are available. */
            PetscTools::IsolateProcesses(true);
        }

        /* Loop over available (cell cycle) models. */
        std::vector<CryptProliferationModel::ModelType> model_types = boost::assign::list_of
                (CryptProliferationModel::UNIFORM_WNT)
                (CryptProliferationModel::VARIABLE_WNT)
                (CryptProliferationModel::STOCHASTIC_GEN_BASED);
        for (unsigned i=0; i<model_types.size(); i++)
        {
            if (modelsInParallel && PetscTools::IsParallel() && i % PetscTools::GetNumProcs() != PetscTools::GetMyRank())
            {
                // Let another process run this model
                continue;
            }

            /* Get a human readable name for the model to run in this loop iteration. */
            CryptProliferationModel::ModelType model_type = model_types[i];
            std::string model_name = CryptProliferationModel::GetModelName(model_type);

            /* Output for each model gets written to a sub-folder of the main output folder, named after the model. */
            std::string sub_folder_name = model_name;
            FileFinder::ReplaceSpacesWithUnderscores(sub_folder_name);
            OutputFileHandler sub_handler(handler.FindFile(sub_folder_name));

            /* Load the model to simulate. */
            boost::shared_ptr<AbstractSystemWithOutputs> p_model(new CryptProliferationModel(model_type));

            /* Load the protocol to run on it. */
            ProtocolParser parser;
            ProtocolPtr p_protocol = parser.ParseFile(proto_file);
            p_protocol->SetOutputFolder(sub_handler);
            p_protocol->SetModel(p_model);

            /* As an alternative to running different models on different processes, the Functional Curation system
             * can also parallelise nested simulation loops automatically in some circumstances.  This line turns
             * that functionality on if modelsInParallel is false, which is the case for the main parameter sweep.
             * Thus while generating the steady state plots can use at most 3 processes (corresponding to the 3 cell
             * cycle models) the parameter sweep can use up to 5 (the number of crypt heights to test).
             */
            p_protocol->SetParalleliseLoops(!modelsInParallel);

            /* Override some of the protocol's inputs if requested. */
            typedef std::pair<std::string, double> StringDoublePair;
            BOOST_FOREACH(StringDoublePair input, rProtocolInputs)
            {
                p_protocol->SetInput(input.first, boost::make_shared<ValueExpression>(boost::make_shared<SimpleValue>(input.second)));
            }

            /* By default the Functional Curation system uses the plot titles specified in the protocol, and names
             * the generated files after the title too.  However, where multiple models are being simulated under
             * the same protocol, it is more useful to title plots based on the model name.  We also add a sub-figure
             * index, and adjust the plot page size, so that the generated figures can be included directly in the paper.
             */
            std::string letter(1, 'a' + model_type);
            std::string plot_title = letter + ") " + model_name;
            BOOST_FOREACH(PlotSpecificationPtr p_plot_spec, p_protocol->rGetPlotSpecifications())
            {
                p_plot_spec->SetDisplayTitle(plot_title);
                p_plot_spec->SetGnuplotTerminal("postscript eps enhanced size 4,3 font 16");
            }

            /* Finally, run the protocol on this model, using 'outputs' as the prefix for result file names. */
            try
            {
                p_protocol->RunAndWrite("outputs");

                /* Optionally copy generated plots, as described above.  We find all .eps files in the sub-folder,
                 * and copy them, with a different name, into the main output folder.
                 */
                if (copyPlots && PetscTools::AmMaster())
                {
                    FileFinder model_output_folder = sub_handler.FindFile("");
                    BOOST_FOREACH(FileFinder graph, model_output_folder.FindMatches("*.eps"))
                    {
                        FileFinder dest = handler.FindFile(sub_folder_name + "-" + graph.GetLeafName());
                        graph.CopyTo(dest);
                    }
                }

                /* Check against recorded values for specific results files. */
                BOOST_FOREACH(const std::string& r_result_name, rCheckResults)
                {
                    std::string csv_name = "outputs_" + r_result_name + ".csv";
                    FileFinder new_data = sub_handler.FindFile(csv_name);
                    FileFinder reference_data("data/" + sub_folder_name + "-" + csv_name, this_test);
                    NumericFileComparison comp(new_data, reference_data, false);
                    // The arguments to CompareFiles are absolute tolerance, number of header lines, relative tolerance
                    TS_ASSERT(comp.CompareFiles(1e-4, 1, 1e-6));
                }
            }
            catch (const Exception& r_error)
            {
                /* If an error occurs while running the protocol etc. we display the error message,
                 * but don't terminate execution (since the other models may run successfully).
                 */
                std::cerr << r_error.GetMessage() << std::endl;
                TS_FAIL("Error running " + model_name);
            }
        }

        if (modelsInParallel)
        {
            // Stop isolating processes, to avoid problems running the next protocol.
            PetscTools::IsolateProcesses(false);
        }
    }

public:
    /*
     * This test runs the inner `CryptProliferation` protocol on each model, for the default crypt
     * height of 10 nominal cell diameters.  The raw results from the underlying cell-based Chaste
     * simulation can then be used with the Chaste visualisation tools to produce the crypt schematic
     * figure in the paper (Figure 1).
     *
     * To visualise the results, open a new terminal, `cd` to the Chaste directory,
     * then `cd` to `anim`. Then do:
     * {{{
     * java -cp . Visualize2dCentreCells /tmp/$USER/testoutput/CryptProliferationSteadyState/Stochastic_Generation-based/raw_results/results_from_time_0
     * }}}
     * and
     * {{{
     * java -cp . Visualize2dCentreCells /tmp/$USER/testoutput/CryptProliferationSteadyState/Uniform_Wnt/raw_results/results_from_time_0
     * }}}
     *
     * You may have to do "`javac Visualize2dCentreCells.java`" beforehand to create the java executable.
     * See ChasteGuides/RunningCellBasedVisualization for more detail on the visualisation tool.
     */
    void TestGenerateSteadyStatePlots() throw (Exception)
    {
        std::map<std::string, double> protocol_inputs;
        protocol_inputs["end_time"] = 130.0;
        protocol_inputs["steady_state_time"] = 0.0;
        RunProtocol("CryptProliferation", "CryptProliferationSteadyState", protocol_inputs, true, false);
    }

    /*
     * This test runs the main parameter sweep protocol on each of our three variant models, producing
     * plots (a)-(c) in Figure 2.
     */
    void TestParameterSweep() throw (Exception)
    {
        std::map<std::string, double> protocol_inputs;
        std::vector<std::string> outputs_to_check = boost::assign::list_of("heights")("freqs")("norm_freqs");
        RunProtocol("CryptProliferationSweep", "CryptProliferationSweep", protocol_inputs, false, true, outputs_to_check);
    }
};

#endif // TESTCRYPTPROLIFERATIONLITERATEPAPER_HPP_
