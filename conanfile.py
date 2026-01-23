from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMakeDeps, cmake_layout

class MLInferenceConan(ConanFile):
    name = "mlinference"
    version = "1.0.0"

    # Define que o binário depende do ambiente
    settings = "os", "compiler", "build_type", "arch"
    
    # Opções customizáveis
    options = {
        "enable_gpu": [True, False],
    }
    
    default_options = {
        "enable_gpu": False,
    }

    def requirements(self):
        # gRPC depende internamente de protobuf
        # As versões escolhidas aqui são compatíveis entre si
        self.requires("grpc/1.54.3")
        self.requires("protobuf/3.21.12")
        
        # Note: ONNX Runtime não está disponível no ConanCenter
        # Será instalado manualmente ou via script
        # self.requires("onnxruntime/1.16.3")  # Não disponível no Conan

    def generate(self):
        # Gera o toolchain CMake baseado no perfil do Conan
        tc = CMakeToolchain(self)

        # Força o CMake a respeitar flags por build_type
        # (Release, Debug etc.)
        tc.cache_variables["CMAKE_POLICY_DEFAULT_CMP0091"] = "NEW"
        
        # Passa opção de GPU para o CMake
        tc.cache_variables["ENABLE_GPU"] = self.options.enable_gpu

        tc.generate()

        # Gera arquivos grpc-config.cmake, protobuf-config.cmake etc.
        deps = CMakeDeps(self)
        deps.generate()

    def configure(self):
        # Garante linkage estático (coerente com gRPC)
        self.options["grpc"].shared = False
        self.options["protobuf"].shared = False

# from conan import ConanFile
# from conan.tools.cmake import CMakeToolchain, CMakeDeps, cmake_layout

# class MLInferenceConan(ConanFile):
#     name = "mlinference"
#     version = "1.0.0"

#     # Define que o binário depende do ambiente
#     settings = "os", "compiler", "build_type", "arch"

#     def requirements(self):
#         # gRPC depende internamente de protobuf
#         # As versões escolhidas aqui são compatíveis entre si
#         self.requires("grpc/1.54.3")
#         self.requires("protobuf/3.21.12")

#     def generate(self):
#         # Gera o toolchain CMake baseado no perfil do Conan
#         tc = CMakeToolchain(self)

#         # Força o CMake a respeitar flags por build_type
#         # (Release, Debug etc.)
#         tc.cache_variables["CMAKE_POLICY_DEFAULT_CMP0091"] = "NEW"

#         tc.generate()

#         # Gera arquivos grpc-config.cmake, protobuf-config.cmake etc.
#         deps = CMakeDeps(self)
#         deps.generate()

#     def configure(self):
#         # Garante linkage estático (coerente com gRPC)
#         self.options["grpc"].shared = False
#         self.options["protobuf"].shared = False
