
### Overview of the backend OCCA architecture

Currently OCCA supports the following backends: Serial, OpenMP, CUDA, HIP, OpenCL, metal.

Each backend is referred to in the OCCA code as a "mode". Each mode provides implementation for: generating backend speific kernel code generation, interacting with the backend API, managing kernel execution, managing data migration.

#### Header files

All modes are structured with API specification via a common set of header files:

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

