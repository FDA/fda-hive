=======================================================
pyhive.QPSvc – interface for launching new computations
=======================================================
.. autoclass:: pyhive.QPSvc
    :members:

    Basic usage example to insert a new computational request into the current request group::

        >>> qpsvc = pyhive.QPSvc(pyhive.proc.svc.name) # inherit the current request's service
        >>> qpsvc.set_form(pyhive.proc.form) # inherit the current request's form
        >>> qpsvc.set_var("foobar", "wombat") # ... but override and set "foobar" : "wombat" in the new request's form
        >>> req_id = qpsvc.launch()
        >>> print("Launched new request %s and inserted it into current request group" % req_id)
