/*

Copyright (C) University of Oxford, 2005-2012

University of Oxford means the Chancellor, Masters and Scholars of the
University of Oxford, having an administrative office at Wellington
Square, Oxford OX1 2JD, UK.

This file is part of Chaste.

Chaste is free software: you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License as published
by the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Chaste is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
License for more details. The offer of Chaste under the terms of the
License is subject to the License being interpreted in accordance with
English Law and subject to any action against the University of Oxford
being under the jurisdiction of the English Courts.

You should have received a copy of the GNU Lesser General Public License
along with Chaste. If not, see <http://www.gnu.org/licenses/>.

*/

#include "VariableWntCellCycleModel.hpp"
#include "Exception.hpp"
#include "PanethCellProliferativeType.hpp"


VariableWntCellCycleModel::VariableWntCellCycleModel()
    : AbstractSimpleCellCycleModel(),
      mWntTransitThreshold(0.65),
      mInitialWntLevel(1.0) // Initially all cells will be stem like cells
{
}

void VariableWntCellCycleModel::UpdateCellCyclePhase()
{
    // Paneth Cells remain differentiated
    if (mpCell->GetCellProliferativeType()->IsType<PanethCellProliferativeType>())
    {
        mG1Duration = DBL_MAX;
    }
    else
    {
        AbstractSimpleCellCycleModel::UpdateCellCyclePhase();
    }
}

AbstractCellCycleModel* VariableWntCellCycleModel::CreateCellCycleModel()
{
    // Create a new cell-cycle model
    VariableWntCellCycleModel* p_model = new VariableWntCellCycleModel();

    /*
     * Set each member variable of the new cell-cycle model that inherits
     * its value from the parent.
     *
     * Note 1: some of the new cell-cycle model's member variables (namely
     * mBirthTime, mCurrentCellCyclePhase, mReadyToDivide) will already have been
     * correctly initialized in its constructor.
     *
     * Note 2: one or more of the new cell-cycle model's member variables
     * may be set/overwritten as soon as InitialiseDaughterCell() is called on
     * the new cell-cycle model.
     */
    p_model->SetBirthTime(mBirthTime);
    p_model->SetDimension(mDimension);
    p_model->SetMinimumGapDuration(mMinimumGapDuration);
    p_model->SetStemCellG1Duration(mStemCellG1Duration);
    p_model->SetTransitCellG1Duration(mTransitCellG1Duration);
    p_model->SetSDuration(mSDuration);
    p_model->SetG2Duration(mG2Duration);
    p_model->SetMDuration(mMDuration);
    p_model->SetWntTransitThreshold(mWntTransitThreshold);

    // Set the initial Wnt level in the new ccm
    p_model->SetInitialWntLevel(GetWntLevel());

    return p_model;
}

