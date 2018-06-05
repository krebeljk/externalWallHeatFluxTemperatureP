# externalWallHeatFluxTemperatureP

This is an OpenFOAM-3.0.1 boundary condition.

It is a modification of the 
`externalWallHeatFluxTemperature`
boundary condition distributed with OpenFOAM.

For more info about OpenFOAM see the official website [openfoam.org](https://openfoam.org/), unofficial wiki
[openfoamwiki.net](https://openfoamwiki.net/index.php/Main_Page) and the community forum
[cfd-online.com](https://www.cfd-online.com/Forums/openfoam/).

## Getting Started

An OpenFOAM 3.0.1 installation is required to run the code. This code was run on Centos 6.6 (Rocks 6.2 Sidewinder).

Run `Allwmake` in the utility directory to compile it.

## Details
The usage of the boundary condition.

### My use -- tested
```cpp
boundaryField
{
    wallIn
    {
        type            externalWallHeatFluxTemperatureP;
        kappa           lookup; //Method for reading thermal conductivity.
        //q               uniform 1000; //ali q, ali Ta in h
        Ta              uniform 50.0;// sink temperature
        h               uniform 1000.0;//not really used anymore
        hpr             table (
                        (0 1019.52) // [MPa, W/m2K]
                        (1e8 5383.56)
                        );
        value           uniform 220.0; //initial temperature [K]
        kappaName       mojKappaOut; // Custom field for thermal conductivity
        Qr              none;
        relaxation      1;
    }
}
```
### General use -- did not test this
```cpp
boundaryField
{
    wallIn
    {
        type            externalWallHeatFluxTemperatureP;
        kappa           fluidThermo;  // fluidThermo, solidThermo or lookup//Method for reading thermal conductivity.
        //q               uniform 1000; //ali q, ali Ta in h
        Ta              uniform 50.0;// sink temperature
        h               uniform 1000.0;//not really used anymore
        hpr             table (
                        (0 1019.52) // [MPa, W/m2K]
                        (1e8 5383.56)
                        );
        value           uniform 220.0; //initial temperature [K]
        kappaName       none; // Custom field for thermal conductivity
        Qr              none;
        relaxation      1;
    }
}
```
### Note
The repository log contains all the modifications from the original code.
This is a very quick modification.
The code is not tidy.
Pull request welcome (tidy up, add test case).

## Feedback

Any feedback is apreciated - krebeljk gmail.com.

## Author

* **Kristjan Krebelj**

## License

This project is licensed under the GPU License - see the [LICENSE.md](LICENSE.md) file for details.

## Acknowledgments

* The work was supported by the [Laboratory for Numerical Modelling and Simulation - LNMS](http://lab.fs.uni-lj.si/lnms/).
