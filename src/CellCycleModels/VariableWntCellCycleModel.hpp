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

#ifndef VARIABLEWNTCELLCYCLEMODEL_HPP_
#define VARIABLEWNTCELLCYCLEMODEL_HPP_

#include "SimpleWntCellCycleModel.hpp"
#include "RandomNumberGenerator.hpp"
#include "WntConcentration.hpp"

/**
 *  Wnt-dependent cell-cycle model with Uniform Distributed Cell Cycle Duration which depends on the wnt level.
 */
class VariableWntCellCycleModel : public AbstractSimpleCellCycleModel
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
        archive & boost::serialization::base_object<AbstractSimpleCellCycleModel>(*this);

        archive & mWntTransitThreshold;
        archive & mInitialWntLevel;
    }

protected:

    /**
     * Non-dimensionalized Wnt threshold, above which cells progress through the cell cycle.
     */
    double mWntTransitThreshold;

    /**
     * Store the level of Wnt at the birth of the cell
     */
    double mInitialWntLevel;

    /**
     * Overridden SetG1Duration method to implement different ccm duration
     *
     * Stochastically set the G1 duration. The G1 duration is taken
     * from a uniform distribution whose mean is the G1 duration given
     * in AbstractCellCycleModel for the cell type and whose width 2.
     *
     * Called on cell creation at the start of a simulation, and for both
     * parent and daughter cells at cell division.
     */
    void SetG1Duration();

    /**
     * Get the Wnt level experienced by the cell.
     */
    double GetWntLevel();

public:

    /**
     * Constructor - just a default, mBirthTime is now set in the AbstractCellCycleModel class.
     * mG1Duration is set very high, it is set for the individual cells when InitialiseDaughterCell is called.
     */
    VariableWntCellCycleModel();

    /**
     * Overridden UpdateCellCyclePhase() method.
     */
    virtual void UpdateCellCyclePhase();

    /**
     * Overridden builder method to create new copies of
     * this cell-cycle model.
     */
    virtual AbstractCellCycleModel* CreateCellCycleModel();

    /**
	* Overridden CanCellTerminallyDifferentiate() method.
	*/
	virtual bool CanCellTerminallyDifferentiate();

    /**
     * @return mWntTransitThreshold
     */
    double GetWntTransitThreshold();

    /**
     * Set mWntTransitThreshold.
     *
     * @param wntTransitThreshold the value of mWntTransitThreshold
     */
    void SetWntTransitThreshold(double wntTransitThreshold);

    /**
	 * Set The initial level of Wnt the cell experiences
	 * @param initialWntLevel -the level of wnt
	 */
	void SetInitialWntLevel(double initialWntLevel);

    /**
     * Resets the Wnt level at the start of the cell cycle (this model does not cycle naturally)
     *
     * Should only be called by the Cell::Divide() method.
     */
    void ResetForDivision();

    /**
     * Outputs cell-cycle model parameters to file.
     *
     * @param rParamsFile the file stream to which the parameters are output
     */
    virtual void OutputCellCycleModelParameters(out_stream& rParamsFile);
};

#include "SerializationExportWrapper.hpp"
// Declare identifier for the serializer
CHASTE_CLASS_EXPORT(VariableWntCellCycleModel)

#endif /*VARIABLEWNTCELLCYCLEMODEL_HPP_*/
