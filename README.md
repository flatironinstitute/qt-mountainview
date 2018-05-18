# qt-mountainview

Visualization of spike sorting experiments to be used as a plugin package to mountainlab-js.

This is an intermediate solution while we continue to develop ephys-viz and ev-mountainview.

Since ephys-viz is currently being developed, it does not have nearly as much functionality as our previous viewer (mountainview). Therefore, you will probably also want to install the newly packaged version of this GUI called qt-mountainview, which is designed to be compatible with mountainlab-js.

## Installation instructions

* Be sure to install [mountainlab-js](https://github.com/flatironinstitute/mountainlab-js) along with the plugin packages required for spike sorting (ml_ephys and ml_ms4alg).

* Install Qt5 (see below)

* Clone this repository into ~/.mountainlab/packages (or whatever directory you are using for mountainlab-js packages. It is important that you install this in a location where mountainlab-js searches for processor packages.

* Compile using `./compile_components.sh`

* Add mountainlab-js/bin to your PATH environment variable

## Installing Qt5 (a prerequisite for compiling qt-mountainview)

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

Anaconda users may need to un-export anaconda/miniconda path in order to make qt5 from the operating system available rather than the one supplied with anaconda. To do this, edit your ~/.bashrc file, comment out the export command containing anaconda or miniconda path, and open a new terminal. Make sure you are using your OS' installation by running which qmake
