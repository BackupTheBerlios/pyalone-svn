================
pyalone README
================
:Author: Miki Tebeka <miki.tebeka@gmail.com>


What?
=====
pyalone allows you to distribute your python programs to computers that don't
have Python installed.

Unlike py2exe, cx_Freeze and otheres it does not create a frozen image of your
script but bundles everything needed to run the script in a directory.

This allows for easy update of files and debugging in client machine.

How?
====
pyalone packs your Python script and all other modules it depends on in a
directory. It also packs the python executable and shared library.

pyalone creates a small executable in the name of the script and then runs the
local python on this script.
