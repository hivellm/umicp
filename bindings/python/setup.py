#!/usr/bin/env python3
"""Setup script for UMICP Python bindings."""

from setuptools import setup, find_packages

setup(
    packages=find_packages(include=["umicp", "umicp.*"]),
    package_data={
        "umicp": ["py.typed"],
    },
)

