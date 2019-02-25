=======================
LUA-XRDB bridge library
=======================

A little (and mostly incomplete) library for reading and writing entries in the X11 Resource Database

Installation and usage
======================

Compile the x11 bridge library
------------------------------

The first step is to compile the X11 bridge library, which is a little C glue program.
The provided makefile comes with a couple targets (for different versions of the lua interpreter).

Compile the x11 bridge module and install that in the Lua cpath:

.. code:: bash
    # make

    # cp xresources53.so $HOME/.config/lualibs/xresources.so

The default target is Lua5.3, but additional targets are available for lua5.2 and lua5.1

The lua library expects to find a file named 'xresources.so' in the CPATH.
This file can be installed system-wide or in the user home under ~/.config/lualibs.

How to use the library
----------------------

Once the X11 glue bits are in place, just import the lua library in your script:

.. code:: lua

    require 'xrdb'

The module has functions to read and write entries in the Default Resource Database.

TODO
====

Many things, but just to name a few:

- Better error handling
- Better support for x11 resource bits
- Support for import/export of resource databases
