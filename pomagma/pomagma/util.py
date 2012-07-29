

def TODO(message=''):
    raise NotImplementedError('TODO {}'.format(message))


def union(sets):
    if sets:
        return set.union(*sets)
    else:
        return set()
