[![Slack Status](http://sdlslack.herokuapp.com/badge.svg)](http://slack.smartdevicelink.com)
[![codecov.io](https://codecov.io/github/smartdevicelink/sdl_core/coverage.svg?branch=develop)](https://codecov.io/github/smartdevicelink/sdl_core?branch=develop)
[![Build Status](http://opensdl-jenkins.luxoft.com:8080/view/develop/job/develop_nightly_coverage/badge/icon)](http://opensdl-jenkins.luxoft.com:8080/view/develop/job/develop_nightly_coverage/)

# SmartDeviceLink (SDL)

SmartDeviceLink (SDL) is a standard set of protocols and messages that connect applications on a smartphone to a vehicle head unit. This messaging enables a consumer to interact with their application using common in-vehicle interfaces such as a touch screen display, embedded voice recognition, steering wheel controls and various vehicle knobs and buttons. There are three main components that make up the SDL ecosystem.

  * The [Core](https://github.com/smartdevicelink/sdl_core) component is the software which Vehicle Manufacturers (OEMs)  implement in their vehicle head units. Integrating this component into their head unit and HMI based on a set of guidelines and templates enables access to various smartphone applications.
  * The optional [SDL Server](https://github.com/smartdevicelink/sdl_server) can be used by Vehicle OEMs to update application policies and gather usage information for connected applications.
  * The [iOS](https://github.com/smartdevicelink/sdl_ios) and [Android](https://github.com/smartdevicelink/sdl_android) libraries are implemented by app developers into their applications to enable command and control via the connected head unit.

Pull Requests Welcome!

## Documentation

  * [Software Architecture Document](https://smartdevicelink.com/en/docs/sdl-core/master/software-architecture-document/table-of-contents/)
  * [Transport Manager Programming Guide](https://smartdevicelink.com/en/guides/core/transport-manager-programming/)
  * [Software Detailed Design](https://app.box.com/s/ohgrvemtx39f8hfea1ab676xxrzvyx1y)
  * [Integration Guidelines](https://smartdevicelink.com/en/docs/hmi/master/overview/)
  * [Evolution Proposals versus Issues](https://github.com/smartdevicelink/sdl_evolution/blob/master/proposals_versus_issues.md)

## SDL Core

The Core component of SDL runs on a vehicle's computing system (head unit). Core’s primary responsibility is to pass messages between connected smartphone applications and the vehicle HMI, and pass notifications from the vehicle to those applications. It can connect a smartphone to a vehicle's head unit via a variety of transport protocols such as Bluetooth, USB, Android AOA, and TCP. Once a connection is established, Core discovers compatible applications and displays them to the driver for interaction via voice or display. The core component is implemented into the vehicle HMI based on the integration guidelines above. The core component is configured to follow a set of policies defined in a policy database and updated by a [policy server](https://www.github.com/smartdevicelink/sdl_server). The messaging between a connected application and core is defined by the [Mobile API](https://github.com/smartdevicelink/sdl_core/blob/master/src/components/interfaces/MOBILE_API.xml) and the messaging between sdl core and the vehicle is defined by the [HMI API](https://github.com/smartdevicelink/sdl_core/blob/master/src/components/interfaces/HMI_API.xml).

## Project Status
We're ramping up our efforts to get SmartDeviceLink developed and maintained directly in the open. For the Mobile libraries, we're expecting better integration soon, SDL Core is slightly more complicated. We are currently working on generating documentation, creating a developer portal, an open forum, Mobile validation, and everything else that we've been asked for to renew the community's interest in this project. From a technical standpoint, SDL is stable, and the most work is being put into making it a more robust solution for app connectivity. We are, however, definitely looking for and interested in other people and company's contributions to SDL whether it be feature based, bug fixes, healthy conversation, or even just suggestions for improvement.

# Getting Started
A quick guide to installing, configuring, and running an instance of the SDL Core on a linux OS.

  1. Clone this repository
  2. Create a folder for your build outside of the source folder and run `cmake {path_to_sdl_core_source_folder}` from the build folder you created
  3. If there are any dependency issues, install missing dependencies
  4. Run the following commands to compile and install smartdevicelink



```
%make
%make install
```

# How to Build SDL for QNX7
## Install environment:
1. Install QNX7 SDP into `/opt/qnx700` using QNX Software Center (with [myQNX account](https://www.qnx.com/account))
2. Clone sdl_core repository
3. Set environment variables
```bash
mkdir <path_to_3rd_party_folder>/<3rd_party_folder_name>
export THIRD_PARTY_INSTALL_PREFIX=<path_to_3rd_party_folder>/<3rd_party_folder_name>
export QNX_HOST=/opt/qnx700/host/linux/x86_64
export QNX_TARGET=/opt/qnx700/target/qnx7
```

## Build SDL for QNX7:
1. Build sdl using `CMAKE_TOOLCHAIN_FILE`
```bash
mkdir <path_to_build_folder>/<build_folder_name>
cd <path_to_build_folder>/<build_folder_name>
cmake -DCMAKE_TOOLCHAIN_FILE=<path_to_sdl_core>/toolchains/Toolchain-QNX7-ACC-x86.cmake <path_to_sdl>
make install
```

## Instal qnx image:
1. Open `qnxsoftwarecenter`
2. Open the tab `available`
3. Install the package `QNX SDP 7.0 x86-64 virtual machine for VMware` which is in the `QNX Software Development Platform -> Reference Images`
4. Run image using `VMware`

## Set up environment in qnx image:
1. From `/opt/qnx700/target/qnx7/x86_64/usr/lib` in ubuntu to `usr/lib` in qnx image need to copy:
  * libqdb.so.1
  * libaoi.so.1
  * libicudata.so.58
  * libicui18n.so.58
  * libicuuc.so.58
  * libsqlite3.so.1
  * libstrm.so.1

2. From `/opt/qnx700/target/qnx7/x86_64/usr/sbin` in ubuntu to `usr/sbin` in qnx image need to copy:
  * pps
  * qdb

## Run SDL in QNX7 image
1. copy `<path_to_build_folder>/<build_folder_name>/bin` to qnx image
2. in the image, run `qdbserver.sh` which is located in the bin folder
3. in the image, run `smartDeviceLinkCore` which is located in the bin folder

## Start SDL Core
Once SDL Core is compiled and installed you can start it from the executable in the bin folder

```
%cd bin/
%./start.sh
```

## Start WEB HMI
Web HMI is separated from SDL Core and located in another repository. So to make it workable please do next steps.

  1. Clone http://github.com/smartdevicelink/sdl_hmi.git
  2. Follow the instruction from readme file in sdl_hmi repository.


## A quick note about dependencies
The dependencies for SDL Core vary based on the configuration. You can change SDL Core's configuration in the top level CMakeLists.txt. We have defaulted this file to a configuration which we believe is common for people who are interested in getting up and running quickly, generally on a Linux VM.

### Dependencies list

| Flag | Description | Dependencies |
|------|-------------|--------------|
|Web HMI|Use HTML5 HMI|chromium-browser|
|EXTENDED_MEDIA_MODE|Support Video and Audio Streaming|Opengl es2, gstreamer1.0*|
|Bluetooth|Enable bluetooth transport adapter|libbluetooth3, libbluetooth-dev, bluez-tools|
|USB|Enable USB transport adapter|libusb-dev|
|Testing framework|Needed to support running unit tests|libgtest-dev|
|Cmake|Needed to configure SDL prior to compilation|cmake|

#### Sample Policy Manager
The included sample policy manager (for use with `EXTERNAL_PROPRIETARY` policy mode) requires the following packages:
  - python-pip
  - python-dev

### Known Dependency Issues
  * log4cxx - We know that the version of log4cxx on a linux machine can conflict with the one used, which is why it is provided in the repository. To avoid the conflict, we recommend removing liblog4cxx*.
  * cmake - on some versions of linux, the included cmake package doesn't have the right version. If apt-get is your package manager, you can find the correct version using
```
sudo apt-get install cmake
sudo add-apt-repository ppa:kalakris/cmake
sudo apt-get update
sudo apt-get upgrade
```

## Required RPCs
There are several RPCs that are "required" to be implemented in order for SDL to work across vehicle manufacturers and applications, listed below.  The RPC specification can be found in the [Mobile API Spec](src/components/interfaces/MOBILE_API.xml).

  * RegisterAppInterface
  * UnregisterAppInterface
  * SetGlobalProperties
  * ResetGlobalProperties
  * AddCommand
  * DeleteCommand
  * AddSubMenu
  * DeleteSubMenu
  * CreateInteractionChoiceSet
  * PerformInteraction
  * DeleteInteractionChoiceSet
  * Alert
  * Show
  * SetMediaClockTimer
  * SubscribeButton
  * UnsubscribeButton
  * ChangeRegistration
  * GenericResponse
  * SystemRequest
  * OnHMIStatus
  * OnAppInterfaceUnregistered
  * OnButtonEvent
  * OnButtonPress
  * OnCommand
  * OnDriverDistraction
  * OnPermissionsChange
  * OnLanguageChange
  * OnSystemRequest
  * Speak

## Frequently Asked Questions

If you have any questions regarding the setup of SDL Core or regarding the integration of SDL into an infotainment system, first visit our [SDL Core FAQ](https://smartdevicelink.com/en/guides/core/faq/). If you still have questions, you can post a question in the `core-help` channel of our Slack group (sign up [here](http://slack.smartdevicelink.com/))

## Test Coverage
### Used technologies
  * GCOV - test coverage program.
  * LCOV - graphical front-end for GCC's coverage testing tool for gcov.
  * codecov.io - service for assembling code coverage and representing it in a clear for reading form.

### Excluded folders
_We test only sources written by us and we don`t need to test external sources(open source libraries)._
  * '/usr/\*' - local libraries shouldn`t be covered by tests.
  * '\*/test/\*' - we don`t need to cover tests.
  * '\*/src/3rd\*' - open source libraries shouldn`t be covered by tests.

### Current test coverage
You can find it in [Coverage report](https://codecov.io/gh/smartdevicelink/sdl_core/branch/develop)

### How to get Test Coverage locally
 1. Build project with enabled flag _-DBUILD_TESTS=on_
 2. Execute command 'make test'
 3. Execute './tools/Utils/collect_coverage.sh <path_to_build_directory>'

## Contributions

Conversation regarding the design and development of SmartDeviceLink technology should be directed at the [GENIVI mailing list](https://lists.genivi.org/mailman/listinfo/genivi-smartdevicelink), which anyone can join. Public conference calls regarding the SmartDeviceLink technology will be announced to the GENIVI mailing list, we expect to have conversations every other week. We also encourage interested parties to write issues against our software, and submit pull requests right here in the GitHub repository.
