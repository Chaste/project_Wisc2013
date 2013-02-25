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

#include "CryptProliferationModel.hpp"

#include <cassert>
#include <boost/foreach.hpp>

// Functional curation includes
#include "RestrictedEnvironment.hpp"
#include "ArrayFileReader.hpp"
#include "ValueTypes.hpp"
#include "NdArray.hpp"
#include "ProtoHelperMacros.hpp"

// Cell-based Chaste includes
#include "CylindricalHoneycombMeshGenerator.hpp"
#include "Cylindrical2dMesh.hpp"
#include "Cell.hpp"
#include "CryptCellsGenerator.hpp"
#include "SimpleWntUniformDistCellCycleModel.hpp"
#include "StochasticDurationGenerationBasedCellCycleModel.hpp"
#include "ContactInhibitionGenerationBasedCellCycleModel.hpp"
#include "VariableWntCellCycleModel.hpp"
#include "MeshBasedCellPopulationWithGhostNodes.hpp"
#include "VolumeTrackedOffLatticeSimulation.hpp"
#include "GeneralisedLinearSpringForce.hpp"
#include "CellRetainerForce.hpp"
#include "SloughingCellKiller.hpp"
#include "WntConcentration.hpp"
#include "CryptSimulationBoundaryCondition.hpp"


std::string CryptProliferationModel::GetModelName(ModelType modelType)
{
    std::string name;
    switch (modelType)
    {
    case UNIFORM_WNT:
        name = "Uniform Wnt";
        break;
    case VARIABLE_WNT:
        name = "Variable Wnt";
        break;
    case STOCHASTIC_GEN_BASED:
        name = "Stochastic Generation-based";
        break;
    case CONTACT_INHIBITION:
        name = "Contact Inhibition";
        break;
    default:
        NEVER_REACHED;
    }
    return name;
}


CryptProliferationModel::CryptProliferationModel(ModelType modelType,
                                                 const FileFinder& rOutputFolder)
    : mModelType(modelType),
      mOutputFolder(rOutputFolder)
{
    // Set up our parameters environment with default values.
    // CV is a helper macro that converts a double into the wrapped Functional Curation equivalent.
    std::map<std::string, AbstractValuePtr> default_model_params;
    default_model_params["cells_across"] = CV(14);
    default_model_params["crypt_width"] = CV(10);
    default_model_params["crypt_length"] = CV(20);
    default_model_params["cells_up"] = CV(24);
    default_model_params["thickness_of_ghost_layer"] = CV(2);
    default_model_params["end_time"] = CV(50);
    default_model_params["dt_divisor"] = CV(360);
    mpModelParameters.reset(new RestrictedEnvironment(default_model_params));
    // Set up what outputs are available
    mOutputNames.push_back("divisions");
    mOutputUnits.push_back("mixed");
}


void CryptProliferationModel::SetNamespaceBindings(const std::map<std::string, std::string>& rNamespaceBindings)
{
    // Associate whatever prefix is bound to the cell-based namespace with our parameters environment
    mEnvironmentMap.clear();
    typedef std::pair<std::string, std::string> StringPair;
    BOOST_FOREACH(StringPair binding, rNamespaceBindings)
    {
        if (binding.second == "https://chaste.cs.ox.ac.uk/nss/cellbased/0.1#")
        {
            mEnvironmentMap[binding.first] = mpModelParameters;
        }
    }
}


EnvironmentCPtr CryptProliferationModel::GetOutputs()
{
    EnvironmentPtr p_outputs(new Environment);
    FileFinder raw_results("results_from_time_0/divisions.dat", mOutputFolder);
    // The raw results have four whitespace-separated columns: time, x co-ord, y co-ord, parent age
    // We convert this into a 2d array, with the last dimension having extent 4
    ArrayFileReader reader;
    NdArray<double> raw_result_data = reader.ReadFile(raw_results);
    AbstractValuePtr p_results(new ArrayValue(raw_result_data));
    p_results->SetUnits(mOutputUnits.front());
    p_outputs->DefineName(mOutputNames.front(), p_results, "CryptProliferationModel::GetOutputs");
    return p_outputs;
}


/**
 * A handy macro to save typing when reading a typical parameter value
 * @param name  the parameter name (no quotes needed)
 */
#define PARAM(name) GET_SIMPLE_VALUE(mpModelParameters->Lookup(#name, "CryptProliferationModel::SolveModel"))

