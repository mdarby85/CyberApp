from abc import ABC, abstractmethod


class Middleware (ABC):
    """
    An abstract class to define the interface for Middleware
    """

    @staticmethod
    @abstractmethod
    def run (environ, cookies):
        """
        Runs the middlware and either returns an HTTP Response or None if the
        middleware has no reason to stop execution of main route.

        @return Response of None
        """
        pass
