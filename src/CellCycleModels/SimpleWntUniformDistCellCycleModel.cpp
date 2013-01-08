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

#include "SimpleWntUniformDistCellCycleModel.hpp"
#include "PanethCellProliferativeType.hpp"
#include "Exception.hpp"

SimpleWntUniformDistCellCycleModel::SimpleWntUniformDistCellCycleModel()
    : SimpleWntCellCycleModel()
{
}


AbstractCellCycleModel* SimpleWntUniformDistCellCycleModel::CreateCellCycleModel()
{
    // Create a new cell-cycle model
    SimpleWntUniformDistCellCycleModel* p_model = new SimpleWntUniformDistCellCycleModel();

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
    p_model->SetUseCellProliferativeTypeDependentG1Duration(mUseCellProliferativeTypeDependentG1Duration);
    p_model->SetWntStemThreshold(mWntStemThreshold);
    p_model->SetWntTransitThreshold(mWntTransitThreshold);
    p_model->SetWntLabelledThreshold(mWntLabelledThreshold);

    return p_model;
}

void SimpleWntUniformDistCellCycleModel::SetG1Duration()
{
    assert(mpCell != NULL);

    RandomNumberGenerator* p_gen = RandomNumberGenerator::Instance();

    if (mpCell->GetCellProliferativeType()->IsType<StemCellProliferativeType>())
    {
        //All cells should behave the same  in a Variable Wnt simulation
        mG1Duration = GetTransitCellG1Duration() -2.0 + 4*p_gen->ranf(); //  U[-2,2]
    }
    else if (mpCell->GetCellProliferativeType()->IsType<TransitCellProliferativeType>())
    {
        mG1Duration = GetTransitCellG1Duration() -2.0 + 4.0*p_gen->ranf(); // U[-2,2]
    }
    else if (mpCell->GetCellProliferativeType()->IsType<DifferentiatedCellProliferativeType>()
            || mpCell->GetCellProliferativeType()->IsType<PanethCellProliferativeType>() )
    {
        mG1Duration = DBL_MAX;
    }
    else
    {
        NEVER_REACHED;
    }

    // Check that the uniform random deviate has not returned a small or negative G1 duration
    if (mG1Duration < mMinimumGapDuration)
    {
        mG1Duration = mMinimumGapDuration;
    }
}


void SimpleWntUniformDistCellCycleModel::UpdateCellCyclePhase()
{
    // Paneth Cells remain differentiated
    if (!(mpCell->GetCellProliferativeType()->IsType<PanethCellProliferativeType>()))
    {
        SimpleWntCellCycleModel::UpdateCellCyclePhase();
    }
}

void SimpleWntUniformDistCellCycleModel::OutputCellCycleModelParameters(out_stream& rParamsFile)
{
	// No extra parameters to output

	// Call method on direct parent class
	SimpleWntCellCycleModel::OutputCellCycleModelParameters(rParamsFile);
}

// Serialization for Boost >= 1.36
#include "SerializationExportWrapperForCpp.hpp"
CHASTE_CLASS_EXPORT(SimpleWntUniformDistCellCycleModel)
