# Agent prompt — Verify DisplayXR Unreal install from a fresh clone

This file is an **agent prompt**, not a skill. Paste the body below (everything under `## Prompt`) into an agent invocation — general-purpose subagent, Claude Code task, or a future `/verify-install` skill — whenever you want to verify that the DisplayXR Unreal plugin's current release works end-to-end for a new developer.

The prompt is self-contained: it carries enough context that the executing agent does not need to read this repo's README. It encodes exactly the steps a human first-time user would take, and the log lines that must appear for the run to count as a pass.

**When to run it:**
- After cutting a new plugin release in `DisplayXR/displayxr-unreal` (pair with the `/release` skill — bump `.displayxr-version`, then run this).
- Before any externally-visible demo to sanity-check the whole pipeline.
- After any change to `displayxr-unreal-test` itself (config, fetch script, uproject, README).

**Prerequisites on the executing machine** (the prompt re-lists these but they must actually be satisfied):
- Windows 10/11
- Unreal Engine 5.7 installed (default path `C:\Program Files\Epic Games\UE_5.7\`)
- Visual Studio 2022 with "Game development with C++" workload
- DisplayXR runtime installed (JSON discoverable at `C:\Program Files\DisplayXR\Runtime\DisplayXR_win64.json`)
- GitHub CLI (`gh`) authenticated (`gh auth status` exits 0)
- ~10 GB free disk in `%TEMP%`

First-run wall time is 15–25 minutes (shader compile dominates). Subsequent runs with a warm DDC finish in 2–3 minutes.

---

## Prompt

> Verify the current pinned DisplayXR Unreal plugin release end-to-end by walking the `displayxr-unreal-test` README as if you were a new developer. Use exact commands, grep the resulting UE log for the success lines below, and return a PASS/FAIL report.
>
> ### Scratch dir
> Work inside `%TEMP%\displayxr-install-verify`. If the directory exists from a previous run, delete it first (`Remove-Item -Recurse -Force`). Create it fresh.
>
> ### Steps (run in order, stop on first failure)
>
> **1. Clone the test repo.**
> ```
> git clone https://github.com/DisplayXR/displayxr-unreal-test.git "$env:TEMP\displayxr-install-verify\displayxr-unreal-test"
> ```
> Confirm the clone succeeded and `.displayxr-version` exists at the root. Record the tag it contains (e.g. `v0.1.1`) — you will reference it in the report.
>
> **2. Fetch the pinned plugin build.**
> ```
> powershell.exe -NoProfile -ExecutionPolicy Bypass -File Scripts\fetch-plugin.ps1
> ```
> Run this from the cloned repo root. On success it prints `DisplayXR <tag> installed at <path>`. Verify both files exist:
> - `Plugins\DisplayXR\DisplayXR.uplugin`
> - `Plugins\DisplayXR\Binaries\Win64\UnrealEditor-DisplayXRCore.dll`
>
> If either is missing, FAIL with the fetch script's output.
>
> **3. Generate Visual Studio project files.**
> ```
> "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" -projectfiles -project="<absolute path to DisplayXRTest.uproject>" -game -rocket -progress
> ```
> Verify `DisplayXRTest.sln` appears at the repo root and the command ended with `Result: Succeeded`.
>
> **4. Build the Development Editor target.** Use PowerShell — `Build.bat` can't be invoked through the Bash tool because of the space in the UE install path.
> ```
> & "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" DisplayXRTestEditor Win64 Development -project="<absolute path to DisplayXRTest.uproject>" -WaitMutex
> ```
> The last line before `Total execution time` must be `Result: Succeeded`. A single `warning C4701` in `DisplayXRPreviewSession.cpp` was fixed in v0.1.1 — any remaining C4701 in plugin sources is a regression worth flagging (but not a hard failure unless the overall build fails).
>
> **5. Launch the editor, let it load `SimpleCube`, then quit.** Set a **30-minute timeout** on this step (first-run shader compile).
> ```
> & "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "<absolute path to DisplayXRTest.uproject>" -unattended -nosound -nosplash -ExecCmds="quit"
> ```
> **Do NOT pass `-nullrhi`** — it triggers a false-positive `Null assigned to TNotNull` fatal during teardown that is not related to the plugin.
>
> With `EditorStartupMap=/Game/SimpleCube.SimpleCube` set in `Config/DefaultEngine.ini`, the editor loads the sample scene before the queued `quit` fires. Process exit code must be `0`.
>
> **6. Inspect the fresh UE log.**
> Path: `<repo root>\Saved\Logs\DisplayXRTest.log`
>
> ### Success criteria (log assertions)
>
> The log **must contain all** of the following (order doesn't matter, but the first few will appear near the start):
>
> - `LogPluginManager: Mounting Project plugin DisplayXR`
> - `LogDisplayXRSession: DisplayXR Session: Loaded runtime DLL`
> - `LogDisplayXRSession: DisplayXR Session: Instance created, SystemId=`
> - `LogDisplayXRSession: DisplayXR Session: Display` ... `m,` ... `px`
> - `LogDisplayXRSession: DisplayXR Session: ViewConfig initialized`
> - `LogDisplayXRCore: DisplayXR: Session initialized`
> - `LogDisplayXRCore: DisplayXR: Core module started`
> - `LogDisplayXRDevice: DisplayXR Device: Created`
> - `LogDisplayXRCore: DisplayXR: Tracking system created`
> - `LogDisplayXREditor: DisplayXR: Editor module started`
> - `LogInit: Display: Engine is initialized. Leaving FEngineLoop::Init()`
> - `SimpleCube` in a `LogWorld:` or `LogLoad:` line (proves the correct level loaded — do not accept a log that only mentions `OpenWorldDefault` or `Minimal_Default`)
> - At least one `LogDisplayXRDevice:` line containing `SetupViewFamily #` (stereo render path fired against the loaded level)
> - At least one `LogDisplayXRDevice:` line containing `ComputeViews #` and `cameraCentric=` (Kooima view computation ran)
>
> The log **must NOT contain**:
>
> - `Fatal error:`
> - `[Callstack]` dumps
> - `Null assigned to TNotNull`
> - Any `LogWorld:` line about `OpenWorldDefault` or `Minimal_Default` as the loaded level — if that appears, `Config/DefaultEngine.ini` is broken or missing.
>
> ### Expected benign warnings (do NOT fail on these)
>
> - `LogDisplayXRSession: Warning: DisplayXR Session: xrCreateSession failed (-38)` followed by `Session creation failed, will retry` — the runtime retries successfully. Counts as passing.
> - Early `ERROR: The system was unable to find the specified registry key or value.` / `Install path for Unreal Engine 5.7 not found!` in `PackagePlugin.bat` output — benign fallback probe (irrelevant here since we don't run PackagePlugin in this prompt, but mirrors the release skill).
>
> ### Failure modes and how to classify them
>
> - **Fetch script fails with `gh release download` non-zero** → plugin release asset missing or renamed. FAIL, report the asset name the script tried.
> - **Build fails with missing-include errors** → regression in `displayxr-unreal`'s BuildPlugin isolation. FAIL, report the exact missing symbol(s) and file.
> - **Runtime JSON not found** (`LogDisplayXRSession` never logs `Loaded runtime DLL`) → DisplayXR runtime not installed on this machine. Report as `PREREQ_MISSING`, not FAIL.
> - **Editor exits non-zero with a `Fatal error:`** → real crash. FAIL, include the 20 lines of log immediately before the `Fatal error:` line plus the callstack (even if symbols are stripped).
> - **Wrong map loaded** (`OpenWorldDefault` appears) → `DefaultEngine.ini` fix regressed. FAIL.
> - **Timeout at step 5** → shader compile genuinely exceeded 30 min, or the editor hung. Report with the last 50 lines of the UE log.
>
> ### Reporting
>
> Return a single report under 200 words with:
>
> - `PASS` or `FAIL` or `PREREQ_MISSING`
> - Plugin version pulled (from `.displayxr-version`)
> - UE version detected (parse from log: `LogEngineVersion` or `LogInit: Version`)
> - Time elapsed per step (build + editor launch are the two that matter)
> - If FAIL: one-line cause + path to `Saved\Logs\DisplayXRTest.log` so a human can dig in
> - If PASS: confirm `SimpleCube` loaded and `SetupViewFamily` fired at least once
>
> Do not clean up the scratch dir on FAIL — leave the log in place for inspection. On PASS, cleanup is optional.
