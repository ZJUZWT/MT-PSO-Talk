# PSO Compile Tester

A cross-platform tool for testing PSO (Pipeline State Object) compilation performance.

## Supported Platforms
- OpenGL
- Vulkan  
- Metal

## Dependencies
- Mesa (located in libs/mesa)

## Build
`
mkdir build && cd build
cmake ..
make
`

## Project Structure
- src/ - Main source code
- tests/ - Test cases
- shaders/ - Shader files for testing
- libs/ - External libraries (mesa)
