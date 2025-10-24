#!/usr/bin/env python3
"""Setup script for UMICP Python SDK."""

from setuptools import setup, find_packages

setup(
    packages=find_packages(include=["umicp_sdk", "umicp_sdk.*"]),
    package_data={
        "umicp_sdk": ["py.typed"],
    },
)

