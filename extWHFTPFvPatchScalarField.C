/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2011-2015 OpenFOAM Foundation
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "extWHFTPFvPatchScalarField.H"
#include "addToRunTimeSelectionTable.H"
#include "fvPatchFieldMapper.H"
#include "volFields.H"
#include "mappedPatchBase.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
    template<>
    const char*
    NamedEnum
    <
        extWHFTPFvPatchScalarField::operationMode,
        3
    >::names[] =
    {
        "fixed_heat_flux",
        "fixed_heat_transfer_coefficient",
        "unknown"
    };

} // End namespace Foam

const Foam::NamedEnum
<
    Foam::extWHFTPFvPatchScalarField::operationMode,
    3
> Foam::extWHFTPFvPatchScalarField::operationModeNames;


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::extWHFTPFvPatchScalarField::
extWHFTPFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF
)
:
    mixedFvPatchScalarField(p, iF),
    temperatureCoupledBase(patch(), "undefined", "undefined", "undefined-K"),
    mode_(unknown),
    q_(p.size(), 0.0),
    h_(p.size(), 0.0),
    Ta_(p.size(), 0.0),
    thicknessLayers_(),
    kappaLayers_(),
    hpr_()
{
    refValue() = 0.0;
    refGrad() = 0.0;
    valueFraction() = 1.0;
}


Foam::extWHFTPFvPatchScalarField::
extWHFTPFvPatchScalarField
(
    const extWHFTPFvPatchScalarField& ptf,
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const fvPatchFieldMapper& mapper
)
:
    mixedFvPatchScalarField(ptf, p, iF, mapper),
    temperatureCoupledBase(patch(), ptf),
    mode_(ptf.mode_),
    q_(ptf.q_, mapper),
    h_(ptf.h_, mapper),
    Ta_(ptf.Ta_, mapper),
    thicknessLayers_(ptf.thicknessLayers_),
    kappaLayers_(ptf.kappaLayers_),
    hpr_(ptf.hpr_().clone().ptr())
{}


Foam::extWHFTPFvPatchScalarField::
extWHFTPFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const dictionary& dict
)
:
    mixedFvPatchScalarField(p, iF),
    temperatureCoupledBase(patch(), dict),
    mode_(unknown),
    q_(p.size(), 0.0),
    h_(p.size(), 0.0),
    Ta_(p.size(), 0.0),
    thicknessLayers_(),
    kappaLayers_(),
    hpr_(DataEntry<scalar>::New("hpr", dict))
{
    if (dict.found("q") && !dict.found("h") && !dict.found("Ta"))
    {
        mode_ = fixedHeatFlux;
        q_ = scalarField("q", dict, p.size());
    }
    else if (dict.found("h") && dict.found("Ta") && !dict.found("q"))
    {
        mode_ = fixedHeatTransferCoeff;
        h_ = scalarField("h", dict, p.size());
        Ta_ = scalarField("Ta", dict, p.size());
        if (dict.found("thicknessLayers"))
        {
            dict.lookup("thicknessLayers") >> thicknessLayers_;
            dict.lookup("kappaLayers") >> kappaLayers_;
        }
    }
    else
    {
        FatalErrorIn
        (
            "extWHFTPFvPatchScalarField::"
            "extWHFTPFvPatchScalarField\n"
            "(\n"
            "    const fvPatch& p,\n"
            "    const DimensionedField<scalar, volMesh>& iF,\n"
            "    const dictionary& dict\n"
            ")\n"
        )   << "\n patch type '" << p.type()
            << "' either q or h and Ta were not found '"
            << "\n for patch " << p.name()
            << " of field " << dimensionedInternalField().name()
            << " in file " << dimensionedInternalField().objectPath()
            << exit(FatalError);
    }

    fvPatchScalarField::operator=(scalarField("value", dict, p.size()));

    if (dict.found("refValue"))
    {
        // Full restart
        refValue() = scalarField("refValue", dict, p.size());
        refGrad() = scalarField("refGradient", dict, p.size());
        valueFraction() = scalarField("valueFraction", dict, p.size());
    }
    else
    {
        // Start from user entered data. Assume fixedValue.
        refValue() = *this;
        refGrad() = 0.0;
        valueFraction() = 1.0;
    }
}


Foam::extWHFTPFvPatchScalarField::
extWHFTPFvPatchScalarField
(
    const extWHFTPFvPatchScalarField& tppsf
)
:
    mixedFvPatchScalarField(tppsf),
    temperatureCoupledBase(tppsf),
    mode_(tppsf.mode_),
    q_(tppsf.q_),
    h_(tppsf.h_),
    Ta_(tppsf.Ta_),
    thicknessLayers_(tppsf.thicknessLayers_),
    kappaLayers_(tppsf.kappaLayers_),
    hpr_(tppsf.hpr_().clone().ptr())
{}


