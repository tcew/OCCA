
### Overview of the backend OCCA architecture

Currently OCCA supports the following backends: Serial, OpenMP, CUDA, HIP, OpenCL, metal.

Each backend is referred to in the OCCA code as a "mode". Each mode provides implementation for: generating backend speific kernel code generation, interacting with the backend API, managing kernel execution, managing data migration.