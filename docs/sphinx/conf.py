import os
import sys
import sphinx_rtd_theme

project = 'NuttX ESP32 Lab01'
copyright = '2025, AndoniXXR'
author = 'AndoniXXR'

extensions = [
    'breathe',
    'sphinx_rtd_theme',
]

templates_path = ['_templates']
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']

language = 'es'

html_theme = "sphinx_rtd_theme"
html_static_path = ['_static']

# Breathe configuration
breathe_projects = { "NuttX ESP32 Lab01": "../doxygen/xml" }
breathe_default_project = "NuttX ESP32 Lab01"
