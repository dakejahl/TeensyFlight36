
Overview
=========

Ellipsoid fitting in C++ using Eigen. Widely inspired by https://www.mathworks.com/matlabcentral/fileexchange/24693-ellipsoid-fit

The license that applies to the whole package content is **GNULGPL**. Please look at the license.txt file at the root of this repository.

Here are the basic steps to have OpenPHRI up and running:
 * Clone the PID workspace: `git clone https://github.com/lirmm/pid-workspace.git` or `git clone git@github.com:lirmm/pid-workspace.git` to use SSH instead of HTTPS
 * Go to the `pid` directory: `cd pid-workspace/pid`
 * Configure the workspace: `cmake ..`
 * Deploy ellipsoid-fit and its dependencies: `make deploy package=ellipsoid-fit`
 * Now the library is compiled and available under `pid-workspace/instal/your-architecture/ellipsoid-fit/current-version/`. The dynamic library files are under `lib` while the headers are in the `include` folder.
 * If you want to try out the example applications, go to the package build directory: `cd pid-workspace/packages/ellipsoid-fit/build` then turn the CMake option `BUILD_EXAMPLES` to `ON` with the help of: `ccmake ..`
 * Rebuild the package: `make build`
 * Find and run the applications under the `bin` folder of the install path `pid-workspace/instal/your-architecture/ellipsoid-fit/current-version/`


Installation and Usage
=======================

The procedures for installing the ellipsoid-fit package and for using its components is based on the [PID](http://pid.lirmm.net/pid-framework/pages/install.html) build and deployment system called PID. Just follow and read the links to understand how to install, use and call its API and/or applications.

About authors
=====================

ellipsoid-fit has been developped by following authors: 
+ Benjamin Navarro (LIRMM)

Please contact Benjamin Navarro (navarro@lirmm.fr) - LIRMM for more information or questions.




