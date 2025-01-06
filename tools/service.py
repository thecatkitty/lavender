import os

from fastapi import FastAPI, Form, HTTPException, Query, status
from fastapi.responses import PlainTextResponse
from lavender import remote
from typing import Annotated

assert "LAV_KEY" in os.environ

REQUEST_PATTERN = "^[0-9a-f]{36}$"

app = FastAPI()
directory = remote.AllowAllDirectory()
service = remote.LavenderService(directory, int(os.environ["LAV_KEY"], 16))


def _handle_rc(rc: str) -> bytes:
    try:
        request = bytes([int(rc[i:i+2], base=16) for i in range(0, 36, 2)])
        return service.handle_request(request)

    except remote.AccessError as e:
        raise HTTPException(
            status_code=status.HTTP_403_FORBIDDEN, detail=str(e))

    except remote.RequestError as e:
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED, detail=str(e))


@app.get("/qr")
def handle_qr(rc: Annotated[str, Query(pattern=REQUEST_PATTERN)]):
    return PlainTextResponse(remote.to_decimal_code(_handle_rc(rc)))


@app.post("/verify")
async def handle_verify(rc: Annotated[str, Form(pattern=REQUEST_PATTERN)]):
    return {
        "code": "".join(f"{b:02x}" for b in _handle_rc(rc))
    }
