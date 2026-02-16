from setuptools import Extension, setup


setup(
    packages=["predicates"],
    package_dir={"": "python"},
    ext_modules=[
        Extension(
            "predicates._predicates",
            sources=[
                "python/predicates/_predicatesmodule.c",
                "src/constants.c",
                "src/predicates.c",
                "src/random.c",
                "src/printing.c",
            ],
            include_dirs=["include", "src"],
        )
    ],
)
