import os

from fastapi import FastAPI, HTTPException, Query, status
from fastapi.responses import PlainTextResponse
from lavender import remote
from typing import Annotated

assert "LAV_KEY" in os.environ

app = FastAPI()
directory = remote.AllowAllDirectory()
service = remote.LavenderService(directory, int(os.environ["LAV_KEY"], 16))


@app.get("/qr")
def handle_qr(rc: Annotated[str, Query(pattern="^[0-9a-f]{36}$")]):
    try:
        request = bytes([int(rc[i:i+2], base=16) for i in range(0, 36, 2)])
        response = service.handle_request(request)
        return PlainTextResponse(remote.to_decimal_code(response))
    except remote.AccessError as e:
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED, detail=str(e))
