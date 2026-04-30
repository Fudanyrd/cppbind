from skbuild import setup  # This line replaces 'from setuptools import setup'

setup(
    name="demo",
    version="0.0.1",
    description="a minimal example package (cpp version)",
    author='Fudanyrd',
    license="MIT",
    packages=['demo'],
    python_requires=">=3.9",
)

