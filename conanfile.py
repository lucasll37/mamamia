from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMakeDeps, cmake_layout

class MLInferenceConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    
    def requirements(self):
        self.requires("grpc/1.54.3")
        self.requires("protobuf/3.21.12")
    
    def generate(self):
        # CRÍTICO: Este método GERA o conan_toolchain.cmake
        tc = CMakeToolchain(self)
        tc.generate()
        
        deps = CMakeDeps(self)
        deps.generate()
    
    def configure(self):
        self.options["grpc"].shared = False
        self.options["protobuf"].shared = False