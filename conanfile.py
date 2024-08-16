from conan import ConanFile

class ConanPackage(ConanFile):
    name = 'libws'
    version = "0.1.0"

    settings = "build_type"

    generators = 'CMakeDeps'

    requires = [
        'boost/1.74.0',
    ]

    default_options = {
        'boost/*:shared': False
    }
    