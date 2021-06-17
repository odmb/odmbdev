# ODMB Development Software

UCSB software to test ODMB functionality, in the process of being updated 
for ODMB7/5 testing. Original from me11/odmbdev.

## Setup
ODMBDEV is a package within the CMS-emu (CSC online software) project that 
runs on the XDAQ platform. To install ODMBDEV, first install 
[XDAQ](https://twiki.cern.ch/twiki/bin/view/CMSPublic/CMSOS), then checkout 
and build the [CMS-emu project](https://gitlab.cern.ch/CMS-Emu/CERN/TriDAS).
To install ODMBDEV, use the following bash commands

``` bash
cd TriDAS/emu
git clone https://github.com/odmb/odmbdev.git
cd odmbdev
./complie.sh
```

After it's setup, the next time it can be started by
``` bash
cd TriDAS/emu/odmbdev
./scripts/startXXX.sh
```
