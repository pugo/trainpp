# TrainPP

A naive attempt to squeeze all my model train controlling needs into one C++ library.

Note: this project is in a very early stage.


## Future plans

The idea is to support at least the following interfaces.

* Roco Z21
* Loconet (through USB dongle)


## Interface idea

My plan is to interface TrainPP up to Python, to allow all states on the track and all controls to be
available in Python.

My next plan is to make a web interface towards the Python layer. That web interface will be written using Vue.js.

The above interface projects will likely be separated from TrainPP.

# Roco Z21 support

The Roco Z21 is a DCC command station. It comes in several models such as the basic Z21 Start as well as the
professional Z21 Pro.

## Supported features

The Roco Z21 support will be complete up to at least the functionality available in the Z21 Start. The
Z21 Pro has support for more interfaces and protocols, for example support for CAN bus and LocoNet. That might
be implemented at some point, but not tested unless I later get a Z21 Pro.

## Implementation

Since the Z21 is very asynchronous in its nature the Z21 integration will be that as well. When creating a `Z21`
object and using that to connect to a Z21 device, the current state on the track should be available automatically.
Commands to control trains, turnouts, etc., will be sent asynchronously to the Z21. The updated status on
the track will be updated at some point, with the regular flow of status information from Z21. 

# Comments

Any comments, help or anything can be sent to me:

Anders Piniesjö <pugo (at) pugo.org>
