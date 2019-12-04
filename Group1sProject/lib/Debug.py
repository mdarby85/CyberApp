
class Debug:
    """
    Singleton class to be used for adding debug print statements.
    """
    debug_string = ""

    @classmethod
    def reset(cls):
        cls.debug_string = ""

    @classmethod
    def print (cls, string_to_print):
        cls.debug_string = cls.debug_string + string_to_print + '\n'
