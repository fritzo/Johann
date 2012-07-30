

def TODO(message=''):
    raise NotImplementedError('TODO {}'.format(message))


def union(sets):
    if sets:
        return set.union(*sets)
    else:
        return set()


def set_with(set_, element):
    result = set_.copy()
    result.add(element)
    return result


def set_without(set_, element):
    result = set_.copy()
    result.remove(element)
    return result