void VariableWntCellCycleModel::SetG1Duration()
{
    assert(mpCell != NULL);

    RandomNumberGenerator* p_gen = RandomNumberGenerator::Instance();

    // Adjust duration based on WNt level so ~24 at base and 12 at mid point
    double g_1_duration = (1.0+12.0*(mInitialWntLevel-mWntTransitThreshold)/(1.0-mWntTransitThreshold))*GetTransitCellG1Duration();

	//All cells should behave the same  in a Variable Wnt simulation
    mG1Duration =  g_1_duration -2.0+4.0*p_gen->ranf(); // U[-2,2]

     // Check that the uniform random deviate has not returned a small or negative G1 duration
    if (mG1Duration < mMinimumGapDuration)
    {
        mG1Duration = mMinimumGapDuration;
    }

    // Now specify the cell type  base on Wnt level

    // Paneth Cells remain differentiated
    if (!(mpCell->GetCellProliferativeType()->IsType<PanethCellProliferativeType>()))
    {
        double wnt_division_threshold = DBL_MAX;

	// Set up under what level of Wnt stimulus a cell will divide
	if (mpCell->GetMutationState()->IsType<WildTypeCellMutationState>())
	{
		wnt_division_threshold = mWntTransitThreshold;
	}
	else if (mpCell->GetMutationState()->IsType<ApcOneHitCellMutationState>())
	{
		// should be less than healthy values
		wnt_division_threshold = 0.77*mWntTransitThreshold;
	}
	else if (mpCell->GetMutationState()->IsType<BetaCateninOneHitCellMutationState>())
	{
		// less than above value
		wnt_division_threshold = 0.155*mWntTransitThreshold;
	}
	else if (mpCell->GetMutationState()->IsType<ApcTwoHitCellMutationState>())
	{
		// should be zero (no Wnt-dependence)
		wnt_division_threshold = 0.0;
	}
	else
	{
		NEVER_REACHED;
	}

	if (mInitialWntLevel<wnt_division_threshold)
        {
            /*
             * This method is usually called within a CellBasedSimulation, after the CellPopulation
             * has called CellPropertyRegistry::TakeOwnership(). This means that were we to call
             * CellPropertyRegistry::Instance() here when setting the CellProliferativeType, we
             * would be creating a new CellPropertyRegistry. In this case the cell proliferative
             * type counts, as returned by AbstractCellPopulation::GetCellProliferativeTypeCount(),
             * would be incorrect. We must therefore access the CellProliferativeType via the cell's
             * CellPropertyCollection.
             */
            boost::shared_ptr<AbstractCellProperty> p_diff_type =
                mpCell->rGetCellPropertyCollection().GetCellPropertyRegistry()->Get<DifferentiatedCellProliferativeType>();
            mpCell->SetCellProliferativeType(p_diff_type);
        }
        else
        {
            boost::shared_ptr<AbstractCellProperty> p_transit_type =
                mpCell->rGetCellPropertyCollection().GetCellPropertyRegistry()->Get<TransitCellProliferativeType>();
            mpCell->SetCellProliferativeType(p_transit_type);
        }
    }
}

double VariableWntCellCycleModel::GetWntLevel()
{
    assert(mpCell != NULL);
    double level = 0;

    switch (mDimension)
    {
        case 1:
        {
            const unsigned DIM = 1;
            level = WntConcentration<DIM>::Instance()->GetWntLevel(mpCell);
            break;
        }
        case 2:
        {
            const unsigned DIM = 2;
            level = WntConcentration<DIM>::Instance()->GetWntLevel(mpCell);
            break;
        }
        case 3:
        {
            const unsigned DIM = 3;
            level = WntConcentration<DIM>::Instance()->GetWntLevel(mpCell);
            break;
        }
        default:
            NEVER_REACHED;
    }

    return level;
}

void VariableWntCellCycleModel::ResetForDivision()
{
    // Reset the initial Wnt level in this ccm and use this to reset the G1Duration
    SetInitialWntLevel(GetWntLevel());
	AbstractSimpleCellCycleModel::ResetForDivision();
}

bool VariableWntCellCycleModel::CanCellTerminallyDifferentiate()
{
    return false;
}

double VariableWntCellCycleModel::GetWntTransitThreshold()
{
    return mWntTransitThreshold;
}

void VariableWntCellCycleModel::SetWntTransitThreshold(double wntTransitThreshold)
{
    //assert(wntTransitThreshold <= 1.0);
    //assert(wntTransitThreshold >= 0.0);
    mWntTransitThreshold = wntTransitThreshold;
}

void VariableWntCellCycleModel::SetInitialWntLevel(double initialWntLevel)
{
	mInitialWntLevel = initialWntLevel;
}

void VariableWntCellCycleModel::OutputCellCycleModelParameters(out_stream& rParamsFile)
{
	  *rParamsFile << "\t\t\t<WntTransitThreshold>" << mWntTransitThreshold << "</WntTransitThreshold>\n";

	// Call method on direct parent class
	  AbstractSimpleCellCycleModel::OutputCellCycleModelParameters(rParamsFile);
}

// Serialization for Boost >= 1.36
#include "SerializationExportWrapperForCpp.hpp"
CHASTE_CLASS_EXPORT(VariableWntCellCycleModel)
