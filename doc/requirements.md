# Alazar Library Requirements

## Goals
* Create a portable C API to interface to the Alazar card to simplify integration with other languages like Matlab, Python, etc ...
 * Alazar already provides a C library in the form of a dll but the goal of libAlazar is to minimize the buffering and averaging complexity required by the application.  The libAlazar processing will encapsulate all of the buffer management, averaging, and decimation.
* Mimic the libx6 API as much as possible to keep digitizer interfaces standardized
* SW architecture/API will be portable to other digitizer cards

## Build Requirements
* libAlazar will have its own git repository
* libAlazr will be dynamically linked to the Alazar dll 
 * Assumes that the Alazar SDK has been installed in the correct location, C:\AlazarTech\ATS-SDK\6.0.3
 * Assumes we'll be integrating with the v6.0.3 Alazar SDK
   * NOTE: Tried downloading latest 7.1.1 version but it was a password locked zip file.  Do we have a maintenance contract with Alazar?
* The release will consist of a binary shared library
* Will be built with cmake in an MSYS2 environment - similar to libx6
* src code will be developed using C++
* Require using the Intel IPP Library
 * Is a development license available? 


## Processing Requirements
### Data Flow
* Will provide multiple streams of data that the user can "tap off"
 * The tap points will be the raw physical channel, the demodulated data from a digital downconverter (DDC), and the kernel interated data
* 2 physical channels will be available and 2 DDCs per channel will be available
 * There will be a total of 10 streams of data available; 2 raw streams, 4 demodulated streams, and 4 result streams
 
### Buffering
* Will provide assigning buffers and performing buffer management in a similar fashion to the existing Matlab driver
 * Will determine number of buffers required based on segment length, number of waveforms, and number of round robins
 
### Averaging
* Averaging will be user selectable ( similar to how it is set in the Experiment GUI) and averaging will be done over waveforms and round robins

### Decimation
* The DDC will be implemented in a similar fashion to what is currently done in Matlab
* Decimation filters will be implemented using the IPP API to the extent possible

## Testing Requirements

### Unit Testing
* Unit tests will be developed using the Catch framework [TBD] and continuous
integration testing will be done using Travis [TBD]
* Test vectors will be developed using the APS2 as signal generator

## Risks/Issues/Questions
* Licensing
 * Can the Alazar SDK zip be kept as a binary in github?
* Windows 10 compatability - is the Alazar 6.0.3 driver still compatible
* Throughput should exceed or match the measurement filter processing currently implemented in Matlab
* Are the X6 use cases different than the Alazar - are they supposed to be swappable within a setup?