Foam::extWHFTPFvPatchScalarField::
extWHFTPFvPatchScalarField
(
    const extWHFTPFvPatchScalarField& tppsf,
    const DimensionedField<scalar, volMesh>& iF
)
:
    mixedFvPatchScalarField(tppsf, iF),
    temperatureCoupledBase(patch(), tppsf),
    mode_(tppsf.mode_),
    q_(tppsf.q_),
    h_(tppsf.h_),
    Ta_(tppsf.Ta_),
    thicknessLayers_(tppsf.thicknessLayers_),
    kappaLayers_(tppsf.kappaLayers_),
    hpr_(tppsf.hpr_().clone().ptr())
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void Foam::extWHFTPFvPatchScalarField::autoMap
(
    const fvPatchFieldMapper& m
)
{
    mixedFvPatchScalarField::autoMap(m);
    q_.autoMap(m);
    h_.autoMap(m);
    Ta_.autoMap(m);
}


void Foam::extWHFTPFvPatchScalarField::rmap
(
    const fvPatchScalarField& ptf,
    const labelList& addr
)
{
    mixedFvPatchScalarField::rmap(ptf, addr);

    const extWHFTPFvPatchScalarField& tiptf =
        refCast<const extWHFTPFvPatchScalarField>(ptf);

    q_.rmap(tiptf.q_, addr);
    h_.rmap(tiptf.h_, addr);
    Ta_.rmap(tiptf.Ta_, addr);
}


void Foam::extWHFTPFvPatchScalarField::updateCoeffs()
{
    if (updated())
    {
        return;
    }

    const scalarField Tp(*this);
    scalarField hp(patch().size(), 0.0);
    scalarField pressure(patch().size(), 0.0);
    pressure = patch().lookupPatchField<volScalarField, scalar>("p");
    scalarField htcpr(patch().size(), 0.0);

    switch (mode_)
    {
        case fixedHeatFlux:
        {
            refGrad() = (q_)/kappa(Tp);
            refValue() = 0.0;
            valueFraction() = 0.0;

            break;
        }
        case fixedHeatTransferCoeff:
        {
            scalar totalSolidRes = 0.0;
            if (thicknessLayers_.size() > 0)
            {
                forAll (thicknessLayers_, iLayer)
                {
                    const scalar l = thicknessLayers_[iLayer];
                    if (kappaLayers_[iLayer] > 0.0)
                    {
                        totalSolidRes += l/kappaLayers_[iLayer];
                    }
                }
            }

            forAll (htcpr, faceI)
            {
                htcpr[faceI] = hpr_->value(pressure[faceI]);
            }

            hp = 1.0/(1.0/htcpr + totalSolidRes);

            refGrad() = 0.0;
            refValue() = hp*Ta_/hp;
            valueFraction() =
                hp/(hp + kappa(Tp)*patch().deltaCoeffs());

            break;
        }
        default:
        {
            FatalErrorIn
            (
                "extWHFTPFvPatchScalarField"
                "::updateCoeffs()"
            )   << "Illegal heat flux mode " << operationModeNames[mode_]
                << exit(FatalError);
        }
    }

    mixedFvPatchScalarField::updateCoeffs();

    if (debug)
    {
        scalar Q = gSum(kappa(Tp)*patch().magSf()*snGrad());

        Info<< patch().boundaryMesh().mesh().name() << ':'
            << patch().name() << ':'
            << this->dimensionedInternalField().name() << " :"
            << " heat transfer rate:" << Q
            << " walltemperature "
            << " min:" << gMin(*this)
            << " max:" << gMax(*this)
            << " avg:" << gAverage(*this)
            << endl;
    }
}


void Foam::extWHFTPFvPatchScalarField::write
(
    Ostream& os
) const
{
    mixedFvPatchScalarField::write(os);
    temperatureCoupledBase::write(os);

    switch (mode_)
    {

        case fixedHeatFlux:
        {
            q_.writeEntry("q", os);
            break;
        }
        case fixedHeatTransferCoeff:
        {
            h_.writeEntry("h", os);
            Ta_.writeEntry("Ta", os);
            thicknessLayers_.writeEntry("thicknessLayers", os);
            kappaLayers_.writeEntry("kappaLayers", os);
            hpr_->writeData(os);
            break;
        }
        default:
        {
            FatalErrorIn
            (
                "void extWHFTPFvPatchScalarField::write"
                "("
                    "Ostream& os"
                ") const"
            )   << "Illegal heat flux mode " << operationModeNames[mode_]
                << abort(FatalError);
        }
    }
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
    makePatchTypeField
    (
        fvPatchScalarField,
        extWHFTPFvPatchScalarField
    );
}

// ************************************************************************* //