/**
 * Another short-hand, for getting the cell-cycle model from a cell as the appropriate concrete type.
 * @param type  the desired type
 * @param pCell  the cell
 */
#define CCM(type, pCell) dynamic_cast<type*>(pCell->GetCellCycleModel())

void CryptProliferationModel::SolveModel(double endPoint)
{
    //
    // Set up the simulation object
    //

    // Set up singletons
    SimulationTime::Instance()->SetStartTime(0.0);
    RandomNumberGenerator::Instance()->Reseed(0);
    CellPropertyRegistry::Instance()->Clear();

    // Create the mesh
    CylindricalHoneycombMeshGenerator generator((unsigned)PARAM(cells_across), (unsigned)PARAM(cells_up),
                                                (unsigned)PARAM(thickness_of_ghost_layer),
                                                PARAM(crypt_width)/PARAM(cells_across));
    Cylindrical2dMesh* p_mesh = generator.GetCylindricalMesh();
    std::vector<unsigned> location_indices = generator.GetCellLocationIndices();

    // Create the cells
    std::vector<CellPtr> cells;
    switch (mModelType)
    {
        case CryptProliferationModel::UNIFORM_WNT:
        {
            CryptCellsGenerator<SimpleWntUniformDistCellCycleModel> cells_generator;
            cells_generator.Generate(cells, p_mesh, location_indices, true);
            BOOST_FOREACH(CellPtr p_cell, cells)
            {
                  CCM(SimpleWntUniformDistCellCycleModel, p_cell)->SetWntTransitThreshold(0.5);   // So only proliferate in bottom half of the crypt

                  CCM(SimpleWntUniformDistCellCycleModel, p_cell)->SetMDuration(4.0);
	       		  CCM(SimpleWntUniformDistCellCycleModel, p_cell)->SetSDuration(4.0);
    			  CCM(SimpleWntUniformDistCellCycleModel, p_cell)->SetG2Duration(2.0);
				  CCM(SimpleWntUniformDistCellCycleModel, p_cell)->SetTransitCellG1Duration(2.0);  // so total CCM is U[10,14] at threshold
                  CCM(SimpleWntUniformDistCellCycleModel, p_cell)->SetStemCellG1Duration(14.0);  // so total CCM is U[10,14] at base
            }
            break;
        }
        case CryptProliferationModel::VARIABLE_WNT:
        {
            CryptCellsGenerator<VariableWntCellCycleModel> cells_generator;
            cells_generator.Generate(cells, p_mesh, location_indices, true);
            BOOST_FOREACH(CellPtr p_cell, cells)
            {
                CCM(VariableWntCellCycleModel, p_cell)->SetWntTransitThreshold(0.5);   // So only proliferate in bottom half of the crypt

                CCM(VariableWntCellCycleModel, p_cell)->SetMDuration(4.0);
				CCM(VariableWntCellCycleModel, p_cell)->SetSDuration(4.0);
				CCM(VariableWntCellCycleModel, p_cell)->SetG2Duration(2.0);
				CCM(VariableWntCellCycleModel, p_cell)->SetTransitCellG1Duration(2.0);  // so total CCM is U[10,14] at threshold
				CCM(VariableWntCellCycleModel, p_cell)->SetStemCellG1Duration(14.0);  // so total CCM is U[10,14] at base
            }
            break;
        }
        case CryptProliferationModel::STOCHASTIC_GEN_BASED:
        {
            CryptCellsGenerator<StochasticDurationGenerationBasedCellCycleModel> cells_generator;
            cells_generator.Generate(cells, p_mesh, location_indices, true, 0.0, 3.0, 6.5, 8.0);
            BOOST_FOREACH(CellPtr p_cell, cells)
            {
                CCM(StochasticDurationGenerationBasedCellCycleModel, p_cell)->SetMaxTransitGenerations(4u); // So only proliferate roughly in bottom half of the crypt

                CCM(StochasticDurationGenerationBasedCellCycleModel, p_cell)->SetMDuration(4.0);
				CCM(StochasticDurationGenerationBasedCellCycleModel, p_cell)->SetSDuration(4.0);
				CCM(StochasticDurationGenerationBasedCellCycleModel, p_cell)->SetG2Duration(2.0);
				CCM(StochasticDurationGenerationBasedCellCycleModel, p_cell)->SetTransitCellG1Duration(2.0);  // so total CCM is U[10,14] at threshold
				CCM(StochasticDurationGenerationBasedCellCycleModel, p_cell)->SetStemCellG1Duration(14.0);  // so total CCM is U[10,14] at base


            }
            break;
        }
        case CryptProliferationModel::CONTACT_INHIBITION:
        {
            CryptCellsGenerator<ContactInhibitionGenerationBasedCellCycleModel> cells_generator;
            cells_generator.Generate(cells, p_mesh, location_indices, true, 0.0, 3.0, 6.5, 8.);
            BOOST_FOREACH(CellPtr p_cell, cells)
            {
                CCM(ContactInhibitionGenerationBasedCellCycleModel, p_cell)->SetMaxTransitGenerations(4u); // So only proliferate roughly in bottom half of the crypt
                CCM(ContactInhibitionGenerationBasedCellCycleModel, p_cell)->SetEquilibriumVolume(0.866); //sqrt(3)/2
                CCM(ContactInhibitionGenerationBasedCellCycleModel, p_cell)->SetQuiescentVolumeFraction(0.8);

                CCM(ContactInhibitionGenerationBasedCellCycleModel, p_cell)->SetMDuration(4.0);
				CCM(ContactInhibitionGenerationBasedCellCycleModel, p_cell)->SetSDuration(4.0);
				CCM(ContactInhibitionGenerationBasedCellCycleModel, p_cell)->SetG2Duration(2.0);
				CCM(ContactInhibitionGenerationBasedCellCycleModel, p_cell)->SetTransitCellG1Duration(2.0);  // so total CCM is U[10,14] at threshold
				CCM(ContactInhibitionGenerationBasedCellCycleModel, p_cell)->SetStemCellG1Duration(14.0);  // so total CCM is U[10,14] at base
            }
            break;
        }
        default:
            NEVER_REACHED;
            break;
    }

    // Wrap cells & mesh into a population, and set what outputs to record
    MeshBasedCellPopulationWithGhostNodes<2> crypt(*p_mesh, cells, location_indices);

    // Create the simulator, and set some extra parameters
    VolumeTrackedOffLatticeSimulation<2> simulator(crypt);
    FileFinder test_output_root("", RelativeTo::ChasteTestOutput);
    simulator.SetOutputDirectory(mOutputFolder.GetRelativePath(test_output_root));
    simulator.SetOutputDivisionLocations(true); // The output we're really interested in
    simulator.SetDt(1.0/PARAM(dt_divisor));
    simulator.SetSamplingTimestepMultiple(PARAM(dt_divisor));
    simulator.SetEndTime(PARAM(end_time));

    // The simulation depends on the Wnt concentration
    WntConcentration<2>::Instance()->SetType(LINEAR);
    WntConcentration<2>::Instance()->SetCellPopulation(crypt);
    WntConcentration<2>::Instance()->SetCryptLength(PARAM(crypt_length));

    // Set forces acting on cells
    MAKE_PTR(GeneralisedLinearSpringForce<2>, p_force);
    p_force->SetMeinekeSpringStiffness(100.0); //normally 15.0 but 30 in all CellBased Papers; modified to stop crowding at base of crypt
    p_force->SetCutOffLength(1.5);
    simulator.AddForce(p_force);
    // As there is a WntConcentration the stem cells aren't fixed so we use a CellRetainerForce
    MAKE_PTR(CellRetainerForce<2>, p_retainer_force);
    p_retainer_force->SetStemCellForceMagnitudeParameter(50.0);
    simulator.AddForce(p_retainer_force);

    // Set how cells get killed
    MAKE_PTR_ARGS(SloughingCellKiller<2>, p_cell_killer,(&crypt, PARAM(crypt_length)));
    simulator.AddCellKiller(p_cell_killer);

    // Set boundary conditions
    MAKE_PTR_ARGS(CryptSimulationBoundaryCondition<2>, p_bc, (&crypt));
    p_bc->SetUseJiggledBottomCells(true);
    simulator.AddCellPopulationBoundaryCondition(p_bc);

    //
    // Run the simulation
    //
    simulator.Solve();

    // Clean up singletons
    WntConcentration<2>::Destroy();
    RandomNumberGenerator::Destroy();
    SimulationTime::Destroy();
}
