# qt-mountainview

Visualization of spike sorting experiments to be used as a plugin package to mountainlab-js.

This is an intermediate solution while we continue to develop [ephys-viz](github.com/flatironinstitute/ephys-viz) and ev-mountainview.

Since ephys-viz is currently being developed, it does not have nearly as much functionality as our previous viewer (mountainview). Therefore, you will probably also want to install this application, qt-mountainview, which has been modified to be compatible with mountainlab-js.

## Installation instructions

`qt-mountainvew` is available for both Linux and MacOS.

### Install using conda (recommended)

`qt-mountainview` is available as a [conda](conda.io) pacakge from the same channels as MountainLab. See the [main MountainLab page](https://github.com/flatironinstitute/mountainlab-js/blob/master/README.md) for the latest install instructions.

### Pre-compiled release

Go to this repository's "Releases" page (https://github.com/flatironinstitute/qt-mountainview/releases), and download the latest precompiled binary version available for your OS. Uncompress the archive and place it in your mountainlab processor search path (e.g. in `~/.mountainlab/packages`). You will also likely want to add the folder containing qt-mountainview to your path, so you can call it from the command line.

## Compiling yourself (not recommended)

* Be sure to install [mountainlab-js](https://github.com/flatironinstitute/mountainlab-js) along with the plugin packages required for spike sorting (ml_ephys and ml_ms4alg).

* Install Qt5 (see below)

* Clone this repository into ~/.mountainlab/packages (or whatever directory you are using for mountainlab-js packages). It is important that you install this in a location where mountainlab-js searches for processor packages.

* Compile using `./compile_components.sh`

* Add mountainlab-js/bin to your PATH environment variable

#### Installing Qt5 (a prerequisite for compiling qt-mountainview)

If you are on a later version of Ubuntu (such as 16.04), you can get away with installing Qt5 using the package manager (which is great news. For example, on 16.04, you can do the following:

```
# Note: Run the apt-get commands as root, or using sudo

# Install qt5
apt-get update
apt-get install software-properties-common
apt-add-repository ppa:ubuntu-sdk-team/ppa
apt-get update
apt-get install qtdeclarative5-dev qt5-default qtbase5-dev qtscript5-dev make g++
```

Otherwise (for example if you are on 14.04, or using Mac or other Linux flavor) you should install Qt5 via qt.io website. Go to https://www.qt.io/download/ and click through that you want to install the open source version. Download and run the appropriate installer. Note that you do not need to set up a Qt account -- you can skip that step in the install wizard.

We recommend installing this in your home directory, which does not require admin privileges.

Once installed you will need to prepend the path to qmake to your PATH environment variable. On my system that is /home/magland/Qt/5.7/gcc_64/bin. You may instead do sudo ln -s /home/magland/Qt/5.7/gcc_64/bin/qmake /usr/local/bin/qmake.

Anaconda users may need to un-export anaconda/miniconda path in order to make qt5 from the operating system available rather than the one supplied with anaconda. To do this, edit your ~/.bashrc file, comment out the export command containing anaconda or miniconda path, and open a new terminal. 

Confirm you are using the intended installation of Qt by running which `qmake -v`, and look for the line beginning "Using Qt version <5.x.x> in </path/to/Qt_install>"

### Building the standalone version:

These are instructions for developers who may be building the standalone release. 

* Add `CONFIG+=standalone` to the qmake command line. This will force qmake to bundle all the Qt libraries alongside the binary and link against them.
