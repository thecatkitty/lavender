class UserDirectory(object):
    def has_access(self, uid: int, cid: int) -> bool:
        raise NotImplementedError()


class AllowAllDirectory(UserDirectory):
    def has_access(self, uid: int, cid: int) -> bool:
        return True
