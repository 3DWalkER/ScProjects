# Getting Started

## Building the projects:

### Minimum Requirements:
* Visual Studio 2017 or later
* OR GNU Compiler Collection (GCC) 4.9 or later
* OR Clang 3.3 or later
* 4GB of RAM (required for building some of the larger clients; SDK build may fail on EC2 instance types t2.micro, t2.small, and other small instance types due to insufficient memory)
* Supported platforms
  * Linux
  * Windows
  * Mac
  
### Building From Source:

#### To create an **out-of-source build**:
1. Install CMake and the relevant build tools for your platform. Ensure these are available in your executable path.
2. Clone this repository with submodules

    ```sh
    git clone --recurse-submodules https://github.com/3DWalkER/ScProjects
    ```
	
3. Create your build directory. Replace `<BUILD_DIR>` with your build directory name:
4. Build the project:

	```sh
	cd <BUILD_DIR>
	cmake <path-to-root-of-this-source-code> \
		-DSC_PROJECT_BUILD_ONLY="ScServer"
	cmake --build . --config=Debug
   ```