"""
Setup configuration for ML Inference System Python components
"""

from setuptools import setup, find_packages
from pathlib import Path

# Read README
this_directory = Path(__file__).parent
long_description = (this_directory / "README.md").read_text() if (this_directory / "README.md").exists() else ""

# Read requirements
requirements = []
requirements_file = this_directory / "requirements.txt"
if requirements_file.exists():
    with open(requirements_file) as f:
        requirements = [line.strip() for line in f if line.strip() and not line.startswith('#')]

setup(
    name="ml-inference-system",
    version="1.0.0",
    author="Your Name",
    author_email="your.email@example.com",
    description="Distributed ML Model Inference System",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/yourusername/ml-inference-system",
    packages=find_packages(exclude=["tests", "tests.*"]),
    classifiers=[
        "Development Status :: 4 - Beta",
        "Intended Audience :: Developers",
        "Intended Audience :: Science/Research",
        "License :: OSI Approved :: MIT License",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
        "Programming Language :: Python :: 3.12",
        "Topic :: Scientific/Engineering :: Artificial Intelligence",
        "Topic :: Software Development :: Libraries :: Python Modules",
    ],
    python_requires=">=3.9",
    install_requires=requirements,
    extras_require={
        "dev": [
            "pytest>=7.4.3",
            "pytest-asyncio>=0.21.1",
            "pytest-cov>=4.1.0",
            "black>=23.12.0",
            "isort>=5.13.2",
            "flake8>=6.1.0",
            "mypy>=1.7.1",
        ],
        "docs": [
            "sphinx>=7.2.6",
            "sphinx-rtd-theme>=2.0.0",
        ],
        "gpu": [
            "onnxruntime-gpu>=1.16.3",
        ],
    },
    entry_points={
        "console_scripts": [
            "ml-inference=cli.main:main",
            "ml-inference-api=api_gateway.main:main",
        ],
    },
    package_data={
        "": ["*.yaml", "*.json", "*.proto"],
    },
    include_package_data=True,
    zip_safe=False,
    keywords=[
        "machine-learning",
        "deep-learning",
        "inference",
        "model-serving",
        "distributed-systems",
        "grpc",
        "onnx",
        "gpu",
    ],
    project_urls={
        "Bug Reports": "https://github.com/yourusername/ml-inference-system/issues",
        "Source": "https://github.com/yourusername/ml-inference-system",
        "Documentation": "https://ml-inference-system.readthedocs.io/",
    },
)
