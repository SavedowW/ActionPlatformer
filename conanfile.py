from conan import ConanFile

class SomeSmallFG(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    def build_requirements(self):
        self.tool_requires("cmake/3.27.0")

    def requirements(self):
        self.requires("sdl_image/2.0.5")
        self.requires("sdl_ttf/2.20.1")
        self.requires("sdl_mixer/2.8.0")
        self.requires("libpng/1.6.40", override=True)
        self.requires("sdl/2.26.5", override=True)
        self.requires("lz4/1.9.4")