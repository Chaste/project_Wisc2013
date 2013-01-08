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

#ifndef CRYPTPROLIFERATIONMODEL_HPP_
#define CRYPTPROLIFERATIONMODEL_HPP_

#include "AbstractSystemWithOutputs.hpp"

#include "FileFinder.hpp"

#include "Environment.hpp"

/**
 * This class wraps a particular kind of crypt simulation as a functional curation model.
 */
class CryptProliferationModel : public AbstractSystemWithOutputs
{
public:
    /**
     * This class can represent several variant models; this type is used to select between them.
     */
    enum ModelType
    {
        UNIFORM_WNT,          ///< Use SimpleWntUniformDistCellCycleModel
        VARIABLE_WNT,         ///< Use VariableWntCellCycleModel
        STOCHASTIC_GEN_BASED, ///< Use StochasticDurationGenerationBasedCellCycleModel
        CONTACT_INHIBITION    ///< Use ContactInhibitionGenerationBasedCellCycleModel
    };

    /**
     * Create a new model instance.
     *
     * @param modelType  which specific model to create
     * @param rOutputFolder  where to place temporary model outputs
     */
    CryptProliferationModel(ModelType modelType,
                            const FileFinder& rOutputFolder);

    /**
     * Get the simulation outputs of interest in the Functional Curation data structures.
     * SolveModel must have been called prior to using this method.
     */
    EnvironmentCPtr GetOutputs();

    /**
     * Solve the model - initialises (if not already done) and runs a cell-based simulation.
     *
     * @param endPoint  ignored
     */
    void SolveModel(double endPoint);


    /**
     * Set the bindings from prefix to namespace URI used by the protocol for accessing model
     * variables.  The Environment wrappers around this model can then be created and
     * retrieved with rGetEnvironmentMap.
     *
     * Only the https://chaste.cs.ox.ac.uk/nss/cellbased/0.1# namespace URI is recognised at present.
     *
     * @param rNamespaceBindings  the prefix->URI map
     */
    void SetNamespaceBindings(const std::map<std::string, std::string>& rNamespaceBindings);

private:
    /** Input parameters for the model. */
    EnvironmentPtr mpModelParameters;

    /** Which specific kind of model this is. */
    ModelType mModelType;

    /** Where to place temporary model outputs. */
    FileFinder mOutputFolder;
};

#endif // CRYPTPROLIFERATIONMODEL_HPP_
