from dotenv import load_dotenv
from litellm import completion
import os
from rich import print

load_dotenv()

api_key = os.environ["DEEP_API_KEY"]

def get_llm_response(message):
    response = completion(
        model="deepseek/deepseek-chat",
        temperature = 0.7,
        api_key = api_key,
        messages=message,
        stream=False
    )
    return response

messages = [
    {
        "role":"system",
        "content":"You are a helpful AI assistance"
    },
    {
        "role":"user",
        "content":"Are u OK bae?"
    }
]

print(get_llm_response(messages))