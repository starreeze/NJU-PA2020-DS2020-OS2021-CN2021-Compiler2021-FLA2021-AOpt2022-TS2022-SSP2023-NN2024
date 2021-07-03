@staticmethod
def match(dst, src) -> bool:
    i, j = len(dst) - 1, len(src) - 1
    while i >= 0 and j >= 0:
        if dst[i] == '*':
            return True
        if dst[i] != src[j]:
            return False
        i -= 1
        j -= 1
    return i == j