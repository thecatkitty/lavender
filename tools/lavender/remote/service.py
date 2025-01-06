import time

from .directory import UserDirectory
from .request import from_request_bytes
from .response import to_response


class AccessError(Exception):
    def __init__(self, *args):
        super().__init__(*args)


class RequestError(Exception):
    def __init__(self, *args):
        super().__init__(*args)


class LavenderService(object):
    def __init__(self, directory: UserDirectory, key: int):
        self._directory = directory
        self._key = key

    def handle_request(self, request: bytes) -> bytes:
        cid, uid, stamp = from_request_bytes(request)

        now = int(time.time()) & 0xFFFFFFFF

        if stamp - now > 2 * 60:
            raise RequestError("The request is from the future!")

        if now - stamp > 15 * 60:
            raise RequestError("The request is stale!")

        if not self._directory.has_access(uid, cid):
            raise AccessError("The user is not authorized!")

        return to_response(request, self._key)
