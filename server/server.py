
from fastapi import FastAPI, HTTPException, Depends, Header
from pydantic import BaseModel
from passlib.hash import bcrypt
from typing import Optional, Dict, List
from uuid import uuid4
import time

app = FastAPI(title="Secure Chat Server", version="0.2-simple-e2e")

class UserRecord(BaseModel):
    username: str
    password_hash: str
    identity_public_key: str  # base64 string
    created_at: float

class MessageRecord(BaseModel):
    id: int
    from_user: str
    to_user: str
    ciphertext: str  # base64
    nonce: str       # base64
    timestamp: int

USERS: Dict[str, UserRecord] = {}
MESSAGES: List[MessageRecord] = []
SESSIONS: Dict[str, str] = {}
NEXT_MESSAGE_ID = 1

class RegisterRequest(BaseModel):
    username: str
    password: str
    identity_public_key: str

class LoginRequest(BaseModel):
    username: str
    password: str

class LoginResponse(BaseModel):
    status: str
    token: str

class PublicKeyResponse(BaseModel):
    username: str
    identity_public_key: str

class SendMessageRequest(BaseModel):
    to: str
    ciphertext: str
    nonce: str
    timestamp: Optional[int] = None

class SendMessageResponse(BaseModel):
    status: str
    message_id: int

class MessageResponse(BaseModel):
    id: int
    from_: str
    to: str
    ciphertext: str
    nonce: str
    timestamp: int

class GetMessagesResponse(BaseModel):
    messages: List[MessageResponse]


def get_current_username(authorization: Optional[str] = Header(None)) -> str:
    if not authorization:
        raise HTTPException(status_code=401, detail="Missing Authorization header")
    parts = authorization.split()
    if len(parts) != 2 or parts[0].lower() != "bearer":
        raise HTTPException(status_code=401, detail="Invalid Authorization header")
    token = parts[1]
    username = SESSIONS.get(token)
    if not username:
        raise HTTPException(status_code=401, detail="Invalid or expired token")
    return username


@app.post("/register")
def register(req: RegisterRequest):
    if req.username in USERS:
        raise HTTPException(status_code=400, detail="Username already exists")
    if not req.identity_public_key:
        raise HTTPException(status_code=400, detail="identity_public_key required")
    pw_hash = bcrypt.hash(req.password)
    USERS[req.username] = UserRecord(
        username=req.username,
        password_hash=pw_hash,
        identity_public_key=req.identity_public_key,
        created_at=time.time(),
    )
    return {"status": "ok", "message": "User created"}


@app.post("/login", response_model=LoginResponse)
def login(req: LoginRequest):
    user = USERS.get(req.username)
    if not user or not bcrypt.verify(req.password, user.password_hash):
        raise HTTPException(status_code=401, detail="Invalid username or password")
    token = uuid4().hex
    SESSIONS[token] = req.username
    return LoginResponse(status="ok", token=token)


@app.get("/users/{username}/public-key", response_model=PublicKeyResponse)
def get_public_key(username: str):
    user = USERS.get(username)
    if not user:
        raise HTTPException(status_code=404, detail="User not found")
    return PublicKeyResponse(
        username=user.username,
        identity_public_key=user.identity_public_key,
    )


@app.post("/messages", response_model=SendMessageResponse)
def send_message(req: SendMessageRequest, current_user: str = Depends(get_current_username)):
    global NEXT_MESSAGE_ID
    if req.to not in USERS:
        raise HTTPException(status_code=404, detail="Target user not found")
    msg_id = NEXT_MESSAGE_ID
    NEXT_MESSAGE_ID += 1
    ts = req.timestamp or int(time.time())
    MESSAGES.append(
        MessageRecord(
            id=msg_id,
            from_user=current_user,
            to_user=req.to,
            ciphertext=req.ciphertext,
            nonce=req.nonce,
            timestamp=ts,
        )
    )
    return SendMessageResponse(status="ok", message_id=msg_id)


@app.get("/messages", response_model=GetMessagesResponse)
def get_messages(since_id: int = 0, current_user: str = Depends(get_current_username)):
    out: List[MessageResponse] = []
    for m in MESSAGES:
        if m.id <= since_id:
            continue
        if m.to_user != current_user:
            continue
        out.append(
            MessageResponse(
                id=m.id,
                from_=m.from_user,
                to=m.to_user,
                ciphertext=m.ciphertext,
                nonce=m.nonce,
                timestamp=m.timestamp,
            )
        )
    return GetMessagesResponse(messages=out)


if __name__ == "__main__":
    import uvicorn
    uvicorn.run("server:app", host="0.0.0.0", port=8000, reload=True)
