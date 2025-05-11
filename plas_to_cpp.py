import sys
import requests
import json

api_key = "key_here"
model = "model_here" # eg. mistralai/Mixtral-8x7B-Instruct-v0.1

with open("plas_input.txt", "r", encoding="utf-8") as f:
    plas_input = f.read()

prompt = f"""Translate the following PLAS code into valid modern C++. Output raw code only. No explanation. Do not say anything more than just sending the code. Make sure the code is a

PLAS:
{plas_input}
"""

res = requests.post(
    "link_here", # eg. https://api.together.xyz/v1/chat/completions
    headers={
        "Authorization": f"Bearer {api_key}",
        "Content-Type": "application/json"
    },
    json={
        "model": model,
        "messages": [
            { "role": "system", "content": "You are a compiler that converts structured readable PLAS code into clean, modern C++." },
            { "role": "user", "content": prompt }
        ],
        "max_tokens": 1024,
        "temperature": 0.2
    }
)

try:
    out = res.json()["choices"][0]["message"]["content"]
except Exception as e:
    out = f"[LLM Error] {e}\\n{res.text}"

print(out)
