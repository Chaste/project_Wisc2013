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

#ifndef SIMPLEWNTUNIFORMDISTCELLCYCLEMODEL_HPP_
#define SIMPLEWNTUNIFORMDISTCELLCYCLEMODEL_HPP_

#include "SimpleWntCellCycleModel.hpp"
#include "RandomNumberGenerator.hpp"
#include "WntConcentration.hpp"

/**
 *  Simple Wnt-dependent cell-cycle model with Uniform Distributed Cell Cycle Duration.
 */
class SimpleWntUniformDistCellCycleModel : public SimpleWntCellCycleModel
{
private:

    /** Needed for serialization. */
    friend class boost::serialization::access;
    /**
     * Archive the cell-cycle model, never used directly - boost uses this.
     *
     * @param archive the archive
     * @param version the current version of this class
     */
    template<class Archive>
    void serialize(Archive & archive, const unsigned int version)
    {
        archive & boost::serialization::base_object<SimpleWntCellCycleModel>(*this);
    }

protected:


    /**
     * Overridden SetG1Duration method to implement different ccm duration
     *
     * Stochastically set the G1 duration. The G1 duration is taken
     * from a uniform distribution whose mean is the G1 duration given
     * in AbstractCellCycleModel for the cell type and whose width is 2.
     *
     * Called on cell creation at the start of a simulation, and for both
     * parent and daughter cells at cell division.
     */
    void SetG1Duration();

public:

    /**
     * Constructor - just a default, mBirthTime is now set in the AbstractCellCycleModel class.
     * mG1Duration is set very high, it is set for the individual cells when InitialiseDaughterCell is called.
     */
    SimpleWntUniformDistCellCycleModel();

    /**
     * Overridden builder method to create new copies of
     * this cell-cycle model.
     */
    virtual AbstractCellCycleModel* CreateCellCycleModel();

    /**
     * Overridden UpdateCellCyclePhase() method.
     */
    virtual void UpdateCellCyclePhase();

    /**
     * Set whether Whether the duration of the G1 phase is dependent on Wnt concentration
     * @param useWntDependentDuration - boolean, defaults to true.
     */
    void SetUseWntDependentDuration(bool useWntDependentDuration=true);



    /**
     * Outputs cell-cycle model parameters to file.
     *
     * @param rParamsFile the file stream to which the parameters are output
     */
    virtual void OutputCellCycleModelParameters(out_stream& rParamsFile);
};

#include "SerializationExportWrapper.hpp"
// Declare identifier for the serializer
CHASTE_CLASS_EXPORT(SimpleWntUniformDistCellCycleModel)

#endif /*SIMPLEWNTUNIFORMDISTCELLCYCLEMODEL_HPP_*/
