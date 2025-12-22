# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Augmented Reality Sandbox (SARndbox) v2.8 - An AR application that scans a sand surface using a Kinect 3D camera and projects real-time topography maps with contour lines, hillshading, and water flow simulation back onto the sand surface using a calibrated projector.

## Build Commands

```bash
make              # Build all targets (SARndbox, CalibrateProjector, SARndboxClient)
make config       # Display build configuration
sudo make install # Install to INSTALLDIR (default: current directory)
```

**Build System:** Makefile-based, requires Vrui-8.0 build environment. Key makefile variables:
- `VRUI_MAKEDIR`: Points to `/usr/local/share/Vrui-8.0/make` (configurable)
- `INSTALLDIR`: Installation directory (default: `$(PWD)`)

## Dependencies

**Required:**
- Vrui version 8.0 build 001+ (VR toolkit framework)
- Kinect 3D Video Capture Project version 3.10+
- libfreenect2 (for Kinect v2/Xbox One Kinect)

**Hardware:**
- Kinect camera (v1, v2, or Intel RealSense)
- Calibrated projector
- GPU with OpenGL support (Nvidia GeForce 970+ recommended for water simulation)

## Development Environment

A devcontainer is provided for Ubuntu 24.04 with all build dependencies pre-installed, including:
- CUDA toolkit with GCC 12 (CUDA requires GCC ≤12)
- libfreenect2 built with CUDA support (target: GTX 1060, Pascal SM 6.1)
- Vrui 8.0-002 and Kinect 3.10 from UC Davis distribution

## Architecture

### Core Components

**Main Application (Sandbox.cpp/.h):**
- Vrui application class managing Kinect input, rendering, tool system, and water simulation

**Rendering Pipeline:**
1. `FrameFilter` - Depth frame processing (filters raw Kinect frames, fills holes)
2. `DepthImageRenderer` - GPU-based depth image management
3. `SurfaceRenderer` - Elevation color mapping and contour lines
4. `WaterRenderer` - Water surface visualization

**Water Simulation (WaterTable2.cpp/.h):**
- GPU-based Saint-Venant equation solver
- Adaptive time stepping with Runge-Kutta integration

**Tools (Vrui Tool System):**
- `GlobalWaterTool` / `LocalWaterTool` - Water manipulation
- `DEMTool` - Load digital elevation models
- `BathymetrySaverTool` - Export bathymetry data
- `CalibrateProjector` - Projector-camera calibration utility

**Remote/Networking:**
- `RemoteServer` - Stream bathymetry/water grids
- `SandboxClient` - Remote visualization client

### Key Types (Types.h)

```cpp
typedef double Scalar;
typedef Geometry::Point<Scalar,3> Point;
typedef Geometry::Vector<Scalar,3> Vector;
typedef Geometry::Plane<Scalar,3> Plane;
typedef Geometry::OrthogonalTransformation<Scalar,3> OGTransform;
typedef Geometry::ProjectiveTransformation<Scalar,3> PTransform;
```

### File Organization

- Source files: Root directory (`*.cpp`, `*.h`)
- Configuration: `etc/SARndbox-2.8/` (BoxLayout.txt, SARndbox.cfg, HeightColorMap.cpt)
- Shaders: `share/SARndbox-2.8/Shaders/` (GLSL programs for surface/water rendering)
- Build output: `bin/` and `lib/`

## Running the Application

```bash
./bin/SARndbox -uhm -fpv          # Main app with height map and fixed projector view
./bin/CalibrateProjector -s W H   # Calibration utility (-s specifies projector resolution)
```

**Runtime Control:**
- Command pipe interface (`-cp <pipe name>`) for waterSpeed, waterMaxSteps, waterAttenuation, colorMap
- Interactive water control dialog

## Calibration Workflow

1. Mount Kinect camera above sandbox (looking straight down)
2. Measure base plane equation using RawKinectViewer's "Extract Planes" tool
3. Measure surface extents using KinectViewer's 3D measurement tool
4. Mount projector perpendicular to flattened sand surface
5. Run CalibrateProjector with CD calibration target
6. Results stored in `etc/SARndbox-2.8/ProjectorMatrix.dat`

## Threading Model

- TripleBuffer for Kinect frame synchronization
- Separate thread for frame filtering
- Mutex protection for grid requests between simulation and rendering
