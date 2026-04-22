# DisplayXR Unreal Test Project

A minimal Unreal test project for the [DisplayXR Unreal plugin](https://github.com/DisplayXR/displayxr-unreal). Use this project to validate the plugin against new releases, test rig setups, and try out stereo rendering on a tracked 3D display.

## Requirements

- **Unreal Engine 5.7**
- **Visual Studio 2022** with the "Game development with C++" workload
  - Must include the **.NET Framework 4.6.2+ SDK** and matching targeting pack (installable from the VS Installer under *Individual components*). Without it, Unreal Build Tool fails with `Could not find NetFxSDK install dir` when instantiating its `SwarmInterface` module and no editor target will compile.
- A 3D display supported by the [DisplayXR OpenXR runtime](https://github.com/DisplayXR/displayxr-shell-releases/releases), or use the built-in `sim_display` driver for development without hardware
- **GitHub CLI** (`gh`) authenticated via `gh auth login` — the bootstrap script uses it to download the plugin

## Quickstart

1. Clone this repo:
   ```bash
   git clone https://github.com/DisplayXR/displayxr-unreal-test.git
   cd displayxr-unreal-test
   ```
2. Fetch the pinned plugin build:
   ```powershell
   pwsh Scripts/fetch-plugin.ps1
   ```
   This downloads the plugin ZIP from the [displayxr-unreal release](https://github.com/DisplayXR/displayxr-unreal/releases) matching the tag in `.displayxr-version` and unpacks it into `Plugins/DisplayXR/`.
3. Right-click `DisplayXRTest.uproject` → **Generate Visual Studio project files**.
4. Open `DisplayXRTest.sln`, build the `Development Editor` configuration.
5. Launch the editor and open `Content/SimpleCube.umap`.

## Plugin Reference

The plugin version consumed by this project is declared in `.displayxr-version` at the repo root:

```
v0.1.0
```

Unreal has no native "git URL + tag" package manager (the way Unity's UPM does), so this repo uses a small fetch script against [GitHub Releases](https://github.com/DisplayXR/displayxr-unreal/releases) instead.

### Testing a different plugin version

1. Edit `.displayxr-version` to the desired tag (e.g. `v0.2.0`).
2. Re-run `pwsh Scripts/fetch-plugin.ps1`.
3. Reopen the editor.

### Testing a local development build of the plugin

Clone `displayxr-unreal` directly into `Plugins/DisplayXR/` (the fetch script always wipes that path, so back up first if needed):

```bash
git clone https://github.com/DisplayXR/displayxr-unreal.git Plugins/DisplayXR
```

The `Plugins/DisplayXR/` path is gitignored either way.

## Test Scenes

- `Content/SimpleCube.umap` — baseline scene with a crate, camera rig, and display rig blueprints. Good for verifying stereo output and eye tracking.

## Related Repositories

- [displayxr-unreal](https://github.com/DisplayXR/displayxr-unreal) — the plugin itself
- [displayxr-runtime](https://github.com/DisplayXR/displayxr-shell-releases) — OpenXR runtime + installer
- [displayxr-unity-test](https://github.com/DisplayXR/displayxr-unity-test) — Unity equivalent of this project
