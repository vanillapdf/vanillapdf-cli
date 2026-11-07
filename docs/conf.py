"""Sphinx configuration for the vanillapdf-cli documentation.

Mirrors doc/sphinx/conf.py of the vanillapdf library, so both projects share
the same theme and look. The pages are Markdown, read by MyST.
"""

import os
import re

# -- Read version from CMakeLists.txt ---------------------------------------

repo_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
cmake_lists = os.path.join(repo_root, "CMakeLists.txt")

with open(cmake_lists, encoding="utf-8") as f:
    _version_match = re.search(
        r"project\(vanillapdf-cli\s+VERSION\s+(\d+)\.(\d+)\.(\d+)", f.read()
    )

_major, _minor, _patch = _version_match.groups()

# -- Project information ---------------------------------------------------

project = "vanillapdf-cli"
copyright = "2026, Vanilla.PDF Labs s.r.o."
author = "Vanilla.PDF Labs s.r.o."
version = "{}.{}".format(_major, _minor)
release = "{}.{}.{}".format(_major, _minor, _patch)

# -- General configuration -------------------------------------------------

extensions = [
    "myst_parser",
    "sphinx_copybutton",
    "sphinxext.opengraph",
]

source_suffix = {
    ".md": "markdown",
}

exclude_patterns = ["_build"]

# Generate anchors for headings down to ### so links such as
# commands/encrypt.md#add and commands/index.md#page-ranges resolve
myst_heading_anchors = 3

# -- Options for HTML output -----------------------------------------------

html_theme = "sphinx_rtd_theme"

html_static_path = ["_static"]
html_logo = "_static/logo.png"
html_favicon = "_static/favicon.ico"

html_theme_options = {
    "logo_only": False,
    "navigation_depth": 3,
}

html_context = {
    "display_github": True,
    "github_user": "vanillapdf",
    "github_repo": "vanillapdf-cli",
    "github_version": "main",
    "conf_py_path": "/docs/",
}

# -- OpenGraph metadata (for link previews) --------------------------------

ogp_site_url = "https://vanillapdf.readthedocs.io/projects/cli/"
ogp_site_name = "vanillapdf-cli Documentation"
ogp_description_length = 200
