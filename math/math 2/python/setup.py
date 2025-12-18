#!/usr/bin/env python3
"""
Universal Recovery System - Python Package

Setup script for installing Python bindings.
"""

from setuptools import setup, find_packages
import os

# Read README
with open('README.md', 'r', encoding='utf-8') as f:
    long_description = f.read()

# Read version
version = '1.0.0'

setup(
    name='recovery',
    version=version,
    author='NinjaTech AI',
    author_email='contact@ninjatech.ai',
    description='Universal Recovery System - Python Bindings',
    long_description=long_description,
    long_description_content_type='text/markdown',
    url='https://github.com/justmebob123/crystalline',
    packages=find_packages(),
    classifiers=[
        'Development Status :: 4 - Beta',
        'Intended Audience :: Developers',
        'Intended Audience :: Science/Research',
        'Topic :: Scientific/Engineering',
        'Topic :: Security :: Cryptography',
        'Topic :: Software Development :: Libraries :: Python Modules',
        'License :: OSI Approved :: MIT License',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.7',
        'Programming Language :: Python :: 3.8',
        'Programming Language :: Python :: 3.9',
        'Programming Language :: Python :: 3.10',
        'Programming Language :: Python :: 3.11',
        'Operating System :: POSIX :: Linux',
    ],
    python_requires='>=3.7',
    install_requires=[
        'numpy>=1.19.0',
        'scipy>=1.5.0',
    ],
    extras_require={
        'dev': [
            'pytest>=6.0',
            'pytest-cov>=2.10',
            'black>=20.8b1',
            'flake8>=3.8',
            'mypy>=0.800',
        ],
        'audio': [
            'soundfile>=0.10.0',
        ],
        'visualization': [
            'matplotlib>=3.3.0',
        ],
    },
    entry_points={
        'console_scripts': [
            'recovery-cli=recovery.cli:main',
        ],
    },
    include_package_data=True,
    zip_safe=False,
    keywords='recovery signal-processing cryptography bitcoin ssh ransomware',
    project_urls={
        'Bug Reports': 'https://github.com/justmebob123/crystalline/issues',
        'Source': 'https://github.com/justmebob123/crystalline',
        'Documentation': 'https://github.com/justmebob123/crystalline/tree/main/docs',
    },
)