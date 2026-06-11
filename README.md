# ChromeMV2Launcher 

> _CUHK 2025-26 CSCI4998 Final Year Project_

**Bypass Manifest V2 Deprecation in Chrome/Chromium**

## Background

Google is transforming from Manifest V2 (MV2) to Manifest V3 (MV3) in extensions in order to improve security, privacy, and performance. However, it breaks a variety of useful extensions like adblockers. Although in the current stage MV2 functionality can still be re-enabled with combinations of flags or launch arguments, these switches are expected to be removed in future updates, [as stated by Google](https://developer.chrome.com/docs/extensions/develop/migrate/mv2-deprecation-timeline).

This project aims to explore future proof solutions to circumvent MV2 deprecation without relying on Google provided temporary solutions. Still, if Chrome/Chromium fully remove support for MV2, this project cannot help to restore functionality that does not exist.

Updates: Recently there are rumors that Chrome 150 and 151 will be blocking some of the MV2 flags. While our solution doesn't rely on flags or any switch, its clear that the MV2 ban is still going on despite the lack of updates for ~10 major versions.

## Usage

```
$ ChromeMV2Launcher [--dryrun | --headed] <path to chrome.dll> <path to chrome.exe> [chrome.exe launch arguments]
```
- `--dryrun`: Scans chrome.dll to check if the launcher can find `OnExtensionSystemReady` and `experiment_stage_` without launching chrome.
- `--headed`: Start launcher in verbose mode with terminal visible.

## How it works

MV2 deprecation is handled by a `ManifestV2ExperimentManager`. `CalculateCurrentExperimentStage` reads feature flags and returns MV2 experiment stage to the manager constructor. 
When the extension system is ready, `CheckDisabledExtensions` and `DisableAffectedExtension` calls `ShouldDisableLegacyExtensions` to decide whether MV2 extensions need to be re-enable or disabled.

With proper compilation settings (i.e. similar to Google Chrome Build), `ShouldDisableLegacyExtensions` should be transformed into a inline access to the MV2 experiment stage variable.

ChromeMV2Launcher scans for the `OnExtensionSystemReady` callback in chrome.dll, then attach a breakpoint there on launch to overwrite the MV2 deprecation stage to `kWarning` in memory. Because the launcher does no persistent modification, to revert the change, simply restart Chrome/Chromium without the launcher.

## Limitations

The project targets 64-bit Windows on x86 platform only. 

The breakpoint is only calculated with a static binary pattern, so code changes or simply deviations in compilation options can cause the parser to miss the injection point. We may occasionally update the search pattern or maybe rework the search logics someday, but do not rely on the effectiveness of this program.

## Build

WIP

_Should be trival with Visual Studio_

## Dependencies

The project uses debugger library to attach breakpoints and perform memory manipulations.
- dbgeng.lib

## Alternatives
During the development of this project, we stumble across other efforts to reinstate MV2 functionalities. For example:

- [patch-chrome-mv2](https://gist.github.com/tophf/3fac58988fb94bd83aef69c9f18d1948/) by [@tophf](https://github.com/tophf), which overwrites MV2 feature flags in `chrome.dll` to start Chrome with MV2 enabled by default

These solutions chose different approach from our project, so checkout and see which suits better for you \:\)
