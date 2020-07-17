
### Overview of the backend OCCA architecture

Currently OCCA supports the following backends: Serial, OpenMP, CUDA, HIP, OpenCL, metal.

Each backend is referred to in the OCCA code as a "mode". Each mode provides implementation for: generating backend speific kernel code generation, interacting with the backend API, managing kernel execution, managing data migration.

#### Header files

All modes are structured with API specification via a set of header files located in occa/include/occa/modes:

```
cd occa
tree include/occa/modes
include/occa/modes
├── cuda
│   ├── device.hpp
│   ├── kernel.hpp
│   ├── memory.hpp
│   ├── polyfill.hpp
│   ├── registration.hpp
│   ├── stream.hpp
│   ├── streamTag.hpp
│   └── utils.hpp
├── cuda.hpp
├── hip
│   ├── device.hpp
│   ├── kernel.hpp
│   ├── memory.hpp
│   ├── polyfill.hpp
│   ├── registration.hpp
│   ├── stream.hpp
│   ├── streamTag.hpp
│   └── utils.hpp
├── hip.hpp
├── metal
│   ├── device.hpp
│   ├── kernel.hpp
│   ├── memory.hpp
│   ├── registration.hpp
│   ├── stream.hpp
│   └── streamTag.hpp
├── metal.hpp
├── opencl
│   ├── device.hpp
│   ├── kernel.hpp
│   ├── memory.hpp
│   ├── polyfill.hpp
│   ├── registration.hpp
│   ├── stream.hpp
│   ├── streamTag.hpp
│   └── utils.hpp
├── opencl.hpp
├── openmp
│   ├── device.hpp
│   ├── registration.hpp
│   └── utils.hpp
└── serial
    ├── device.hpp
    ├── kernel.hpp
    ├── memory.hpp
    ├── registration.hpp
    ├── stream.hpp
    └── streamTag.hpp
```

Note that not all modes provide exactly the same set of headers.

#### Backend mode API implementation

Most of the API implementation for a backend is collected in one place (occa/src/modes/backendName). To keep things simple we will focus on CUDA specific mode API files:

```
tree src/modes/cuda/
src/modes/cuda/
├── device.cpp
├── kernel.cpp
├── memory.cpp
├── registration.cpp
├── stream.cpp
├── streamTag.cpp
└── utils.cpp
```

The purpose of each of these files should be evident from their names.

## Backend mode language translation 

The OCCA parser consumes kernels expressed in the OCCA Kernel Language (OKL). It is the responsibility of each mode to perform translation of the OKL keywords and attributes into the target backend native kernel language. In addition to the above backend mode API implementation it is necessary to provide a parser plugin. Examples are available here:

```
tree src/lang/modes/
src/lang/modes/
├── cuda.cpp
├── hip.cpp
├── metal.cpp
├── okl.cpp
├── oklForStatement.cpp
├── opencl.cpp
├── openmp.cpp
├── serial.cpp
└── withLauncher.cpp
```

The CUDA mode implements various required transformations in the cuda.cpp file, with some examples here:

```
 namespace lang {
    namespace okl {
      cudaParser::cudaParser(const occa::properties &settings_) :
        withLauncher(settings_),
        constant("__constant__", qualifierType::custom),
        global("__global__", qualifierType::custom),
        device("__device__", qualifierType::custom),
        shared("__shared__", qualifierType::custom) {

        okl::addAttributes(*this);
      }
      ...
      std::string cudaParser::getOuterIterator(const int loopIndex) {
        std::string name = "blockIdx.";
        name += 'x' + (char) loopIndex;
        return name;
      }

      std::string cudaParser::getInnerIterator(const int loopIndex) {
        std::string name = "threadIdx.";
        name += 'x' + (char) loopIndex;
        return name;
      }
      ...
   }   
}
```

The ```getOuterIterator``` function translates the loop index from the loop annotated with ```@outer(loopIndex)``` to the variable used within the kernel. For example this code:

```
for(int b=0;b<B;++b;@outer(0)){
   for (int t=0;t<T;++t;@inner(0)){
      int n = t + b*B;
      ...;
   }
}
```

would be parsed and the instance of ```t``` and ```b``` would be replaced wth ```threadIdx.x``` and ```blockIdx.x``` respectively.
