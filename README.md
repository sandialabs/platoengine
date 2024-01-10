# PLATO Engine

<p align="center"><img src="https://github.com/platoengine/platoengine/blob/develop/figures/plato_logo_simple.png" width="400"/></p>

# Description
Plato is designed to enable collaborations with academia, national labs and industries by providing open-source tools for optimization-based design. Plato also provides interfaces to enable plug-n-play insertion of external modeling, analysis and optimization tools. Plato comes with a multi-physics finite element solver suited for optimization, algorithms for gradient-based and gradient-free optimization, and many other tools to enable a unique design experience. 

# Getting Started

## Installation
Plato uses [Spack](https://spack.io/) to facilitate and manage its installation. The Plato build instructions can be located in our [Wiki](https://github.com/platoengine/platoengine/wiki/Plato-App).

### Branches
There are two main branches of this repository: 
* **release-v0.1.0** - Most recent stable release of Plato. It may not be up to date with the latest features but it will be more tested. 
* **develop** - Constantly changing and may contain bugs. Checkout the develop branch if you are actively developing in Plato or want access to the latest and greatest features in Plato.

# High Performance Computing
Plato is intended to provide an interoperable platform for optimization-based design. Plato is designed to permit intercommunication of concurrent, parallel modeling, analysis and optimization tools via a Multiple Program, Multiple Data (MPMD) parallel programming model. The MPMD model enables multiple, parallel, independent programs to efficiently communicate in-memory. 

# Required Libraries
Plato relies on [Spack](https://spack.io/) to manage its dependencies. 

# Hardware Requirements
Plato has been mostly installed and tested on Linux operating systems. It has been installed on macOS and Windows OS in the past using Docker. However, the code is not tested nightly on these two operating systems, macOS and Windows, yet. The team strongly advices users to install Plato on Linux systems. 

# Contributing 
Please open a GitHub issue to ask a question, report a bug, request features, etc.  If you'd like to contribute, please fork the repository and use a feature branch.  Make sure to follow the team's [coding style policies](https://github.com/platoengine/platoengine/wiki/Coding-Style).  Pull requests are welcomed.

# User Support
Users are welcomed to submit questions via email to plato3D-help@sandia.gov.
