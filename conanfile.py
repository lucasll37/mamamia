from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMakeDeps, cmake_layout

class MLInferenceConan(ConanFile):
    name = "mlinference"
    version = "1.0.0"

    # Define que o binário depende do ambiente
    settings = "os", "compiler", "build_type", "arch"

    def requirements(self):
        # gRPC depende internamente de protobuf
        # As versões escolhidas aqui são compatíveis entre si
        self.requires("grpc/1.54.3")
        self.requires("protobuf/3.21.12")

    def generate(self):
        # Gera o toolchain CMake baseado no perfil do Conan
        tc = CMakeToolchain(self)

        # Força o CMake a respeitar flags por build_type
        # (Release, Debug etc.)
        tc.cache_variables["CMAKE_POLICY_DEFAULT_CMP0091"] = "NEW"

        tc.generate()

        # Gera arquivos grpc-config.cmake, protobuf-config.cmake etc.
        deps = CMakeDeps(self)
        deps.generate()

    def configure(self):
        # Garante linkage estático (coerente com gRPC)
        self.options["grpc"].shared = False
        self.options["protobuf"].shared = False
