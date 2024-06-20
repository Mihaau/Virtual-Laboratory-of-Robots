# Virtual Laboratory of Robots

Virtual laboratory for industrial robots, featuring 3D models of robots available in an industrial robotics workshop. The virtual robot lab allows users to control the movement of simulated robots, manipulate objects in the virtual environment (such as picking up and releasing blocks), and includes a robot program editor for defining basic robot operations

## Features

- Real-Time Feedback: Visual and numerical feedback is provided in real-time to help users understand the robot's movements and positions.

- User-Friendly Interface: Intuitive controls and a user-friendly interface make it easy for users to interact with the robots.

- Robot Program Editor: Users can create and edit robot programs using a simple programming language, allowing them
to define basic robot operations.

- Pick and Place Operations: Users can simulate picking up, moving, and releasing objects within the virtual environment.

## Table of Contents

- [Installation](#installation)
  - [Pre-requisites](#pre-requisites)
  - [Compilation](#compilation)
  - [Debugging](#debugging)
- [Acknowledgements](#acknowledgements)
- [License](#license)

## Installation

### Pre-requisites

To install Virtual Laboratory of Robots from source, users must have xmake.

On Debian or Ubuntu Linux, it can be installed with:

```sh
apt-get install xmake
```

On Mac, it can be installed with:

```sh
brew install xmake
```

### Compilation

Once xmake is installed, the simplest way to install Virtual Laboratory of Robots is with:

```sh
xmake run robolab
```

After that, the `robolab` executable will be placed in the directory
`./build/$PLATFORM/`, where `.` represents your project directory

### Debugging

## Acknowledgements

The project has been created by Anna Ivanytska and Michał Pryba.

## License
