# Arma3-remoteExecWhitelistCreator

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![MSBuild](https://github.com/elmo128/Arma3-remoteExecWhitelistCreator/actions/workflows/msbuild.yml/badge.svg?branch=master)](https://github.com/elmo128/Arma3-remoteExecWhitelistCreator/actions/workflows/msbuild.yml)
[![Handle Release](https://github.com/elmo128/Arma3-CfgRemoteExecWhitelistCreator/actions/workflows/release.yml/badge.svg)](https://github.com/elmo128/Arma3-CfgRemoteExecWhitelistCreator/actions/workflows/release.yml)

## Purpose
Automatic creation of a whitelist for remote execution from the gamefiles, supported by logs and the existing config.

## Capabilities:
1. Scanning the mission files for RemoteExec and RemoteExecCall calls and retreiving the parameters to determine the required settings for the whitelist in the CfgRemoteExec configuration.

2. As this is is a standalone program, there is no way to get information that is calculated during runtime. Because of this there is an option to check loads of config files after playing, if you had eg. previously manually added the following line of sqf code to each remoteexecuted function or file
```
if(isRemoteExecuted)then{format["A3WLC>[%1,%2,%3,%4]","Tag_fnc_functionname",remoteExecutedOwner,clientOwner,isRemoteExecutedJIP]remoteexec["diag_log",2];};
```
`Tag_fnc_functionname` has to be changed to the function name or the magic variable `_fnc_scriptName`, if it is available. However the name must be identical to the command used by remoteexec. The above code might have a negative impact on your network traffic. If you are fine with getting local logfiles from other players, change the last "2" to "clientOwner". To get meaningful logfiles, it might be neccessary to set CfgRemoteExec to mode = 2 and jip = 1. Also Make sure to join in critical situations to get the best results possible for the jip flag.
You can also write code to your desire, as long as  the result  of `format["A3WLC>[%1,%2,%3,%4]","Tag_fnc_functionname",remoteExecutedOwner,clientOwner,isRemoteExecutedJIP` stays the same. `remoteExecutedOwner` is momentarily not used and might be any number. `clientOwner` is the Machine network ID (https://community.bistudio.com/wiki/Multiplayer_Scripting#Machine_network_ID) of the participant who should execute the function and `isRemoteExecutedJIP` must be `true` or `false`, depending on wether the jip flag should be set or not.

3. The program is also capable of retreiving the whole CfgRemoteExec call three located in your mission files, including external files. In case the config is used as a reference, it is assumed, that it is final and thus the extracted values will be used to overwrite the parameters determined by using other methods earlier.

4. The results can either be saved to description.ext, overwriting the existing CfgRemoteExec, to another file, or just output to the terminal.

## Limitations:
It is always a good idea to double check the result!
The result might contain artefacts.
WIP so there might still be bugs.
`bis_fnc_mp` is not supported yet.
Only customized config entries supported yet.

## Instructions:
Launch the executable and follow the instructions in the UI.
Hint: Copy it to your Mission directory to retreive the working directory "automatically".

## Download sorce:
https://github.com/elmo128/Arma3-CfgRemoteExecWhitelistCreator
