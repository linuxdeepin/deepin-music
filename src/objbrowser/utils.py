
    
# Needed because boolean QSettings in Pyside are converted incorrect the second
# time in Windows (and Linux?) because of a bug in Qt. See:
# https://www.mail-archive.com/pyside@lists.pyside.org/msg00230.html
def setting_str_to_bool(s):
    """ Converts 'true' to True and 'false' to False if s is a string
    """
    if isinstance(s, basestring):
        s = s.lower()
        if s == 'true':
            return True
        elif s == 'false':
            return False
        else:
            return ValueError('Invalid boolean representation: {!r}'.format(s))
    else:
        return s

    