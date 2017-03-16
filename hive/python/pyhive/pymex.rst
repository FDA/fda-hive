================================================
pyhive.Mex – memory buffer or memory-mapped file
================================================

Convenient for moving data between Python and the C++ HIVE API.

.. autoclass:: pyhive.Mex([data[, filename[, flags = pyhive.mex_flag.DEFAULT]]])
    :members:

    Can be initialized by mapping a file into memory::

        >>> x = pyhive.Mex(filename="x.txt") # read-write mode
        >>> y = pyhive.Mex(filename="/tmp/dir/y.csv", flags=pyhive.mex_flag.READONLY) # read-only mode

    Or by taking a copy of a string or buffer::

        >>> z = pyhive.Mex(data="Hello world")

-------------------------------------------
pyhive.Mex flags, combined using bitwise-or
-------------------------------------------

.. automodule:: pyhive.mex_flag
    :members:
